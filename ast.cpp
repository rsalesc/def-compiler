#include "scope.hpp"
#include <string>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

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
};

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
};

struct ParamsASTNode : public ListASTNode{
  string get_text() const {
    return "paramlist";
  }
};

struct CallASTNode : public ASTNode{
  shared_ptr<ASTNode> id;
  shared_ptr<ArgsASTNode> args;

  CallASTNode(shared_ptr<ASTNode> id){
    this->id = id;
    this->args = make_shared<ArgsASTNode>();
  }

  CallASTNode(shared_ptr<ASTNode> id, shared_ptr<ArgsASTNode> args){
    this->id = id;
    this->args = args;
  }

  string get_text() const {
    return "funccall";
  }

  void print_children() const {
    cout << " ";
    id->print_node();
    cout << " ";
    args->print_node();
  }
};

// repensar se devo guardar id ao inves de herdar?
struct VarASTNode : public IdASTNode{
  bool isint;

  VarASTNode(shared_ptr<ASTNode> st, shared_ptr<ASTNode> type) : IdASTNode(st){
    this->isint = type->get_text() == "int";
  }
};

struct AssignASTNode : public ASTNode{
  shared_ptr<ASTNode> id, expr;

  AssignASTNode(shared_ptr<ASTNode> id, shared_ptr<ASTNode> expr){
    this->id = id;
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
};

struct DecvarASTNode : public ASTNode{
  shared_ptr<ASTNode> var, expr;
  DecvarASTNode(shared_ptr<ASTNode> var, shared_ptr<ASTNode> expr = 0){
    this->var = var;
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
};

struct ProgASTNode : public ListASTNode{
  void append(shared_ptr<ASTNode> st){
    child.insert(child.begin(), st);
  }
  string get_text() const {
    return "program";
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
};

struct DecfuncASTNode : public ASTNode {
  shared_ptr<ASTNode> var;
  shared_ptr<ParamsASTNode> params;
  shared_ptr<ASTNode> block;

  DecfuncASTNode(shared_ptr<ASTNode> var, shared_ptr<ASTNode> params,
      shared_ptr<ASTNode> block){
    this->var = var;
    this->params = dynamic_pointer_cast<ParamsASTNode>(params);
    this->block = block;
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
};

struct BreakASTNode : public ASTNode{
  string get_text() const {
    return "break";
  }
};

struct ContinueASTNode : public ASTNode{
  string get_text() const{
    return "continue";
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
};
