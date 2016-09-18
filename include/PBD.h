#ifndef _PBD_H_
#define _PBD_H_

#include "NeighborsSearch.h"
#include "Kernel.h"

class PBD
{
public:
  PBD();
  ~PBD();

  void solveConstraint(Particle  *_particlesList);
  void calDensity();
  void calLambda();
  void calDeltaPos();
  void applyDeltaPos();
  void applyCollision();

private:
  Particle *m_particleList;
  unsigned int m_maxIter;
  std::unique_ptr<Kernel> m_kernel;


};

#endif
