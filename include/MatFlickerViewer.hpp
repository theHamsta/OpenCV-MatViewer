/**
 * \file   MatFlickerViewer.hpp
 * \brief  Defines class MatFlickerViewer
 *
 * \author Stephan Seitz (stephan.seitz@fau.de)
 * \date   04.10.2016
 */


#ifndef MATFLICKERVIEWER_HPP
#define MATFLICKERVIEWER_HPP


#include <QTimer>
#include "QCvMatViewer.hpp"


class MatFlickerViewer : public QCvMatViewer
{
    Q_OBJECT
public:
    explicit MatFlickerViewer(QWidget *parent = 0);
    virtual ~MatFlickerViewer();
	
	/// \name Setters
	/// \{ Set members with corresponding names
	/// \}
	
	/// \name Getters
	/// \{ Return members with corresponding names
	bool firstImageActiveNow() const { return !m_flickerOn; }
	/// \}
	
	inline bool bothImagesSet() const { return !m_mat.empty() && !m_secondMat.empty(); }
	inline bool firstImageSet() const { return !m_mat.empty(); }
	inline bool secondImageSet() const { return !m_secondMat.empty(); }
	
signals:
	void activeImgChanged( bool firstImgActiveNow );
	
public slots:
    void setSecondImage(const cv::Mat& mat);
	void setFlickerInterval(const float mSecs ) { assert(mSecs >= 0); m_flickerIntervalMSecs = mSecs; m_timer.setInterval(mSecs); update(); }
	
private:
    virtual void initializeGL() override;
    virtual void paintGL() override;
	
	
	GLuint m_secondTex; ///< Texture for m_secondMat. To be drawn be OpenGL
	cv::Mat m_secondMat; ///< Second cv::Mat that will be drawn on the right side of the widget
	/// Position where the image splitted. Left to this position m_mat will be drawn and m_secondMat to the right.
	/// The position is relative: 0 is the left border and 1 the right one 
	float m_flickerIntervalMSecs = 500.f;
	bool m_flickerOn = true;
	QTimer m_timer;
	
private slots:
	void toggleFlicker() { m_flickerOn = !m_flickerOn; emit activeImgChanged(!m_flickerOn); update(); }
};

#endif // MATFLICKERVIEWER_HPP
