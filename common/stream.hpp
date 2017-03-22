#pragma once

#include <iostream>
#include <vector>

class Stream {
private:
  std::istream & s;
  int line = 1, col = 0;
  std::vector<int> cols;

public:
  Stream(std::istream & s) : s(s) {}

  char peek();
  char get();
  void unget();
  std::pair<int, int> location();
};
