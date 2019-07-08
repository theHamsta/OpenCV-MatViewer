/**
 * \file   QCvMatFadeViewer.hpp
 * \brief  Defines class QCvMatFadeViewer
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   02.09.2016
 */

#ifndef QCVMATFADEVIEWER_HPP
#define QCVMATFADEVIEWER_HPP

#include "QCvMatViewer.hpp"



class QCvMatFadeViewer : public QCvMatViewer
{
    Q_OBJECT
public:
    explicit QCvMatFadeViewer(QWidget *parent = 0);
    virtual ~QCvMatFadeViewer();
	
	/// \name Setters
	/// \{ Set members with corresponding names
	
	/// \}
	
	/// \name Getters
	/// \{ Return members with corresponding names
	inline float fadeValue() const { return m_fadeValue; }
	/// \}
	
	inline bool bothImagesSet() const { return !m_mat.empty() && !m_secondMat.empty(); }
	inline bool firstImageSet() const { return !m_mat.empty(); }
	inline bool secondImageSet() const { return !m_secondMat.empty(); }
	
public slots:
	void setFadeValue(const float value) { assert(value>= 0); assert(value <= 1); m_fadeValue = value; update(); }
    void setSecondImage(const cv::Mat& mat);
	
private:
    virtual void initializeGL() override;
    virtual void paintGL() override;
	
	
	GLuint m_secondTex; ///< Texture for m_secondMat. To be drawn be OpenGL
	cv::Mat m_secondMat; ///< Second cv::Mat that will be drawn on the right side of the widget
	/// Position where the image splitted. Left to this position m_mat will be drawn and m_secondMat to the right.
	/// The position is relative: 0 is the left border and 1 the right one 
	float m_fadeValue = 0.5f;
};

#endif // QCVMATFADEVIEWER_HPP