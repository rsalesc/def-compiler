#include "nfa.hpp"

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

NState & NFA::add_state(){
  this->m_states.emplace_back();
  return this->m_states.back();
}
