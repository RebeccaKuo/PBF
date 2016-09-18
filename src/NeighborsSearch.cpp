#include "NeighborsSearch.h"

NeighborsSearch* NeighborsSearch::m_instanceNeighborSearch = 0;

NeighborsSearch::NeighborsSearch()
{
  m_grid.m_x = Definition::getInstance()->m_tank_size/Definition::getInstance()->m_kernel_h;
  m_grid.m_y = Definition::getInstance()->m_tank_size/Definition::getInstance()->m_kernel_h;
  m_grid.m_z = Definition::getInstance()->m_tank_size/Definition::getInstance()->m_kernel_h;
  m_totalCell = (float)m_grid.m_x*m_grid.m_y*m_grid.m_z;

  m_instanceNeighborSearch = this;
}

NeighborsSearch::~NeighborsSearch()
{

}

void NeighborsSearch::buildHashTable(Particle  *_particlesList)
{
  Particle *particle;
  unsigned int iKey = 0;

  m_HashMap.clear();

  //#pragma omp parallel for
  for(unsigned int i=0; i<Definition::getInstance()->m_particle_num; i++ )
  {
    particle = &(_particlesList[i]);

    ngl::Vec3 cellPos = ngl::Vec3(0,0,0);
    cellPos = cellMapping(particle->m_pos);
    iKey = buildKey(cellPos);
    //std::cout<<"buildKey id="<<particle->m_id<<" iKey="<<iKey<<std::endl;

    m_HashMap[iKey].push_back(particle->m_id);
  }
}

void NeighborsSearch::findNeighbors(Particle  *_particlesList)
{
  Particle *par;
  Particle *neighborPar;
  ngl::Vec3 cellPos = ngl::Vec3(0,0,0);
  ngl::Vec3 nearPos = ngl::Vec3(0,0,0);
  ngl::Vec3 relPos = ngl::Vec3(0,0,0);
  std::vector<unsigned int> parList;
  unsigned int key=0;
  float dis = 0.0f;

    //omp_set_nested(4);
    //#pragma omp parallel for
    for(unsigned int i = 0; i< Definition::getInstance()->m_particle_num; i++)
    {
      par = &(_particlesList[i]);
      par->m_neighborList.clear();
      cellPos = cellMapping(par->m_pos);
      //std::cout<<"cellPos="<<(cellPos.m_x)<<","<<(cellPos.m_y)<<","<<(cellPos.m_z)<<std::endl;

      for( int x=-1; x<=1; x++ )
      {
        for( int y=-1; y<=1; y++)
        {
          for(int z=-1; z<=1; z++)
          {
            nearPos.m_x = cellPos.m_x+x;
            nearPos.m_y = cellPos.m_y+y;
            nearPos.m_z = cellPos.m_z+z;
            //std::cout<<"nearPos="<<nearPos.m_x<<","<<nearPos.m_y<<","<<nearPos.m_z<<std::endl;

            key = buildKey(nearPos);
            parList = getParticleIndexbyKey(key);
            //std::cout<<"neighborId="<<neighborId<<std::endl;

            if( parList.size()<= 0 )
            {
              continue;
            }

              for( unsigned int j=0; j<parList.size(); j++ )
              {
                neighborPar = &(_particlesList[parList[j]]);
                relPos = (neighborPar->m_pos) - (par->m_pos);

                dis=relPos.lengthSquared();

                if( dis < 0.0000001f || dis >= Definition::getInstance()->m_kernel_2h)
                {
                  continue;
                }
                //else if(par->m_id != neighborPar->m_id)
                //{
                  par->m_neighborList.push_back(neighborPar->m_id);
                  /*std::cout<<par->m_id<<" "<<neighborPar->m_id<<" ("
                           <<par->m_pos.m_x<<","
                           <<par->m_pos.m_y<<","
                           <<par->m_pos.m_z<<") ("
                           <<neighborPar->m_pos.m_x<<","
                           <<neighborPar->m_pos.m_y<<","
                           <<neighborPar->m_pos.m_z<<") "
                           <<" dis="<<dis<<" "<<std::endl;*/
                //}
              }
            }
          }
        }
      }
      //omp_set_nested(0);
}

void NeighborsSearch::updating(Particle  *_particlesList)
{
  //update neighborhood
  buildHashTable(_particlesList);
  findNeighbors(_particlesList);
  testNeighborData(_particlesList);
}

// -------------------------cell index--------------------------

ngl::Vec3 NeighborsSearch::cellMapping( ngl::Vec3 _p_pos )
{
  ngl::Real h = Definition::getInstance()->m_kernel_h;
  ngl::Vec3 cell_pos = ngl::Vec3(0,0,0);
  cell_pos.m_x = ngl::Real(floor(_p_pos.m_x / h)+1);
  cell_pos.m_y = ngl::Real(floor(_p_pos.m_y / h)+1);
  cell_pos.m_z = ngl::Real(floor(_p_pos.m_z / h)+1);
  //std::cout<<"cell_pos="<<(cell_pos.m_x)<<","<<(cell_pos.m_y)<<","<<(cell_pos.m_z)<<std::endl;

  return cell_pos;
}

//spatial hash
unsigned int NeighborsSearch::buildKey(ngl::Vec3 _c_pos)
{
  unsigned int key;
  const int p1 = 73856093 * _c_pos.m_x;
  const int p2 = 19349663 * _c_pos.m_y;
  const int p3 = 83492791 * _c_pos.m_z;
  key = (p1^p2^p3) % nextPrime(2*Definition::getInstance()->m_particle_num);
  return key;
}

bool NeighborsSearch::isPrime(unsigned int _n)
{
  for(unsigned int i=2; i< _n/2; i++)
  {
    if(_n%i==0)
      return false;
  }
  return true;
}

int NeighborsSearch::nextPrime(int _x)
{
  bool q = false;
  if(_x%2 == 0)
    _x--;

  while(q == false)
  {
    _x = _x + 2;
    if(isPrime(_x) == true)
      q = true;
  }
  return _x;
}

std::vector<unsigned int> NeighborsSearch::getParticleIndexbyKey(unsigned int _key)
{
  std::vector<unsigned int> parList;

  std::map<unsigned int,std::vector<unsigned int>>::iterator it = m_HashMap.find(_key);
  if( it != m_HashMap.end() )
  {
    parList = it->second;
  }

  return parList;
}

NeighborsSearch* NeighborsSearch::getInstance()
{
  if( m_instanceNeighborSearch == 0 )
  {
    m_instanceNeighborSearch = new NeighborsSearch();
  }
  return m_instanceNeighborSearch;
}

// ------------------------ debug ---------------------------
void NeighborsSearch::testNeighborData(Particle  *_particlesList)
{
  Particle *par;
  Particle *neighborPar;

  //#pragma omp parallel for ordered schedule(dynamic)
  for(unsigned int i = 0; i< Definition::getInstance()->m_particle_num; i++)
  {
    par = &(_particlesList[i]);

    if(par->m_id == 100)
    {
      for( unsigned int j=0; j<par->m_neighborList.size(); j++ )
      {
        neighborPar = &(_particlesList[par->m_neighborList[j]]);
        neighborPar->m_colorIndex = 1;
        //std::cout<<"testNeighborData "<<neighborPar->m_id<<","<<std::endl;
      }
      par->m_colorIndex = 3;
    }
  }

}

