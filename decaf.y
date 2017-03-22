%{
#include "semantics.cpp"
#include <memory>
#define YYSTYPE shared_ptr<ASTNode>
#define CAST(x) (dynamic_pointer_cast<ASTNode>(x))

int yylex (void);
void yyerror (char const *);

typedef shared_ptr<ASTNode> PASTNode;
shared_ptr<ASTNode> root;

/* AST Helpers */
PASTNode create_binop(PASTNode a, PASTNode b, PASTNode op){
  return make_shared<BinASTNode>(a, b, op->get_text());
}

PASTNode create_unop(PASTNode a, PASTNode op){
  return make_shared<UnASTNode>(a, op->get_text());
}

PASTNode merge_blocks(PASTNode a, PASTNode b){
  shared_ptr<BlockASTNode> blocka = dynamic_pointer_cast<BlockASTNode>(a);
  shared_ptr<BlockASTNode> blockb = dynamic_pointer_cast<BlockASTNode>(b);
  shared_ptr<BlockASTNode> res = make_shared<BlockASTNode>();

  for(const auto & p : blocka->declarations)
    res->append_declaration(p);
  for(const auto & p : blockb->declarations)
    res->append_declaration(p);
  for(const auto & p : blocka->statements)
    res->append_statement(p);
  for(const auto & p : blockb->statements)
    res->append_statement(p);
  return res;
}

/**/

%}

%define parse.error verbose
%locations

%token T_ID T_DEC
%token T_IF T_BREAK T_CONTINUE T_WHILE T_DEF T_ELSE T_INT T_VOID T_RETURN
%token T_LEQ T_GEQ T_EQ T_NEQ T_AND T_OR

%left T_OR
%left T_AND
%left T_EQ T_NEQ
%left '<' '>' T_LEQ T_GEQ
%left '+' '-'
%left '*' '/'
%precedence '!'
%precedence UNARY_MINUS

%%
/* botar decfunc*/
root: program   { $$ = $1; root = $$; }
    ;

program: decvar program { $$ = $2;
                          dynamic_pointer_cast<ProgASTNode>($$)->append($1);}
       | decfunc program { $$ = $2;
                          dynamic_pointer_cast<ProgASTNode>($$)->append($1);}
       | %empty { $$ = make_shared<ProgASTNode>(); }
       ;

decvar: type T_ID ';' {
      shared_ptr<ASTNode> var = make_shared<VarASTNode>($2, $1);
      $$ = make_shared<DecvarASTNode>(var);
      }
      | type T_ID '=' expr ';'{
      shared_ptr<ASTNode> var = make_shared<VarASTNode>($2, $1);
      $$ = make_shared<DecvarASTNode>(var, $4);
      }
      ;

decfunc: T_DEF type T_ID '(' paramlist ')' block {
        shared_ptr<ASTNode> var = make_shared<VarASTNode>($3, $2);
        $$ = make_shared<DecfuncASTNode>(var, $5, $7);
      }
      | T_DEF type T_ID '(' ')' block {
        shared_ptr<ASTNode> var = make_shared<VarASTNode>($3, $2);
        shared_ptr<ASTNode> params = make_shared<ParamsASTNode>();
        $$ = make_shared<DecfuncASTNode>(var, params, $6);
      }
      ;

declarations: declarations decvar { $$ = $1;
            dynamic_pointer_cast<BlockASTNode>($$)->append_declaration($2); }
            | %empty { $$ = make_shared<BlockASTNode>(); }
            ;

statements: statements stmt { $$ = $1;
            dynamic_pointer_cast<BlockASTNode>($$)->append_statement($2); }
          | %empty { $$ = make_shared<BlockASTNode>(); }
          ;

block: '{' declarations statements '}' { $$ = merge_blocks($2, $3); }
     ;

stmt: T_IF '(' expr ')' block { $$ = make_shared<IfASTNode>($3, $5); }
    | T_IF '(' expr ')' block T_ELSE block { $$ = make_shared<IfASTNode>($3, $5, $7); }
    | T_WHILE '(' expr ')' block { $$ = make_shared<WhileASTNode>($3, $5); }
    | T_RETURN ';' { $$ = make_shared<ReturnASTNode>(); }
    | T_RETURN expr ';' { $$ = make_shared<ReturnASTNode>($2); }
    | T_BREAK ';' { $$ = make_shared<BreakASTNode>(); }
    | T_CONTINUE ';' { $$ = make_shared<ContinueASTNode>(); }
    | assign ';' { $$ = $1; }
    | funccall ';' { $$ = $1; }
    ;

