#include "Sph.h"

#include "Particle.h"


Sph::Sph()
{
  std::cout<<"Sph ctor"<<std::endl;

  m_max_particle = NumParticle;
  m_num_particle = m_max_particle;
  m_counter = 0;

  m_Kernel_H = 0.9f;
  m_mass = 0.8f;
  m_Kernel_H_3d = m_Kernel_H*m_Kernel_H*m_Kernel_H;
  m_cell = m_Kernel_H;

  m_poly6_value= 315.0f/(64.0f * M_PI * pow(m_Kernel_H, 9));
  m_spiky_value= -45.0f/(M_PI * pow(m_Kernel_H, 6));
  m_visco_value= 45.0f/(M_PI * pow(m_Kernel_H, 6));

  m_grad_poly6= -945/(32 * M_PI * pow(m_Kernel_H, 9));
  m_lplc_poly6= -945/(8 * M_PI * pow(m_Kernel_H, 9));

  m_rho0 = 1500.0f;
  m_self_density = m_mass*m_poly6_value*pow(m_Kernel_H, 6);
  m_gas_scale= 1.50111f;

  m_viscosity = 8.5f;
  m_delta_t = 0.08;
  m_surf_norm=6.0f;
  m_surf_coe=0.7f;

  m_wall_damping = -0.05f;

  m_gravity.set(-0.2,-1.8,0);
  m_force_pressure.set(0,0,0);
  m_force_viscosity.set(0,0,0);
  m_force_gravity.set(0,0,0);

  //m_list = (Particle *)malloc(sizeof(Particle) * m_max_particle);
  //m_cell_ptr = (Particle **)malloc(sizeof(Particle *)*m_total_cell);

  m_run = 1;
  //m_pbd_ptr.reset(new PBD());
}

Sph::~Sph()
{
  //free(m_list);
  //free(m_cell_ptr);
}

void Sph::init_simulate(float _w, float _h, float _d)
{
  m_width =_w;
  m_height = _h;
  m_depth = _d;

  m_grid.m_x = (float)m_width/m_cell;
  m_grid.m_y = (float)m_height/m_cell;
  m_grid.m_z = (float)m_depth/m_cell;
  m_total_cell = (float)m_grid.m_x*m_grid.m_y*m_grid.m_z;

  ngl::Vec3 zero;
  zero.set(0,0,0);

  unsigned int counter = 0;

  while(counter < m_num_particle)
  {
    Particle *par = &(m_list[counter]);
    par->m_id = counter;
    par->m_pos = zero;
    par->m_vel = zero;
    par->m_acc = zero;
    par->m_ev = zero;
    par->m_density = m_rho0;
    par->m_pressure = 0;
    par->m_link = NULL;
    //par->m_parent_sph = this;

    counter++;
  }

  init_particle();
}

void Sph::init_particle()
{
  unsigned int counter = 0;

  //initial particles position
  for( int i = -m_width/2; i < m_width/2; i++ )
  {
    for( int j = -m_height/2; j < m_height/2; j++ )
    {
      for( int k = -m_depth/2; k < m_depth/2; k++ )
      {
        if( counter <= m_num_particle )
        {
          Particle *par = &(m_list[counter]);
          par->m_pos.set(i, j, k);
          counter++;
          std::cout<<"i="<<i<<" j="<<j<<" k="<<k<<std::endl;
        }
      }
    }
  }

  //for(unsigned int i=0; i<m_num_particle; i++ )
  //{
  //  m_list[i].printInfo();
  //}
}

void Sph::simulate()
{
  if( m_run == 1 )
  {
    return;
  }

  //conNeighbors();
  //calDensity();
  //calForce();
  //calAdvection();
}

//--------------------------------------------------

