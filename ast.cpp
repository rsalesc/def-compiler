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
};

void check_and_generate_expression(shared_ptr<ASTNode>, Code & code, ScopeStack &);

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

  // TODO: maybe optimize it later?
  void check_and_generate(Code & code, ScopeStack & sta){
    code.emitf("li $a0, %d", val);
  }
};

struct IdASTNode : public ASTNode{
  IdASTNode(string s){
    text = s;
  }

  IdASTNode(shared_ptr<ASTNode> st){
    text = st->get_text();
  }

  void check_and_generate(Code & code, ScopeStack & sta){
    ScopeInt & var = sta.get_int(get_text());
    if(var.is_global()){
      code.emitf("lw $a0, %d($t9)", var.offset());
    } else {
      code.emitf("lw $a0, %d($sp)", var.offset() + code.get_machine_offset());
    }
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

  void check_and_generate(Code & code, ScopeStack & sta){
    check_and_generate_expression(left, code, sta);
    code.emit_machine_push("a0");
    check_and_generate_expression(right, code, sta);
    code.emit_machine_top("t0");
    code.emit_binary_operation("a0", "t0", "a0", get_text());
    code.emit_machine_pop();
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

  void check_and_generate(Code & code, ScopeStack & sta){
    check_and_generate_expression(child, code, sta);
    code.emit_unary_operation("a0", get_text());
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
  void check_and_generate(Code & code, ScopeStack & sta){
    for(int i = (int)child.size()-1; i >= 0; i--){
      auto p = child[i];
      check_and_generate_expression(p, code, sta);
      code.emit_machine_push("a0");
    }
  }
};

struct ParamsASTNode : public ListASTNode{
  string get_text() const {
    return "paramlist";
  }

  int _count_declarations() override { return this->size(); }

  void check_and_generate(Code & code, ScopeStack & sta){
    for(unsigned i = 0; i < child.size(); i++){
      auto p = child[i];
      shared_ptr<VarASTNode> var = dynamic_pointer_cast<VarASTNode>(p);
      if(var->is_void())
        throw runtime_error("function argument cannot be void");
      sta.declare_int(var->get_text()) = code.next();
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

  void check_and_generate(Code & code, ScopeStack & sta){
    ScopeFunc & func = sta.get_func(get_func_name());
    if(!func.compatible_with(this->args->size()))
      throw runtime_error("wrong number of arguments in function call");

    code.emit_machine_save();
    args->check_and_generate(code, sta);
    code.set_machine_as_top();
    code.emit_grow(func.count_declarations());
    code.emitf("jal %s", code.get_label(get_func_name()).c_str());
    code.emit_shrink(func.count_declarations() + args->size());
    code.emit_machine_recover();
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

  void check_and_generate(Code & code, ScopeStack & sta){
    ScopeInt & var = sta.get_int(id->get_text());
    check_and_generate_expression(expr, code, sta);
    assert(code.get_machine_offset() == 0);

    if(sta.is_global())
      code.emitf("sw $a0, %d($t9)", var.offset());
    else
      code.emitf("sw $a0, %d($sp)", var.offset());
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

  int _count_declarations() override { return 1; }

  void check_and_generate(Code & code, ScopeStack & sta){
    if(var->is_void())
      throw runtime_error("variables cannot be declared void");

    int off = sta.declare_int(var->get_text(), sta.is_global()) = code.next();

    if(!expr){
      if(sta.is_global())
        code.emitf("sw $0, %d($t9)", off);
      else
        code.emitf("sw $0, %d($sp)", off);
    } else{
      check_and_generate_expression(expr, code, sta);
      if(sta.is_global())
        code.emitf("sw $a0, %d($t9)", off);
      else
        code.emitf("sw $a0, %d($sp)", off);
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

  int _count_declarations() override {
    int res = 0;
    for(auto p : child)
      res += p->count_declarations();
    return res;
  }

  void check_and_generate(Code & code, ScopeStack & sta){

    code.emit_segment();
    int decl = 0;
    for(auto p : child)
      if(dynamic_pointer_cast<DecvarASTNode>(p))
        decl++;

    code.emit_globals(decl+1);
    code.emit_header();

    sta.push();
    sta.declare_func("print", false, 1, 0);
    code.emit_print_code();

    Code glob_code;
    glob_code.emit_entry_point();
    glob_code.emitf("la $t9, %s", GLOBALS_LABEL.c_str());

    for(auto p : child)
      if(dynamic_pointer_cast<DecvarASTNode>(p))
        p->check_and_generate(glob_code, sta);
      else
        p->check_and_generate(code, sta);

    ScopeFunc & func = sta.get_func("main");
    if(!func.compatible_with(0))
      throw runtime_error("main should have no parameters");

    glob_code.emit_grow(func.count_declarations());
    glob_code.emitf("jal %s", glob_code.get_label("main").c_str());
    glob_code.emit_shrink(func.count_declarations());
    glob_code.emit_exit();

    code += glob_code;
  }
};

struct LoopASTNode : public ASTNode{
  Code expr_code;
  int idx = 0;

  Code get_expression_code() const { return expr_code; }
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

    if(sta.loop_block()){
      LoopASTNode * no = (LoopASTNode*)sta.loop_block();
      auto label = code.get_loop_label(no->get_index());
      code.emit_loop_begin(no->get_index());
      code += no->get_expression_code();
      code.emitf("beqz $a0, %s", label.second.c_str());
    }

    for(auto p : statements)
      p->check_and_generate(code, sta);

    if(sta.loop_block()){
      LoopASTNode * no = (LoopASTNode*)sta.loop_block();
      auto label = code.get_loop_label(no->get_index());
      code.emitf("j %s", label.first.c_str());
      code.emit_loop_end(no->get_index());
    }
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

  void check_and_generate(Code & code, ScopeStack & sta){
    sta.declare_func(this->var->get_text(), this->var->is_int(),
      this->params->size(), count_declarations());

    // emit function label
    Code code_func;
    code_func.emit_label(this->var->get_text());

    if(this->var->is_int())
      sta.push_int();
    else
      sta.push_void();

    code_func.set_offset(shift(count_declarations()));
    params->check_and_generate(code_func, sta);
    code_func.set_offset(0);
    block->check_and_generate(code_func, sta);

    sta.pop();

    code_func.emit("jr $ra # should not reach here");
    code += code_func;
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

  void check_and_generate(Code & code, ScopeStack & sta){
    if(this->expr){
      if(!sta.is_int())
        throw runtime_error("return [expr] should be used inside def [int] function");
      else {
        check_and_generate_expression(expr, code, sta);
      }
    } else if(!this->expr && sta.is_int())
      throw runtime_error("returning void value in a function of int return");

    code.emitf("jr $ra");
  }
};

struct BreakASTNode : public ASTNode{
  string get_text() const {
    return "break";
  }

  void check_and_generate(Code & code, ScopeStack & sta){
    if(!sta.is_loop())
      throw runtime_error("break should be used inside a loop");
    LoopASTNode * no = (LoopASTNode*) sta.last_loop();
    int idx = no->get_index();
    auto label = code.get_loop_label(idx);
    code.emitf("j %s", label.second.c_str());
  }
};

struct ContinueASTNode : public ASTNode{
  string get_text() const{
    return "continue";
  }

  void check_and_generate(Code & code, ScopeStack & sta){
    if(!sta.is_loop())
      throw runtime_error("continue should be used inside a loop");
    LoopASTNode * no = (LoopASTNode*) sta.last_loop();
    int idx = no->get_index();
    auto label = code.get_loop_label(idx);
    code.emitf("j %s", label.first.c_str());
  }
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

  void check_and_generate(Code & code, ScopeStack & sta){
    idx = sta.push_loop(this);

    expr_code = Code(code.get_offset(), code.get_machine_offset());
    check_and_generate_expression(expr, expr_code, sta);
    assert(code.get_offset() == expr_code.get_offset());
    assert(code.get_machine_offset() == expr_code.get_machine_offset());

    block->check_and_generate(code, sta);

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

  int _count_declarations() override{
    return block->count_declarations() +
      (else_block ? else_block->count_declarations() : 0);
  }

  void check_and_generate(Code & code, ScopeStack & sta){
    check_and_generate_expression(expr, code, sta);
    int idx = sta.push_if();
    auto label = code.get_if_label(idx);

    code.emitf("beqz $a0, %s", label.first.c_str());

    block->check_and_generate(code, sta);
    code.emitf("j %s", label.second.c_str());

    code.emit_if_false(idx);

    sta.pop();
    if(else_block){
      sta.push_else();
      else_block->check_and_generate(code, sta);
      sta.pop();
    }

    code.emit_if_end(idx);
  }
};

/**
 * Late Helpers
 * */

void check_and_generate_expression(shared_ptr<ASTNode> expr, Code & code, ScopeStack & sta){
  if(ASTNode::get_as<CallASTNode>(expr)){
    if(sta.get_func(ASTNode::get_as<CallASTNode>(expr)->get_func_name()).returns_void())
      throw runtime_error("expression cannot have void terms");
  }

  if(ASTNode::get_as<IdASTNode>(expr)){
    sta.get_int(ASTNode::get_as<IdASTNode>(expr)->get_text());
  }

  expr->check_and_generate(code, sta);
}
