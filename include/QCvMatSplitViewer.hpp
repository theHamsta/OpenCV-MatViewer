/**
 * \file   QCvMatSplitViewer.hpp
 * \brief  Defines class QCvMatSplitViewer
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   August 28, 2016
 */


#ifndef Q_CV_MAT_SPLIT_VIEWER_HPP
#define Q_CV_MAT_SPLIT_VIEWER_HPP

#include "QCvMatViewer.hpp"



class QCvMatSplitViewer : public QCvMatViewer
{
    Q_OBJECT
public:
    explicit QCvMatSplitViewer(QWidget *parent = 0);
    virtual ~QCvMatSplitViewer();
	
	/// \name Setters
	/// \{ Set members with corresponding names
	inline void setShowSplitLine(bool showLine) { m_showSplitLine = showLine; }
	inline void setSplitLineColor( float r, float g, float b) { m_splitLineColor[0] = r; m_splitLineColor[1] = g; m_splitLineColor[2] = b; }
	/// \}
	
	/// \name Getters
	/// \{ Return members with corresponding names
	inline bool showSplitLine() const { return m_showSplitLine; }
	/// \}
	
	inline bool bothImagesSet() const { return !m_mat.empty() && !m_secondMat.empty(); }
	inline bool firstImageSet() const { return !m_mat.empty(); }
	inline bool secondImageSet() const { return !m_secondMat.empty(); }
	
public slots:
    void setSecondImage(const cv::Mat& mat);
	void setSplitPosition(const float splitPosition) { assert(splitPosition >= 0); assert(splitPosition <= 1); m_splitPosition = splitPosition; update(); }
	
private:
    virtual void initializeGL() override;
    virtual void paintGL() override;
	
	
	GLuint m_secondTex; ///< Texture for m_secondMat. To be drawn be OpenGL
	cv::Mat m_secondMat; ///< Second cv::Mat that will be drawn on the right side of the widget
	/// Position where the image splitted. Left to this position m_mat will be drawn and m_secondMat to the right.
	/// The position is relative: 0 is the left border and 1 the right one 
	float m_splitPosition = 0.5f;
	bool m_showSplitLine = true;
	float m_splitLineColor[3] = { 0.f, 0.f, 1.f };
};

#endif // Q_CV_MAT_SPLIT_VIEWER_HPP
