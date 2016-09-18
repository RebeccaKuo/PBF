#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "Definition.h"

#include "PBD.h"
#include "NeighborsSearch.h"
#include "Timer.h"
#include "Boundary.h"

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreOgawa/All.h>

class Simulation
{
//--------------------- Simulation ---------------------
public:
  Simulation();
  ~Simulation();

  int m_run = 1;
  int m_drawing = 1;
  bool m_loadObj;
  void toggleSimulation(){ m_run = 1-m_run; }

  void initParameter();
  void initParticleDefault();
  void initParticleObj(std::vector<ngl::Vec3> &_pos);

  void doSimulate();
  void reset();

  void setAccleration(float _timestep);
  void updateCFLTimeStep();
  void calAdvection(float _timestep);
  void updateNeighbor();
  void solveConstraint();
  void adjustVelocity(float _timestep);
  void calXSPH();
  void updateTimer(float _timestep);

  std::unique_ptr<Kernel> m_kernel;
  std::unique_ptr<PBD> m_pbd;
  std::unique_ptr<NeighborsSearch> m_neighborSearch;
  std::unique_ptr<Timer> m_timer;

private:
  Particle *m_particleList;
  std::vector <Particle> m_particles;
  std::vector <ngl::Vec3> m_particles_pos;
  std::unique_ptr<Boundary> m_boundary;

  void setupInitObjParticles(std::vector<ngl::Vec3> &_pos);
  void setupInitParticlesPos();

  float m_current_timestep;

//--------------------- alembic ------------------
public:
  bool m_export=false;
  void toggleExport(){ m_export = !m_export;}
  void exportFrame();
  std::unique_ptr <Alembic::AbcGeom::OArchive> m_archive;
  std::unique_ptr <Alembic::AbcGeom::OPoints> m_partsOut;

//--------------------- draw ---------------------
public:
  void visualSetup(ngl::Camera *_cam, const std::string &_n);
  inline ngl::Camera * getCam()const {return m_cam;}
  inline const std::string getShaderName()const {return m_shaderName;}
  void visualize(ngl::Mat4 *_mouseGlobalTX);
  void drawFluid(ngl::Mat4 *_mouseGlobalTX);
  void drawBoundary(ngl::Transformation _trans, ngl::Mat4 *_mouseGlobalTX);
  void drawParticle(Particle *_par, ngl::Mat4 *_mouseGlobalTX, unsigned int _color_index);

private:
  ngl::Camera *m_cam;
  std::string m_shaderName;
  ngl::Transformation m_trans;

};

#endif
