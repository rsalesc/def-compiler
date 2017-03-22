#include "parser.hpp"
#include "common/stream.hpp"
#include "lexer/regex.hpp"
#include "lexer/lexer.hpp"
#include "tclap/CmdLine.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>

#define sz(x) ((int)x.size())

using namespace std;

int tokens_ptr;
int cur_ptr;
vector<Token> tokens;
Lexer lexer;

std::streambuf * get_input_buf(const char * s){
  std::ifstream * res = new std::ifstream;
  res->open(s, std::ifstream::in);
  if(!res->is_open()){
    fprintf(stderr, "input file %s could not be opened\n", s);
    exit(1);
  }
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

string escape(const string & s){
  string res;
  for(char c : s){
    res += "\\";
    res += c;
  }
  return res;
}

std::vector<std::string> escape(std::vector<std::string> s){
  for(string & x : s){
    x = escape(x);
  }

  return s;
}

void setup_output(std::string output_fn){
  if(!output_fn.empty()){
    FILE * opened = freopen(output_fn.c_str(), "w", stdout);
  }
}

void run_lexer(std::string input_fn){
  // input setup
  std::istream in(!input_fn.empty() ? get_input_buf(input_fn.c_str()) : std::cin.rdbuf());
  Stream st(in);

  // run lexer
  tokens = lexer.run(st);
  tokens_ptr = cur_ptr = 0;

  // check for lexical errors
  for(Token & tok : tokens){
    if(tok.type == LEXER_ERROR){
      fprintf(stderr, "lexical error in %d:%d\n", tok.location.first, tok.location.second);
      exit(1);
    }
  }
}

void setup_lexer(){
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
    "!"
  };

  syms = escape(syms);

  lexer.add_hidden_rule(0, "[ \n\t\r]+"); // white
  lexer.add_hidden_rule(0, "//[^\n]*"); // comment

  lexer.add_rule(T_IF, escape("if"));
  lexer.add_rule(T_BREAK, escape("break"));
  lexer.add_rule(T_CONTINUE, escape("continue"));
  lexer.add_rule(T_WHILE, escape("while"));
  lexer.add_rule(T_DEF, escape("def"));
  lexer.add_rule(T_ELSE, escape("else"));
  lexer.add_rule(T_INT, escape("int"));
  lexer.add_rule(T_VOID, escape("void"));
  lexer.add_rule(T_RETURN, escape("return"));

  lexer.add_rule(T_ID, "[a-zA-Z][a-zA-Z0-9_]*");
  lexer.add_rule(T_DEC, "[0-9]+");
  lexer.add_rule(0, unite(syms));

  lexer.add_rule(T_LEQ, escape("<="));
  lexer.add_rule(T_GEQ, escape(">="));
  lexer.add_rule(T_EQ, escape("=="));
  lexer.add_rule(T_NEQ, escape("!="));
  lexer.add_rule(T_AND, escape("&&"));
  lexer.add_rule(T_OR, escape("||"));
}

shared_ptr<ProgASTNode> do_parsing(){
  Parser parser(tokens);

  return parser.program();
}

void do_semantics(shared_ptr<ProgASTNode> root, Code & code){
  ScopeStack sta;
  root->check_and_generate(code, sta);
}

int main(int argc, char ** argv){
  /*
  * COMMAND LINE PARSING
  **/
  std::string input_fn, output_fn;
  int phase;
  bool output_data;

  TCLAP::CmdLine cmd("MATA61 Def Compiler", ' ', "2016.2");

  TCLAP::UnlabeledValueArg<std::string> input_fn_cmd("input_file",
    "specifies the input file",
    true,
    "",
    "input_file");

  TCLAP::UnlabeledValueArg<std::string> output_fn_cmd("output_file",
    "specifies an optional output file (or use stdout)",
    false,
    "",
    "output_file");

  TCLAP::ValueArg<int> phase_cmd("p",
    "phase",
    "which phases will run (0: til lex, 1: til syntactic, 2: til semantics, 3: compiles)",
    false,
    3,
    "phase");

  TCLAP::SwitchArg output_cmd("n", "no-output", "supress output data from earlier phases", true);

  cmd.add(input_fn_cmd);
  cmd.add(output_fn_cmd);
  cmd.add(phase_cmd);
  cmd.add(output_cmd);

  cmd.parse(argc, argv);

  input_fn = input_fn_cmd.getValue();
  output_fn = output_fn_cmd.getValue();
  phase = phase_cmd.getValue();
  output_data = output_cmd.getValue();

  /* Actual code */
  setup_output(output_fn);
  setup_lexer();

  run_lexer(input_fn);

  shared_ptr<ProgASTNode> root;

  if(phase >= 1){
    root = do_parsing();
  }

  if(phase >= 2){
    Code code;
    do_semantics(root, code);

    if(phase >= 3){
      code.print();
    }
  }

  if((phase == 1 || phase == 2) && output_data){
    root->print_node();
    puts("");
  }

  return 0;
}
