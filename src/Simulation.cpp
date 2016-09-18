#include "Simulation.h"

#include <QElapsedTimer>
#include <boost/format.hpp>
#include <ngl/RibExport.h>
#include <ngl/Logger.h>
#include <array>

namespace AbcG = Alembic::AbcGeom;

Simulation::Simulation()
{
  m_archive.reset(new  AbcG::OArchive(Alembic::AbcCoreOgawa::WriteArchive(),"particlesOut.abc") );
  // create time sampling of 24 fps at frame 0 to start
  AbcG::TimeSampling ts(1.0f/24.0f, 0.0f);
  // get the archive top
  AbcG::OObject topObj( *m_archive.get(), AbcG::kTop );
  // then add in our time sampling.
  Alembic::Util::uint32_t tsidx = topObj.getArchive().addTimeSampling(ts);
  // this is our particle outputs to write to each frame
  m_partsOut.reset( new AbcG::OPoints(topObj, "simpleParticles", tsidx) );

  m_loadObj = true;
  m_current_timestep = 0.0f;

  m_pbd.reset(new PBD());
  m_boundary.reset(new Boundary());
  m_neighborSearch.reset(new NeighborsSearch());
  m_kernel.reset(new Kernel());
}

Simulation::~Simulation()
{
}

void Simulation::reset()
{
  //stop sim and visualization
  m_run = 1;
  m_drawing = 1;

  m_particles.clear();
  Definition::getInstance()->m_particle_num = 0;
}

void Simulation::initParameter()
{
  // ------------- initial boundary ------------
  m_boundary->setBox(Definition::getInstance()->m_tank_size);

  // ------------- Timer ------------
  Timer::getCurrent()->setStepSize(Definition::getInstance()->m_time_step);
}

void Simulation::initParticleObj(std::vector<ngl::Vec3> &_pos)
{
  if( m_loadObj )
  {
    setupInitObjParticles(_pos);
  }
}

void Simulation::initParticleDefault()
{
  if( !m_loadObj )
  {
    setupInitParticlesPos();
  }
}

//initial particles position (dam break)
void Simulation::setupInitParticlesPos()
{
  unsigned int counter = 0;
  ngl::Vec3 pos = ngl::Vec3(0,0,0);

  for(float i= -Definition::getInstance()->m_tank_size; i< 0; i+=Definition::getInstance()->m_particle_raduis*2 )
  {
    for(float j= -Definition::getInstance()->m_tank_size; j< 0; j+=Definition::getInstance()->m_particle_raduis*2 )
    {
      for(float k= -Definition::getInstance()->m_tank_size; k< 0; k+=Definition::getInstance()->m_particle_raduis*2 )
      {
        if( counter <= Definition::getInstance()->m_particle_num*0.5f )
        {
          pos.set(i,j,k);
          m_particles_pos.push_back(pos);
          m_particles.push_back(Particle(counter, pos));
          counter++;
        }
      }
    }
  }

  for(float i= Definition::getInstance()->m_tank_size-Definition::getInstance()->m_particle_raduis*10; i> 0; i-=Definition::getInstance()->m_particle_raduis*2 )
  {
    for(float j= Definition::getInstance()->m_tank_size-Definition::getInstance()->m_particle_raduis*10; j> 0; j-=Definition::getInstance()->m_particle_raduis*2 )
    {
      for(float k= Definition::getInstance()->m_tank_size-Definition::getInstance()->m_particle_raduis*10; k> 0; k-=Definition::getInstance()->m_particle_raduis*2 )
      {
        if( counter <= Definition::getInstance()->m_particle_num )
        {
          pos.set(i,j,k);
          m_particles_pos.push_back(pos);
          m_particles.push_back(Particle(counter, pos));
          counter++;
        }
      }
    }
  }

  //initial all particles done then draw
  m_drawing = 0;
  m_particleList = &(m_particles[0]);
}

//initial particles position (load obj)
void Simulation::setupInitObjParticles(std::vector<ngl::Vec3> &_pos)
{
  if( m_loadObj )
  {
    ngl::Vec3 pos;
    for(unsigned int i=0; i<_pos.size(); ++i)
    {
      pos = _pos[i];
      m_particles.push_back(Particle(i, pos));
      std::cout<<pos.m_x<<","<<pos.m_y<<","<<pos.m_z<<std::endl;
    }

    //setup new particles number by import particles obj
    Definition::getInstance()->m_particle_num = _pos.size();

    //initial all particles done then draw
    m_drawing = 0;
    m_particleList = &(m_particles[0]);
  }
}

// ---------------- main function for PBD fluid simulation - algorithm ------------------
void Simulation::doSimulate()
{
  if( m_run == 1 )
  {
    return;
  }

  m_current_timestep = Timer::getCurrent()->getStepSize();

  //------- line 1-4 -------
  setAccleration(m_current_timestep);

  //limit timestep
  updateCFLTimeStep();

  //------- line 1-4 -------
  calAdvection(m_current_timestep);

  //------- line 5-7 -------
  updateNeighbor();

  //------- line 8-19 -------
  solveConstraint();

  //------- line 21 -------
  adjustVelocity(m_current_timestep);

  //------- line 22 -------
  //calXSPH();

  //update timer
  updateTimer(m_current_timestep);
}

