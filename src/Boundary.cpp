#include "Boundary.h"


Boundary::Boundary()
{

}

Boundary::~Boundary()
{

}

void Boundary::setBox(const float _tankSize)
{
  m_xMin = -(_tankSize);
  m_xMax =   _tankSize;
  m_yMin = -(_tankSize);
  m_yMax =   _tankSize;
  m_zMin = -(_tankSize);
  m_zMax =   _tankSize;
}

void Boundary::createVAO()
{
  calPosition();

  GLuint point_vbo;
  glGenBuffers(1,&point_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, point_vbo);

  glBufferData(GL_ARRAY_BUFFER, m_point_data.size()*sizeof(ngl::Vec3), &m_point_data[0].m_x, GL_STATIC_DRAW);

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, point_vbo);

  //enable vertex attributes - points
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);

  glEnableVertexAttribArray(0);
  //glBindVertexArray(0);
  //glBindBuffer(GL_ARRAY_BUFFER,0);
  //std::cout<<"createVAO..."<<std::endl;
}

void Boundary::calPosition()
{
  m_point_data.clear();

  //point_data
  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMin, m_zMax));// v1 -1 -1  1
  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMin, m_zMax));// v2  1 -1  1

  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMin, m_zMax));// v2  1 -1  1
  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMin, m_zMin));// v3  1 -1 -1

  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMin, m_zMin));// v3  1 -1 -1
  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMin, m_zMin));// v4 -1 -1 -1

  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMin, m_zMax));// v1 -1 -1  1
  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMin, m_zMin));// v4 -1 -1 -1

  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMin, m_zMax));// v1 -1 -1  1
  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMax, m_zMax));// v5 -1  1  1

  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMin, m_zMin));// v4 -1 -1 -1
  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMax, m_zMin));// v8 -1  1 -1

  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMin, m_zMax));// v2  1 -1  1
  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMax, m_zMax));// v6  1  1  1

  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMin, m_zMin));// v3  1 -1 -1
  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMax, m_zMin));// v7  1  1 -1

  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMax, m_zMax));// v5 -1  1  1
  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMax, m_zMax));// v6  1  1  1

  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMax, m_zMax));// v6  1  1  1
  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMax, m_zMin));// v7  1  1 -1

  m_point_data.push_back( ngl::Vec3(m_xMax, m_yMax, m_zMin));// v7  1  1 -1
  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMax, m_zMin));// v8 -1  1 -1

  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMax, m_zMax));// v5 -1  1  1
  m_point_data.push_back( ngl::Vec3(m_xMin, m_yMax, m_zMin));// v8 -1  1 -1

  /*for ( auto &vec3 : m_point_data )
  {
    std::cout<<"bound box point: "<<vec3.m_x<<" "<<vec3.m_y<<" "<<vec3.m_z<<std::endl;
  }*/
}

void Boundary::drawBox(ngl::Transformation _trans, ngl::Mat4 *_mouseGlobalTX)
{
  createVAO();

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(getShaderName());
  ngl::Transformation trans;

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;

  M=trans.getMatrix()*(*_mouseGlobalTX);
  MV=M*getCam()->getViewMatrix();
  MVP=MV*getCam()->getProjectionMatrix();

  normalMatrix=MV;
  normalMatrix.inverse();

  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParamFromMat4("M",M);

  glBindVertexArray(m_vao);
  glPointSize(5);
  glDrawArrays(GL_LINES, 0, m_point_data.size());
}
