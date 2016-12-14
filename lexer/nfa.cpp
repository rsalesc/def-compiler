#include "nfa.hpp"

int NFA::size() const{
  return this->states.size();
}

State NFA::state(int i = 0) const {
  return this->states[i];
}
