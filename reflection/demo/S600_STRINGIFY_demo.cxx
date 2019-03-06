// Hey Emacs, this is -*- coding: utf-8 mode: c++ -*-
// compile: cpp RH_STRINGIFY_demo.cxx

#include "RH_STRINGIFY.h"

#define S600__LOG_LEVEL_LIST ERROR, WARNING, INFO, TRACE, DEBUG
enum LogLevel {S600__LOG_LEVEL_LIST};
constexpr const char *logLevelNames[] {RH_STRINGIFY_LIST(S600__LOG_LEVEL_LIST)};
#undef S600__LOG_LEVEL_LIST