void Sph::conNeighbors()
{
  for( unsigned int i=0; i<m_total_cell; i++ )
  {
    m_cell_ptr[i] = NULL;
  }

  Particle *particle;
  unsigned int ihash = 0;

  for(unsigned int i=0; i<m_num_particle; i++ )
  {
    particle = &m_list[i];
    ihash = buildHash(cellMapping(particle->m_pos));

    if( m_cell_ptr[ihash] == NULL )
    {
      particle->m_link = NULL;
    }
    else
    {
      particle->m_link = m_cell_ptr[ihash];
    }

    m_cell_ptr[ihash] = particle;
  }
}

void Sph::calDensity()
{
  Particle *particle;
  Particle *next_particle;

  ngl::Vec3 cell_pos;
  ngl::Vec3 nearby_cell;
  unsigned int ihash;

  ngl::Vec3 relative_pos;
  float R3;

  #pragma omp parallel default(shared)
  {

    #pragma omp for schedule(static)
    for(unsigned int i=0; i<m_num_particle; i++ )
    {
      particle = &m_list[i];
      cell_pos = cellMapping(particle->m_pos);
      particle->m_density = 0;
      particle->m_pressure = 0;

      //2D space - 3*3 uniform grid
      for(int i = -1; i <= 1; i++ )
      {
        for(int j = -1; j <= 1; j++ )
        {
          for(int k = -1; k <= 1; k++)
          {
            nearby_cell.m_x = cell_pos.m_x + i;
            nearby_cell.m_y = cell_pos.m_y + j;
            nearby_cell.m_z = cell_pos.m_z + k;

            ihash = buildHash(nearby_cell);

            if( ihash == 0xffffffff )
            {
              continue;
            }

            next_particle = m_cell_ptr[ihash];
            while( next_particle != NULL )
            {
              relative_pos.m_x = (next_particle->m_pos.m_x) - (particle->m_pos.m_x);
              relative_pos.m_y = (next_particle->m_pos.m_y) - (particle->m_pos.m_y);
              relative_pos.m_z = (next_particle->m_pos.m_z) - (particle->m_pos.m_z);

              R3 = relative_pos.m_x*relative_pos.m_x +
                   relative_pos.m_y*relative_pos.m_y +
                   relative_pos.m_z*relative_pos.m_z;

              if( R3 < INF || R3 >= m_Kernel_H_3d )
              {
                next_particle = next_particle->m_link;
                continue;
              }

              particle->m_density += m_mass * m_poly6_value * pow(m_Kernel_H_3d-R3, 3.3);
              next_particle = next_particle->m_link;
            }
          }
        }
      }
    }

    particle->m_density += m_self_density;
    particle->m_pressure = m_gas_scale * (pow(particle->m_density/m_rho0, 7)-1);
  }

}

