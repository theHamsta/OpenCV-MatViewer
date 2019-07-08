#include "FlickerView.hpp"
#include "ui_FlickerView.h"

#include "HeadUpDisplay.hpp"
#include "MatFlickerViewer.hpp"

FlickerView::FlickerView( QWidget* parent ) :
	AbstractDualViewer( parent ),
	ui( new Ui::FlickerView )
{
	ui->setupUi( this );

	// initialize head-up display
	m_headUpDisplay = new HeadUpDisplay();
	ui->flickerDisplay->setFlickerInterval( 1000 ); // one second

	QLayout* layout = new QBoxLayout( QBoxLayout::BottomToTop );
	layout->addWidget( m_headUpDisplay );
	ui->flickerDisplay->setLayout( layout );

	// connect slider to QCvMatSplitViewer
	connect( m_headUpDisplay, &HeadUpDisplay::slider_valueChanged, this, &FlickerView::on_flickerSliderValueChanged );

	// change description when display flickers
	connect( ui->flickerDisplay, &MatFlickerViewer::activeImgChanged, this, &FlickerView::on_activeImageChanged );
}

FlickerView::~FlickerView()
{
	delete ui;
}

QCvMatViewer* FlickerView::primaryVideoOutput()
{
	return ui->flickerDisplay;
}

void FlickerView::setScalingMethod( ScalingMethod method )
{
	if ( ui && ui->flickerDisplay )
		ui->flickerDisplay->setScalingMethod( method );
}


void FlickerView::on_flickerSliderValueChanged( int value )
{
	if ( ui && ui->flickerDisplay )
		ui->flickerDisplay->setFlickerInterval( value * 2 ); // slider: min 0, max 1000 milliseconds
}

void FlickerView::setFirstImage( const cv::Mat& mat )
{
	if ( ui && ui->flickerDisplay )
		ui->flickerDisplay->setImage( mat );

	if ( ui->flickerDisplay->bothImagesSet() ) {
		m_headUpDisplay->makeSliderAvailable();
	} else {
		m_headUpDisplay->hideSlider();
	}
}

void FlickerView::setSecondImage( const cv::Mat& mat )
{
	if ( ui && ui->flickerDisplay )
		ui->flickerDisplay->setSecondImage( mat );

	if ( ui->flickerDisplay->bothImagesSet() ) {
		m_headUpDisplay->makeSliderAvailable();
	} else {
		m_headUpDisplay->hideSlider();
	}
}

void FlickerView::on_activeImageChanged( bool firstImageActiveNow )
{
	if ( m_headUpDisplay && ui->flickerDisplay->bothImagesSet() ) {
		m_headUpDisplay->setRightTextOpposedToMessage( firstImageActiveNow ? m_decriptionFirstImage : m_decriptionSecondImage );
		m_headUpDisplay->setLeftText( "" );

	}
}


void FlickerView::setFirstImageDescription( const QString desc )
{
	AbstractDualViewer::setFirstImageDescription( ui->flickerDisplay->firstImageSet() ? desc : "" );

	if ( m_decriptionSecondImage.isEmpty() ) {
		m_headUpDisplay->setLeftText( ui->flickerDisplay->firstImageSet() ? desc : "" );
	} else {
		if ( ui->flickerDisplay->bothImagesSet() ) {
			m_headUpDisplay->setRightTextOpposedToMessage( ui->flickerDisplay->firstImageActiveNow() ? m_decriptionFirstImage : m_decriptionSecondImage );
			m_headUpDisplay->setLeftText( "" );
		}
	}

}

void FlickerView::setSecondImageDescription( const QString desc )
{
	AbstractDualViewer::setSecondImageDescription( desc );

	if ( m_headUpDisplay ) {
		m_headUpDisplay->setRightTextOpposedToMessage( ui->flickerDisplay->secondImageSet() ? desc : "" );

		if ( ui->flickerDisplay->bothImagesSet() ) {
			m_headUpDisplay->setRightTextOpposedToMessage( ui->flickerDisplay->firstImageActiveNow() ? m_decriptionFirstImage : m_decriptionSecondImage );
			m_headUpDisplay->setLeftText( "" );
		}
	}


}

