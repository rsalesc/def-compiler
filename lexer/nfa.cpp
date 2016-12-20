#include "nfa.hpp"
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <iostream>

int NFA::size() const{
  return this->m_states.size();
}

const std::vector<NState> & NFA::states() const{
  return this->m_states;
}

std::vector<NState> & NFA::states(){
  return this->m_states;
}

NState & NFA::state(int i = 0) {
  return this->m_states[i];
}

const NState & NFA::state(int i = 0) const{
  return this->m_states[i];
}

std::vector<int> NFA::get_floating_finals() const{
  std::vector<int> res;
  for(int i = 0; i < this->size(); i++){
    if(this->state(i).is_final && this->state(i).transitions().empty())
      res.push_back(i);
  }
  return res;
}

int NFA::add_state(){
  this->m_states.emplace_back();
  return this->size()-1;
}

NFA NFA::epsilon_closure() const{
  NFA res;
  for(int i = 0; i < this->size(); i++){
    int nw = res.add_state();
    res.state(nw).is_final = this->state(i).is_final;
  }

  for(int i = 0; i < this->size(); i++){
    std::set<int> vis;
    std::queue<int> q;

    q.push(i);
    vis.insert(i);

    while(!q.empty()){
      int cur = q.front();
      q.pop();

      const NState & st = this->state(cur);
      for(const int x : st.epsilon_transitions()){
        if(!vis.count(x)){
          vis.insert(x);
          q.push(x);
        }
      }
    }

    for(int x : vis){
      const NState & st = this->state(x);
      res.state(i).is_final |= st.is_final;

      for(const auto & p : st.transitions()){
        if(p.first.in_range(EPSILON))
          continue;

        for(int y : p.second)
          res.state(i).add_transition(p.first, y);
      }
    }
  }

  return res;
}

DFA NFA::powerset() const {

  // get epsilon closure
  NFA closure = this->epsilon_closure();

  // compute the alphabet
  std::vector<CharRange> input_symbols;
  for(const NState & s : closure.states()){
    for(const auto & p : s.transitions()){
      input_symbols.push_back(p.first);
    }
  }

  input_symbols = get_disjoint_ranges(input_symbols);

  // force transitions to use the alphabet
  NFA dis;
  for(int i = 0; i < closure.size(); i++){
    int nw = dis.add_state();
    dis.state(i).is_final = closure.state(i).is_final;
  }

  for(int i = 0; i < closure.size(); i++){
    for(const auto & p : closure.state(i).transitions()){
      int st = lower_bound(input_symbols.begin(), input_symbols.end(),
                          CharRange(p.first.left)) - input_symbols.begin();

      for(int j = st; j < input_symbols.size(); j++){
        if(input_symbols[j].left > p.first.right)
          break;

        std::set<int> & st = dis.state(i).t[input_symbols[j]];
        st.insert(p.second.begin(), p.second.end());
      }
    }
  }

  // build DFA
  DFA res;

  std::queue<std::vector<int>> q;
  std::map<std::vector<int>, int> vis;

  std::vector<int> initial;
  initial.push_back(0);

  q.push(initial);
  vis[initial] = res.add_state();

  while(!q.empty()){
    const std::vector<int> & cur = q.front();
    int cur_idx = vis[cur];

    std::map<CharRange, std::vector<int>> t;

    for(int ni : cur){
      const NState & ns = dis.state(ni);
      res.state(cur_idx).is_final |= ns.is_final;

      for(const auto & p : ns.transitions()){
        std::vector<int> & v = t[p.first];
        copy(p.second.begin(), p.second.end(), back_inserter(v));
      }
    }

    for(auto & p : t){
      p.second.resize(unique(p.second.begin(), p.second.end()) - p.second.begin());

      int to;
      if(!vis.count(p.second)){
          to = vis[p.second] = res.add_state();
          q.push(p.second);
      } else to = vis[p.second];

      res.state(cur_idx).add_transition(p.first, to);
    }

    q.pop();
  }

  return res;
}

bool NFA::run(const std::string & s) const{
  std::queue<std::pair<int, int>> q;
  std::set<std::pair<int, int>> vis;

  vis.insert({0, 0});
  q.push({0, 0});

  while(!q.empty()){
    std::pair<int, int> p = q.front();
    q.pop();

    const NState & st = this->state(p.first);
    if(p.second == s.size()){
      if(st.is_final)
        return true;
    }

    if(p.second > s.size())
      break;

    std::vector<int> t = st.next(s[p.second]);
    std::vector<int> eps = st.next(EPSILON);

    for(int x : t){
      if(!vis.count({x, p.second+1})){
        vis.insert({x, p.second+1});
        q.push({x, p.second+1});
      }
    }

    for(int x : eps){
      if(!vis.count({x, p.second})){
        vis.insert({x, p.second});
        q.push({x, p.second});
      }
    }

  }

  return false;
}
