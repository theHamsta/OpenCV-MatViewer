/**
 * \file   DiffView.hpp
 * \brief  Defines class DiffView
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   04.10.2016
 */

#ifndef DIFFVIEW_HPP
#define DIFFVIEW_HPP




#include "AbstractDualViewer.hpp"
// #include <opencv2/opencv.hpp>

namespace Ui
{
	class DiffView;
}

class DiffView : public AbstractDualViewer
{
	Q_OBJECT

public:
	explicit DiffView( QWidget* parent = 0 );
	virtual ~DiffView();

	virtual QCvMatViewer* primaryVideoOutput() override;

	virtual void setScalingMethod( ScalingMethod method ) override;

public slots:
	virtual void setFirstImage( const cv::Mat& mat ) override;
	virtual void setSecondImage( const cv::Mat& mat ) override;
	virtual void setFirstImageDescription( const QString desc ) override;
	virtual void setSecondImageDescription( const QString desc ) override;

private:
	Ui::DiffView* ui;

	cv::Mat m_firstImg;
	cv::Mat m_secondImg;
};

#endif //DIFFVIEW_HPP
