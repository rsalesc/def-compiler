#include "ast.hpp"

/*** Nodes
*/

string DecASTNode::get_text() const {
  return to_string(val);
}

void DecASTNode::check_and_generate(Code & code, ScopeStack & sta){
  code.emitf("li $a0, %d", val);
}

void IdASTNode::check_and_generate(Code & code, ScopeStack & sta){
  ScopeInt & var = sta.get_int(get_text());
  if(var.is_global()){
    code.load_globals();
    code.emitf("lw $a0, %d($t0)", var.offset());
  } else {
    code.emitf("lw $a0, %d($sp)", var.offset() + code.get_machine_offset());
  }
}

void BinASTNode::check_and_generate(Code & code, ScopeStack & sta){
  check_and_generate_expression(left, code, sta);
  code.emit_machine_push("a0");
  check_and_generate_expression(right, code, sta);
  code.emit_machine_top("t0");
  code.emit_binary_operation("a0", "t0", "a0", get_text());
  code.emit_machine_pop();
}

void UnASTNode::check_and_generate(Code & code, ScopeStack & sta){
  check_and_generate_expression(child, code, sta);
  code.emit_unary_operation("a0", get_text());
}

void ArgsASTNode::check_and_generate(Code & code, ScopeStack & sta){
  for(int i = (int)child.size()-1; i >= 0; i--){
    auto p = child[i];
    check_and_generate_expression(p, code, sta);
    code.emit_machine_push("a0");
  }
}

void ParamsASTNode::check_and_generate(Code & code, ScopeStack & sta){
  for(unsigned i = 0; i < child.size(); i++){
    auto p = child[i];
    shared_ptr<VarASTNode> var = dynamic_pointer_cast<VarASTNode>(p);
    if(var->is_void())
      throw runtime_error("function argument cannot be void");
    sta.declare_int(var->get_text()) = code.next();
  }
}

void CallASTNode::check_and_generate(Code & code, ScopeStack & sta){
  ScopeFunc & func = sta.get_func(get_func_name());
  if(!func.compatible_with(this->args->size()))
    throw runtime_error("wrong number of arguments in function call");

  code.emit_machine_save();
  int old_machine_offset = code.get_machine_offset();
  args->check_and_generate(code, sta);
  code.set_machine_as_top();
  code.emit_grow(func.count_declarations());
  code.emitf("jal %s", code.get_label(get_func_name()).c_str());
  code.emit_shrink(func.count_declarations() + args->size());
  code.set_machine_offset(old_machine_offset);
  code.emit_machine_recover();
}

void AssignASTNode::check_and_generate(Code & code, ScopeStack & sta){
  ScopeInt & var = sta.get_int(id->get_text());
  int old_off = code.get_machine_offset();
  check_and_generate_expression(expr, code, sta);
  assert(code.get_machine_offset() == old_off);

  if(var.is_global()){
    code.load_globals();
    code.emitf("sw $a0, %d($t0)", var.offset());
  }
  else
    code.emitf("sw $a0, %d($sp)", var.offset());
}

void DecvarASTNode::check_and_generate(Code & code, ScopeStack & sta){
  if(var->is_void())
    throw runtime_error("variables cannot be declared void");


  if(!expr){
    int off = sta.declare_int(var->get_text(), sta.is_global()) = code.next();
    if(sta.is_global()){
      code.load_globals();
      code.emitf("sw $0, %d($t0)", off);
    }else
      code.emitf("sw $0, %d($sp)", off);
  } else{
    check_and_generate_expression(expr, code, sta);
    int off = sta.declare_int(var->get_text(), sta.is_global()) = code.next();
    if(sta.is_global()){
      code.load_globals();
      code.emitf("sw $a0, %d($t0)", off);
    } else
      code.emitf("sw $a0, %d($sp)", off);
  }
}


void ProgASTNode::check_and_generate(Code & code, ScopeStack & sta){
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

void DecfuncASTNode::check_and_generate(Code & code, ScopeStack & sta){
  sta.declare_func(this->var->get_text(), this->var->is_int(),
    this->params->size(), count_declarations());

  // emit function label
  Code code_func;
  code_func.emitf("nop # %s (%d declarations)", this->var->get_text().c_str(),
                  count_declarations());
  code_func.emit_label(this->var->get_text());

  if(this->var->is_int())
    sta.push_int();
  else
    sta.push_void();

  code_func.set_offset(code.shift(count_declarations()));
  params->check_and_generate(code_func, sta);
  code_func.set_offset(0);
  block->check_and_generate(code_func, sta);

  sta.pop();

  code_func.emit("jr $ra # should not reach here");
  code += code_func;
}

void ReturnASTNode::check_and_generate(Code & code, ScopeStack & sta){
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

void BreakASTNode::check_and_generate(Code & code, ScopeStack & sta){
  if(!sta.is_loop())
    throw runtime_error("break should be used inside a loop");
  LoopASTNode * no = (LoopASTNode*) sta.last_loop();
  int idx = no->get_index();
  auto label = code.get_loop_label(idx);
  code.emitf("j %s", label.second.c_str());
}

void ContinueASTNode::check_and_generate(Code & code, ScopeStack & sta){
  if(!sta.is_loop())
    throw runtime_error("continue should be used inside a loop");
  LoopASTNode * no = (LoopASTNode*) sta.last_loop();
  int idx = no->get_index();
  auto label = code.get_loop_label(idx);
  code.emitf("j %s", label.first.c_str());
}

void WhileASTNode::check_and_generate(Code & code, ScopeStack & sta){
  idx = sta.push_loop(this);

  // expr_code = Code(code.get_offset(), code.get_machine_offset());
  auto label = code.get_loop_label(get_index());
  code.emit_loop_begin(get_index());

  int old_off = code.get_machine_offset();
  check_and_generate_expression(expr, code, sta);
  code.emitf("beqz $a0, %s", label.second.c_str());
  assert(code.get_machine_offset() == old_off);
  // assert(code.get_offset() == expr_code.get_offset());
  // assert(code.get_machine_offset() == expr_code.get_machine_offset());

  block->check_and_generate(code, sta);

  code.emitf("j %s", label.first.c_str());
  code.emit_loop_end(get_index());
  sta.pop();
}

void IfASTNode::check_and_generate(Code & code, ScopeStack & sta){
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

/**
 * Late Helpers
 * */
 void ASTNode::check_and_generate_expression(shared_ptr<ASTNode> expr, Code & code, ScopeStack & sta){
   if(ASTNode::get_as<CallASTNode>(expr)){
     if(sta.get_func(ASTNode::get_as<CallASTNode>(expr)->get_func_name()).returns_void())
       throw runtime_error("expression cannot have void terms");
   }

   if(ASTNode::get_as<IdASTNode>(expr)){
     sta.get_int(ASTNode::get_as<IdASTNode>(expr)->get_text());
   }

   expr->check_and_generate(code, sta);
 }
