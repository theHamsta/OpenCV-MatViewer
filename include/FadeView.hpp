#ifndef FADEVIEW_HPP
#define FADEVIEW_HPP

#include "AbstractDualViewer.hpp"
// #include <opencv2/opencv.hpp>

namespace Ui
{
	class FadeView;
}

//TODO fade? good name?
class FadeView : public AbstractDualViewer
{
	Q_OBJECT

public:
	explicit FadeView( QWidget* parent = 0 );
	~FadeView();

	virtual QCvMatViewer* primaryVideoOutput();
	
	virtual void setScalingMethod( ScalingMethod method ) override;
	
public slots:
	virtual void setFirstImage( const cv::Mat& mat ) override;
	virtual void setSecondImage( const cv::Mat& mat ) override;
	virtual void setFirstImageDescription( const QString desc ) override;
	virtual void setSecondImageDescription( const QString desc ) override;

private slots:
	void on_fadeSliderValueChanged( int value );

// 	void foo();
private:
	Ui::FadeView* ui;

// 	cv::Mat m_leftImg;
// 	cv::Mat m_rightImg;
};

#endif // FADEVIEW_HPP
