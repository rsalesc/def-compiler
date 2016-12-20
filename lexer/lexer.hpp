#pragma once

#include "regex.hpp"
#include "token.hpp"
#include "../common/stream.hpp"
#include <vector>
#include <string>

struct LexerRule {
  std::string name;
  DFA dfa;

  bool hidden = false;

  LexerRule(std::string s, std::string re);
};

class Lexer {
private:
  std::vector<LexerRule> m_rules;
public:

  void add_rule(std::string s, std::string re);
  void add_hidden_rule(std::string s, std::string re);
  std::vector<Token> run(Stream &, bool = false);
};
