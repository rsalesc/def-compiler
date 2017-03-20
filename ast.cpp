#include "scope.hpp"
#include <string>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

/*
 *
 * Nodes and Helpers
 * */

struct ASTNode{
  string text;

  ASTNode(){}
  ASTNode(string s) : text(s){}

  virtual string get_text() const {
    return text;
  }

  virtual void print_node() const {
    cout << "[" << this->get_text();
    this->print_children();
    cout << "]";
  }

  virtual void print_children() const {}

  virtual void check_semantics(ScopeStack &) {}

  template<typename T>
    static shared_ptr<T> get_as(shared_ptr<ASTNode> st){
      return dynamic_pointer_cast<T>(st);
    }
};

void is_expression_void(shared_ptr<ASTNode>, ScopeStack &);

struct DecASTNode : public ASTNode {
  int val;

  DecASTNode(string s) : val(atoi(s.c_str())){}
  DecASTNode(int x) : val(x){}
  DecASTNode(shared_ptr<ASTNode> st){
    val = atoi(st->get_text().c_str());
  }

  string get_text() const {
    return to_string(val);
  }
};

struct IdASTNode : public ASTNode{ 
  IdASTNode(string s){
    text = s;
  }

  IdASTNode(shared_ptr<ASTNode> st){
    text = st->get_text();
  }
};

struct BinASTNode : public ASTNode{
  shared_ptr<ASTNode> left, right;

  BinASTNode(shared_ptr<ASTNode> left, shared_ptr<ASTNode> right, string text = "+"){
    this->left = left;
    this->right = right;
    this->text = text;
  }

  void print_children() const{
    cout << " ";
    left->print_node();
    cout << " ";
    right->print_node();
  }

  void check_semantics(ScopeStack & sta){
    is_expression_void(left, sta);
    is_expression_void(right, sta);
  }
};

struct UnASTNode : public ASTNode{
  shared_ptr<ASTNode> child;

  UnASTNode(shared_ptr<ASTNode> child, string text){
    this->child = child;
    this->text = text;
  }

  void print_children() const {
    cout << " ";
    child->print_node();
  }

  void check_semantics(ScopeStack & sta){
    is_expression_void(child, sta);
  }
};

struct TypeASTNode : public ASTNode{
  shared_ptr<ASTNode> st;

  TypeASTNode(shared_ptr<ASTNode> st){
    this->st = st;
  }

  string get_text() const {
    return this->st->get_text();
  }
  bool is_int() const { return this->get_text() == "int"; }
  bool is_void() const { return this->get_text() == "void"; }
};

struct VarASTNode : public ASTNode {
  shared_ptr<IdASTNode> id;
  shared_ptr<TypeASTNode> type;

  VarASTNode(shared_ptr<ASTNode> id, shared_ptr<ASTNode> type){
    this->id = static_pointer_cast<IdASTNode>(id);
    this->type = static_pointer_cast<TypeASTNode>(type);
  }

  string get_text() const {
    return id->get_text();
  }

  bool is_int() const {
    return type->is_int();
  }

  bool is_void() const {
    return type->is_void();
  }
};

struct ListASTNode : public ASTNode{
  vector<shared_ptr<ASTNode>> child;

  virtual void append(shared_ptr<ASTNode> nw){
    child.push_back(nw);
  }

  void print_children() const {
    for(const auto & no : child){
      cout << " ";
      no->print_node();
    }
  }
};

struct ArgsASTNode : public ListASTNode{
  string get_text() const {
    return "arglist";
  }
  void check_semantics(ScopeStack & sta){
    for(auto p : child){
      is_expression_void(p, sta);
    }
  }  
};

struct ParamsASTNode : public ListASTNode{
  string get_text() const {
    return "paramlist";
  }

  void check_semantics(ScopeStack & sta){
    for(auto p : child){
      shared_ptr<VarASTNode> var = dynamic_pointer_cast<VarASTNode>(p);
      if(var->is_void())
        throw runtime_error("function argument cannot be void");
      sta.declare_int(var->get_text());
    }
  }

};

