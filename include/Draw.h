#ifndef _DRAW_H_
#define _DRAW_H_

#include "Definition.h"

#include <QTime>
#include <QEvent>
#include <QResizeEvent>
#include <QOpenGLWidget>

#include "Simulation.h"

class Draw : public QOpenGLWidget
{
public:
  Draw(QWidget *_parent);
  ~Draw();
  void initializeGL();
  void paintGL();

  // Qt 5.5.1 must have this implemented and uses it
  void resizeGL(QResizeEvent *_event);
  // Qt 5.x uses this instead! http://doc.qt.io/qt-5/qopenglwindow.html#resizeGL
  void resizeGL(int _w, int _h);

  std::unique_ptr<Simulation> m_sim_ptr;

  // ------------------- fluid unit vao -------------------
  void createParticleVAO();

private:

  int m_spinXFace;
  int m_spinYFace;
  bool m_rotate;
  bool m_translate;
  int m_origX;
  int m_origY;
  int m_origXPos;
  int m_origYPos;
  int m_width;
  int m_height;

  ngl::Camera m_cam;
  ngl::Vec3 m_modelPos;
  ngl::Mat4 m_mouseGlobalTX;
  QTime m_timer;

  /// @brief flag for the fps timer
  int m_fpsTimer;
  int m_updateTimer;
  /// @brief the fps to draw
  int m_fps;
  int m_frames;

  //-------------------- text ------------------
  std::unique_ptr<ngl::Text> m_text;
  void printText();

  // ------------------- setup cam, shader, matrix ------------------
  void cameraSetup();
  void initialShader();
  void loadMatricesToShader();

  // ------------------- event -------------------
  void keyPressEvent(QKeyEvent *_event);
  void mouseMoveEvent (QMouseEvent * _event );
  void mousePressEvent ( QMouseEvent *_event);
  void mouseReleaseEvent ( QMouseEvent *_event );
  void wheelEvent( QWheelEvent *_event);
  void timerEvent(QTimerEvent *);

};


#endif
