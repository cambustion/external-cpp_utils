// Hey Emacs, this is -*- coding: utf-8; mode: c++ -*-
// compile: g++ -std=c++14 -Wall -g -ggdb RH_ENUM_CLASS_SHELL_demo.cxx -o RH_ENUM_CLASS_SHELL_demo -I../../server

#include <iostream>

#include "../RH_ENUM_SHELL.hpp"

// see Meyers, S., 2014. Effective Modern C++:
//     42 Specific Ways to Improve Your Use of C++11
//     and C++14, 1 edition. ed. O’Reilly Media, 73.
template<typename E>
constexpr typename std::underlying_type<E>::type toUType(E enumerator) noexcept
{
  return static_cast<typename std::underlying_type<E>::type>(enumerator);
}

// enum class SmokeyStover : short {foo = 1, bar, baz, qux,
//                           foobar = foo + bar, initial = baz,
//                           itemName, item}
RH_ENUM_CLASS_UTYPE_SHELL(SmokeyStover, short, foo = 1, bar, baz, qux,
                          foobar = foo + bar, initial = baz,
                          itemName, item, index, name, itemIndex);

int main(int argc, char **argv) {
  // Itarating over enum items
  for(int i = 0; i < SmokeyStover::itemsCount(); ++i) {
    std::cout << "i = " << i << ", "
              << SmokeyStover::itemsEnumReflectedName()
              << "::" << SmokeyStover::itemsScopeName() << "::"
              << SmokeyStover::itemName(i) << " = "
              << toUType(SmokeyStover::itemValue(i)) << std::endl;
  }

  std::cout << std::endl;

  // Using RH_ENUM_CLASS_SHELL class as enum
  SmokeyStover smokeyStover = SmokeyStover::Item::qux;
  std::cout << "enum item for smokeyStover = "
            << toUType(smokeyStover.itemValue()) << " is ";
  // Getting C++ name from enum value
  std::cout << SmokeyStover::itemsEnumReflectedName()
            << "::" << SmokeyStover::itemsScopeName() << "::"
            << smokeyStover.itemName() << std::endl;

  std::cout << std::endl;

  // Getting enum item from C string
  smokeyStover = "initial";
  std::cout << "First enum item for smokeyStover = "
            << toUType(smokeyStover.itemValue()) << " is ";
  // Getting first C++ name from enum value
  std::cout << SmokeyStover::itemsEnumReflectedName()
            << "::" << SmokeyStover::itemsScopeName() << "::"
            << smokeyStover.itemName() << std::endl;

  {
    std::cout << "All enum items for smokeyStover = "
              << toUType(smokeyStover.itemValue()) << " are ";
    auto itemNames = smokeyStover.itemNames();
    auto iter = itemNames.begin();
    if(iter != itemNames.end()) {
      std::cout << SmokeyStover::itemsEnumReflectedName()
                << "::" << SmokeyStover::itemsScopeName() << "::"
                << *iter; ++iter;
    }
    for(; iter != itemNames.end(); ++iter) {
      std::cout << ", " << SmokeyStover::itemsEnumReflectedName()
                << "::" << SmokeyStover::itemsScopeName() << "::"
                << *iter;
    }
    std::cout << std::endl;
  }
}

// Emacs, here are file hints.
// Local Variables:
// compile-command: "g++ -std=c++14 -Wall -g -ggdb RH_ENUM_CLASS_SHELL_demo.cxx -o RH_ENUM_CLASS_SHELL_demo -I../../server"
// End:
