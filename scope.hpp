#pragma once

#include <map>
#include <vector>
#include <string>
using namespace std;

struct Scope{
  bool inside_loop, inside_int;
  map<string, int32_t> table;

  Scope(){
    inside_loop = inside_int = false;
  }

  bool check(string s) { return table.count(s); }
  int32_t & get (string s) { return table[s]; }
};

struct ScopeStack{
  vector<Scope> st;

  int & declare(string var){
    if(st.back().check(var))
      throw runtime_error("variable " + var + " was declared before");

    return st.back().get(var);
  }

  int & get(string var){
    for(int i = (int)st.size()-1; i >= 0; i--){
      if(st[i].check(var))
        return st[i].get(var);
    }

    throw runtime_error("variable " + var + " not declared in this scope");
  }

  void push(){
    st.emplace_back();
  }

  void push_int(){
    push();
    st.back().inside_int = true;
  }

  void push_loop(){
    push();
    st.back().inside_loop = true;
  }

  void pop(){
    st.pop_back();
  }

  bool is_int() const {
    for(const auto & p : st)
      if(p.inside_int)
        return true;
    return false;
  }

  bool is_loop() const {
    for(const auto & p : st){
      if(p.inside_loop)
        return true;
    }

    return false;
  }
};
