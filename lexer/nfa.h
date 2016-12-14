#include "common.h"
#include <vector>
#include <set>
#include <map>

#pragma once

class NFA{
private:
  std::vector<State> states;

public:
  int size() const;
};
