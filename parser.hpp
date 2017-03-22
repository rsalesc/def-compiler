#pragma once

#include <map>
#include <vector>
#include <string>
#include <cstdarg>
#include <cassert>
#include "lexer/token.hpp"
#include "ast.hpp"

#define T_ID 258
#define T_DEC 259
#define T_IF 260
#define T_BREAK 261
#define T_CONTINUE 262
#define T_WHILE 263
#define T_DEF 264
#define T_ELSE 265
#define T_INT 266
#define T_VOID 267
#define T_RETURN 268
#define T_LEQ 269
#define T_GEQ 270
#define T_EQ 271
#define T_NEQ 272
#define T_AND 273
#define T_OR 274

const int BUF_SZ = (1<<16);

namespace Parsing{
  extern std::map<int, std::string> types;
  extern char buf[BUF_SZ];
}

struct Parser{
  std::vector<Token> tok;
  int ptr;

  void define_types(){
    #define TOKEN(x) Parsing::types[x] = std::string(#x);

    if(!Parsing::types.empty()) return;

    TOKEN(EOF);
    TOKEN(T_ID);
    TOKEN(T_DEC);
    TOKEN(T_IF);
    TOKEN(T_BREAK);
    TOKEN(T_CONTINUE);
    TOKEN(T_WHILE);
    TOKEN(T_DEF);
    TOKEN(T_ELSE);
    TOKEN(T_INT);
    TOKEN(T_VOID);
    TOKEN(T_RETURN);
    TOKEN(T_LEQ);
    TOKEN(T_GEQ);
    TOKEN(T_EQ);
    TOKEN(T_NEQ);
    TOKEN(T_AND);
    TOKEN(T_OR);
  }

  std::string get_type(int x){
    return !Parsing::types.count(x) ?
      std::string("\'") + std::string(1, (char)x) + std::string("\'")
      : Parsing::types[x];
  }

  std::runtime_error syntax_error(std::pair<int, int> loc, const char * fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    vsprintf(Parsing::buf, fmt, arg);
    va_end(arg);

    std::string prefix(Parsing::buf);

    sprintf(Parsing::buf, " on position %d:%d", loc.first, loc.second);
    std::string suffix(Parsing::buf);

    return std::runtime_error(prefix + suffix);
  }

  std::runtime_error syntax_error(const char * fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    vsprintf(Parsing::buf, fmt, arg);
    va_end(arg);

    std::string prefix(Parsing::buf);

    return std::runtime_error(prefix);
  }

  /**
  * END OF HELPERS
  */

  Parser(const std::vector<Token> & tok) : tok(tok) { define_types(); ptr = 0; }

  int token_val(const Token & tok) const {
    return !tok.type ? tok.lexeme[0] : tok.type;
  }

  const char * lex(const Token & tok) const {
    return tok.type == 0 ? "" : tok.lexeme.c_str();
  }

  std::pair<int, int> loc() const {
    if(ptr >= (int)tok.size())
      return tok.empty() ? std::make_pair(0, 0) : tok[ptr-1].location;
    return tok[ptr].location;
  }

  int peek() const {
    return ptr < (int)tok.size() ? token_val(tok[ptr]) : EOF;
  }

  int consume() {
    if(peek() == EOF)
      return EOF;
    return token_val(tok[ptr++]);
  }

  Token consume_token(){
    assert(peek() != EOF);
    return tok[ptr++];
  }

  template<typename T>
  shared_ptr<T> consume_node(){
    return make_shared<T>(consume_token().lexeme);
  }

  void expect(int x){
    if(peek() != x)
      throw syntax_error(loc(),
        "expected %s, found %s %s", get_type(x).c_str(),
        get_type(peek()).c_str(), ptr < (int)tok.size() ? lex(tok[ptr]) : "");
  }

  int consume(int x){
    expect(x);
    return token_val(tok[ptr++]);
  }

  void unconsume(){
    assert(ptr > 0);
    ptr--;
  }

  void unexpected(){
    throw syntax_error(loc(),
      "unexpected %s %s",
      get_type(peek()).c_str(), ptr < (int)tok.size() ? lex(tok[ptr]) : "");
  }

  /*
    Parsing procedures
  */
  shared_ptr<ProgASTNode> program();
  shared_ptr<TypeASTNode> type();
  shared_ptr<DecvarASTNode> decvar();
  shared_ptr<DecfuncASTNode> decfunc();
  shared_ptr<ParamsASTNode> params();
  shared_ptr<VarASTNode> params1();
  shared_ptr<BlockASTNode> block();
  shared_ptr<ASTNode> statement();
  shared_ptr<ReturnASTNode> returns();
  shared_ptr<AssignASTNode> assign();

  shared_ptr<WhileASTNode> loop();
  shared_ptr<IfASTNode> conditional();

  /* Expressions */
  shared_ptr<ASTNode> expr();
  shared_ptr<ASTNode> expr0();
  shared_ptr<ASTNode> expr1();
  shared_ptr<ASTNode> expr2();
  shared_ptr<ASTNode> expr3();
  shared_ptr<ASTNode> expr4();
  shared_ptr<ASTNode> expr5();
  shared_ptr<ASTNode> expr_name();
  shared_ptr<IdASTNode> expr_id();

  shared_ptr<CallASTNode> funccall();
  shared_ptr<ArgsASTNode> args();
};
