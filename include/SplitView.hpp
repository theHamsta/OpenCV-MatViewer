/**
 * \file   SplitView.hpp
 * \brief  Defines class SplitView
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   28.08.2016
 */

#ifndef SPLITVIEW_HPP
#define SPLITVIEW_HPP


#include "AbstractDualViewer.hpp"

namespace Ui
{
	class SplitView;
}

class SplitView : public AbstractDualViewer
{
	Q_OBJECT

public:
	explicit SplitView( QWidget* parent = 0 );
	~SplitView();

	/// \name Getters
	/// \{ Return the value of members with corresponding names
	virtual QCvMatViewer* primaryVideoOutput() override;
	/// \}
		
	/// \name Setters
	/// \{ Set value of members with corresponding names
	virtual void setScalingMethod( ScalingMethod method ) override;
	/// \}
	
public slots:
	virtual void setFirstImage( const cv::Mat& mat ) override;
	virtual void setSecondImage( const cv::Mat& mat ) override;
	virtual void setFirstImageDescription( const QString desc ) override;
	virtual void setSecondImageDescription( const QString desc ) override;

private slots:
	void on_splitSliderValueChanged( int value );

// 	void foo();
private:
	Ui::SplitView* ui;
	
	

// 	cv::Mat m_leftImg;
// 	cv::Mat m_rightImg;
};

#endif // SPLITVIEW_HPP