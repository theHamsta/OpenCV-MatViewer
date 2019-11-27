#include "QCvMatViewer.hpp"

#include <cassert>

#include "glColors.hpp"
// #include <opencv2/cudaarithm.hpp>
// #include <cuda_gl_interop.h>

float someGlColors[][12] = {

    { OGL_ORANGE },     // 1
    { OGL_RED },        // 2
    { OGL_YELLOW },     // 3
    { OGL_BLUE },       // 4
    { OGL_PURPLE },     // 5
    { OGL_SALMON },     // 6
    { OGL_GREEN },      // 7
    { OGL_PAPAYA },     // 8
    { OGL_INDIGO },     // 9
    { OGL_WHITE },      // 10
    { OGL_DARK_GREEN }, // 11
    { OGL_BLACK }       // 12
};

struct GLPoint
{
    GLfloat x;
    GLfloat y;
};

QCvMatViewer::QCvMatViewer(QWidget* parent)
    : QOpenGLWidget(parent),
      QOpenGLFunctions(),
#ifdef NDEBUG
      m_backgroundColor
{
    OGL_BLACK
}
#else
      m_backgroundColor
{
    OGL_PURPLE
} // purple background for debugging (virtually no video has this color)
#endif
{}

QCvMatViewer::~QCvMatViewer()
{
    if (m_glInitialised)
    {
        // no danger of double frees or deletes of never allocated data
        glDeleteTextures(1, &m_tex);
        glDeleteBuffers(1, &m_pointsVbo);
        glDeleteBuffers(1, &m_gridVbo);
        for (auto b : m_linesVbo)
        {
            glDeleteBuffers(1, &b);
        }
    }
}

void QCvMatViewer::initializeGL()
{
    if (!m_glInitialised)
    {
        initializeOpenGLFunctions();

        glClearColor(0, 0, 0, 1);
        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_SMOOTH);

        // Create texture
        glGenTextures(1, &m_tex);

        glGenBuffers(1, &m_pointsVbo);

        m_glInitialised = true;

        // Set image again if it was set before initialisation
        setImage(m_mat);
        setPointsToDraw(m_pointMat, m_mat.total());

        if (m_drawGrid)
        {
            initializeGrid();
        }
    }
}

// TODO double buffering?
void QCvMatViewer::paintGL()
{
    // Clear background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearColor(m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2], 1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (m_interactiveMode)
    {
        glScalef(m_glZoom, m_glZoom, 1.0f);
        glTranslatef(m_glTranslateX, m_glTranslateY, 0.f);
    }

    if (!m_mat.empty())
    {
        paintMat();
    }

    paintGrid();
    paintLines();
    paintPoints();
    paintHighlightRect();
    paintMinimap();

    glFlush();
}

