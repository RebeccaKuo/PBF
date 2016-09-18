#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include "Definition.h"


class Particle
{
public:
  Particle();
  Particle(unsigned int _id, ngl::Vec3 _position);
  ~Particle();

  unsigned int m_id;
  float m_density;
  float m_lamda;
  ngl::Vec3 m_acc;
  ngl::Vec3 m_vel;
  ngl::Vec3 m_pos;
  ngl::Vec3 m_old_pos;
  ngl::Vec3 m_last_pos;
  ngl::Vec3 m_delta_pos;
  float m_scorr;
  std::vector<unsigned int> m_neighborList;

  // ------- debug -------
  void printInfo();
  int m_colorIndex;

};

#endif