struct CallASTNode : public ASTNode{
  shared_ptr<IdASTNode> id;
  shared_ptr<ArgsASTNode> args;

  CallASTNode(shared_ptr<ASTNode> id){
    this->id = static_pointer_cast<IdASTNode>(id);
    this->args = make_shared<ArgsASTNode>();
  }

  CallASTNode(shared_ptr<ASTNode> id, shared_ptr<ArgsASTNode> args){
    this->id = static_pointer_cast<IdASTNode>(id);
    this->args = args;
  }

  string get_text() const {
    return "funccall";
  }

  string get_func_name() const {
    return this->id->get_text();
  }

  void print_children() const {
    cout << " ";
    id->print_node();
    cout << " ";
    args->print_node();
  }

  void check_semantics(ScopeStack & sta){
    sta.get_func(get_func_name());
    args->check_semantics(sta);
  }
};

struct AssignASTNode : public ASTNode{
  shared_ptr<IdASTNode> id;
  shared_ptr<ASTNode> expr;

  AssignASTNode(shared_ptr<ASTNode> id, shared_ptr<ASTNode> expr){
    this->id = static_pointer_cast<IdASTNode>(id);
    this->expr = expr;
  }

  string get_text() const {
    return "assign";
  }

  void print_children() const {
    cout << " ";
    id->print_node();
    cout << " ";
    expr->print_node();
  }

  void check_semantics(ScopeStack & sta){
    sta.get_int(id->get_text());
    expr->check_semantics(sta);
    is_expression_void(expr, sta);
  }
};

struct DecvarASTNode : public ASTNode{
  shared_ptr<VarASTNode> var;
  shared_ptr<ASTNode> expr;

  DecvarASTNode(shared_ptr<ASTNode> var, shared_ptr<ASTNode> expr = 0){
    this->var = static_pointer_cast<VarASTNode>(var);
    this->expr = expr;
  }

  string get_text() const {
    return "decvar";
  }

  void print_children() const {
    cout << " ";
    var->print_node();
    if(expr){
      cout << " ";
      expr->print_node();
    }
  }

  void check_semantics(ScopeStack & sta){
    if(var->is_void())
      throw runtime_error("variables cannot be declared void");

    sta.declare_int(var->get_text());
    if(expr) is_expression_void(expr, sta);
  }
};

struct ProgASTNode : public ListASTNode{
  void append(shared_ptr<ASTNode> st){
    child.insert(child.begin(), st);
  }

  string get_text() const {
    return "program";
  }

  void check_semantics(ScopeStack & sta){
    for(auto p : child)
      p->check_semantics(sta);
  }
};

struct BlockASTNode : public ASTNode{
  vector<shared_ptr<ASTNode>> declarations, statements;

  void append_declaration(shared_ptr<ASTNode> st){
    declarations.push_back(st);
  }

  void append_statement(shared_ptr<ASTNode> st){
    statements.push_back(st);
  }

  string get_text() const {
    return "block";
  }

  void print_children() const {
    for(const auto & p : declarations){
      cout << " ";
      p->print_node();
    }
    for(const auto & p : statements){
      cout << " ";
      p->print_node();
    }
  }

  void check_semantics(ScopeStack & sta){
    for(auto p : declarations)
      p->check_semantics(sta);
    for(auto p : statements)
      p->check_semantics(sta);
  }
};

struct DecfuncASTNode : public ASTNode {
  shared_ptr<VarASTNode> var;
  shared_ptr<ParamsASTNode> params;
  shared_ptr<BlockASTNode> block;

  DecfuncASTNode(shared_ptr<ASTNode> var, shared_ptr<ASTNode> params,
      shared_ptr<ASTNode> block){
    this->var = static_pointer_cast<VarASTNode>(var);
    this->params = dynamic_pointer_cast<ParamsASTNode>(params);
    this->block = static_pointer_cast<BlockASTNode>(block);
  }

