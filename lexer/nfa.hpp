#pragma once

#include "common.hpp"
#include "dfa.hpp"
#include <vector>
#include <set>
#include <map>
#include <iostream>

class DFA;
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

  int add_state();

  NFA epsilon_closure() const;
  DFA powerset() const;

  static NFA empty_string(){
    NFA res;
    res.add_state();
    res.state(0).is_final = true;
    return res;
  }

  bool run(const std::string &) const;
  void dump() const{
    std::cerr<< this->size() << std::endl;
    for(int i = 0; i < this->size(); i++){
      for(auto & p : this->state(i).transitions()){
        std::cerr << "[" << p.first.left << "-" << p.first.right
        << "] " << i << " to ";
        for(int x : p.second)
          std::cerr << x << ",";
        std::cout << std::endl;
      }
    }

    for(int i = 0; i < this->size(); i++){
      if(this->state(i).is_final)
        std::cerr << i << ", ";
    }
    std::cerr << std::endl;
  }
};
