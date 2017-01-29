#include <bits/stdc++.h>
#pragma once

#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

#define LABEL int16_t
#define EPSILON ((LABEL)(1<<14))

struct CharRange{
  LABEL left, right;

  CharRange(LABEL l, LABEL r) : left(l), right(r){}
  CharRange(LABEL x) : CharRange(x, x) {}

  bool in_range(LABEL c) const{
    return left <= c && c <= right;
  }

  bool operator<(const CharRange & rhs) const {
    if(left == rhs.left)
      return right < rhs.right;
    return left < rhs.left;
  }
};

struct State{
  std::map<CharRange, int> t;
  bool is_final = false;

  void add_transition(CharRange range, int idx){
    t[range] = idx;
  }

  decltype(t) transitions() const {
    return t;
  }

  decltype(t)::const_iterator next_iterator(LABEL c) const {
    auto it = t.lower_bound(CharRange(c, std::numeric_limits<LABEL>::max()));
    if(it == t.cbegin())
      return t.cend();
    return --it;
  }

  int next(LABEL c) const {
    auto it = this->next_iterator(c);
    if(it == t.end() || !it->first.in_range(c))
      return -1;
    return it->second;
  }
};

struct NState{
  std::map<CharRange, std::set<int>> t;
  bool is_final = false;

  std::set<int> epsilon_transitions() const {
    if(this->t.count(EPSILON))
      return this->t.at(EPSILON);
    return std::set<int>();
  }

  void add_transition(CharRange range, int idx){
    t[range].insert(idx);
  }

  decltype(t) transitions() const {
    return t;
  }

  decltype(t)::iterator next_iterator(LABEL c){
    return t.lower_bound(CharRange(c));
  }

  std::vector<int> next(LABEL c) const{
    std::vector<int> res;

    for(const auto & p : this->transitions()){
      if(p.first.in_range(c)){
        copy(p.second.begin(), p.second.end(), back_inserter(res));
      }
    }

    return res;
  }
};

inline std::vector<CharRange> get_disjoint_ranges(std::vector<CharRange> v){
  std::vector<std::pair<LABEL, int>> vs;
  std::vector<CharRange> res;

  for(auto range : v)
    vs.push_back(std::make_pair(range.left, -1)),
    vs.push_back(std::make_pair(range.right+1, 1));

  sort(vs.begin(), vs.end());

  int acc = 0;
  for(int i = 0; i+1 < vs.size(); i++){
    acc -= vs[i].second;
    if(acc){
      if(vs[i].first < vs[i+1].first){
        res.push_back(CharRange(vs[i].first, vs[i+1].first-1));
      }
    }
  }

  return res;
}

inline std::vector<CharRange> get_united_ranges(std::vector<CharRange> v){
  sort(v.begin(), v.end());

  std::vector<CharRange> res;

  LABEL lastl = -2;
  LABEL lastr = -2;
  for(const CharRange & range : v){
    if(range.in_range(EPSILON))
      continue;

    if(range.left <= lastr+1){
      lastr = std::max(lastr, range.right);
    } else {
      if(lastl != -2)
        res.push_back({lastl, lastr});
      lastl = range.left;
      lastr = range.right;
    }
  }

  if(lastl != -2)
    res.push_back({lastl, lastr});

  return res;
}

inline std::vector<CharRange> negate_ranges(std::vector<CharRange> v){
  v = get_united_ranges(v);
  LABEL last = -1;
  std::vector<CharRange> res;

  for(const CharRange & range : v){
    if(range.in_range(EPSILON))
      continue;

    if(last+1 <= range.left-1)
      res.push_back({(LABEL)(last+1), (LABEL)(range.left-1)});
    last = range.right;
  }

  if(last < EPSILON-1)
    res.push_back({(LABEL)(last+1), (LABEL)(EPSILON-1)});
  return res;
}

inline bool get_range_intersection(CharRange a, CharRange b,
                            CharRange * res){
    if(a.right < b.left || b.right < a.left)
      return false;

    if(!(a < b))
      std::swap(a, b);

    *res = CharRange(b.left, std::min(a.right, b.right));
    return true;
  }
#pragma once

#pragma once

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
#pragma once

#include <utility>

struct Token {
  std::string type;
  std::string lexeme;
  std::pair<int, int> location;
};
#pragma once

#include <sstream>
#include <iostream>
#include <vector>

