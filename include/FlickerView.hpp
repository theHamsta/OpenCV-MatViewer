/**
 * \file   FlickerView.hpp
 * \brief  Defines class FlickerView
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   04.10.2016
 */

#ifndef FLICKERVIEW_HPP
#define FLICKERVIEW_HPP




#include "AbstractDualViewer.hpp"
// #include <opencv2/opencv.hpp>

namespace Ui
{
	class FlickerView;
}

class FlickerView : public AbstractDualViewer
{
	Q_OBJECT

public:
	explicit FlickerView( QWidget* parent = 0 );
	~FlickerView();

	virtual QCvMatViewer* primaryVideoOutput() override;

	virtual void setScalingMethod( ScalingMethod method ) override;

public slots:
	virtual void setFirstImage( const cv::Mat& mat ) override;
	virtual void setSecondImage( const cv::Mat& mat ) override;
	virtual void setFirstImageDescription( const QString desc ) override;
	virtual void setSecondImageDescription( const QString desc ) override;

private slots:
	void on_flickerSliderValueChanged( int value );
	void on_activeImageChanged( bool firstImageActiveNow );

// 	void foo();
private:
	Ui::FlickerView* ui;

// 	cv::Mat m_leftImg;
// 	cv::Mat m_rightImg;
};

#endif // FLICKERVIEW_HPP
