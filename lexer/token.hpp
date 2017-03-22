#pragma once

#include <string>
#include <utility>

struct Token {
  int type;
  std::string lexeme;
  std::pair<int, int> location;

  bool operator==(const int rhs) const {
    return type == rhs;
  }

  const char * c_str() const {
    return lexeme.c_str();
  }
};
