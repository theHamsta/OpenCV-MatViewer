/**
 * \file   AbstractDualViewer.hpp
 * \brief  Defines interface class AbstractDualViewer
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   03.09.2016
 */

#ifndef ABSTRACT_DUAL_VIEWER_HPP
#define ABSTRACT_DUAL_VIEWER_HPP

#include <QWidget>

#include "QCvMatViewer.hpp"

namespace cv
{
	class Mat;
}

class HeadUpDisplay;



class AbstractDualViewer : public QWidget
{
	Q_OBJECT

public:
	virtual QCvMatViewer* primaryVideoOutput() = 0;
	
	virtual void setScalingMethod( ScalingMethod method ) =  0;
	inline HeadUpDisplay* headUpDisplay() const { return m_headUpDisplay; }

public slots:
	virtual void setFirstImage( const cv::Mat& mat ) = 0;
	virtual void setSecondImage( const cv::Mat& mat ) = 0;
	virtual inline void setFirstImageDescription( const QString desc ) { m_decriptionFirstImage = desc; }
	virtual inline void setSecondImageDescription( const QString desc ){ m_decriptionSecondImage = desc; };
	
protected:
	explicit AbstractDualViewer(QWidget* parent = nullptr ) : QWidget(parent) { }
	
	HeadUpDisplay* m_headUpDisplay;
	
	QString m_decriptionFirstImage;
	QString m_decriptionSecondImage;
};

#endif // ABSTRACT_DUAL_VIEWER_HPP
