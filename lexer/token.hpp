#pragma once

#include <utility>

struct Token {
  std::string type;
  std::string lexeme;
  std::pair<int, int> location;
};
