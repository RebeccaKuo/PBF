#include "HashMap.h"

HashMap::HashMap(const unsigned int _bucketCount)
{
  unsigned int count =_bucketCount;
  unsigned int power2 = 1u;
  while( power2 < count )
  {
    power2 <<= 1;
  }

  m_bucketCount = power2;
  m_moduloValue = m_bucketCount -1u;

  m_hashmap = new DefHashMap*[m_bucketCount];
  for( unsigned int i =0; i<m_bucketCount; i++ )
  {
    m_hashmap[i] = NULL;
  }
}

HashMap::~HashMap()
{
  if(m_hashmap)
  {
    for( unsigned int i=0; i < m_bucketCount; i++ )
    {
      if( m_hashmap[i] != NULL )
      {
        m_hashmap[i]->clear();
        delete[] m_hashmap[i];
      }
    }
    delete[] m_hashmap;
    m_hashmap = NULL;
  }
}

/*HashMap::DefHashMap* HashMap::getDefHashMap(const ngl::Vec3 &_index)
{
  return m_hashmap[_index];
}*/


HashMap::HashIndex* HashMap::findValue(const ngl::Vec3 *_key)
{
  const unsigned int hashVal = hashFunction(*_key);
  const unsigned int mapIndex = hashVal & m_moduloValue;

  if( m_hashmap[mapIndex] != NULL )
  {
    std::map<ngl::Vec3, HashIndex>::iterator &iter = (*m_hashmap[mapIndex]).find(hashVal);
    if(iter != (*m_hashmap[mapIndex]).end())
      return &iter->second;
  }
  return NULL;
}

void HashMap::insertValue(const ngl::Vec3 &_key, const HashIndex &_value)
{

}

inline unsigned int HashMap::hashFunction(const ngl::Vec3 _pos)
{
  unsigned int hashKey = 0;
  const int p1 = 73856093;
  const int p2 = 73856093;
  const int p3 = 73856093;

  hashKey = (int(floor(_pos.m_x/Kernel)*p1)) ^
            (int(floor(_pos.m_y/Kernel)*p2)) ^
            (int(floor(_pos.m_z/Kernel)*p3)) % MaxParticle;

  return hashKey;
}
