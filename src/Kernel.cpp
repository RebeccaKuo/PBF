#include "Kernel.h"

Kernel::Kernel()
{
  m_poly6 = 315.0f/(64.0f * (float)M_PI * pow(Definition::getInstance()->m_kernel_h, 9));
  m_gradPoly6 = -945.0f/(32.0f* (float)M_PI * pow(Definition::getInstance()->m_kernel_h, 9));
  m_gradSpiky = -45/(float)M_PI * pow(Definition::getInstance()->m_kernel_h, 6);
  m_gradViscosity = 45/(float)M_PI * pow(Definition::getInstance()->m_kernel_h, 6);
}

Kernel::~Kernel()
{

}

//------------------------------------------------------------------------------

float Kernel::checkDistanceSquared(ngl::Vec3 _par, ngl::Vec3 _nPar)
{
  float diff = Definition::getInstance()->m_kernel_2h - (_par - _nPar).lengthSquared();
  if( diff < 0.0f )
  {
    return 0.0f;
  }
  return diff;
}

float Kernel::checkDistance(ngl::Vec3 _par, ngl::Vec3 _nPar)
{
  float diff = Definition::getInstance()->m_kernel_h - (_par - _nPar).length();
  if( diff < 0.0f )
  {
    return 0.0f;
  }
  return diff;
}


ngl::Vec3 Kernel::checkDistanceSquared(ngl::Vec3 _vec)
{
  float diff = Definition::getInstance()->m_kernel_2h - _vec.lengthSquared();
  if( diff < 0.0f )
  {
    return ngl::Vec3(0,0,0);
  }
  return _vec;
}

//------------------------------------------------------------------------------
// Reference InteractiveComputerGraphics - SPHKernels
float Kernel::W(ngl::Vec3 _par, ngl::Vec3 _nPar)
{
  float diff = 0.0f;
  float normLength = (_par - _nPar).length();
  float q = normLength/Definition::getInstance()->m_kernel_h;

  float m_k = 8.0f / (M_PI*pow(Definition::getInstance()->m_kernel_h, 3));

  if( q<= 1.0f )
  {
    if( q<= 0.5f )
    {
      float q2 = q*q;
      float q3 = q2*q;
      diff = m_k*(6.0f*q3 - 6.0*q2 +1);
    }
    else
    {
      diff = m_k*(2*pow(1-q,3));
    }
  }

  //std::cout<<"W diff= "<<diff<<std::endl;
  return diff;
}

//------------------------------------------------------------------------------
//Reference InteractiveComputerGraphics - SPHKernels
ngl::Vec3 Kernel::gradW(ngl::Vec3 _par, ngl::Vec3 _nPar)
{
  ngl::Vec3 diff = ngl::Vec3(0,0,0);
  ngl::Vec3 vec_r = _par - _nPar;
  float normLength = vec_r.length();
  float q = normLength/Definition::getInstance()->m_kernel_h;

  float m_l = 48.0f / (M_PI*pow(Definition::getInstance()->m_kernel_h, 3));

  if( q<= 1.0f )
  {
    if(normLength > Definition::getInstance()->m_epo)
    {
      const ngl::Vec3 gradq = vec_r*(1.0f/ (normLength*Definition::getInstance()->m_kernel_h));
      if( q<= 0.5f )
      {
        diff = m_l* q* (3.0f*q - 2.0f)* gradq;
      }
      else
      {
        const float fac = 1.0f - q;
        diff = m_l* (-fac* fac)* gradq;
      }
    }
  }

  return diff;

}

//------------------------------------------------------------------------------

float Kernel::Poly6(ngl::Vec3 _par, ngl::Vec3 _nPar)
{
  float diff = Definition::getInstance()->m_kernel_2h - (_par - _nPar).lengthSquared();
  if( diff < 0.0f )
  {
    return 0.0f;
  }
  std::cout<<"Poly6 diff="<<m_poly6 *diff *diff *diff<<std::endl;
  return m_poly6 *diff *diff *diff;
}

ngl::Vec3 Kernel::gradSpiky(ngl::Vec3 _par, ngl::Vec3 _nPar)
{
  ngl::Vec3 vec = _par - _nPar;
  float diff = Definition::getInstance()->m_kernel_2h - vec.lengthSquared();

  if( vec == ngl::Vec3(0,0,0) || diff < 0.0f)
  {
    return ngl::Vec3(0,0,0);
  }

  float vecLength = vec.length();
  float diff2 = Definition::getInstance()->m_kernel_h - vecLength;
  return m_gradSpiky *diff2 *diff2 *vec /vecLength;
}