assign: T_ID '=' expr { $$ = make_shared<AssignASTNode>($1, $3); }
      ;

funccall: T_ID '(' arglist ')' { $$ = make_shared<CallASTNode>($1, dynamic_pointer_cast<ArgsASTNode>($3));}
        | T_ID '(' ')' { $$ = make_shared<CallASTNode>($1); }
        ;

arglist: expr { $$ = make_shared<ArgsASTNode>();
                dynamic_pointer_cast<ArgsASTNode>($$)->append($1);
              }
       | arglist ',' expr { $$ = $1;
                            dynamic_pointer_cast<ArgsASTNode>($$)->append($3);
                          }
       ;

paramlist: type T_ID { $$ = make_shared<ParamsASTNode>();
         shared_ptr<VarASTNode> var = make_shared<VarASTNode>($2, $1);
         dynamic_pointer_cast<ParamsASTNode>($$)->append(static_pointer_cast<ASTNode>(var)); }
         | paramlist ',' type T_ID {
            $$ = $1;
            shared_ptr<VarASTNode> var = make_shared<VarASTNode>($4, $3);
            dynamic_pointer_cast<ParamsASTNode>($$)->append(static_pointer_cast<ASTNode>(var));
         }
         ;

expr: expr '+' expr { $$ = create_binop($1, $3, $2); }
    | expr '-' expr { $$ = create_binop($1, $3, $2); }
    | expr '*' expr { $$ = create_binop($1, $3, $2); }
    | expr '/' expr { $$ = create_binop($1, $3, $2); }
    | expr T_EQ expr { $$ = create_binop($1, $3, $2); }
    | expr T_NEQ expr { $$ = create_binop($1, $3, $2); }
    | expr T_LEQ expr { $$ = create_binop($1, $3, $2); }
    | expr T_GEQ expr { $$ = create_binop($1, $3, $2); }
    | expr '<' expr { $$ = create_binop($1, $3, $2); }
    | expr '>' expr { $$ = create_binop($1, $3, $2); }
    | expr T_AND expr { $$ = create_binop($1, $3, $2); }
    | expr T_OR expr { $$ = create_binop($1, $3, $2); }
    | '!' expr { $$ = create_unop($2, $1); }
    | '-' expr %prec UNARY_MINUS { $$ = create_unop($2, $1); }
    | '(' expr ')' { $$ = $2; }
    | funccall { $$ = $1; }
    | T_DEC { $$ = make_shared<DecASTNode>($1); }
    | T_ID { $$ = make_shared<IdASTNode>($1); }
    ;

type: T_INT { $$ = make_shared<TypeASTNode>($1); }
    | T_VOID { $$ = make_shared<TypeASTNode>($1); }
%%

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

int yylex(){
  if(tokens_ptr < sz(tokens) && cur_ptr == sz(tokens[tokens_ptr].lexeme)){
    tokens_ptr++;
    cur_ptr = 0;
  }
  if(tokens_ptr == sz(tokens))
    return 0;

  // clear yyval here in any case

  if(tokens[tokens_ptr].type == LEXER_ERROR){
    const Token & tok = tokens[tokens_ptr];
    fprintf(stderr, "lexical error on %d:%d\n",
        tok.location.first, tok.location.second);
    exit(1);
  } else if(!tokens[tokens_ptr].type){
    yylval = make_shared<ASTNode>(string(1, tokens[tokens_ptr].lexeme[cur_ptr]));
    return tokens[tokens_ptr].lexeme[cur_ptr++];
  } else {
    cur_ptr = 0;
    // set yyval here if needed
    yylval = make_shared<ASTNode>(tokens[tokens_ptr].lexeme);
    return tokens[tokens_ptr++].type;
  }
}
void yyerror(const char * s){
  fprintf(stderr, "%s\n", s);
  exit(1);
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
  setup_lexer();
  setup_output(output_fn);

  run_lexer(input_fn);

  if(phase >= 1){
    yyparse();
  }

  if(phase >= 2){
    do_semantics(root, phase >= 3);
  }

  if((phase == 1 || phase == 2) && output_data){
    root->print_node();
    puts("");
  }

  return 0;
}
