// Hey Emacs, this is -*- coding: utf-8; mode: c++ -*-
// compile: g++ -std=c++14 -Wall -g -ggdb RH_ENUM_RSHELL_demo.cxx -o RH_ENUM_RSHELL_demo -I../../server

#include "../RH_ENUM_SHELL.hpp"

#include <iostream>

RH_ENUM_RSHELL(SmokeyStover,
               ((this_, "this"), (default_,  "* d e, f a u l t *")),
               foo = 1, bar, baz, qux, foobar = foo + bar,
               default_ = baz, index, symbol, this_ = foo);

int main(int argc, char **argv) {
  // Itarating over enum items
  for(int i = 0; i < SmokeyStover::itemsCount(); ++i) {
    std::cout << "i = " << i
              << ", " << SmokeyStover::itemName(i) << " = "
              << SmokeyStover::itemValue(i) << std::endl;
  }

  std::cout << std::endl;

  SmokeyStover smokeyStover = SmokeyStover::qux;
  // Using RH_ENUM_CLASS_SHELL class as enum
  std::cout << "enum item for smokeyStover = "
            << smokeyStover << " is ";
  // Getting enum name from EnumShell object
  std::cout << smokeyStover.itemName() << std::endl;

  std::cout << std::endl;

  // Getting enum item from string (either C or C++ string).
  // Can throw std::invalid_argument if assigned string is not
  // the valid enum  name.
  smokeyStover = "* d e, f a u l t *";
  // smokeyStover = "baz";
  std::cout << "First enum item for smokeyStover = "
            << smokeyStover << " is ";
  // Getting enum item name from enum value
  std::cout << smokeyStover.itemName() << std::endl;

  {
    std::cout << "All enum items for smokeyStover = "
              << smokeyStover << " are ";
    auto itemNames = smokeyStover.itemNames();
    auto iter = itemNames.begin();
    if(iter != itemNames.end()) {
      std::cout << *iter; ++iter;
    }
    for(; iter != itemNames.end(); ++iter) {
      std::cout << ", " << *iter;
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;

  // constexpr SmokeyStover smokeyStoverCe = SmokeyStover::qux;
  // smokeyStoverCe = "default";

  constexpr SmokeyStover smokeyStoverRnCe = "* d e, f a u l t *";

  constexpr SmokeyStover smokeyStoverCe = "foo";
  static_assert(smokeyStoverCe == "foo", "not 'foo'!");
  static_assert(smokeyStoverCe == SmokeyStover::foo, "not SmokeyStover::foo!");
  static_assert(smokeyStoverCe.itemValue() == 1, "not 1!");
  constexpr int smokeyStoverN = smokeyStoverCe.itemValue();
  std::cout << "constexpr SmokeyStover item value for smokeyStoverCe = "
            << smokeyStoverCe.itemName() << " is ";
  std::cout << smokeyStoverN << std::endl;
}

// Emacs, here are this file hints.
// Local Variables:
// compile-command: "g++ -std=c++14 -Wall -g -ggdb RH_ENUM_RSHELL_demo.cxx -o RH_ENUM_RSHELL_demo -I../../server"
// End:
