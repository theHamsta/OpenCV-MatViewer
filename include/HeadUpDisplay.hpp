/**
 * \file   HeadUpDisplay.hpp
 * \brief  Defines class HeadUpDisplay
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   03.02.2017
 */

#ifndef HEADUPDISPLAY_HPP
#define HEADUPDISPLAY_HPP

#include <QWidget>

#include <QGraphicsOpacityEffect>
#include <QTimer>

namespace Ui
{
	class HeadUpDisplay;
}
class QPushButton;
class AbstractCommandPaletteWidget;

/**
 * @brief This class provides a head-up display that can be displayed on top of other widgets (for instance QCvMatViewer)
 *
 */
class HeadUpDisplay : public QWidget
{
	Q_OBJECT

public:
	explicit HeadUpDisplay( QWidget* parent = 0 );
	~HeadUpDisplay();

	void hideSlider();
	void showSlider();
	void showSettingsButton();
	void hideSettingsButton();
	void hideInfoBox();
	void showInfoBox( QString text );
	inline void makeSliderAvailable() { m_showSliderOnMouseMove = true; setMouseTracking(true); }

	/// \name Getters
	/// \{ Return the value of members with corresponding names
	double sliderValue() const;
	QPushButton* settingsButton(); 
	AbstractCommandPaletteWidget* commandPalette(); 
	/// \}

	/// \name Setters
	/// \{ Set value of members with corresponding names
	void setSliderValue( double value );
	void setOpacityLeftLabel( double value );
	void setOpacityRightLabel( double value );
	inline void setAnimationDuration( uint milliseconds ) { m_animationDuration = milliseconds; }
	inline void setMessageDisplayDuration( uint milliseconds ) { m_messageDisplayDuration = milliseconds; }
	/// \}

    virtual void mouseMoveEvent(QMouseEvent* event) override;

signals:
	void slider_valueChanged( int value );
	void buttonQuitCalculation_clicked();

public slots:
	void setLeftText( QString leftText );
	void setRightText( QString rightText );
	void setRightTextOpposedToMessage( QString rightText );
	/// Displays a message for m_messageDisplayDuration milliseconds
	void showMessage( QString messageText );
	/// Displays a message until the next message is issued
	void showPermanentMessage( QString messageText );
	/// Sets the value of the progress bar. The progress bar will disappear automatically after some time if it has reached 100%.
	void setProgess( int percentage, QString currentStep );
	/// Displays a big label in the middle of the screen with short message (e.g. "Calculating..." or "Please Wait!")
	void showPleaseWaitMessage( QString message );
	/// Displays a big label in the middle of the screen with short message for a short time (e.g. "Calculation finished")
	void showCalculationFinishedMessage( QString message );
	/// Hides labelPleaseWait
	void hidePleaseWaitMessage();
	/// Hides progress bar
	void hideProgressBar();
	/// Hides slider until the mouse is moved
	void hideSliderUntilMouseMove();
	/// Hides buttonQuitCalculations
	void hideButtonQuitCalculations();
	/// Shows buttonQuitCalculations
	void showButtonQuitCalculations();

private slots:
	void on_slider_valueChanged( int value );
	void on_slider_sliderPressed();
	void on_slider_sliderReleased();
	void on_buttonQuitCalculation_clicked();
	void hideLabelMessage();
	void showLabelMessage();

private:
	Ui::HeadUpDisplay* ui;
	/// \{ Transparency effects for widgets
	QGraphicsOpacityEffect* m_opacityEffectLeft;
	QGraphicsOpacityEffect* m_opacityEffectRight;
	QGraphicsOpacityEffect* m_opacityEffectMessages;
	QGraphicsOpacityEffect* m_opacityEffectSlider;
	QGraphicsOpacityEffect* m_opacityEffectPleaseWait;
	/// \}
	
	QTimer m_timerMessageFadeOut; ///< Timer that hides messages after they have been displayed for m_messageDisplayDuration milliseconds
	QTimer m_timerCalculationFinishedMessageFadeOut; ///< Timer that hides the message calculation finished after some time 
	QTimer m_timerProgressBarFadeOut; ///< Timer that hides the progress bar after it had show 100% for a while
	QTimer m_timerSliderFadeOut; ///< Timer that hides the slider after the mouse hadn't been moved for while
	
	uint m_animationDuration = 250; ///< Duration in milliseconds of fade-in and fade-out animations
	uint m_messageDisplayDuration = 5000; ///< Messages will be displayed for that many milliseconds unless they have been set by showPermanentMessage
	
	bool m_showSliderOnMouseMove = false;
	bool m_showInfoBox = false;
};

#endif // HEADUPDISPLAY_HPP