void QCvMatViewer::paintMat()
{
    // Map texture onto a rectangle
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glEnable(GL_TEXTURE_2D);
    glColor4f(OGL_WHITE, m_glOpacity);
    glBegin(GL_QUADS);
    {
        // Texture coordinates are flipped in y-direction due to OpenCV's storage format (row=0 is the topmost row)
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(m_leftX, m_topY); // top left
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(m_rightX, m_topY); // top right
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(m_rightX, m_bottomY); // bottom right
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(m_leftX, m_bottomY); // bottom left
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void QCvMatViewer::paintPoints()
{
    if (!m_pointMat.empty() && m_drawPoints)
    {
        glPushMatrix();
        glTranslatef(m_leftX, m_topY, 0.f);
        glScalef(m_scale, -m_scale, 1.f);
        glTranslatef(0.5f, 0.5f, 0.f); // half-pixel shift

        glBindBuffer(GL_ARRAY_BUFFER, m_pointsVbo);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, 0);
        glPointSize(3);

        if (m_numPointsPerColor == 0)
        {
            glColor3f(OGL_ORANGE);
            glDrawArrays(GL_POINTS, 0, m_pointMat.total());
        }
        else
        {
            uint numImgs = (m_pointMat.total() + m_numPointsPerColor - 1) / m_numPointsPerColor;

            assert(numImgs < sizeof(someGlColors) / 3 / sizeof(float) &&
                   "Define more colors in global array someGlColors");

            for (uint i = 0; i < numImgs; i++)
            {
                glColor3f(someGlColors[i][0], someGlColors[i][1], someGlColors[i][2]);
                glDrawArrays(GL_POINTS, i * m_numPointsPerColor, m_numPointsPerColor);
            }
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glPopMatrix();
    }
}

void QCvMatViewer::paintLines()
{
    if (!m_lines.empty() && m_drawTriangles)
    {
        glPushMatrix();
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // 			glDisable(GL_CULL_FACE);
        // 			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        // 			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        glTranslatef(m_leftX, m_topY, 0.f);
        glScalef(m_scale, -m_scale, 1.f);
        glTranslatef(0.5f, 0.5f, 0.f); // half-pixel shift

        for (size_t i = 0; i < m_lines.size(); ++i)
        {
            if (i >= m_linesVbo.size())
            {
                GLuint linesVbo;
                glGenBuffers(1, &linesVbo);
                m_linesVbo.push_back(linesVbo);
                if (!m_lines[i].empty())
                {
                    m_drawTriangles = true;
                    assert(m_lines[i].type() == CV_32FC2);

                    glBindBuffer(GL_ARRAY_BUFFER, linesVbo);
                    glBufferData(GL_ARRAY_BUFFER, m_lines[i].elemSize() * m_lines[i].total(), m_lines[i].data,
                                 GL_STATIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }
            }
            glBindBuffer(GL_ARRAY_BUFFER, m_linesVbo[i]);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, 0);

            glLineWidth(m_lineThickness);
            glColor4f(m_lineColors[i][0], m_lineColors[i][1], m_lineColors[i][2], m_lineOpacity);
            glDrawArrays(GL_LINES, 0, m_lines[i].total());
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisable(GL_BLEND);

        glPopMatrix();
    }
}

void QCvMatViewer::paintGrid()
{
    if (m_drawGrid)
    {
        uint numCellsX = m_gridSize.width / m_cellSize.width;
        uint numCellsY = m_gridSize.height / m_cellSize.height;

        glEnable(GL_BLEND);
        glPushMatrix();
        glTranslatef(m_leftX, m_topY, 0.f);
        glScalef(m_scale, -m_scale, 1.f);
        glTranslatef(0.5f, 0.5f, 0.f); // half-pixel shift

        glBindBuffer(GL_ARRAY_BUFFER, m_gridVbo);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, 0);

        glColor4f(OGL_BLACK, 0.4f);
        glDrawArrays(GL_LINES, 0, (numCellsX + numCellsY + 2) * 2);

        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glPopMatrix();
    }
}

void QCvMatViewer::paintHighlightRect()
{
    if (!m_highlightRec.isNull())
    {
        glPushMatrix();

        switch (m_highlightCoordinates)
        {
        case HighlighWidgetCoordinates: {
            glLoadIdentity();
            break;
        }

        case HighlighImageCoordinates: {
            glTranslatef(m_leftX, m_topY, 0.f);
            glScalef(m_scale, -m_scale, 1.f);
            break;
        }
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);
        {
            glColor4f(m_highlightColor[0], m_highlightColor[1], m_highlightColor[2], m_highlightColor[3]);
            glVertex2f(m_highlightRec.topLeft().x(), m_highlightRec.topLeft().y());
            glVertex2f(m_highlightRec.topRight().x(), m_highlightRec.topRight().y());
            glVertex2f(m_highlightRec.bottomRight().x(), m_highlightRec.bottomRight().y());
            glVertex2f(m_highlightRec.bottomLeft().x(), m_highlightRec.bottomLeft().y());
        }
        glEnd();

        glPopMatrix();
    }
}

void QCvMatViewer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 0, 2);

    updateRenderRectDimensions(m_mat.cols, m_mat.rows);

    if (isVisible())
    {
        update();
    }
}

// http://stackoverflow.com/questions/18086519/is-it-possible-to-bind-a-opencv-gpumat-as-an-opengl-texture
// void QCvMatViewer::setGpuImage( const cv::cuda::GpuMat& mat )
// {
// 	cv::gpu::GpuMat d_mat(768, 1024, CV_8UC3);
// cv::ogl::Texture2D tex;
// tex.copyFrom(d_mat);
// tex.bind();
// }

