/**
 * \file   QCvMatViewer.hpp
 * \brief  Defines class QCvMatViewer
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   August 19, 2016
 */

#ifndef Q_CV_MAT_VIEWER_HPP
#define Q_CV_MAT_VIEWER_HPP

#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QRubberBand>
#include <QtOpenGL/QGLFunctions>
#include <array>
#include <opencv2/opencv.hpp>
// #include <opencv2/core/cuda.hpp>
// #include <opencv2/core/opengl.hpp>

#include <QOpenGLContext>
#include <QtOpenGL/QtOpenGL>

// TODO remove cv::Mat members. Not necessary if data is uploaded

enum ScalingMethod { NearestNeighbor = GL_NEAREST, Bilinear = GL_LINEAR };
enum HighlighCoordinates { HighlighImageCoordinates, HighlighWidgetCoordinates };

struct QCvMatViewerViewPort
{
    float glTranslationX;
    float glTranslationY;
    float glZoom;

    static QCvMatViewerViewPort standard() { return { 0.f, 0.f, 1.f }; }
};

class QCvMatViewer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
  public:
    explicit QCvMatViewer(QWidget* parent = 0);
    virtual ~QCvMatViewer();

    QCvMatViewer(QCvMatViewer const&) = delete;            // TODO implement
    QCvMatViewer& operator=(QCvMatViewer const&) = delete; // TODO implement

    void showGridLines(cv::Size gridSize, cv::Size_< float > cellSize)
    {
        m_drawGrid = true;
        m_gridSize = gridSize;
        m_cellSize = cellSize;
        initializeGrid();
        update();
    }
    void showHighlightedRect(QRect rect, HighlighCoordinates coordinateSystem, float colorR = .8f, float colorG = 0.8f,
                             float colorB = 0.0f, float colorA = 0.8f);
    void hideGrid()
    {
        m_drawGrid = false;
        update();
        if (m_glInitialised)
            glDeleteBuffers(1, &m_gridVbo);
    }
    void hidePointCloud()
    {
        m_drawPoints = false;
        m_pointMat.release();
        update();
    }
    void hideTriangles()
    {
        m_drawTriangles = false;
        for (auto& l : m_lines)
        {
            l.release();
        }
        update();
    }
    void hideHighlightedRect()
    {
        m_highlightRec = QRect();
        update();
    }

    Q_PROPERTY(float glZoom READ glZoom WRITE setGlZoom)
    Q_PROPERTY(float glTranslationX READ glTranslationX WRITE setGlTranslationX)
    Q_PROPERTY(float glTranslationY READ glTranslationY WRITE setGlTranslationY)
    Q_PROPERTY(float glOpacity READ glOpacity WRITE setGlOpacity)

    /// \name Setters
    /// \{ Set value of members with corresponding names
    inline void setSubImgSelectionActivated(bool activated)
    {
        m_subImgSelectionActivated = activated;
        emit subImageSelectionActivated_changed(activated);
    } // TODO better name: e.g. activateSubimage selection
    inline void setScalingMethod(ScalingMethod method) { m_scalingMethod = method; }
    inline void setInteractiveMode(bool enabled) { m_interactiveMode = enabled; }
    inline void setGlZoom(float zoom)
    {
        m_glZoom = zoom;
        assert(zoom >= 0 && "zoom must be positive");
        update();
    }
    inline void setGlTranslationX(float offsetX)
    {
        m_glTranslateX = offsetX;
        update();
    }
    inline void setGlTranslationY(float offsetY)
    {
        m_glTranslateY = offsetY;
        update();
    }
    inline void setGlOpacity(float opacity)
    {
        m_glOpacity = opacity;
        update();
    }
    inline void setViewPort(const QCvMatViewerViewPort& view)
    {
        m_glTranslateX = view.glTranslationX;
        m_glTranslateY = view.glTranslationY;
        m_glZoom       = view.glZoom;
    }
    inline void setBackgroundColor(float r, float g, float b)
    {
        m_backgroundColor[0] = r;
        m_backgroundColor[1] = g;
        m_backgroundColor[2] = b;
    }
    inline void setLineThickness(float value)
    {
        m_lineThickness = value;
        update();
    }
    inline void setLineOpacity(float value)
    {
        m_lineOpacity = value;
        update();
    }
    /// \}

    /// \name Getters
    /// \{ Return the value of members with corresponding names
    inline bool subImgSelectionActivated() const { return m_subImgSelectionActivated; }
    inline uint imgWidth() const { return m_mat.cols; }
    inline uint imgHeight() const { return m_mat.rows; }
    inline ScalingMethod scalingMethod() const { return m_scalingMethod; }
    virtual QSize sizeHint() const override;
    inline cv::Mat& img() { return m_mat; }
    inline void resetTranslationAndZoom()
    {
        m_glZoom       = 1.f;
        m_glTranslateX = m_glTranslateY = 0.f;
        update();
    }
    inline QCvMatViewerViewPort currentViewPort() { return { m_glTranslateX, m_glTranslateY, m_glZoom }; }
    inline float glZoom() { return m_glZoom; }
    inline float glTranslationX() { return m_glTranslateX; }
    inline float glTranslationY() { return m_glTranslateY; }
    inline float glOpacity() { return m_glOpacity; }
    /// \}

  signals:
    void subImageSelectionActivated_changed(bool);
    void subImg_selected(cv::Rect);

  public slots:
    void setImage(const cv::Mat& mat);
    //     void setGpuImage(const cv::cuda::GpuMat& mat);
    void setPointsToDraw(cv::Mat points, uint numPointsPerColor = 0);
    void appendLinesToDraw(const cv::Mat& pointPairs, float red, float blue, float green);
    void clearLinesToDraw();

  protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int width, int height) override;
    virtual void paintGL() override;
    void paintMat();
    void paintPoints();
    void paintLines();
    void paintGrid();
    void paintMinimap();
    void paintHighlightRect();
    void loadMatIntoTex(cv::Mat& mat, GLuint tex);
    void initializeGrid();
    void updateRenderRectDimensions(uint imgWidth, uint imgHeight);

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

    ScalingMethod m_scalingMethod = Bilinear;
    cv::Mat m_mat;
    GLuint m_tex;

    int m_topY;
    int m_bottomY;
    int m_leftX;
    int m_rightX;
    float m_scale;

    bool m_interactiveMode = true;
    float m_glTranslateX   = 0.f;
    float m_glTranslateY   = 0.f;
    float m_glZoom         = 1.f;
    float m_glOpacity      = 1.f;

    float m_backgroundColor[3]; ///< Color of background (default is black in release mode and purple in debug mode)
    bool m_glInitialised = false;

    bool m_minimap = true;

  private:
    QPoint m_selectionOrigin;
    QRubberBand* m_rubberBand       = nullptr;
    bool m_subImgSelectionActivated = false;

    bool m_drag     = false;
    bool m_dragZoom = false;
    QPoint m_oldMousePos;
    // 	QSize m_oldSize;

    cv::Mat m_pointMat;
    GLuint m_pointsVbo;
    uint m_numPointsPerColor;
    bool m_drawPoints = true;

    GLuint m_gridVbo;
    bool m_drawGrid     = false;
    cv::Size m_gridSize = cv::Size(-1, -1);
    cv::Size_< float > m_cellSize;

    std::vector< GLuint > m_linesVbo;
    std::vector< cv::Mat > m_lines;
    std::vector< std::array< float, 3 > > m_lineColors;
    bool m_drawTriangles  = true;
    float m_lineThickness = 2.f;
    float m_lineOpacity   = .9f;

    QRect m_highlightRec;
    float m_highlightColor[4];
    HighlighCoordinates m_highlightCoordinates;
};

#endif // Q_CV_MAT_VIEWER_HPP
