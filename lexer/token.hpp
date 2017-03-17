#pragma once

#include <utility>

struct Token {
  int type;
  std::string lexeme;
  std::pair<int, int> location;
};