void QCvMatViewer::setImage(const cv::Mat& mat)
{
    m_mat = mat;

    if (m_glInitialised)
    {
        // 	cv::imshow("foo", mat);
        // 	cv::waitKey(10);
        if (!mat.empty())
        {
            loadMatIntoTex(m_mat, m_tex);
        }

        updateRenderRectDimensions(m_mat.cols, m_mat.rows);

        if (isVisible())
        {
            update();
        }
    }
}

void QCvMatViewer::setPointsToDraw(cv::Mat points, uint numPointsPerColor)
{
    m_pointMat = points;

    if (m_glInitialised)
    {
        if (points.empty())
        {
            m_drawPoints = false;
        }
        else
        {
            m_drawPoints = true;
            assert(points.type() == CV_32FC2);
            glBindBuffer(GL_ARRAY_BUFFER, m_pointsVbo);
            glBufferData(GL_ARRAY_BUFFER, points.elemSize() * points.total(), points.data, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            m_numPointsPerColor = numPointsPerColor;
        }

        update();
    }
}

void QCvMatViewer::clearLinesToDraw()
{
    m_lines.clear();
    m_lineColors.clear();

    for (auto b : m_linesVbo)
    {
        glDeleteBuffers(1, &b);
    }
    m_linesVbo.clear();
    m_drawTriangles = false;
    update();
}

void QCvMatViewer::appendLinesToDraw(const cv::Mat& pointPairs, float red, float green, float blue)
{
    m_lines.push_back(pointPairs);
    m_lineColors.push_back({ red, green, blue });

    m_drawTriangles = true;
    if (m_glInitialised)
    {
        GLuint linesVbo;
        glGenBuffers(1, &linesVbo);
        m_linesVbo.push_back(linesVbo);
        if (!pointPairs.empty())
        {
            assert(pointPairs.type() == CV_32FC2);

            glBindBuffer(GL_ARRAY_BUFFER, linesVbo);
            glBufferData(GL_ARRAY_BUFFER, pointPairs.elemSize() * pointPairs.total(), pointPairs.data, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
    update();
}

void QCvMatViewer::initializeGrid()
{
    if (!m_glInitialised)
        return;

    uint numCellsX = m_gridSize.width / m_cellSize.width;
    uint numCellsY = m_gridSize.height / m_cellSize.height;

    std::vector< GLPoint > points;
    uint numGridLineEndPoints = 2 * (numCellsX + numCellsY);

    points.reserve(numGridLineEndPoints);

    for (float y = 0; y < numCellsY; y++)
    {
        points.push_back({ 0.f, y * m_cellSize.height });
        points.push_back({ (float) m_gridSize.width, y * m_cellSize.height });
    }

    for (float x = 0; x < numCellsX; x++)
    {
        points.push_back({ x * m_cellSize.width, 0.f });
        points.push_back({ x * m_cellSize.width, (float) m_gridSize.height - 1 });
    }

    glGenBuffers(1, &m_gridVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLPoint) * numGridLineEndPoints, points.data(), GL_STATIC_DRAW);
}

void QCvMatViewer::loadMatIntoTex(cv::Mat& mat, GLuint tex)
{
    // Upload texture
    glBindTexture(GL_TEXTURE_2D, tex);

    // Number of pixels in a line == step (but not in bytes: in pixels!)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, mat.step / mat.elemSize());

    switch (mat.type())
    {
    case CV_8UC1:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mat.cols, mat.rows, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, mat.data);
        break;

    case CV_8UC3:
        if (mat.step % 4 != 0 /*(mat.cols * 3) % 4 != 0*/)
        { // Rows not alligned to int32
            qWarning() << "Removed" << (4 - (mat.cols * 3) % 4) % 4
                       << "pixels columns on the right to display CV_8UC3 image correctly on GPU (and made an "
                          "additional copy of the image)";
            mat = mat(cv::Rect(0, 0, mat.cols - (4 - (mat.cols * 3) % 4) % 4, mat.rows)).clone();
            glPixelStorei(GL_UNPACK_ROW_LENGTH, mat.step / mat.elemSize());
            assert((mat.cols * 3 % 4) == 0 &&
                   "Rows must be alligned to word size (4 bytes) to be displayed correctly on GPU");
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mat.cols, mat.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, mat.data);
        break;

    case CV_8UC4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mat.cols, mat.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, mat.data);
        break;

    case CV_32FC1:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mat.cols, mat.rows, 0, GL_LUMINANCE, GL_FLOAT, mat.data);
        break;

    case CV_32FC3:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mat.cols, mat.rows, 0, GL_BGR, GL_FLOAT, mat.data);
        break;

    default:
        assert(false && "unsupported cv::Mat type");
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_scalingMethod);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    Q_ASSERT(!glGetError());
}

