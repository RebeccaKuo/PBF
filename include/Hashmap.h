/*
 * InteractiveComputerGraphics - Hashmap
*/

#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <map>
#include "Define.h"


class HashMap
{

public:
  struct HashIndex
  {
    unsigned long timestep;
    std::vector<unsigned int> particleIndex;
  };

  typedef typename std::map<ngl::Vec3, HashIndex> DefHashMap;

  /*HashIndex& operator[](const ngl::Vec3 _key)
  {
    const int hashVal = hashFunction(_key);
    const unsigned int mapIndex = hashVal & m_moduloValue;
    if(m_hashmap[mapIndex] == NULL)
    {
      m_hashmap[mapIndex] = new HashIndex();
    }
    return (*m_hashmap[mapIndex])[hashVal];
  }*/

  HashMap(const unsigned int _bucketCount);
  ~HashMap();

  DefHashMap* getDefHashMap(const ngl::Vec3 &_index);
  HashIndex* findValue(const ngl::Vec3 *_key);
  void insertValue(const ngl::Vec3 &_key, const HashIndex &_value);

  inline unsigned int hashFunction(const ngl::Vec3 _pos);


private:
  DefHashMap** m_hashmap;
  unsigned int m_bucketCount;
  unsigned int m_moduloValue;

};

#endif