  string get_text() const {
    return "decfunc";
  }

  void print_children() const {
    cout << " ";
    var->print_node();
    cout << " ";
    params->print_node();
    cout << " ";
    block->print_node();
  }

  void check_semantics(ScopeStack & sta){
    sta.declare_func(this->var->get_text(), this->var->is_int());

    if(this->var->is_int())
      sta.push_int();
    else
      sta.push_void();
    params->check_semantics(sta);
    block->check_semantics(sta);
    sta.pop();
  }
};

// RESOLVER TODO SEMANTICO ATE AQUI

struct ReturnASTNode : public ASTNode{
  shared_ptr<ASTNode> expr;

  ReturnASTNode(shared_ptr<ASTNode> expr = 0){
    this->expr = expr;
  }

  string get_text() const{
    return "return";
  }

  void print_children() const {
    if(expr){
      cout << " ";
      expr->print_node();
    }
  }

  void check_semantics(ScopeStack & sta){
    if(this->expr){
      if(!sta.is_int())
        throw runtime_error("return [expr] should be used inside def [int] function");
      else is_expression_void(expr, sta);
    } else if(!this->expr && sta.is_int())
      throw runtime_error("returning void value in a function of int return");
  }
};

struct BreakASTNode : public ASTNode{
  string get_text() const {
    return "break";
  }

  void check_semantics(ScopeStack & sta){
    if(!sta.is_loop())
      throw runtime_error("break should be used inside a loop");
  }
};

struct ContinueASTNode : public ASTNode{
  string get_text() const{
    return "continue";
  }

  void check_semantics(ScopeStack & sta){
    if(!sta.is_loop())
      throw runtime_error("continue should be used inside a loop");
  }
};

struct WhileASTNode : public ASTNode{
  shared_ptr<ASTNode> expr;
  shared_ptr<BlockASTNode> block;

  WhileASTNode(shared_ptr<ASTNode> expr, shared_ptr<ASTNode> block){
    this->expr = expr;
    this->block = dynamic_pointer_cast<BlockASTNode>(block);
  }

  string get_text() const {
    return "while";
  }

  void print_children() const {
    cout << " ";
    expr->print_node();
    cout << " ";
    block->print_node();
  }

  void check_semantics(ScopeStack & sta){
    sta.push_loop();
    is_expression_void(expr, sta);
    block->check_semantics(sta);
    sta.pop();
  }
};

struct IfASTNode : public ASTNode{
  shared_ptr<ASTNode> expr;
  shared_ptr<BlockASTNode> block, else_block;

  IfASTNode(shared_ptr<ASTNode> expr, shared_ptr<ASTNode> block,
      shared_ptr<ASTNode> else_block = 0){
    this->expr = expr;
    this->block = dynamic_pointer_cast<BlockASTNode>(block);
    this->else_block = dynamic_pointer_cast<BlockASTNode>(else_block);
  }

  string get_text() const {
    return "if";
  }

  void print_children() const {
    cout << " ";
    expr->print_node();
    cout << " ";
    block->print_node();
    if(else_block){
      cout << " ";
      else_block->print_node();
    }
  }

  void check_semantics(ScopeStack & sta){
    is_expression_void(expr, sta);
    sta.push();
    block->check_semantics(sta);
    sta.pop();
    if(else_block){
      sta.push();
      else_block->check_semantics(sta);
      sta.pop();
    }
  }
};

/**
 * Late Helpers
 * */

void is_expression_void(shared_ptr<ASTNode> expr, ScopeStack & sta){
  if(ASTNode::get_as<CallASTNode>(expr)){
    if(sta.get_func(ASTNode::get_as<CallASTNode>(expr)->get_func_name()).returns_void())
      throw runtime_error("expression cannot have void terms");
  }
  
  if(ASTNode::get_as<IdASTNode>(expr)){
    sta.get_int(ASTNode::get_as<IdASTNode>(expr)->get_text());
  }

  expr->check_semantics(sta);
}