void Sph::calForce()
{
  Particle *particle;
  Particle *next_particle;

  ngl::Vec3 cell_pos;
  ngl::Vec3 nearby_cell;
  unsigned int ihash;

  ngl::Vec3 relative_pos;
  ngl::Vec3 relative_vel;

  float R3;
  float R;
  float Kernel_H_R;
  float volume;

  float pressure_Kernel_H;
  float visc_Kernel_H;
  float temp_force;

  ngl::Vec3 grad_color;
  float lplc_color;

  for(unsigned int i=0; i<m_num_particle; i++ )
  {
    particle = &m_list[i];
    cell_pos = cellMapping(particle->m_pos);

    particle->m_acc.m_x = 0;
    particle->m_acc.m_y = 0;
    particle->m_acc.m_z = 0;
    grad_color.set(0.0f, 0.0f,0.0f);
    lplc_color = 0;

    for( int i = -1; i <= 1; i++ )
    {
      for(int j = -1; j <= 1; j++ )
      {
        for(int k = -1; k <= 1; k++)
        {
          nearby_cell.m_x = cell_pos.m_x + i;
          nearby_cell.m_y = cell_pos.m_y + j;
          nearby_cell.m_z = cell_pos.m_z + k;

          ihash = buildHash(nearby_cell);

          if( ihash == 0xffffffff )
          {
            continue;
          }

          next_particle = m_cell_ptr[ihash];

          while( next_particle != NULL )
          {
            relative_pos.m_x = particle->m_pos.m_x - next_particle->m_pos.m_x;
            relative_pos.m_y = particle->m_pos.m_y - next_particle->m_pos.m_y;
            relative_pos.m_z = particle->m_pos.m_z - next_particle->m_pos.m_z;
            R3 = relative_pos.m_x*relative_pos.m_x +
                 relative_pos.m_y*relative_pos.m_y +
                 relative_pos.m_z*relative_pos.m_z;

            //within Kernel_H then calculate
            if( R3 < m_Kernel_H_3d && R3 > INF )
            {
              R = pow(R3,1/2);
              volume = m_mass/next_particle->m_density/3;
              Kernel_H_R = m_Kernel_H - R;

              //pressure to acc
              pressure_Kernel_H = m_spiky_value * Kernel_H_R * Kernel_H_R * Kernel_H_R;
              temp_force = volume * (particle->m_pressure + next_particle->m_pressure) * pressure_Kernel_H;
              particle->m_acc.m_x -= relative_pos.m_x * temp_force/ R;
              particle->m_acc.m_y -= relative_pos.m_y * temp_force/ R;
              particle->m_acc.m_z -= relative_pos.m_z * temp_force/ R;

              //viscosity to acc
              relative_vel.m_x = next_particle->m_ev.m_x - particle->m_ev.m_x;
              relative_vel.m_y = next_particle->m_ev.m_y - particle->m_ev.m_y;
              relative_vel.m_z = next_particle->m_ev.m_z - particle->m_ev.m_z;

              visc_Kernel_H = m_visco_value*(m_Kernel_H- R);
              temp_force = volume * m_viscosity * visc_Kernel_H;
              particle->m_acc.m_x += relative_vel.m_x * temp_force;
              particle->m_acc.m_y += relative_vel.m_y * temp_force;
              particle->m_acc.m_z += relative_vel.m_z * temp_force;

              //surface
              float temp = (-1.0f)* m_poly6_value* volume* pow(m_Kernel_H_3d-R3, 2);
              grad_color.m_x += temp * relative_pos.m_x;
              grad_color.m_y += temp * relative_pos.m_y;
              grad_color.m_z += temp * relative_pos.m_z;
              lplc_color += m_lplc_poly6 * volume * (m_Kernel_H_3d-R3) * (R3-3/4*(m_Kernel_H_3d-R3));
            }

            next_particle = next_particle->m_link;
          }
        }
      }
    }

    lplc_color += m_self_lplc_color/particle->m_density;
    particle->surf_norm = sqrt(grad_color.m_x*grad_color.m_x +grad_color.m_y*grad_color.m_y+ grad_color.m_z*grad_color.m_z);

    if( particle->surf_norm > m_surf_norm )
    {
      particle->m_acc.m_x += m_surf_coe * lplc_color * grad_color.m_x/particle->surf_norm;
      particle->m_acc.m_y += m_surf_coe * lplc_color * grad_color.m_y/particle->surf_norm;
      particle->m_acc.m_z += m_surf_coe * lplc_color * grad_color.m_z/particle->surf_norm;
    }

  }
}

