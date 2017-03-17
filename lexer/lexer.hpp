#pragma once

#define LEXER_ERROR ((int16_t)(-1))

#include "regex.hpp"
#include "token.hpp"
#include "../common/stream.hpp"
#include <vector>
#include <string>

struct LexerRule {
  int16_t name;
  DFA dfa;

  bool hidden = false;

  LexerRule(int16_t, std::string re);
};

class Lexer {
private:
  std::vector<LexerRule> m_rules;
public:

  void add_rule(int16_t s, std::string re);
  void add_hidden_rule(int16_t, std::string re);
  std::vector<Token> run(Stream &, bool = false);
};
