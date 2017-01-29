#include "regex.hpp"
#include "lexer.hpp"
#include "common/stream.hpp"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

std::streambuf * get_input_buf(const char * s){
  std::ifstream * res = new std::ifstream;
  res->open(s, std::ifstream::in);
  return res->rdbuf();
}

std::streambuf * get_output_buf(const char * s){
  std::ofstream * res = new std::ofstream;
  res->open(s, std::ofstream::out);
  return res->rdbuf();
}


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

int main(int argc, char * argv[]){
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
  lexer.add_rule("DEC", "[0-9]+"); // botar - aqui ou é operador?
  lexer.add_rule("SYM", unite(syms));

  std::istream in(argc > 1 ? get_input_buf(argv[1]) : std::cin.rdbuf());
  std::ostream out(argc > 2 ? get_output_buf(argv[2]) : std::cout.rdbuf());
  std::ostream err(argc > 3 ? get_output_buf(argv[3]) : std::cerr.rdbuf());

  Stream s(in);
  std::vector<Token> tokens = lexer.run(s);

  for(Token tok : tokens){
    if(tok.type != "ERROR")
      out << tok.type << "\t\"" << tok.lexeme << "\"" << std::endl;
  }

  if(!tokens.empty() && tokens.back().type == "ERROR"){
    const Token & error = tokens.back();
    err << "Found error on " << error.location.first << ":" <<
      error.location.second << ", string:\t" << error.lexeme << std::endl;
    return 1;
  }

  return 0;
}