void Simulation::setAccleration(float _timestep)
{
  ngl::Vec3 grav;
  grav.set(0.0, Definition::getInstance()->m_gravity, 0.0);

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i=0; i <Definition::getInstance()->m_particle_num; i++ )
  {
    m_particleList[i].m_acc = grav;
  }
}

void Simulation::updateCFLTimeStep()
{
  float delta_t = Timer::getCurrent()->getStepSize();
  float minStep = Timer::getCurrent()->getMinStepSize();
  float maxStep = Timer::getCurrent()->getMaxStepSize();
  float maxVel = 0.1;
  float cflFactor = 1;

  ngl::Vec3 vec3 = ngl::Vec3(0,0,0);
  Particle *par;

  //#pragma omp parallel for ordered schedule(dynamic)
  for( unsigned int i=0; i < Definition::getInstance()->m_particle_num; i++  )
  {
    par = &(m_particleList[i]);
    vec3 = par->m_vel + delta_t * par->m_acc;

    float velMag = vec3.length();

    if( velMag>maxVel )
    {
      maxVel = velMag;
    }
  }

  delta_t = cflFactor*0.4*(2*Definition::getInstance()->m_particle_raduis/(sqrt(maxVel)));
  delta_t = fmin(delta_t, minStep);
  delta_t = fmax(delta_t, maxStep);
  Timer::getCurrent()->setStepSize(delta_t);
}


void Simulation::calAdvection(float _timestep)
{
  Particle *par;

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++ )
  {
    par = &(m_particleList[i]);

    //PBD update position
    par->m_delta_pos = ngl::Vec3(0,0,0);
    par->m_last_pos = par->m_old_pos;
    par->m_old_pos = par->m_pos;

    par->m_vel = par->m_vel + _timestep * par->m_acc;
    par->m_pos = par->m_pos + _timestep * par->m_vel;

    /*std::cout<<i<<" par->m_vel="<<par->m_vel.m_x<<","<<
                                  par->m_vel.m_y<<","<<
                                  par->m_vel.m_z<<","<<std::endl;*/

    float factor = -0.6f;
    // axis-x
    if(par->m_pos.m_x > Definition::getInstance()->m_tank_size)
    {
      par->m_pos.m_x = (float)(Definition::getInstance()->m_tank_size-2*Definition::getInstance()->m_particle_raduis);
      par->m_vel.m_x = par->m_vel.m_x * factor;
    }
    else if(par->m_pos.m_x <= -Definition::getInstance()->m_tank_size)
    {
      par->m_pos.m_x = (float)(-Definition::getInstance()->m_tank_size+2*Definition::getInstance()->m_particle_raduis);
      par->m_vel.m_x = par->m_vel.m_x * factor;
    }

    // axis-y
    if(par->m_pos.m_y < -Definition::getInstance()->m_tank_size)
    {
      par->m_pos.m_y = (float)(-Definition::getInstance()->m_tank_size+2*Definition::getInstance()->m_particle_raduis);
      par->m_vel.m_y = par->m_vel.m_y * factor;
    }
    else if(par->m_pos.m_y >= (float)Definition::getInstance()->m_tank_size )
    {
      par->m_pos.m_y = (float)(Definition::getInstance()->m_tank_size-2*Definition::getInstance()->m_particle_raduis);
      par->m_vel.m_y = par->m_vel.m_y * factor;
    }

    // axis-z
    if(par->m_pos.m_z < -Definition::getInstance()->m_tank_size)
    {
      par->m_pos.m_z = (float)(-Definition::getInstance()->m_tank_size+2*Definition::getInstance()->m_particle_raduis);
      par->m_vel.m_z = par->m_vel.m_z * factor;
    }
    else if(par->m_pos.m_z >= Definition::getInstance()->m_tank_size)
    {
      par->m_pos.m_z = (float)(Definition::getInstance()->m_tank_size-2*Definition::getInstance()->m_particle_raduis);
      par->m_vel.m_z = par->m_vel.m_z * factor;
    }
  }
}

// -------------------------------Neighbor searching---------------------------------
void Simulation::updateNeighbor()
{
  m_neighborSearch->updating(m_particleList);
}

// -------------------------------PBD constraints---------------------------------
void Simulation::solveConstraint()
{
  m_pbd->solveConstraint(m_particleList);
}

