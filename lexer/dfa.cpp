#include "dfa.hpp"

int DFA::size() const{
  return this->m_states.size();
}

const std::vector<State> & DFA::states() const{
  return this->m_states;
}

std::vector<State> & DFA::states(){
  return this->m_states;
}

const State & DFA::state(int i = 0) const{
  return this->m_states[i];
}

State & DFA::state(int i = 0){
  return this->m_states[i];
}

int DFA::add_state(){
  this->m_states.emplace_back();
  return this->size()-1;
}

bool DFA::run(std::string s) const{
  int cur = 0;
  for(char c : s){
    int nxt = this->state(cur).next(c);
    if(nxt == -1)
      return false;
    cur = nxt;
  }

  return this->state(cur).is_final;
}

void DFA::reset(){
  this->m_cur = 0;
}

int DFA::step(char c){
  if(this->m_cur == -1)
    return -1;

  int nxt = this->state(this->m_cur).next(c);
  this->m_cur = nxt;
  if(nxt == -1)
    return -1;

  if(this->state(nxt).is_final)
    return 1;

  return 0;
}

NFA DFA::reversal() const {
  NFA res;
  res.add_state();

  for(int i = 0; i < this->size(); i++)
    res.add_state();

  res.state(1).is_final = true;

  for(int i = 0; i < this->size(); i++){
    const State & ns = this->state(i);
    if(ns.is_final)
      res.state(0).add_transition(EPSILON, i+1);

    for(const auto & p : ns.transitions()){
      res.state(p.second+1).add_transition(p.first, i+1);
    }
  }

  return res;
}

DFA DFA::minimized() const {
  return this->reversal().powerset().reversal().powerset();
}