void Sph::calAdvection()
{
  ngl::Vec3 vec3;

  Particle *particle;

  for(unsigned int i=0; i<m_num_particle; i++ )
  {
    particle = &m_list[i];

    vec3.set(particle->m_vel.m_x + particle->m_acc.m_x*m_delta_t + m_gravity.m_x,
             particle->m_vel.m_y + particle->m_acc.m_y*m_delta_t + m_gravity.m_y,
             particle->m_vel.m_z + particle->m_acc.m_z*m_delta_t + m_gravity.m_z);

    particle->setVelocity(vec3);

    vec3.set(particle->m_pos.m_x + particle->m_vel.m_x * m_delta_t,
             particle->m_pos.m_y + particle->m_vel.m_y * m_delta_t,
             particle->m_pos.m_z + particle->m_vel.m_z * m_delta_t);

    particle->setPosition(vec3);

    if(particle->m_pos.m_x > m_width)
    {
      particle->m_vel.m_x *= m_wall_damping;
      particle->m_pos.m_x = m_width;
    }

    if(particle->m_pos.m_x <= -m_width)
    {
      particle->m_vel.m_x *= m_wall_damping;
      particle->m_pos.m_x = -m_width;
    }

    if(particle->m_pos.m_y < -m_height)
    {
      particle->m_vel.m_y *= m_wall_damping;
      particle->m_pos.m_y = -m_height;
    }

    if(particle->m_pos.m_y >= m_height )
    {
      particle->m_vel.m_y *= m_wall_damping;
    }

    if(particle->m_pos.m_z < -m_depth)
    {
      particle->m_vel.m_z *= m_wall_damping;
      particle->m_pos.m_z = -m_depth;
    }

    if(particle->m_pos.m_z >= m_depth)
    {
      particle->m_vel.m_z *= m_wall_damping;
      particle->m_pos.m_z = m_depth;
    }

    particle->m_ev.m_x = (particle->m_ev.m_x + particle->m_vel.m_x)/3;
    particle->m_ev.m_y = (particle->m_ev.m_y + particle->m_vel.m_y)/3;
    particle->m_ev.m_z = (particle->m_ev.m_z + particle->m_vel.m_z)/3;
  }
}

//--------------------------------------------------

ngl::Vec3 Sph::cellMapping( ngl::Vec3 _p_pos )
{
  ngl::Vec3 cell_pos;
  cell_pos.m_x = (int)(_p_pos.m_x / m_cell);
  cell_pos.m_y = (int)(_p_pos.m_y / m_cell);
  cell_pos.m_z = (int)(_p_pos.m_z / m_cell);

  return cell_pos;
}

unsigned int Sph::buildHash(ngl::Vec3 _c_pos)
{
  //create hash number
  _c_pos.m_x = (unsigned int)(_c_pos.m_x) & ((unsigned int)(m_grid.m_x - 1));
  _c_pos.m_y = (unsigned int)(_c_pos.m_y) & ((unsigned int)(m_grid.m_y - 1));
  _c_pos.m_z = (unsigned int)(_c_pos.m_z) & ((unsigned int)(m_grid.m_z - 1));

  return  _c_pos.m_x + (_c_pos.m_y*m_grid.m_x) + (_c_pos.m_z*m_grid.m_x*m_grid.m_y);
  //return (unsigned int)(_c_pos.m_z +(_c_pos.m_y * m_grid.m_x + _c_pos.m_x));
}

//--------------------------------------------------

void Sph::reset()
{
  m_counter = 0;
  init_simulate(m_width, m_height, m_depth);
}

//--------------------------------------------------
/*void Sph::drawFluid(ngl::Mat4 *_mouseGlobalTX)
{
  unsigned int index = 0;

  for(unsigned int i=0; i<m_num_particle; i++ )
  {
    Particle *par = &m_list[i];
    if( i % 100 == 0 )
    {
      //while( par->m_link != NULL )
      //{
        if( index >= 7 )
        {
          index = 0;
        }
        drawParticle(par, _mouseGlobalTX, index);
        //par = par->m_link;
        index ++;
      //}
    }
    else
    {
      drawParticle(par, _mouseGlobalTX, 8);
    }

  }
}*/

/*void Sph::drawParticle(Particle *_par, ngl::Mat4 *_mouseGlobalTX, unsigned int _color_index)
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

  ngl::Material m(_color_index);
  m.loadToShader("material");

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->draw("fluid_unit");
}*/


