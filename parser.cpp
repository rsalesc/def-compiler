#include "parser.hpp"

std::map<int, std::string> Parsing::types;
char Parsing::buf[BUF_SZ];

shared_ptr<ProgASTNode> Parser::program(){
  auto res = make_shared<ProgASTNode>();
  while(peek() == T_INT || peek() == T_VOID || peek() == T_DEF){
    if(peek() == T_DEF)
      res->append(decfunc());
    else if(peek() == T_INT || peek() == T_VOID)
      res->append(decvar());
    else
      unexpected();
  }
  expect(EOF);
  return res;
}

shared_ptr<TypeASTNode> Parser::type(){
  if(peek() == T_INT) {
    consume(T_INT);
    return make_shared<TypeASTNode>(make_shared<ASTNode>("int"));
  }
  else if(peek() == T_VOID) {
    consume(T_VOID);
    return make_shared<TypeASTNode>(make_shared<ASTNode>("void"));
  }
  else unexpected();

  throw runtime_error("parsing recursion error");
}

shared_ptr<DecvarASTNode> Parser::decvar(){
  auto t = type();
  expect(T_ID);
  auto id = consume_node<IdASTNode>();
  auto var = make_shared<VarASTNode>(id, t);
  shared_ptr<DecvarASTNode> res;

  if(peek() == '='){
    consume('=');
    res = make_shared<DecvarASTNode>(var, expr());
  } else {
    res = make_shared<DecvarASTNode>(var);
  }

  consume(';');
  return res;
}

shared_ptr<DecfuncASTNode> Parser::decfunc(){
  consume(T_DEF);
  auto t = type();
  expect(T_ID);
  auto id = consume_node<IdASTNode>();
  auto var = make_shared<VarASTNode>(id, t);
  auto list = make_shared<ParamsASTNode>();

  consume('(');
  if(peek() != ')')
    list = params();
  consume(')');

  consume('{');
  shared_ptr<BlockASTNode> code = block();
  consume('}');

  return make_shared<DecfuncASTNode>(var, list, code);
}

shared_ptr<ParamsASTNode> Parser::params(){
  auto res = make_shared<ParamsASTNode>();
  res->append(params1());
  while(peek() == ','){
    consume(',');
    res->append(params1());
  }
  return res;
}

shared_ptr<VarASTNode> Parser::params1(){
  auto t = type();
  expect(T_ID);

  return make_shared<VarASTNode>(consume_node<IdASTNode>(), t);
}

shared_ptr<BlockASTNode> Parser::block(){
  auto res = make_shared<BlockASTNode>();
  while(peek() == T_INT || peek() == T_VOID)
    res->append_declaration(decvar());
  while(peek() != '}')
    res->append_statement(statement());
  return res;
}

shared_ptr<ASTNode> Parser::statement(){
  if(peek() == T_IF)
    return conditional();
  else if(peek() == T_WHILE)
    return loop();
  else if(peek() == T_RETURN){
    auto res = returns();
    consume(';');
    return res;
  } else if(peek() == T_BREAK || peek() == T_CONTINUE){
    if(peek() == T_BREAK){
      consume(T_BREAK);
      consume(';');
      return make_shared<BreakASTNode>();
    } else{
      consume(T_CONTINUE);
      consume(';');
      return make_shared<ContinueASTNode>();
    }
  } else if(peek() == T_ID){
    consume(T_ID);
    if(peek() == '('){
      unconsume();
      auto res = funccall();
      consume(';');
      return res;
    } else {
      unconsume();
      auto res = assign();
      consume(';');
      return res;
    }
  } else unexpected();

  throw runtime_error("parsing recursion error");
}

shared_ptr<WhileASTNode> Parser::loop(){
  consume(T_WHILE);
  consume('(');
  auto ex = expr();
  consume(')');
  consume('{');
  auto code = block();
  consume('}');
  return make_shared<WhileASTNode>(ex, code);
}

