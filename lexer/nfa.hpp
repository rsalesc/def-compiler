#include "common.hpp"
#include <vector>
#include <set>
#include <map>

#pragma once

class NFA{
private:
  std::vector<NState> m_states;

public:
  int size() const;

  const std::vector<NState> & states() const;
  std::vector<NState> & states();

  NState & state(int);
  const NState & state(int) const;
  std::vector<int> get_floating_finals() const;

  NState & add_state();
};
