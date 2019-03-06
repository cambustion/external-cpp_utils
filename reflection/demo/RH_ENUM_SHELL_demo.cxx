// Hey Emacs, this is -*- coding: utf-8; mode: c++ -*-
// compile: g++ -std=c++14 -Wall -g -ggdb RH_ENUM_SHELL_demo.cxx -o RH_ENUM_SHELL_demo -I../../server

#include <iostream>

#include "../RH_ENUM_SHELL.hpp"

// enum SmokeyStover {foo = 1, bar, baz, qux,
//                    foobar = foo + bar, initial = baz}
RH_ENUM_SHELL(SmokeyStover,
              foo = 1, bar, baz, qux,
              foobar = foo + bar, initial = baz);

int main(int argc, char **argv) {
  // Itarating over enum items
  for(int i = 0; i < SmokeyStover::itemsCount(); ++i) {
    std::cout << "i = " << i
              << ", " << SmokeyStover::itemName(i) << " = "
              << SmokeyStover::itemValue(i) << std::endl;
  }

  std::cout << std::endl;

  // Using RH_ENUM_CLASS_SHELL class as enum
  SmokeyStover smokeyStover = SmokeyStover::qux;
  std::cout << "enum item for smokeyStover = "
            << smokeyStover << " is ";
  // Getting std::string name from enum value
  std::cout << smokeyStover.itemName() << std::endl;

  std::cout << std::endl;

  // Getting enum item from C string
  smokeyStover = "initial";
  std::cout << "First enum item for smokeyStover = "
            << smokeyStover << " is ";
  // Getting first std::string name from enum value
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
}

// Emacs, here are file hints.
// Local Variables:
// compile-command: "g++ -std=c++14 -Wall -g -ggdb RH_ENUM_SHELL_demo.cxx -o RH_ENUM_SHELL_demo -I../../server"
// End:
