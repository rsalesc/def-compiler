#include "regex.hpp"
#include "lexer.hpp"
#include <string>
#include <iostream>

using namespace std;

std::string unite(std::vector<std::string> s){
  std::string res;
  for(std::string x : s){
    res += x;
    res += '|';
  }

  if(!s.empty())
    res.pop_back();

  return res;
}

std::vector<std::string> escape(std::vector<std::string> s){
  for(string & x : s){
    string nw;
    for(char c : x){
      nw += "\\";
      nw += c;
    }

    x = nw;
  }

  return s;
}

int main(){
  std::vector<std::string> keys = {
    "if",
    "break",
    "continue",
    "while",
    "def",
    "else",
    "int",
    "void",
    "return"
  };

  std::vector<std::string> syms = {
    "(",
    "{",
    "[",
    "]",
    "}",
    ")",
    ",",
    ";",
    "=",
    "+",
    "-",
    "*",
    "/",
    "<",
    ">",
    "<=",
    ">=",
    "==",
    "!=",
    "&&",
    "||",
    "!"
  };

  keys = escape(keys);
  syms = escape(syms);

  Lexer lexer;
  lexer.add_hidden_rule("WHITE", "[ \n\t\r]+");
  lexer.add_hidden_rule("COMMENT", "//[^\n]*");
  lexer.add_rule("KEY", unite(keys));
  lexer.add_rule("ID", "[a-zA-Z][a-zA-Z0-9_]*");
  lexer.add_rule("DEC", "[1-9][0-9]*|0"); // botar - aqui ou é operador?
  lexer.add_rule("SYM", unite(syms));

  for(Lexeme lex : lexer.run(std::cin)){
    std::cout << lex.token << " \"" << lex.str << "\"" << std::endl;
  }
}
