#ifndef _NEIGHBORSSEARCH_H_
#define _NEIGHBORSSEARCH_H_

#include "Definition.h"
#include "Particle.h"


class NeighborsSearch
{
public:
  NeighborsSearch();
  ~NeighborsSearch();

  static NeighborsSearch* getInstance();
  std::vector<unsigned int> getParticleIndexbyKey(unsigned int _key);

  void buildHashTable(Particle  *_particlesList);
  void findNeighbors(Particle  *_particlesList);
  void testNeighborData(Particle  *_particlesList);

  void updating(Particle  *_particlesList);
  ngl::Vec3 cellMapping(ngl::Vec3 _p_pos);
  unsigned int buildKey(ngl::Vec3 _c_pos);
  inline unsigned int hashFunction(const ngl::Vec3  _pos);
  bool isPrime(unsigned int _n);
  int nextPrime(int _x);

private:
  ngl::Vec3 m_grid;
  unsigned int m_totalCell;
  std::map<unsigned int,  std::vector<unsigned int>> m_HashMap;

  static NeighborsSearch* m_instanceNeighborSearch;

};

#endif
