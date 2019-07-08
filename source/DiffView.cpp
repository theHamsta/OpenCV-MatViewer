#include "DiffView.hpp"
#include "ui_DiffView.h"

#include "HeadUpDisplay.hpp"
#include "QCvMatViewer.hpp"

DiffView::DiffView( QWidget* parent ) :
	AbstractDualViewer( parent ),
	ui( new Ui::DiffView )
{
	ui->setupUi( this );

	// initialize head-up display
	m_headUpDisplay = new HeadUpDisplay();

	QLayout* layout = new QBoxLayout( QBoxLayout::BottomToTop );
	layout->addWidget( m_headUpDisplay );
	ui->display->setLayout( layout );

}

DiffView::~DiffView()
{
	delete ui;
}

QCvMatViewer* DiffView::primaryVideoOutput()
{
	return ui->display;
}

void DiffView::setScalingMethod( ScalingMethod method )
{
	if ( ui && ui->display )
		ui->display->setScalingMethod( method );
}



void DiffView::setFirstImage( const cv::Mat& mat )
{
	if ( mat.channels() == 3 ) {
		cv::cvtColor( mat, m_firstImg, cv::COLOR_BGR2GRAY );
	}
	else {
		m_firstImg = mat;
	}

	if ( ui && ui->display ) {
		if ( m_secondImg.empty() ) {
			ui->display->setImage( mat );
		}
		else {
			cv::Mat secondImgScaled;
			if ( m_firstImg.size() == m_secondImg.size() ) {
				secondImgScaled = m_secondImg;
			}
			else {
				cv::resize(m_secondImg, secondImgScaled, m_firstImg.size(),0,0,cv::INTER_CUBIC);
				if ( m_headUpDisplay ) {
					m_headUpDisplay->showMessage( tr("Dimensions of images do not match! Scaling of image 2 was performed") );
				}
			}
			
			if ( m_firstImg.type() != m_secondImg.type() ) {
			}
			else {
			}
			ui->display->setImage( cv::abs( m_firstImg - secondImgScaled ) );
		}
	}

}

void DiffView::setSecondImage( const cv::Mat& mat )
{

	if ( mat.channels() == 3 ) {
		cv::cvtColor( mat, m_secondImg, cv::COLOR_BGR2GRAY );
	}
	else {
		m_secondImg = mat;
	}

	if ( ui && ui->display ) {
		if ( m_firstImg.empty() ) {
			ui->display->setImage( mat );
		}
		else {
			if ( m_firstImg.size() == m_secondImg.size() ) {
				if ( m_firstImg.type() == m_secondImg.type() ) {
					ui->display->setImage( cv::abs( m_firstImg - m_secondImg ) );
				}
			}
			else {
				if ( m_headUpDisplay ) {
					m_headUpDisplay->showMessage( tr( "Dimensions of images do not match!" ) );
					ui->display->setImage( cv::Mat::zeros(3,4,CV_8UC1) );
				}
			}
		}
	}
}


void DiffView::setFirstImageDescription( const QString desc )
{
	AbstractDualViewer::setFirstImageDescription( desc );

	if ( !desc.isEmpty() ) {
		if ( m_decriptionSecondImage.isEmpty() ) {
			m_headUpDisplay->setRightTextOpposedToMessage( desc );
		}
		else {
			m_headUpDisplay->setRightTextOpposedToMessage( tr( "Luminance difference between these images:\n%1\n%2" ).arg( m_decriptionFirstImage ).arg( m_decriptionSecondImage ) );
		}
	}

}

void DiffView::setSecondImageDescription( const QString desc )
{
	AbstractDualViewer::setSecondImageDescription( desc );

	if ( !desc.isEmpty() ) {
		if ( m_decriptionFirstImage.isEmpty() && !desc.isEmpty() ) {
			m_headUpDisplay->setRightTextOpposedToMessage( desc );
		}
		else {
			m_headUpDisplay->setRightTextOpposedToMessage( tr( "Luminance difference between these images:\n%1\n%2" ).arg( m_decriptionFirstImage ).arg( m_decriptionSecondImage ) );
		}
	}
}

