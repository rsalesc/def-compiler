#pragma once

#include "scope.hpp"
#include "code.hpp"
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
  int memo = -1;

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

  virtual void check_and_generate(Code &, ScopeStack &) {}
  virtual int _count_declarations() { return 0; }
  virtual int count_declarations() {
    if(memo != -1) return memo;
    return memo = _count_declarations();
  }

  template<typename T>
  static shared_ptr<T> get_as(shared_ptr<ASTNode> st){
    return dynamic_pointer_cast<T>(st);
  }

  static void check_and_generate_expression(shared_ptr<ASTNode>, Code & code, ScopeStack &);
};

struct DecASTNode : public ASTNode {
  int val;

  DecASTNode(string s) : val(atoi(s.c_str())){}
  DecASTNode(int x) : val(x){}
  DecASTNode(shared_ptr<ASTNode> st){
    val = atoi(st->get_text().c_str());
  }

  string get_text() const;

  void check_and_generate(Code & code, ScopeStack & sta);
};

struct IdASTNode : public ASTNode{
  IdASTNode(string s){
    text = s;
  }

  IdASTNode(shared_ptr<ASTNode> st){
    text = st->get_text();
  }

  void check_and_generate(Code & code, ScopeStack & sta);
};

struct BinASTNode : public ASTNode{
  shared_ptr<ASTNode> left, right;

  BinASTNode(shared_ptr<ASTNode> left, shared_ptr<ASTNode> right, string text = "+"){
    this->left = left;
    this->right = right;
    this->text = text;
  }

  BinASTNode(shared_ptr<ASTNode> left, shared_ptr<ASTNode> right,
      shared_ptr<ASTNode> op){
    this->left = left;
    this->right = right;
    this->text = op->get_text();
  }

  void print_children() const{
    cout << " ";
    left->print_node();
    cout << " ";
    right->print_node();
  }

  void check_and_generate(Code & code, ScopeStack & sta);
};

struct UnASTNode : public ASTNode{
  shared_ptr<ASTNode> child;

  UnASTNode(shared_ptr<ASTNode> child, string text){
    this->child = child;
    this->text = text;
  }

  UnASTNode(shared_ptr<ASTNode> child, shared_ptr<ASTNode> op){
    this->child = child;
    this->text = op->get_text();
  }

  void print_children() const {
    cout << " ";
    child->print_node();
  }

  void check_and_generate(Code & code, ScopeStack & sta);
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

  int size() const { return child.size(); }
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
  void check_and_generate(Code & code, ScopeStack & sta);
};

struct ParamsASTNode : public ListASTNode{
  string get_text() const {
    return "paramlist";
  }

  int _count_declarations() override { return this->size(); }

  void check_and_generate(Code & code, ScopeStack & sta);

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

  void check_and_generate(Code & code, ScopeStack & sta);
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

  void check_and_generate(Code & code, ScopeStack & sta);
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

  int _count_declarations() override { return 1; }

  void check_and_generate(Code & code, ScopeStack & sta);
};

struct ProgASTNode : public ListASTNode{
  void append(shared_ptr<ASTNode> st){
    //child.insert(child.begin(), st);
    child.push_back(st);
  }

  string get_text() const {
    return "program";
  }

  int _count_declarations() override {
    int res = 0;
    for(auto p : child)
      res += p->count_declarations();
    return res;
  }

  void check_and_generate(Code & code, ScopeStack & sta);
};

struct LoopASTNode : public ASTNode{
  int idx = 0;

  int get_index() const { return idx; }
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

  int _count_declarations() override{
    int res = declarations.size();
    for(auto p : statements)
      res += p->count_declarations();
    return res;
  }

  void check_and_generate(Code & code, ScopeStack & sta){
    for(auto p : declarations)
      p->check_and_generate(code, sta);

    for(auto p : statements)
      p->check_and_generate(code, sta);
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

  int _count_declarations() override{
    return block->count_declarations();
  }

  void check_and_generate(Code & code, ScopeStack & sta);
};


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

  void check_and_generate(Code & code, ScopeStack & sta);
};

struct BreakASTNode : public ASTNode{
  string get_text() const {
    return "break";
  }

  void check_and_generate(Code & code, ScopeStack & sta);
};

struct ContinueASTNode : public ASTNode{
  string get_text() const{
    return "continue";
  }

  void check_and_generate(Code & code, ScopeStack & sta);
};

struct WhileASTNode : public LoopASTNode{
  shared_ptr<ASTNode> expr;
  shared_ptr<BlockASTNode> block;

  WhileASTNode(shared_ptr<ASTNode> expr, shared_ptr<ASTNode> block){
    this->expr = expr;
    this->block = dynamic_pointer_cast<BlockASTNode>(block);
  }

  string get_text() const {
    return "while";
  }

  int _count_declarations() override {
    return block->count_declarations();
  }

  void print_children() const {
    cout << " ";
    expr->print_node();
    cout << " ";
    block->print_node();
  }

  void check_and_generate(Code & code, ScopeStack & sta);
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

  int _count_declarations() override{
    return block->count_declarations() +
      (else_block ? else_block->count_declarations() : 0);
  }

  void check_and_generate(Code & code, ScopeStack & sta);
};