void QCvMatViewer::updateRenderRectDimensions(uint imgWidth, uint imgHeight)
{
    // We have widget borders in horizontal and vertical direction that limit maximum possible scaling
    float possibleScaleX = width() / (float) imgWidth;
    float possibleScaleY = height() / (float) imgHeight;

    // Select smaller one of the above number to achieve maximum scaling without clipping the image
    if (possibleScaleX > possibleScaleY)
    {
        m_scale   = possibleScaleY;
        m_bottomY = 0;
        m_topY    = height();
        m_leftX   = (width() - imgWidth * m_scale) * 0.5f;
        m_rightX  = (width() + imgWidth * m_scale) * 0.5f;
    }
    else
    {
        m_scale   = possibleScaleX;
        m_bottomY = (height() - imgHeight * m_scale) * 0.5f;
        m_topY    = (height() + imgHeight * m_scale) * 0.5f;
        m_leftX   = 0.0f;
        m_rightX  = width();
    }
}

QSize QCvMatViewer::sizeHint() const
{
    if (m_mat.empty())
    {
        return QSize(400, 300);
    }
    else
    {
        return QSize(imgWidth(), imgHeight());
    }
}

void QCvMatViewer::mousePressEvent(QMouseEvent* event)
{
    if (m_subImgSelectionActivated)
    {
        m_selectionOrigin = event->pos();

        if (!m_rubberBand)
        {
            m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
        }

        m_rubberBand->setGeometry(QRect(m_selectionOrigin, QSize()));
        m_rubberBand->show();
    }

    if (m_interactiveMode)
    {
        m_oldMousePos = event->pos();

        if (event->button() == Qt::LeftButton)
        {
            m_drag = true;
        }
        else if (event->button() == Qt::MidButton)
        {
            m_dragZoom = true;
        }
    }
}

void QCvMatViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (m_subImgSelectionActivated)
    {
        if (m_rubberBand)
        {
            m_rubberBand->setGeometry(QRect(m_selectionOrigin, event->pos()).normalized());
        }
    }

    if (m_interactiveMode && m_drag)
    {
        m_glTranslateX -= (m_oldMousePos.x() - event->pos().x()) / m_glZoom;
        m_glTranslateY += (m_oldMousePos.y() - event->pos().y()) /
                          m_glZoom; // y-inverted because Qt and GL use different coordinate systems
        m_oldMousePos = event->pos();
        update();
    }

    if (m_interactiveMode && m_dragZoom)
    {
        // Make event->pos() fix point of transform
        float mousePosBeforeTransfX = event->pos().x() / m_glZoom - m_glTranslateX;
        float mousePosBeforeTransfY = (height() - event->pos().y()) / m_glZoom - m_glTranslateY;
        m_glZoom += (m_oldMousePos.y() - event->pos().y()) * m_glZoom /
                    300.f; // y-inverted because Qt and GL use different coordinate systems
        m_glTranslateX = event->pos().x() / m_glZoom - mousePosBeforeTransfX;
        m_glTranslateY = (height() - event->pos().y()) / m_glZoom - mousePosBeforeTransfY;
        m_oldMousePos  = event->pos();
        update();
    }
}

void QCvMatViewer::mouseReleaseEvent(QMouseEvent* event)
{
    this->setFocus();
    if (m_subImgSelectionActivated)
    {
        if (m_rubberBand)
        {
            m_rubberBand->hide();
        }

        QRect selection = QRect(m_selectionOrigin, event->pos()).normalized();
        QRect videoRect(m_leftX, m_bottomY, m_rightX - m_leftX, m_topY - m_bottomY);
        QRect selectionInVideo = selection & videoRect;

        qDebug() << "Rectangle" << selectionInVideo << "selected on QCvMatViewer" << objectName();
        setSubImgSelectionActivated(false);

        // This widget shows a scaled version of the image. So the selection must be scaled back to the image. Also
        // subtract the video offset relative to the widget before.
        cv::Rect subImgRect((selectionInVideo.x() - m_leftX) / m_scale, (selectionInVideo.y() - m_bottomY) / m_scale,
                            selectionInVideo.width() / m_scale, selectionInVideo.height() / m_scale);

        if (subImgRect.width > 0 && subImgRect.height > 0)
        {
            qDebug() << "Sub-region" << QRect(subImgRect.x, subImgRect.y, subImgRect.width, subImgRect.height)
                     << "selected";
            emit subImg_selected(subImgRect);
        }
    }

    if (m_interactiveMode)
    {
        m_drag     = false;
        m_dragZoom = false;
    }
}

