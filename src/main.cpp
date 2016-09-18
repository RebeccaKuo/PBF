
//#include <QtGui/QGuiApplication>
#include <QApplication>
#include "MainWindow.h"
#include "Definition.h"
#include "Simulation.h"
#include "Timer.h"


int main(int argc, char *argv[])
{
 std::cout<<"Position Based Fluids\n";

  // ----------------- Timer -------------------
  Timer::getCurrent()->setStepSize(Definition::getInstance()->m_time_step);

  //---------------- simulation -----------------
  std::unique_ptr<Simulation> sim_ptr;
  sim_ptr.reset(new Simulation());
  //sim_ptr->init();


  QSurfaceFormat format;
  // set the number of samples for multisampling
  // will need to enable glEnable(GL_MULTISAMPLE); once we have a context
  format.setSamples(4);
  #if defined( DARWIN)
    // at present mac osx Mountain Lion only supports GL3.2
    // the new mavericks will have GL 4.x so can change
    format.setMajorVersion(3);
    format.setMinorVersion(2);
  #else
    // with luck we have the latest GL version so set to this
    format.setMajorVersion(4);
    format.setMinorVersion(3);
  #endif
  // now we are going to set to CoreProfile OpenGL so we can't use and old Immediate mode GL
  format.setProfile(QSurfaceFormat::CoreProfile);
  // now set the depth buffer to 24 bits
  format.setDepthBufferSize(24);
  // now we are going to create our scene window

  QSurfaceFormat::setDefaultFormat(format);

  QApplication app(argc, argv);

  MainWindow w;
  //w.m_gl->setFormat(format);
  w.m_gl->resize(1280, 960);
  w.m_gl->m_sim_ptr = std::move(sim_ptr);
  w.show();

  /*Draw drawScene;
  drawScene.setFormat(format);
  std::cout<<"Profile is "<<format.majorVersion()<<" "<<format.minorVersion()<<"\n";
  drawScene.resize(1024, 720);
  drawScene.m_sim_ptr = std::move(sim_ptr);
  drawScene.show();*/

  return app.exec();
}
