#ifndef DEFINITION_H_
#define DEFINITION_H_

//system
#include <omp.h>
#include <memory>

//ngl
#include <ngl/Colour.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/Text.h>
#include <ngl/Camera.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>

//data data structure
#include <math.h>
#include <vector>


class Definition
{

 public:
  Definition();
  ~Definition();
  static Definition* getInstance();

  float m_tank_size;
  float m_kernel_h;
  float m_kernel_2h;
  unsigned int m_particle_num;
  float m_rest_density;
  float m_particle_raduis;
  float m_particle_mass;
  float m_gravity;
  unsigned int m_max_iteration;
  float m_time_step;
  float m_epo;

 private:
  static Definition* m_instanceDefine;

};


#endif