shared_ptr<IfASTNode> Parser::conditional(){
  consume(T_IF);
  consume('(');
  auto ex = expr();
  consume(')');
  consume('{');
  auto code = block();
  consume('}');

  if(peek() == T_ELSE){
    consume(T_ELSE);
    consume('{');
    auto code2 = block();
    consume('}');
    return make_shared<IfASTNode>(ex, code, code2);
  } else {
    return make_shared<IfASTNode>(ex, code);
  }
}

shared_ptr<ReturnASTNode> Parser::returns(){
  consume(T_RETURN);
  if(peek() != ';')
    return make_shared<ReturnASTNode>(expr());
  else
    return make_shared<ReturnASTNode>();
}

shared_ptr<AssignASTNode> Parser::assign(){
  expect(T_ID);
  auto id = consume_node<IdASTNode>();
  consume('=');
  return make_shared<AssignASTNode>(id, expr());
}

shared_ptr<ASTNode> Parser::expr(){
  auto res = expr0();
  while(peek() == T_OR){
    auto op = consume_node<ASTNode>();
    res = make_shared<BinASTNode>(res, expr0(), op);
  }
  return res;
}

shared_ptr<ASTNode> Parser::expr0(){
  auto res = expr1();
  while(peek() == T_AND){
    auto op = consume_node<ASTNode>();
    res = make_shared<BinASTNode>(res, expr1(), op);
  }
  return res;
}

shared_ptr<ASTNode> Parser::expr1(){
  auto res = expr2();
  while(peek() == T_EQ || peek() == T_NEQ){
    auto op = consume_node<ASTNode>();
    res = make_shared<BinASTNode>(res, expr2(), op);
  }
  return res;
}

shared_ptr<ASTNode> Parser::expr2(){
  auto res = expr3();
  while(peek() == '<' || peek() == '>'
    || peek() == T_LEQ || peek() == T_GEQ){
      auto op = consume_node<ASTNode>();
      res = make_shared<BinASTNode>(res, expr3(), op);
  }
  return res;
}

shared_ptr<ASTNode> Parser::expr3(){
  auto res = expr4();
  while(peek() == '+' || peek() == '-'){
    auto op = consume_node<ASTNode>();
    res = make_shared<BinASTNode>(res, expr4(), op);
  }
  return res;
}

shared_ptr<ASTNode> Parser::expr4(){
  auto res = expr5();
  while(peek() == '*' || peek() == '/'){
    auto op = consume_node<ASTNode>();
    res = make_shared<BinASTNode>(res, expr5(), op);
  }
  return res;
}

shared_ptr<ASTNode> Parser::expr5(){
  if(peek() == '-' || peek() == '!'){
    auto op = consume_node<ASTNode>();
    return make_shared<UnASTNode>(expr5(), op);
  } else if(peek() == '('){
    consume('(');
    auto res = expr();
    consume(')');
    return res;
  } else if(peek() == T_DEC){
    return consume_node<DecASTNode>();
  } else if(peek() == T_ID){
    return expr_name();
  } else unexpected();

  throw runtime_error("parsing recursion error");
}

shared_ptr<ASTNode> Parser::expr_name(){
  consume(T_ID);
  if(peek() == '('){
    unconsume();
    return funccall();
  } else {
    unconsume();
    return expr_id();
  }
}

shared_ptr<IdASTNode> Parser::expr_id(){
  return consume_node<IdASTNode>();
}

shared_ptr<CallASTNode> Parser::funccall(){
  expect(T_ID);
  auto id = consume_node<IdASTNode>();
  auto ar = make_shared<ArgsASTNode>();

  consume('(');
  if(peek() != ')'){
    ar = args();
  }
  consume(')');

  return make_shared<CallASTNode>(id, ar);
}

shared_ptr<ArgsASTNode> Parser::args(){
  auto res = make_shared<ArgsASTNode>();
  res->append(expr());
  while(peek() == ','){
    consume();
    res->append(expr());
  }
  return res;
}
