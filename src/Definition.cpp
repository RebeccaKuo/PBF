#include "Definition.h"

Definition* Definition::m_instanceDefine = 0;

Definition::Definition()
{
  m_tank_size = 10.0f;
  m_particle_num = 0;
  m_kernel_h = 1.2f;
  m_kernel_2h = m_kernel_h*m_kernel_h;
  //m_max_neighbors = 40;
  m_rest_density = 1000.0f;
  m_particle_raduis = 0.1f;
  m_particle_mass = 65.0f;
  m_gravity = -60.0f;
  m_max_iteration = 5;
  m_time_step = 0.1;
  m_epo = 0.000001f;

  m_instanceDefine = this;
}

Definition::~Definition()
{

}

Definition* Definition::getInstance()
{
  if( m_instanceDefine == 0 )
  {
    m_instanceDefine = new Definition();
  }
  return m_instanceDefine;
}