void Simulation::adjustVelocity(float _timestep)
{
  Particle *par;

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++ )
  {
    par = &m_particleList[i];
    par->m_vel = (1.0f/_timestep)*(par->m_pos - par->m_old_pos);

    //par->m_vel = (1/_timestep)*(1.5f*par->m_pos - 2.0f*par->m_old_pos + 0.5f*par->m_last_pos);

    /*std::cout<<i<<"par.m_id= "<<par->m_id<<" "
                              <<par->m_vel.m_x<<","
                              <<par->m_vel.m_y<<","
                              <<par->m_vel.m_z<<std::endl;*/
  }
}

void Simulation::calXSPH()
{
  Particle *par;
  Particle *nPar;
  ngl::Vec3 par_vel = ngl::Vec3(0,0,0);

  //omp_set_nested(1);
  //#pragma omp parallel for ordered schedule(dynamic)
  //#pragma omp parallel for
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++)
  {
    par = &m_particleList[i];
    par_vel = par->m_vel;

    //#pragma omp parallel for ordered schedule(dynamic)
    for( unsigned int j=0; j< par->m_neighborList.size(); j++ )
    {
      nPar = &(m_particleList[par->m_neighborList[j]]);
      par_vel = par_vel - (0.02f*
                          (Definition::getInstance()->m_particle_mass /  nPar->m_density *(nPar->m_vel - par->m_vel)*
                          (m_kernel->W(par->m_pos, nPar->m_pos))));
    }

    par->m_vel = par_vel;

    /*std::cout<<"par.m_id= "<<par->m_id<<" "
                           <<par->m_vel.m_x<<","
                           <<par->m_vel.m_y<<","
                           <<par->m_vel.m_z<<std::endl;*/
  }

  //omp_set_nested(0);
}

void Simulation::updateTimer( float _timestep )
{
  Timer::getCurrent()->setTime(Timer::getCurrent()->getTime() + _timestep);
}

// ----------------------- alembic ---------------------
void Simulation::exportFrame()
{
  static int frame=0;
  ++frame;

  // this is the data we are going to store, alembic uses Imath
  // internally so we convert from ngl
  // this is the array of particle positions for the frame
  std::vector<Imath::V3f> positions;
  // these are the particle id's which are required so use use index no
  std::vector<Alembic::Util::uint64_t> id;
  // set this to push back into the array
  Imath::V3f data;
  // colour values
  Imath::C4f c;
  std::vector<Imath::C4f> colours;

  for(unsigned int  i=0; i<Definition::getInstance()->m_particle_num; ++i)
  {
    positions.push_back(Imath::V3f(m_particleList[i].m_pos.m_x,m_particleList[i].m_pos.m_y,m_particleList[i].m_pos.m_z));
    id.push_back(i);
  }
  // create as samples we need to do this else we get a most vexing parse
  // https://en.wikipedia.org/wiki/Most_vexing_parse using below
  // psamp(V3fArraySample( positions),UInt64ArraySample(id))
  AbcG::V3fArraySample pos(positions);
  AbcG::UInt64ArraySample ids(id);
  AbcG::OPointsSchema::Sample psamp( pos,ids );

  m_partsOut->getSchema().set( psamp );

}

// ----------------------- draw ------------------------
void Simulation::visualSetup(ngl::Camera *_cam, const std::string &_n)
{
  m_cam=_cam;
  m_shaderName=_n;
  m_boundary->setCam(m_cam);
  m_boundary->setShaderName("Phong");
}

void Simulation::visualize(ngl::Mat4 *_mouseGlobalTX)
{
  if( m_drawing == 1 )
  {
    return;
  }

  drawFluid(_mouseGlobalTX);
  drawBoundary(m_trans, _mouseGlobalTX);

  QElapsedTimer timer;
  timer.start();
  ngl::Logger *log = ngl::Logger::instance();

  //export simulation result to alembic
  if(m_export == true)
  {
    timer.restart();
    exportFrame();
    log->logMessage("Alembic Export took %d milliseconds\n",timer.elapsed());
  }
}

void Simulation::drawFluid(ngl::Mat4 *_mouseGlobalTX)
{
  Particle *par;

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++ )
  {
    par = &m_particleList[i];
    drawParticle(par, _mouseGlobalTX, par->m_colorIndex);
  }
}

void Simulation::drawBoundary(ngl::Transformation _trans, ngl::Mat4 *_mouseGlobalTX)
{
  m_boundary->drawBox(_trans, _mouseGlobalTX);
}

void Simulation::drawParticle(Particle *_par, ngl::Mat4 *_mouseGlobalTX, unsigned int _color_index)
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Phong"]->use();

  m_trans.setPosition(_par->m_pos);

  ngl::Mat4 M;
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;

  M=m_trans.getMatrix()*(*_mouseGlobalTX);
  MV=M*(m_cam->getViewMatrix());
  MVP=MV*(m_cam->getProjectionMatrix());

  normalMatrix=MV;
  normalMatrix.inverse();

  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParamFromMat4("M",M);

  //std::cout<<"color_index="<<_color_index<<std::endl;
  ngl::Material m(_color_index);
  m.loadToShader("material");

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->draw("fluid_unit");
}


