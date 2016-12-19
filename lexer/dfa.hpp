#pragma once

#include "common.hpp"
#include "nfa.hpp"
#include <iostream>

class NFA;
class DFA {
private:
  std::vector<State> m_states;
  int m_cur = 0;

public:
  int size() const;

  const std::vector<State> & states() const;
  std::vector<State> & states();

  const State & state(int i) const;
  State & state(int i);

  int add_state();

  void dump() const{
    std::cerr<< this->size() << std::endl;
    for(int i = 0; i < this->size(); i++){
      for(auto & p : this->state(i).transitions()){
        std::cerr << "[" << p.first.left << "-" << p.first.right
        << "] " << i << " to ";
        std::cerr << p.second << std::endl;
      }
    }

    for(int i = 0; i < this->size(); i++){
      if(this->state(i).is_final)
        std::cerr << i << ", ";
    }
    std::cerr << std::endl;
  }

  bool run(std::string s) const;
  void reset();
  int step(char c);

  NFA reversal() const;
  DFA minimized() const;
};
