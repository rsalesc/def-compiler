#include "lexer.hpp"

LexerRule::LexerRule(std::string s, std::string re){
  this->name = s;
  this->dfa = Regex(re).get_dfa();
}

void Lexer::add_rule(std::string s, std::string re){
    this->m_rules.emplace_back(s, re);
}

void Lexer::add_hidden_rule(std::string s, std::string re){
    this->m_rules.emplace_back(s, re);
    this->m_rules.back().hidden = true;
}

std::vector<Token> Lexer::run(Stream & s, bool show_hidden){
  int sz = this->m_rules.size();
  int consumed;
  std::vector<int> munch(sz);
  std::vector<Token> res;

  while(s.peek() != EOF){
    consumed = 0;
    fill(munch.begin(), munch.end(), 0);

    for(LexerRule & rule : this->m_rules){
      rule.dfa.reset();
    }

    std::string str;
    bool works = true;
    std::pair<int, int> location;

    while(works && s.peek() != EOF){
      works = false;
      if(!consumed)
        location = s.location();

      consumed++;
      char c = s.get();
      str += c;

      for(int i = 0; i < sz; i++){
        LexerRule & rule = this->m_rules[i];
        int sig = rule.dfa.step(c);
        works |= sig >= 0;

        if(sig > 0)
          munch[i] = consumed;
      }
    }

    int maxrule = max_element(munch.begin(), munch.end()) - munch.begin();
    int maxmunch = munch[maxrule];

    // std::cerr << "consumed [" << str << "]" << std::endl;

    if(maxmunch == 0){
      res.push_back({"ERROR", str.substr(0,1), location});
    }

    for(int i = 0; i < consumed - std::max(maxmunch, 1); i++){
      s.unget();
      str.pop_back();
    }

    if(maxmunch > 0 && (show_hidden || !this->m_rules[maxrule].hidden))
      res.push_back({this->m_rules[maxrule].name, str, location});
  }

  return res;
}
