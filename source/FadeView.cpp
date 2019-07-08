#include "FadeView.hpp"
#include "ui_FadeView.h"

#include "HeadUpDisplay.hpp"


FadeView::FadeView( QWidget* parent ) :
	AbstractDualViewer( parent ),
	ui( new Ui::FadeView )
{
	ui->setupUi( this );

	// initialize head-up display
	m_headUpDisplay = new HeadUpDisplay();

	QLayout* layout = new QBoxLayout( QBoxLayout::BottomToTop );
	layout->addWidget( m_headUpDisplay );
	ui->fadeViewer->setLayout( layout );

	// connect slider to QCvMatSplitViewer
	connect( m_headUpDisplay, &HeadUpDisplay::slider_valueChanged, this, &FadeView::on_fadeSliderValueChanged );
// 	connect(ui->fadeViewer,SIGNAL(resized()), this, SLOT(foo()));

	m_headUpDisplay->setOpacityLeftLabel( 0.5 );
	m_headUpDisplay->setOpacityRightLabel( 0.5 );
}

FadeView::~FadeView()
{
	delete ui;
}

void FadeView::on_fadeSliderValueChanged( int value )
{
	if ( ui && ui->fadeViewer )
		ui->fadeViewer->setFadeValue( value / 1000.f );

	m_headUpDisplay->setOpacityLeftLabel( 1.f - value / 1000.f );
	m_headUpDisplay->setOpacityRightLabel( value / 1000.f );
}

QCvMatViewer* FadeView::primaryVideoOutput()
{
	return ui->fadeViewer;
}

void FadeView::setScalingMethod( ScalingMethod method )
{
	if ( ui && ui->fadeViewer )
		ui->fadeViewer->setScalingMethod( method );
}


void FadeView::setFirstImage( const cv::Mat& mat )
{
	if ( ui && ui->fadeViewer )
		ui->fadeViewer->setImage( mat );
	if ( ui->fadeViewer->bothImagesSet() ) {
		m_headUpDisplay->makeSliderAvailable();
	}
	else {
		m_headUpDisplay->hideSlider();
	}
}


void FadeView::setSecondImage( const cv::Mat& mat )
{
	if ( ui && ui->fadeViewer )
		ui->fadeViewer->setSecondImage( mat );
	if ( ui->fadeViewer->bothImagesSet() ) {
		m_headUpDisplay->makeSliderAvailable();
	}
	else {
		m_headUpDisplay->hideSlider();
	}
}

void FadeView::setFirstImageDescription( const QString desc )
{
	AbstractDualViewer::setFirstImageDescription( desc );
	if ( m_headUpDisplay ) {
		m_headUpDisplay->setLeftText( ui->fadeViewer->firstImageSet() ? desc : "" );
	}
}

void FadeView::setSecondImageDescription( const QString desc )
{
	AbstractDualViewer::setSecondImageDescription( desc );
	if ( m_headUpDisplay ) {
		m_headUpDisplay->setRightText( ui->fadeViewer->secondImageSet() ? desc : "" );
	}
}
// void FadeView::foo(){
// 	m_leftImg = cv::imread( "/home/stephan/Bilder/carpe_diem.jpg" );
// 	m_rightImg = cv::imread( "/home/stephan/Bilder/carpe_diem2.jpg" );
// 	ui->fadeViewer->setImage(m_leftImg);
// 	ui->fadeViewer->setSecondImage(m_rightImg);
// }

