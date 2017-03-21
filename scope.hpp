#pragma once

#include <map>
#include <vector>
#include <string>
#include <typeinfo>
#include <memory>
#include <iostream>
#include <cassert>

using namespace std;

struct ScopeValue{
  virtual bool is_func() const { return false; }
  virtual bool is_int() const { return false; }
};

struct ScopeFunc : public ScopeValue {
  bool returns;
  int no_params;
  int no_var;
  ScopeFunc(bool returns = false, int no_params = 0, int no_var = 0)
    : returns(returns), no_params(no_params), no_var(no_var){}
  bool returns_int() const { return returns; }
  bool returns_void() const { return !returns; }
  bool compatible_with(int x) const { return x == no_params; }
  bool is_func() const override{
    return true;
  }

  int count_declarations(){ return no_var; } const
  void set_declarations(int x) { no_var = x; }
};

struct ScopeInt : public ScopeValue {
  int val;
  bool global;

  ScopeInt(bool global = false) : global(global){}

  operator int() const { return val; }
  ScopeInt & operator=(int x){ val = x; return *this; }

  int offset() const { return int(*this); }

  bool is_global() const { return global; }
  bool is_int() const override{
    return true;
  }
};

struct Scope{
  bool inside_int;
  void * inside_loop;

  map<string, shared_ptr<ScopeInt>> table_int;
  map<string, shared_ptr<ScopeFunc>> table_func;
  Scope(){
    inside_int = false;
    inside_loop = 0;
  }

  bool check_int(string s) { return table_int.count(s); }
  bool check_func(string s) { return table_func.count(s); }
  shared_ptr<ScopeInt> & get_int(string s) { return table_int[s]; }
  shared_ptr<ScopeFunc> & get_func(string s) { return table_func[s]; }
};

struct ScopeStack{
  int loop_cnt = 0, if_cnt = 0;
  vector<Scope> st;

  shared_ptr<ScopeInt> & _declare_int(string var){
    if(st.back().check_int(var))
      throw runtime_error("variable " + var + " was declared before");

    return st.back().get_int(var);
  }
  shared_ptr<ScopeFunc> & _declare_func(string var){
    if(st.back().check_func(var))
      throw runtime_error("function " + var + " was declared before");

    return st.back().get_func(var);
  }
  ScopeFunc & declare_func(string var, bool returns_int = false, int no_params = 0, int no_var = 0){
    return *(this->_declare_func(var) = make_shared<ScopeFunc>(returns_int, no_params, no_var));
  }

  ScopeInt & declare_int(string var, bool is_global = false){
    return *(this->_declare_int(var) = make_shared<ScopeInt>(is_global));
  }

  shared_ptr<ScopeInt> _get_int(string var){
    for(int i = (int)st.size()-1; i >= 0; i--){
      if(st[i].check_int(var))
        return st[i].get_int(var);
    }

    throw runtime_error("variable " + var + " not declared in this scope");
  }

  shared_ptr<ScopeFunc> _get_func(string var){
    for(int i = (int)st.size()-1; i >= 0; i--){
      if(st[i].check_func(var))
        return st[i].get_func(var);
    }

    throw runtime_error("function " + var + " not declared in this scope");
  }
  ScopeFunc & get_func(string var){
    shared_ptr<ScopeFunc> res = 0;
    try{
      res = this->_get_func(var);
    } catch(std::exception & e){
      throw runtime_error("name " + var + " not declared in this scope");
    }

    if(res == 0)
      throw runtime_error("name " + var + " was declared before but is not a function");
    return *res;
  }

  ScopeInt & get_int(string var){
    shared_ptr<ScopeInt> res = 0;
    try{
      res = this->_get_int(var);
    } catch(std::exception & e){
      throw runtime_error("name " + var + " not declared in this scope");
    }
    if(res == 0)
      throw runtime_error("name " + var + " was declared before but is not an int");
    return *res;

  }

  void push(){
    st.emplace_back();
  }

  void push_int(){
    push();
    st.back().inside_int = true;
  }

  void push_void(){
    push();
  }

  int push_if(){
    push();
    if_cnt++;
    return if_cnt;
  }

  int push_else(){
    push();
    return if_cnt;
  }

  int push_loop(void * no){
    push();
    loop_cnt++;
    st.back().inside_loop = no;
    return loop_cnt;
  }

  void pop(){
    st.pop_back();
  }

  void * last_loop() const {
    for(int i = (int)st.size()-1; i >= 0; i--)
      if(st[i].inside_loop)
        return st[i].inside_loop;
    return 0;
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

  void * loop_block() const {
    return st.back().inside_loop;
  }

  bool is_global() const {
    return st.size() == 1;
  }
};