void QCvMatViewer::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);

    if (m_interactiveMode)
    {
        // Make event->pos() fix point of transform
        float mousePosBeforeTransfX = event->pos().x() / m_glZoom - m_glTranslateX;
        float mousePosBeforeTransfY = (height() - event->pos().y()) / m_glZoom - m_glTranslateY;
        m_glZoom += (0.001 * event->angleDelta().y()) * m_glZoom;
        m_glTranslateX = event->pos().x() / m_glZoom - mousePosBeforeTransfX;
        m_glTranslateY = (height() - event->pos().y()) / m_glZoom - mousePosBeforeTransfY;
        // 		qDebug() << "zoom" << m_glZoom << "/" << event->angleDelta();
        update();
    }
}

void QCvMatViewer::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);

    if (m_interactiveMode)
    {
        resetTranslationAndZoom();
    }
}

void QCvMatViewer::showHighlightedRect(QRect rect, HighlighCoordinates coordinateSystem, float colorR, float colorG,
                                       float colorB, float colorA)
{
    m_highlightRec         = rect;
    m_highlightCoordinates = coordinateSystem;
    m_highlightColor[0]    = colorR;
    m_highlightColor[1]    = colorG;
    m_highlightColor[2]    = colorB;
    m_highlightColor[3]    = colorA;
    update();
}

void QCvMatViewer::paintMinimap()
{
    if (!m_minimap)
        return;
    // 	float top = height() - 100.f;
    // 	float bottom = height() - 20.f;
    // 	float right = width() - 20.f;
    // 	float left = right - width() / (float) height() * (bottom - top) ;
    //
    // 	glLoadIdentity();
    // // 	glScalef( 1.f, -1.f, 1.f );
    // 	glEnable( GL_BLEND );
    // 	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    // 	glBegin( GL_QUADS );
    // 	{
    // 		glColor4f( 0.2f, 0.2f, 0.2f, 0.5f  );
    // // 		glColor3f( 0.8f, 0.2f, 0.2f);
    // 		// Texture coordinates are flipped in y-direction due to OpenCV's storage format (row=0 is the topmost row)
    // 		glVertex2f( left,  top ); //top left
    // 		glVertex2f( right, top ); //top right
    // 		glVertex2f( right, bottom ); //bottom right
    // 		glVertex2f( left,  bottom); //bottom left
    // 	}
    // 	glEnd();
    //
    // 	float minimapScale = 80.f;
    // 	top = height() - 20.f ;
    // 	bottom = height() - 20.f - 1.f/ m_glZoom * minimapScale;
    // 	right = width() - 20.f;
    // 	left = right - width() / (float) height() * (bottom-top) ;
    //
    // 	top -= m_glTranslateY * m_glZoom* 1/minimapScale;
    // 	bottom -= m_glTranslateY * m_glZoom*  1/minimapScale;
    // 	left -= m_glTranslateX * m_glZoom*  1/minimapScale;
    // 	right -= m_glTranslateX * m_glZoom*  1/minimapScale;
    //
    //
    // 	glBegin( GL_QUADS );
    // 	{
    // 		glColor4f( 0.8f, 0.2f, 0.2f, 0.5f  );
    // // 		glColor3f( 0.8f, 0.2f, 0.2f);
    // 		// Texture coordinates are flipped in y-direction due to OpenCV's storage format (row=0 is the topmost row)
    // 		glVertex2f( left,  top ); //top left
    // 		glVertex2f( right, top ); //top right
    // 		glVertex2f( right, bottom ); //bottom right
    // 		glVertex2f( left,  bottom); //bottom left
    // 	}
    // 	glEnd();
}
