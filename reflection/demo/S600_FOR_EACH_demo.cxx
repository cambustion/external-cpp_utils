// Hey Emacs, this is -*- coding: utf-8 mode: c++ -*-
// compile: cpp RH_FOR_EACH_demo.cxx

#include "RH_FOR_EACH.h"

#define S600__AR_BOOST_SERIALIZATION_NVP(name) \
ar & BOOST_SERIALIZATION_NVP(name);
//#define

#define S600_STORE(...)                                        \
template<class Archive>                                        \
void serialize(Archive &ar, const unsigned int file_version) { \
  RH_FOR_EACH(S600__AR_BOOST_SERIALIZATION_NVP, __VA_ARGS__)   \
}                                                              \
friend class boost::serialization::access;
//#define

class ProbeStorable : public s600::persistence::Storable {
 public:
  ProbeStorable(int ti, double td, std::string ts)
      : m_testInt(ti), m_testDouble(td), m_testString(ts) {}

 private:
  int m_testInt;
  double m_testDouble;
  std::string m_testString;

  S600_STORE(m_testInt,
             m_testDouble,
             m_testString)
};
