#include "PBD.h"

PBD::PBD()
{
  m_maxIter = Definition::getInstance()->m_max_iteration;
  m_kernel.reset(new Kernel());
}

PBD::~PBD()
{
}

void PBD::solveConstraint(Particle  *_particlesLists)
{
  unsigned int iter = 0;
  m_particleList = _particlesLists;

  while( iter < m_maxIter )
  {
    calDensity();

    calLambda();

    calDeltaPos();

    applyDeltaPos();

    iter++;
  }
}

void PBD::calDensity()
{
   Particle *par;
   Particle *nPar;
   float temp_density = 0.0f;

   //#pragma omp parallel for ordered schedule(dynamic)
   for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++)
   {
     temp_density = 0.0f;//Definition::getInstance()->m_rest_density;
     par = &(m_particleList[i]);

     for( unsigned int j=0; j<par->m_neighborList.size(); j++ )
     {
       nPar = &(m_particleList[par->m_neighborList[j]]);

       //calculate density
       temp_density = temp_density + Definition::getInstance()->m_particle_mass * m_kernel->W(par->m_pos, nPar->m_pos);
     }
     par->m_density = temp_density;
   }
}

void PBD::calLambda()
{
  Particle *par;
  Particle *nPar;
  float C = 0;
  ngl::Vec3 gradCi = ngl::Vec3(0,0,0);
  float sum_grad_C2 = 0.0f;
  float scorr = 0.0;
  float temp1 = 0.0;
  float temp2 = m_kernel->m_poly6 * pow(0.3f*Definition::getInstance()->m_kernel_h, 3);
  float temp3 = 0;
  float temp4 = 0;

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++)
  {
    par = &(m_particleList[i]);
    //C = par->m_density/Definition::getInstance()->m_rest_density - 1;
    //calculate constraint
    C = fmax(par->m_density/Definition::getInstance()->m_rest_density - 1, 0.0f);
    //std::cout<<i<<" C="<<C<<std::endl;
    if( C != 0.0 )
    {
      sum_grad_C2 = 0.0f;
      gradCi.set(0,0,0);

      for( unsigned int j=0; j<par->m_neighborList.size(); j++ )
      {
        nPar = &(m_particleList[par->m_neighborList[j]]);

        const ngl::Vec3 gradCj = -(Definition::getInstance()->m_particle_mass)/Definition::getInstance()->m_rest_density * m_kernel->gradW(par->m_pos, nPar->m_pos);
        /*std::cout<<"gradSpiky "<<m_kernel->gradSpiky(par->m_pos, nPar->m_pos).m_x<<","<<
                                 m_kernel->gradSpiky(par->m_pos, nPar->m_pos).m_y<<","<<
                                 m_kernel->gradSpiky(par->m_pos, nPar->m_pos).m_z<<
                                 std::endl;*/

        /*std::cout<<i<<" gradCj="<<gradCj.m_x<<","<<
                                  gradCj.m_y<<","<<
                                  gradCj.m_z<<","<<std::endl;*/

        sum_grad_C2 += gradCj.lengthSquared();
        gradCi -= gradCj;

        //scorr
        temp3 = m_kernel->checkDistanceSquared(par->m_pos, nPar->m_pos);
        temp1 = m_kernel->m_poly6* temp3* temp3* temp3;
        temp4 = temp1/temp2;
        scorr = -0.1f*temp4*temp4*temp4*temp4;
      }
      sum_grad_C2 += gradCi.lengthSquared();
      //std::cout<<i<<" C="<<C<<" sum_grad_C2="<<sum_grad_C2<<std::endl;

      par->m_lamda = -C / (sum_grad_C2 + Definition::getInstance()->m_epo);
      par->m_scorr = scorr;
    }
    else
    {
      par->m_lamda = 0.0f;
      par->m_scorr = 0.0f;
    }
  }
}

void PBD::calDeltaPos()
{
  Particle *par;
  Particle *nPar;
  ngl::Vec3 delatPos = ngl::Vec3(0,0,0);

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++)
  {
    par = &(m_particleList[i]);
    delatPos.set(0,0,0);

    for( unsigned int j=0; j<par->m_neighborList.size(); j++ )
    {
      nPar = &(m_particleList[par->m_neighborList[j]]);

      //calculate delta position
      const ngl::Vec3 gradCj = -(Definition::getInstance()->m_particle_mass)/Definition::getInstance()->m_rest_density * m_kernel->gradW(par->m_pos, nPar->m_pos);
      /*std::cout<<"gradCj "<<gradCj.m_x<<","<<
                              gradCj.m_y<<","<<
                              gradCj.m_z<<
                              std::endl;*/

      delatPos = (delatPos - (par->m_lamda + nPar->m_lamda + par->m_scorr) * gradCj);
      //delatPos = delatPos - (par->m_lamda + nPar->m_lamda) * gradCj;
    }

    //par->m_delta_pos = m_kernel->checkDistanceSquared(delatPos);
    par->m_delta_pos = delatPos;
  }
}

void PBD::applyCollision()
{
  Particle *par;

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++ )
  {
    par = &(m_particleList[i]);

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

void PBD::applyDeltaPos()
{
  Particle *par;

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++)
  {
    par = &(m_particleList[i]);
    par->m_pos = par->m_pos + par->m_delta_pos;
    /*std::cout<<"par.m_id= "<<par->m_id<<" "
             <<par->m_pos.m_x<<","
             <<par->m_pos.m_y<<","
             <<par->m_pos.m_z<<std::endl;*/
  }
}



