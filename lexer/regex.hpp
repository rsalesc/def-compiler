#pragma once

#include "common.hpp"
#include "nfa.hpp"
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
    dfa = regex().powerset().minimized();
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
