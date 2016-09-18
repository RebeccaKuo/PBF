#include "Particle.h"

Particle::Particle()
{

}

Particle::Particle(unsigned int _id, ngl::Vec3 _position)
{
  m_id = _id;
  m_pos = _position;

  m_old_pos = m_pos;
  m_last_pos = m_pos;
  m_delta_pos = ngl::Vec3(0,0,0);

  m_vel = ngl::Vec3(0,0,0);
  m_acc = ngl::Vec3(0,0,0);

  m_density = 0.0f;
  m_lamda = 0.0f;
  m_scorr = 0.0f;

  m_colorIndex = 6;

  //m_neighborList.reserve(Definition::getInstance()->m_max_neighbors);
}

Particle::~Particle()
{

}

//--------------------------debug------------------------------
void Particle::printInfo()
{
  std::cout<<"id="<<this->m_id<<" "<<
             this->m_pos.m_x<<" "<<this->m_pos.m_y<<" "<<this->m_pos.m_z<<std::endl<<
             this->m_vel.m_x<<" "<<this->m_vel.m_y<<" "<<this->m_vel.m_z<<std::endl<<
             this->m_acc.m_x<<" "<<this->m_acc.m_y<<" "<<this->m_acc.m_z<<std::endl;
}