namespace RegexNFA{
  NFA kleene(const NFA &);
  NFA cat(const NFA &, const NFA &);
  NFA unite(const NFA &, const NFA &);
  NFA kleene_plus(const NFA &);
  NFA zero_one(const NFA &);
  NFA from_range(const std::vector<CharRange> &);
  NFA from_range(CharRange);
}

class Regex{
private:
  std::istringstream in;
  DFA dfa;

  int peek(){ return this->in.peek(); }
  int consume(char c) {
    if(this->peek() != c)
      throw std::runtime_error("didnt expect this char");
    return this->in.get();
  }
  int next(){ return this->consume(this->peek()); }
  bool more(){ return !this->in.eof(); }

  CharRange char_range(){
    char first = this->next();
    char second = first;
    if(this->peek() == '-'){
      this->consume('-');
      second = this->next();
    }

    return CharRange(first, second);
  }

  NFA range(){
    this->consume('[');
    bool invert = false;
    if(this->peek() == '^'){
      this->consume('^');
      invert = true;
    }

    std::vector<CharRange> v;

    while(this->peek() != ']'){
      v.push_back(this->char_range());
    }

    this->consume(']');

    if(invert)
      v = negate_ranges(v);

    return RegexNFA::from_range(v);
  }

  NFA base(){
    NFA res;
    if(this->peek() == '\\'){
      this->consume('\\');
      res = RegexNFA::from_range(this->next());
    } else if(this->peek() == '['){
      res = this->range();
    } else if(this->peek() == '('){
      this->consume('(');
      res = this->regex();
      this->consume(')');
    } else{
      res = RegexNFA::from_range(this->next());
    }

    return res;
  }

  NFA factor(){
    NFA res = this->base();
    while(this->more()){
      if(this->peek() == '*'){
        this->consume('*');
        res = RegexNFA::kleene(res);
      }
      else if(this->peek() == '+'){
        this->consume('+');
        res = RegexNFA::kleene_plus(res);
      }
      else if(this->peek() == '?'){
        this->consume('?');
        res = RegexNFA::zero_one(res);
      }
      else
        break;
    }
    return res;
  }

  NFA term(){
    NFA res = NFA::empty_string();
    while(this->more() && this->peek() != ')' && this->peek() != '|'){
      res = RegexNFA::cat(res, this->factor());
    }

    return res;
  }

  NFA regex(){
    NFA res = this->term();
    if(this->more() && this->peek() == '|'){
      this->consume('|');
      return RegexNFA::unite(res, this->regex());
    } else
      return res;
  }

  void build(){
    dfa = regex().powerset();
    // dfa.dump();
  }
public:
  Regex(std::string s) : in(s) {
    this->build();
  }

  bool run(const std::string & s) const {
    return dfa.run(s);
  }

  DFA get_dfa() const {
    return this->dfa;
  }
};
#include <exception>

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
      int ni = res.add_state();
      NState & ns = res.state(ni);

      ns.is_final = s.is_final;
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
    res.add_state();
    for(const NState & s : a.states()){
      int ni = res.add_state();
      NState & ns = res.state(ni);

      ns.is_final = s.is_final;

      for(auto & p : s.transitions()){
        for(int x : p.second){
          ns.add_transition(p.first, x + 1);
        }
      }
    }

    for(const NState & s : b.states()){
      int ni = res.add_state();
      NState & ns = res.state(ni);

      ns.is_final = s.is_final;

      for(const auto & p : s.transitions()){
        for(int x : p.second){
          ns.add_transition(p.first, x + a.size() + 1);
        }
      }
    }

    res.state(0).add_transition(EPSILON, 1);
    res.state(0).add_transition(EPSILON, a.size() + 1);
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
    res.add_state();

    for(int i = 0; i < v.size(); i++){
      res.state(0).add_transition(v[i], 1);
    }

    res.add_state();
    res.state(1).is_final = true;

    return res;
  }

  NFA from_range(CharRange range){
    std::vector<CharRange> v;
    v.push_back(range);
    return from_range(v);
  }

}

using namespace std;

string unpoint(const string & x){
  string res;
  for(char c : x) if(c != '.') res += c;
  return res;
}

int main(){
  ios::sync_with_stdio(false);
  cin.tie(0);

  string R;
  while(cin >> R){
    Regex re(unpoint(R));
    int P;
    cin >> P;

    getline(cin, R);
    while(P--){
      getline(cin, R);

      cout << (re.run(R) ? "Y" : "N") << endl;
    }

    cout << endl;
  }
}
