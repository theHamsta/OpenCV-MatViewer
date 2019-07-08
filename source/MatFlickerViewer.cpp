#include "MatFlickerViewer.hpp"
#include "glColors.hpp"

MatFlickerViewer::MatFlickerViewer( QWidget* parent ):
	QCvMatViewer( parent )
{
	m_timer.start( m_flickerIntervalMSecs );
}

MatFlickerViewer::~MatFlickerViewer()
{
	if ( m_glInitialised ) {
		glDeleteTextures( 1, &m_secondTex );
	}
}

void MatFlickerViewer::setSecondImage( const cv::Mat& mat )
{
// 	assert( (m_mat.empty() || mat.cols / ( float ) mat.rows == m_mat.cols / ( float ) m_mat.rows)
// 	        && "Images must have the aspect ratio (or the second image will be stretched to the size of the first). Use ::setImg first!" );
	if ( !m_mat.empty() && mat.cols / ( float ) mat.rows != m_mat.cols / ( float ) m_mat.rows ) {
		qWarning() << "Aspect ratio of second image does not match the one of the first";
	}

	m_secondMat = mat;
	m_timer.disconnect();

	if ( m_glInitialised ) {
		if ( !mat.empty() ) {
			loadMatIntoTex( m_secondMat, m_secondTex );
			connect( &m_timer, &QTimer::timeout, this, &MatFlickerViewer::toggleFlicker );
		}

		if ( isVisible() ) {
			update();
		}
	}
}


void MatFlickerViewer::initializeGL()
{
	QCvMatViewer::initializeGL();

	// Create second texture
	glGenTextures( 1, &m_secondTex );
	m_glInitialised = true;

	// Set second image again if it was set before initialisation
	setSecondImage( m_secondMat );
}


void MatFlickerViewer::paintGL()
{
	if ( m_secondMat.empty() ) {
		QCvMatViewer::paintGL();
	}
	else {

		if ( m_mat.empty() ) { // first mat sets image dimensions if available
			updateRenderRectDimensions( m_secondMat.cols, m_secondMat.rows );
		}


		// Clear background
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glClearColor( m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2], 1.f );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		if ( m_interactiveMode ) {
			glScalef( m_glZoom, m_glZoom, 1.0 );
			glTranslatef( m_glTranslateX, m_glTranslateY, 0.f );
		}


		// Map texture onto a rectangle
		glBindTexture( GL_TEXTURE_2D, ( m_flickerOn | m_mat.empty() ) ? m_secondTex : m_tex );
		glEnable( GL_TEXTURE_2D );
		glBegin( GL_QUADS );
		{
			glColor3f( OGL_WHITE );

			// Texture coordinates are flipped in y-direction due to OpenCV's storage format (row=0 is the topmost row)
			glTexCoord2f( 0.0f, 0.0f ); glVertex2f( m_leftX,  m_topY );    //top left
			glTexCoord2f( 1.0f, 0.0f ); glVertex2f( m_rightX, m_topY );    //top right
			glTexCoord2f( 1.0f, 1.0f ); glVertex2f( m_rightX, m_bottomY ); //bottom right
			glTexCoord2f( 0.0f, 1.0f ); glVertex2f( m_leftX,  m_bottomY ); //bottom left
		}
		glEnd();
		glDisable( GL_TEXTURE_2D );

		paintGrid();
		paintLines();
		paintPoints();
		paintHighlightRect();

		glFlush();

	}
}


