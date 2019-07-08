#include "HeadUpDisplay.hpp"
#include "ui_HeadUpDisplay.h"


#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QMouseEvent>

#include <QDebug>

HeadUpDisplay::HeadUpDisplay( QWidget* parent ) :
	QWidget( parent ),
	ui( new Ui::HeadUpDisplay )
{
	ui->setupUi( this );

	// hide widgets on start-up
	ui->progressBar->hide();
	ui->progressBarSpacer->changeSize( 0, 0 );
	ui->spacerInfoBox->changeSize( 0, 0 );
	ui->labelLeft->hide();
	ui->labelRight->hide();
	ui->labelMessages->hide();
	ui->labelOpposedToMessage->hide();
	ui->slider->hide();
	ui->buttonQuitCalculation->hide();
	ui->btnPopupSettings->hide();

	// set transparency effects
	m_opacityEffectLeft = new QGraphicsOpacityEffect( this );
	m_opacityEffectRight = new QGraphicsOpacityEffect( this );
	m_opacityEffectMessages = new QGraphicsOpacityEffect( this );
	m_opacityEffectSlider = new QGraphicsOpacityEffect( this );
	m_opacityEffectPleaseWait = new QGraphicsOpacityEffect( this );

	ui->labelMessages->setGraphicsEffect( m_opacityEffectMessages );
	ui->labelLeft->setGraphicsEffect( m_opacityEffectLeft );
	ui->labelRight->setGraphicsEffect( m_opacityEffectRight );
	ui->slider->setGraphicsEffect( m_opacityEffectSlider );

	m_opacityEffectLeft->setOpacity( 1. );
	m_opacityEffectRight->setOpacity( 1. );
	m_opacityEffectPleaseWait->setOpacity( 1. );

	// init timers that fades out message and the progressBar after some time
	m_timerMessageFadeOut.setSingleShot( true );
	m_timerMessageFadeOut.setInterval( m_messageDisplayDuration );
	connect( &m_timerMessageFadeOut, &QTimer::timeout, this, &HeadUpDisplay::hideLabelMessage );

	m_timerCalculationFinishedMessageFadeOut.setSingleShot( true );
	m_timerCalculationFinishedMessageFadeOut.setInterval( m_messageDisplayDuration / 4 );
	connect( &m_timerCalculationFinishedMessageFadeOut, &QTimer::timeout, this, &HeadUpDisplay::hideLabelMessage );

	m_timerProgressBarFadeOut.setSingleShot( true );
	m_timerProgressBarFadeOut.setInterval( m_messageDisplayDuration / 2 );
	connect( &m_timerProgressBarFadeOut, &QTimer::timeout, this, &HeadUpDisplay::hideProgressBar );

	m_timerSliderFadeOut.setSingleShot( true );
	m_timerSliderFadeOut.setInterval( m_messageDisplayDuration / 3 );
	connect( &m_timerSliderFadeOut, &QTimer::timeout, this, &HeadUpDisplay::hideSliderUntilMouseMove );

	// Activate mouse tracking (fade cursor and slider in and out)
	setMouseTracking( true );
	m_timerSliderFadeOut.start(); // ok, if user doesn't move the mouse, fade out slider and mouse

}

HeadUpDisplay::~HeadUpDisplay()
{
	delete ui;
}

void HeadUpDisplay::setLeftText( QString leftText )
{
	ui->labelLeft->setVisible( !leftText.isEmpty() );
	ui->labelLeft->setText( leftText );
}

void HeadUpDisplay::setRightText( QString rightText )
{
	ui->labelRight->setVisible( !rightText.isEmpty() );
	ui->labelRight->setText( rightText );
}

void HeadUpDisplay::setRightTextOpposedToMessage( QString rightText )
{
	ui->labelOpposedToMessage->setVisible( !rightText.isEmpty() );
	ui->labelOpposedToMessage->setText( rightText );
}

void HeadUpDisplay::showMessage( QString messageText )
{
	m_timerMessageFadeOut.stop();

	if ( !messageText.isEmpty() ) {
		showLabelMessage();
		ui->labelMessages->setText( messageText );
		m_timerMessageFadeOut.start();
	} else {
		hideLabelMessage();
	}

}

