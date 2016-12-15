#include "regex.hpp"

namespace RegexNFA{

  NFA kleene(const NFA & a){
    NFA res = a;
    for(int i = 0; i < a.size(); i++){
      if(a.state(i).is_final){
        res.state(i).add_transition(EPSILON, 0);
        res.state(i).is_final = false;
      }
    }

    res.state(0).is_final = true;
    return res;
  }

  NFA cat(const NFA & a, const NFA & b){
    NFA res = a;
    int binit = res.size();

    for(const NState & s : b.states()){
      NState & ns = res.add_state();
      for(const auto & p : s.transitions()){
        for(int x : p.second){
          ns.add_transition(p.first, x + binit);
        }
      }
    }

    for(int i = 0; i < binit; i++){
      if(res.state(i).is_final){
        res.state(i).is_final = false;
        res.state(i).add_transition(EPSILON, binit);
      }
    }

    return res;
  }

  NFA unite(const NFA & a, const NFA & b){
    NFA res;
    NState & ini = res.add_state();
    for(const NState & s : a.states()){
      NState & ns = res.add_state();

      for(auto & p : s.transitions()){
        for(int x : p.second){
          ns.add_transition(p.first, x + 1);
        }
      }
    }

    for(const NState & s : b.states()){
      NState & ns = res.add_state();

      for(const auto & p : s.transitions()){
        for(int x : p.second){
          ns.add_transition(p.first, x + a.size() + 1);
        }
      }
    }

    ini.add_transition(EPSILON, 1);
    ini.add_transition(EPSILON, a.size() + 1);
    return res;
  }

  NFA kleene_plus(const NFA & a){
    NFA res = a;
    for(int i = 0; i < res.size(); i++){
      if(res.state(i).is_final){
        res.state(i).add_transition(EPSILON, 0);
      }
    }

    return res;
  }

  NFA zero_one(const NFA & a){
    NFA res = a;
    res.state(0).is_final = true;
    return res;
  }

  NFA from_range(const std::vector<CharRange> & v){
    NFA res;
    NState & ini = res.add_state();
    for(int i = 0; i < v.size(); i++){
      res.add_state().is_final = true;
      ini.add_transition(v[i], i+1);
    }
    return res;
  }

  NFA from_range(CharRange range){
    std::vector<CharRange> v;
    v.push_back(range);
    return from_range(v);
  }

}

namespace Regex{
  NFA nfa_from_regex(std::string regex){
    return NFA();
  }
}
