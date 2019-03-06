// Hey Emacs, this is -*- coding: utf-8 mode: c++ -*-
#ifndef __s600_refactorables_hpp__
#define __s600_refactorables_hpp__

#include <cstddef>
#include "RH_STRINGIFY.h"

// Extracting data source name.
// D stands for Data
// template<typename R, typename C>
// inline void S600__DataTypeSelector(R(C::*)()) {}
template<typename R, typename C>
inline void S600__DataTypeSelector(R(C::*)() const) {}
template<typename R, typename C>
// inline void S600__DataTypeSelector(R(C::*)(size_t)) {}
// template<typename R, typename C>
inline void S600__DataTypeSelector(R(C::*)(size_t) const) {}
#define S600_D(data) \
  ((S600__DataTypeSelector(&data)), (RH__STRINGIFY(data)))

// DFD stands for Data Formatter Default
#define S600_DFD(data) &data, S600_D(data)

#endif //__s600_refactorables_hpp__
