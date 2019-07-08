#include "QCvMatFadeViewer.hpp"

#include "glColors.hpp"

QCvMatFadeViewer::QCvMatFadeViewer( QWidget* parent ): QCvMatViewer( parent )
{

}

QCvMatFadeViewer::~QCvMatFadeViewer()
{
	if ( m_glInitialised ) {
		glDeleteTextures( 1, &m_secondTex );
	}
}

void QCvMatFadeViewer::setSecondImage( const cv::Mat& mat )
{

// 	assert( (m_mat.empty() || (mat.cols / ( float ) mat.rows == m_mat.cols / ( float ) m_mat.rows))
// 	        && "Images must have the aspect ratio (or the second image will be stretched to the size of the first). Use ::setImg first!" );
	if ( !m_mat.empty() && mat.cols / ( float ) mat.rows != m_mat.cols / ( float ) m_mat.rows ) {
		qWarning() << "Aspect ratio of second image does not match the one of the first";
	}


	m_secondMat = mat;

	if ( m_glInitialised ) {
		if ( !mat.empty() ) {
			loadMatIntoTex( m_secondMat, m_secondTex );
		}

		if ( isVisible() ) {
			update();
		}
	}

}

void QCvMatFadeViewer::initializeGL()
{
	QCvMatViewer::initializeGL();

	// Create second texture
	glGenTextures( 1, &m_secondTex );
	m_glInitialised = true;

	// Set second image again if it was set before initialisation
	setSecondImage( m_secondMat );
}


void QCvMatFadeViewer::paintGL()
{
	if ( m_secondMat.empty() ) {
		QCvMatViewer::paintGL();
	}
	else {

		if ( m_mat.empty() ) {  // first mat sets image dimensions if available
			updateRenderRectDimensions( m_secondMat.cols, m_secondMat.rows );
		}

		// Clear background
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glClearColor( m_backgroundColor[0], m_backgroundColor[1], m_backgroundColor[2], 1.f );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		if ( m_interactiveMode ) {
			glScalef( m_glZoom, m_glZoom, 1.0 );
			glTranslatef( m_glTranslateX, m_glTranslateY, 0.f );
		}

		glEnable( GL_TEXTURE_2D );
		glColor3f( 1.0f, 1.0f, 1.0f );

		if ( !m_mat.empty() ) {
			// Left image
			glBindTexture( GL_TEXTURE_2D, m_tex );
			glBegin( GL_QUADS );
			{
				// Texture coordinates are flipped in y-direction due to OpenCV's storage format (row=0 is the topmost row)
				glTexCoord2f( 0.0f, 0.0f ); glVertex2f( m_leftX , m_topY );  //top left
				glTexCoord2f( 1.0f, 0.0f ); glVertex2f( m_rightX, m_topY );  //top right
				glTexCoord2f( 1.0f, 1.0f );	glVertex2f( m_rightX, m_bottomY ); //bottom right
				glTexCoord2f( 0.0f, 1.0f ); glVertex2f( m_leftX , m_bottomY ); //bottom left
			}
			glEnd();
		}

		// Right image
		glBindTexture( GL_TEXTURE_2D, m_secondTex );

		if ( !m_mat.empty() ) {
			glColor4f( 1.0f, 1.0f, 1.0f , m_fadeValue );
		}

		glBegin( GL_QUADS );
		{
			glTexCoord2f( 0.0f, 0.0f ); glVertex2f( m_leftX , m_topY );  //top left
			glTexCoord2f( 1.0f, 0.0f ); glVertex2f( m_rightX, m_topY );  //top right
			glTexCoord2f( 1.0f, 1.0f ); glVertex2f( m_rightX, m_bottomY ); //bottom right
			glTexCoord2f( 0.0f, 1.0f ); glVertex2f( m_leftX , m_bottomY ); //bottom left
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


