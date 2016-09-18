/*
   Reference from jingquanalex/fluid
   Reference InteractiveComputerGraphics - SPHKernels
*/


#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "Definition.h"

class Kernel
{
public:
  Kernel();
  ~Kernel();

  float checkDistanceSquared(ngl::Vec3 _par, ngl::Vec3 _nPar);
  float checkDistance(ngl::Vec3 _par, ngl::Vec3 _nPar);
  ngl::Vec3 checkDistanceSquared(ngl::Vec3 _vec);

  float W(ngl::Vec3 _par, ngl::Vec3 _nPar);
  ngl::Vec3 gradW(ngl::Vec3 _par, ngl::Vec3 _nPar);

  float Poly6(ngl::Vec3 _par, ngl::Vec3 _nPar);
  ngl::Vec3 gradSpiky(ngl::Vec3 _par, ngl::Vec3 _nPar);

  float m_poly6;
  float m_gradPoly6;
  float m_gradSpiky;
  float m_gradViscosity;

};


#endif