void HeadUpDisplay::showPermanentMessage( QString messageText )
{
	m_timerMessageFadeOut.stop();
	m_timerCalculationFinishedMessageFadeOut.stop();

	if ( !messageText.isEmpty() ) {
		showLabelMessage();
		ui->labelMessages->setText( messageText );
	} else {
		hideLabelMessage();
	}
}

void HeadUpDisplay::on_slider_valueChanged( int value )
{
	emit slider_valueChanged( value );
}

void HeadUpDisplay::on_slider_sliderReleased()
{
	if ( m_showSliderOnMouseMove ) {
		m_timerSliderFadeOut.start();
		setMouseTracking( true );
	}
}

void HeadUpDisplay::on_slider_sliderPressed()
{
	m_opacityEffectSlider->setOpacity( 1.f );
	m_timerSliderFadeOut.stop();
}

void HeadUpDisplay::setProgess( int percentage, QString currentStep )
{
	m_timerProgressBarFadeOut.stop();

	// Show busy indicatior if percentage is greater 100
	ui->progressBar->setMaximum( percentage > 100 ? 0 : 100 );

	ui->progressBar->setValue( percentage > 100 ? 0 : percentage );
	ui->progressBar->setVisible( true );
	ui->progressBarSpacer->changeSize( 40, 20, QSizePolicy::Expanding );

	if ( !currentStep.isEmpty() ) {
		//         ui->labelCurrentStep->setVisible( true );
		//         ui->labelCurrentStep->setText( currentStep );
	}

	if ( percentage == 100 ) {
		m_timerProgressBarFadeOut.start();
	}
}

void HeadUpDisplay::hideSlider()
{
	m_timerSliderFadeOut.stop();
	hideSliderUntilMouseMove();
	m_showSliderOnMouseMove = false;
}
void HeadUpDisplay::hideSliderUntilMouseMove()
{
	setCursor( Qt::BlankCursor );

	if ( m_showSliderOnMouseMove && ui->slider->isVisible() ) {
		QPropertyAnimation* a = new QPropertyAnimation( m_opacityEffectSlider, "opacity" );
		a->setDuration( m_animationDuration );
		a->setStartValue( m_opacityEffectSlider->opacity() );
		a->setEndValue( 0. );
		a->setEasingCurve( QEasingCurve::InBack );
		a->start( QPropertyAnimation::DeleteWhenStopped );
		connect( a, &QPropertyAnimation::finished, [ = ]() {
			ui->slider->hide();
		} );
		// 		ui->labelInfoBox->hide();
	}
}

void HeadUpDisplay::showSlider()
{
	m_showSliderOnMouseMove = true;

	QPropertyAnimation* a = new QPropertyAnimation( m_opacityEffectSlider, "opacity" );
	a->setDuration( m_animationDuration );
	a->setStartValue( m_opacityEffectSlider->opacity() );
	a->setEndValue( 1. );
	a->setEasingCurve( QEasingCurve::InBack );
	a->start( QPropertyAnimation::DeleteWhenStopped );

	ui->slider->show();
	m_timerSliderFadeOut.start();
}

void HeadUpDisplay::hideLabelMessage()
{
	QPropertyAnimation* a = new QPropertyAnimation( m_opacityEffectMessages, "opacity" );
	a->setDuration( m_animationDuration );
	a->setStartValue( m_opacityEffectMessages->opacity() );
	a->setEndValue( 0. );
	a->setEasingCurve( QEasingCurve::InBack );
	a->start( QPropertyAnimation::DeleteWhenStopped );
}

void HeadUpDisplay::showLabelMessage()
{
	ui->labelMessages->show();
	QPropertyAnimation* a = new QPropertyAnimation( m_opacityEffectMessages, "opacity" );
	a->setDuration( m_animationDuration );
	a->setStartValue( m_opacityEffectMessages->opacity() );
	a->setEndValue( 1. );
	a->setEasingCurve( QEasingCurve::InBack );
	a->start( QPropertyAnimation::DeleteWhenStopped );

	// 	QPropertyAnimation* a = new QPropertyAnimation( ui->labelMessages, "pos" );
	// 	a->setDuration( m_animationDuration);
	// 	a->setStartValue( QPoint( -ui->labelMessages->width(), ui->labelMessages->pos().y() ) );
	// 	a->setEndValue( ui->labelMessages->pos() );
	// 	a->setEasingCurve( QEasingCurve::InBack );
	// 	a->start( QPropertyAnimation::DeleteWhenStopped );
	// 	ui->labelMessages->show();


}



