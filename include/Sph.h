#ifndef _SPH_H_
#define _SPH_H_

#include "Define.h"
#include "Particle.h"



class Sph
{
public:
  unsigned int m_max_particle;
  unsigned int m_num_particle;
  unsigned int m_counter;
  float m_mass;

  //--------- Kernel_H -----------
  float m_poly6_value;
  float m_spiky_value;
  float m_visco_value;


  float m_cell;
  ngl::Vec3 m_grid;
  unsigned int m_total_cell;

  //--------- neighbor ---------
  float m_Kernel_H;
  float m_Kernel_H_3d;

  //--------- density ----------
  float m_self_density;
  float m_scale_pres;
  float m_wall_damping;
  float m_rho0;
  float m_gas_scale;

  //--------- viscosity ---------
  float m_viscosity;

  //--------- force ------------
  ngl::Vec3 m_gravity;
  ngl::Vec3 m_force_gravity;
  ngl::Vec3 m_force_pressure;
  ngl::Vec3 m_force_viscosity;
  ngl::Vec3 m_force;

  //--------- time step --------
  float m_delta_t;

  //--------- boundary ---------
  float m_width;
  float m_height;
  float m_depth;

  //--------- surface ------
  float m_grad_poly6;
  float m_lplc_poly6;
  float m_self_lplc_color;
  float m_surf_norm;
  float m_surf_coe;


  Particle **m_cell_ptr;

  //-------- system -----------
  int m_run;

public:
  Sph();
  ~Sph();


  void simulate();
  void init_simulate(float _w, float _h, float _d);
  void init_particle();

  void reset();

private:
  Particle *m_list;

  void conNeighbors();
  void calDensity();
  void calPressure();
  void calForce();
  void calAdvection();

  ngl::Vec3 cellMapping(ngl::Vec3 _p_pos);
  unsigned int buildHash(ngl::Vec3 _c_pos);

};

#endif
