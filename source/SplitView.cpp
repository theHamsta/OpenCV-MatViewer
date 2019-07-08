#include "SplitView.hpp"
#include "ui_SplitView.h"

#include "HeadUpDisplay.hpp"

SplitView::SplitView( QWidget* parent ) :
	AbstractDualViewer( parent ),
	ui( new Ui::SplitView )
{
	ui->setupUi( this );

	// initialize head-up display
	m_headUpDisplay = new HeadUpDisplay();

	QLayout* layout = new QBoxLayout( QBoxLayout::BottomToTop );
	layout->addWidget( m_headUpDisplay );
	ui->splitDisplay->setLayout( layout );

	// connect slider to QCvMatSplitViewer
	connect( m_headUpDisplay, &HeadUpDisplay::slider_valueChanged, this, &SplitView::on_splitSliderValueChanged );
}

SplitView::~SplitView()
{
	delete ui;
}

QCvMatViewer* SplitView::primaryVideoOutput()
{
	return ui->splitDisplay;
}

void SplitView::setScalingMethod( ScalingMethod method )
{
	if ( ui && ui->splitDisplay )
		ui->splitDisplay->setScalingMethod( method );
}


void SplitView::on_splitSliderValueChanged( int value )
{
	if ( ui && ui->splitDisplay )
		ui->splitDisplay->setSplitPosition( value / 1000.f );
}

void SplitView::setFirstImage( const cv::Mat& mat )
{
	if ( ui && ui->splitDisplay )
		ui->splitDisplay->setImage( mat );
	if ( ui->splitDisplay->bothImagesSet() ) {
		m_headUpDisplay->makeSliderAvailable();
	}
	else {
		m_headUpDisplay->hideSlider();
	}
}


void SplitView::setSecondImage( const cv::Mat& mat )
{
	if ( ui && ui->splitDisplay )
		ui->splitDisplay->setSecondImage( mat );
	if ( ui->splitDisplay->bothImagesSet() ) {
		m_headUpDisplay->makeSliderAvailable();
	}
	else {
		m_headUpDisplay->hideSlider();
	}
}

void SplitView::setFirstImageDescription( const QString desc )
{
	AbstractDualViewer::setFirstImageDescription( desc );
	if ( m_headUpDisplay ) {
		m_headUpDisplay->setLeftText( ui->splitDisplay->firstImageSet() ? desc : "" );
	}

}

void SplitView::setSecondImageDescription( const QString desc )
{
	AbstractDualViewer::setSecondImageDescription( desc );
	if ( m_headUpDisplay ) {
		m_headUpDisplay->setRightText( ui->splitDisplay->secondImageSet() ? desc : "" );
	}
}



// void SplitView::foo(){
// 	m_leftImg = cv::imread( "/home/stephan/Bilder/carpe_diem.jpg" );
// 	m_rightImg = cv::imread( "/home/stephan/Bilder/carpe_diem2.jpg" );
// 	ui->splitDisplay->setImage(m_leftImg);
// 	ui->splitDisplay->setSecondImage(m_rightImg);
// }
