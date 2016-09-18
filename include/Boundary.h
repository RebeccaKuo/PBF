#ifndef BOUNDARY_H_
#define BOUNDARY_H_

#include "Definition.h"


class Boundary
{
  public:
    Boundary();
    ~Boundary();

    void setBox(const float _tankSize);

    inline void setCam(ngl::Camera *_cam){m_cam=_cam;}
    inline ngl::Camera * getCam()const {return m_cam;}
    inline void setShaderName(const std::string &_n){m_shaderName=_n;}
    inline const std::string getShaderName()const {return m_shaderName;}

    void createVAO();
    void calPosition();
    void drawBox(ngl::Transformation _trans, ngl::Mat4 *_mouseGlobalTX);

    float m_xMin;
    float m_xMax;
    float m_yMin;
    float m_yMax;
    float m_zMin;
    float m_zMax;

  private:
    ngl::Camera *m_cam;
    std::string m_shaderName;
    ngl::Transformation m_trans;

    std::vector<ngl::Vec3> m_point_data;
    GLuint m_vao;

};

#endif