double HeadUpDisplay::sliderValue() const
{
	return ui->slider->value() / ui->slider->maximum();
}

void HeadUpDisplay::setSliderValue( double value )
{
	ui->slider->setValue( value * ui->slider->maximum() );
}

void HeadUpDisplay::setOpacityLeftLabel( double value )
{
	m_opacityEffectLeft->setOpacity( value );
}

void HeadUpDisplay::setOpacityRightLabel( double value )
{
	m_opacityEffectRight->setOpacity( value );
}

void HeadUpDisplay::showPleaseWaitMessage( QString message )
{
	// 	if ( !message.isEmpty() ) {
	// 		ui->labelPleaseWait->show();
	// 		ui->labelPleaseWait->setText( message );
	// 		QPropertyAnimation* a = new QPropertyAnimation( m_opacityEffectPleaseWait, "opacity" );
	// 		a->setDuration( m_animationDuration );
	// 		a->setStartValue( m_opacityEffectPleaseWait->opacity() );
	// 		a->setEndValue( 1. );
	// 		a->setEasingCurve( QEasingCurve::InBack );
	// 		a->start( QPropertyAnimation::DeleteWhenStopped );
	// 	} else {
	// 		ui->labelPleaseWait->hide();
	// 	}
	showPermanentMessage( message );
	ui->spinningWheelWidget->startAnimation();
}

void HeadUpDisplay::showCalculationFinishedMessage( QString message )
{
	showPermanentMessage( message );
	m_timerCalculationFinishedMessageFadeOut.start();
	ui->spinningWheelWidget->stopAnimation();
}



void HeadUpDisplay::hidePleaseWaitMessage()
{
	QPropertyAnimation* a = new QPropertyAnimation( m_opacityEffectPleaseWait, "opacity" );
	a->setDuration( m_animationDuration );
	a->setStartValue( m_opacityEffectPleaseWait->opacity() );
	a->setEndValue( 0. );
	a->setEasingCurve( QEasingCurve::InBack );
	a->start( QPropertyAnimation::DeleteWhenStopped );
	ui->spinningWheelWidget->stopAnimation();
	// 	ui->labelPleaseWait->hide();
}

void HeadUpDisplay::hideProgressBar()
{
	ui->progressBar->hide();
	ui->progressBarSpacer->changeSize( 0, 0 );
	//     ui->labelCurrentStep->hide();
}

void HeadUpDisplay::mouseMoveEvent( QMouseEvent* event )
{
	QWidget::mouseMoveEvent( event );

	unsetCursor();

	m_timerSliderFadeOut.stop();
	m_timerSliderFadeOut.start();

	// Show slider if mouse moved in the lower area of the widget
	if ( m_showSliderOnMouseMove && ui->slider->isVisible() == false && event->y() >= this->height() - 30 ) {
		showSlider();
	}
}

void HeadUpDisplay::showInfoBox( QString text )
{
	if ( text.isEmpty() ) {
		hideInfoBox();
		return;
	}

	ui->spacerInfoBox->changeSize( 40, 20, QSizePolicy::Expanding );
	ui->labelInfoBox->show();
	ui->labelInfoBox->setText( text );
	m_showInfoBox = true;
}

void HeadUpDisplay::hideInfoBox()
{
	ui->labelInfoBox->hide();
	ui->spacerInfoBox->changeSize( 40, 0 );
	m_showInfoBox = false;

}

void HeadUpDisplay::hideButtonQuitCalculations()
{
	ui->buttonQuitCalculation->hide();
}

void HeadUpDisplay::showButtonQuitCalculations()
{
	ui->buttonQuitCalculation->show();
}

void HeadUpDisplay::on_buttonQuitCalculation_clicked()
{
	hideButtonQuitCalculations();
	hideProgressBar();
	emit buttonQuitCalculation_clicked();
}


QPushButton* HeadUpDisplay::settingsButton()
{
	return ui->btnPopupSettings;
}

void HeadUpDisplay::showSettingsButton()
{
	ui->btnPopupSettings->show();
}

void HeadUpDisplay::hideSettingsButton()
{
	ui->btnPopupSettings->hide();
}

