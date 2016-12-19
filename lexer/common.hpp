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
