//#include <QMouseEvent>
//#include <QGuiApplication>
//#include "Define.h"
#include "Draw.h"

#include <ngl/NGLInit.h>


const static float INCREMENT=0.5;
const static float ZOOM=0.5;

Draw::Draw( QWidget *_parent ):QOpenGLWidget(_parent)
{
  m_rotate=false;
  m_spinXFace=0;
  m_spinYFace=0;
  m_fps=0;
  m_frames=0;
}

Draw::~Draw()
{

}

void Draw::resizeGL(QResizeEvent *_event )
{
  m_width=_event->size().width()*devicePixelRatio();
  m_height=_event->size().height()*devicePixelRatio();
  m_cam.setShape(45.0f,(float)_event->size().width()/_event->size().height(),0.05f,350.0f);
}

void Draw::resizeGL(int _w , int _h)
{
  m_width=_w*devicePixelRatio();
  m_height=_h*devicePixelRatio();
  m_cam.setShape(45.0f,(float)_w/_h,0.05f,350.0f);
}

void Draw::createParticleVAO()
{
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("fluid_unit",Definition::getInstance()->m_particle_raduis,50);
}

void Draw::initializeGL()
{
  ngl::NGLInit::instance();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
  glViewport(0,0,width(),height());

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  //----------- sim -----------
  m_sim_ptr->visualSetup(&m_cam, "Phong");

  initialShader();

  cameraSetup();

  // ------------- text ------------
  m_text.reset( new  ngl::Text(QFont("Arial",14)));
  m_text->setScreenSize(width(),height());

  m_fpsTimer =startTimer(0);
  m_timer.start();

  m_updateTimer = startTimer(80);
}

void Draw::paintGL()
{
  createParticleVAO();

  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_width,m_height);

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Phong"]->use();

  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);

  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;

  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  //--------------- text ---------------
  ++m_frames;
  printText();

  //-------------- simulation ----------
  m_sim_ptr->visualize(&m_mouseGlobalTX);
}

void Draw::initialShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called Phong
  shader->createShaderProgram("Phong");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("PhongVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("PhongFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("PhongVertex","shaders/PhongVertex.glsl");
  shader->loadShaderSource("PhongFragment","shaders/PhongFragment.glsl");
  // compile the shaders
  shader->compileShader("PhongVertex");
  shader->compileShader("PhongFragment");
  // add them to the program
  shader->attachShaderToProgram("Phong","PhongVertex");
  shader->attachShaderToProgram("Phong","PhongFragment");
  // now bind the shader attributes for most NGL primitives we use the following
  // layout attribute 0 is the vertex data (x,y,z)
  shader->bindAttribute("Phong",0,"inVert");
  // attribute 1 is the UV data u,v (if present)
  shader->bindAttribute("Phong",1,"inUV");
  // attribute 2 are the normals x,y,z
  shader->bindAttribute("Phong",2,"inNormal");

  // now we have associated this data we can link the shader
  shader->linkProgramObject("Phong");

  (*shader)["Phong"]->use();
    shader->setShaderParam1i("Normalize",1);
}

void Draw::cameraSetup()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  ngl::Vec3 from(-1,1,6);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);

  m_cam.set(from,to,up);
  m_cam.setShape(45.0f,(float)720.0/576.0,0.05,350);
  shader->setUniform("viewerPos",m_cam.getEye().toVec3());

  ngl::Mat4 iv=m_cam.getViewMatrix();
  iv.transpose();
  ngl::Light light(ngl::Vec3(50,50,50),ngl::Colour(1,1,1,1),ngl::Colour(1,1,1,1),ngl::LightModes::DIRECTIONALLIGHT );
  light.setTransform(iv);
  light.loadToShader("light");

  glViewport(0,0,width(),height());
}

void Draw::printText()
{
  //FPS
  QString text1=QString("FPS: %1").arg(m_fps);
  m_text->setColour(ngl::Colour(1,1,1,0.6f));
  m_text->renderText(10,18, text1);

  //particle num
  text1=QString("NumParticle: %1").arg(Definition::getInstance()->m_particle_num);
  m_text->renderText(10,36, text1);

  if(m_sim_ptr->m_export == true)
  {
    text1=QString("exporting... filename: particlesOut.abc");
    m_text->renderText(10, 900, text1);
  }
}

//----------------------------------------------------------------------------------------------------------------------
void Draw::mouseMoveEvent (QMouseEvent * _event)
{
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    update();
  }
  // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
   }
}


//----------------------------------------------------------------------------------------------------------------------
void Draw::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void Draw::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
  // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void Draw::wheelEvent(QWheelEvent *_event)
{

  // check the diff of the wheel position (0 means no change)
  if(_event->delta() > 0)
  {
    m_modelPos.m_z+=ZOOM;
  }
  else if(_event->delta() <0 )
  {
    m_modelPos.m_z-=ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void Draw::keyPressEvent(QKeyEvent *_event)
{
  switch (_event->key())
  {
  //simulation switch 0:stop sim 1:start sim
  case Qt::Key_S :
    m_sim_ptr->m_run = 1-(m_sim_ptr->m_run);
    break;
  case Qt::Key_E : m_sim_ptr->toggleExport(); break;
  // show full screen
  case Qt::Key_F :
    showFullScreen();
    break;

  default : break;
  }

  update();
}

//----------------------------------------------------------------------------------------------------------------------
void Draw::timerEvent( QTimerEvent *_event )
{
  if( _event->timerId() == m_updateTimer )
  {
    m_sim_ptr->doSimulate();
  }

  if(_event->timerId() == m_fpsTimer)
  {
    if( m_timer.elapsed() > 1000.0)
    {
      m_fps=m_frames;
      m_frames=0;
      m_timer.restart();
    }
  }

  update();
}

