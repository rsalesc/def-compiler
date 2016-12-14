#include <map>
#include <vector>
#include <algorithm>
#pragma once

#define LABEL int16_t

struct CharRange{
  LABEL left, right;

  CharRange(LABEL l, LABEL r) : left(l), right(r){}
  CharRange(LABEL x) : CharRange(x, x) {}

  bool operator<(const CharRange & rhs) const {
    if(left == rhs.left)
      return right < rhs.right;
    return left < rhs.left;
  }
};

struct State{
  std::map<CharRange, int> t;

  /*
    Add with no assertion a deterministic transition
  */
  void add_transition(CharRange range, int idx){
    t[range] = idx;
  }

  decltype(t)::iterator next_iterator(LABEL c){
    return t.lower_bound(CharRange(c));
  }

  int next(LABEL c){
    auto it = this->next_iterator(c);
    if(it == t.end())
      return -1;
    return it->second;
  }
};

struct NState{
  std::vector<std::pair<CharRange, int>> t;

  void add_transition(CharRange range, int idx){
    t.push_back(std::make_pair(range, idx));
  }

  std::vector<int> next(LABEL c){
    std::vector<int> res;
    for(auto & p : t){
      if(p.first.left <= c && c <= p.first.right){
        res.push_back(p.second);
      }
    }

    sort(res.begin(), res.end());
    res.resize(unique(res.begin(), res.end()) - res.begin());

    return res;
  }
};

std::vector<CharRange> get_disjoint_ranges(std::vector<CharRange> v){
  sort(v.begin(), v.end());
  reverse(v.begin(), v.end());
  std::vector<CharRange> res;

  while(v.size() > 1){
    CharRange cur = v.back();
    v.pop_back();

    LABEL new_right = std::min(cur.right, static_cast<LABEL>(v.back().left-1));
    if(new_right >= cur.left)
      res.push_back(CharRange(cur.left, new_right));
    v.back().right = std::max(v.back().right, cur.right);
  }

  if(!v.empty())
    res.push_back(v.back());

  return res;
}

bool get_range_intersection(const CharRange & a, const CharRange & b,
                            CharRange * res){
    if(a.right < b.left || b.right < a.left)
      return false;

    if(!(a < b))
      std::swap(a, b);

    *res = CharRange(b.left, std::min(a.right, b.right));
    return true;
  }
