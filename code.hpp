#include <iostream>
#include <string>
#include <vector>
#include <cstdarg>
#include <cstdio>

const int WORD = 4;
const std::string GLOBALS_LABEL = "globals__";
const std::string ENTRY_POINT_LABEL = "main";
const std::string LABEL_PREFIX = "_lb_";

const std::string LOOP_BEGIN_PREFIX = "_lp_begin_";
const std::string LOOP_END_PREFIX = "_lp_end_";

const std::string IF_FALSE_PREFIX = "_if_false_";
const std::string IF_END_PREFIX = "_if_end_";

// a0 saves both the return of a function call and of an expression
// t0 is a helper for machine operations
// t1 and t2 are helpers for binary operations, they should
  // never be used in other places
// t9 saves the address of the heap array
// ra saves the return address of a funtion call
const std::vector<std::string> SAVED_REGISTERS = {"ra"};

char _build_buf[256];

int shift(int x){
  return WORD*x;
}

// void buildf(const char *, ...)
//     __attribute__((format (printf, 2, 3)));

std::string buildf(const char *fmt, va_list arg) {
    vsprintf(_build_buf, fmt, arg);
    return std::string(_build_buf);
}

struct Code{
  std::string buf;
  int offset;
  int machine_offset;

  Code(int offset = 0, int machine_offset = 0) :
        offset(offset), machine_offset(machine_offset) {}

  void set_offset(int x) { offset = x; }
  void add_offset(int x = 1) { offset += x*WORD; }
  int get_offset() const { return offset; }
  int next() { add_offset(); return get_offset(); }

  void set_machine_offset(int x) { machine_offset = x; }
  void add_machine_offset(int x) { machine_offset += WORD*x; }
  int get_machine_offset() const { return machine_offset; }

  int set_machine_as_top() {
    int old = machine_offset;
    machine_offset = 0;
    return old;
  }

  std::string get_label(std::string s) { return LABEL_PREFIX + s; }
  std::pair<std::string, std::string> get_loop_label(int idx){
    std::string s = std::to_string(idx);
    return std::make_pair(LOOP_BEGIN_PREFIX + s, LOOP_END_PREFIX + s);
  }

  std::pair<std::string, std::string> get_if_label(int idx){
    std::string s = std::to_string(idx);
    return std::make_pair(IF_FALSE_PREFIX + s, IF_END_PREFIX + s);
  }

  void push_line(std::string s){ buf += s; buf += '\n'; }
  void emit(std::string s) { push_line(std::string("\t") + s); }
  void emitf(const char * fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    emit(buildf(fmt, arg));
    va_end(arg);
  }

  // stack
  void emit_grow(int x = 1){ if(x != 0) emitf("addiu $sp, $sp, %d", -shift(x)); }
  void emit_shrink(int x = 1) { if(x != 0) emitf("addiu $sp, $sp, %d", shift(x)); }
  void emit_push(std::string reg) { emitf("sw $%s, 0($sp)", reg.c_str()); emit_grow();}
  void emit_pop() { emit_shrink(); }
  void emit_top(std::string reg) { emitf("lw $%s, %d($sp)", reg.c_str(), WORD); }
  void emit_save(){
    emit_grow(SAVED_REGISTERS.size());
    for(unsigned i = 0; i < SAVED_REGISTERS.size(); i++){
      emitf("sw $%s, %d($sp)", SAVED_REGISTERS[i].c_str(), shift(i+1));
    }
  }

  int emit_save_no_grow(){
    for(unsigned i = 0; i < SAVED_REGISTERS.size(); i++){
      emitf("sw $%s, %d($sp)", SAVED_REGISTERS[i].c_str(), -shift(i));
    }

    return SAVED_REGISTERS.size();
  }

  int emit_recover_no_grow(){
    for(unsigned i = 0; i < SAVED_REGISTERS.size(); i++){
      emitf("lw $%s, %d($sp)", SAVED_REGISTERS[i].c_str(), -shift(i));
    }

    return SAVED_REGISTERS.size();
  }

  // machine
  void emit_machine_push(std::string reg) { emit_push(reg); machine_offset += WORD;}
  void emit_machine_pop() { emit_pop(); machine_offset -= WORD; }
  void emit_machine_top(std::string reg) { emit_top(reg); }
  int emit_machine_save(){
    for(unsigned i = 0; i < SAVED_REGISTERS.size(); i++){
      emit_machine_push(SAVED_REGISTERS[i]);
    }

    return SAVED_REGISTERS.size();
  }

  void emit_machine_recover(){
    for(unsigned i = 0; i < SAVED_REGISTERS.size(); i++){
      emit_machine_top(SAVED_REGISTERS[i]);
      emit_machine_pop();
    }
  }

  void emit_globals(int sz) { emitf("%s: .space %d", GLOBALS_LABEL.c_str(), WORD*(sz+1)); }
  void emit_segment(){ push_line(".data"); }
  void emit_header(){ push_line(".text"); }
  void emit_label(std::string s) { push_line(LABEL_PREFIX + s + std::string(":")); }
  void emit_loop_begin(int idx) {
    push_line(get_loop_label(idx).first.c_str() + std::string(":"));
  }
  void emit_loop_end(int idx) {
    push_line(get_loop_label(idx).second.c_str() + std::string(":"));
  }

  void emit_if_false(int idx){
    push_line(get_if_label(idx).first.c_str() + std::string(":"));
  }
  void emit_if_end(int idx){
    push_line(get_if_label(idx).second.c_str() + std::string(":"));
  }

  void emit_entry_point() { push_line(ENTRY_POINT_LABEL + std::string(":")); }
  void emit_exit(){
    emitf("li $v0, 10");
    emitf("syscall");
  }

  void emit_to_bool(std::string res, std::string reg){
    emitf("sltu $%s, $0, $%s", res.c_str(), reg.c_str());
  }

  void emit_to_bool(std::string reg){
    emit_to_bool(reg, reg);
  }

  void emit_not(std::string reg){
    emitf("xori $%s, $%s, 1", reg.c_str(), reg.c_str());
  }

  // operations
  void emit_binary_operation(std::string res, std::string r1,
                              std::string r2, std::string type){
      #define BINARY_PARAMS res.c_str(), r1.c_str(), r2.c_str()
      #define SELF_RES res.c_str(), res.c_str()

      if(type == "+")
        emitf("addu $%s, $%s, $%s", BINARY_PARAMS);
      else if(type == "-")
        emitf("subu $%s, $%s, $%s", BINARY_PARAMS);
      else if(type == "*")
        emitf("mul $%s, $%s, $%s", BINARY_PARAMS); // macro by mips assembler
      else if(type == "/")
        emitf("div $%s, $%s, $%s", BINARY_PARAMS); // "     "       "
      else if(type == "=="){
        emitf("xor $%s, $%s, $%s", BINARY_PARAMS);
        emit_to_bool(res);
        emit_not(res);
      } else if(type == "!="){
        emitf("xor $%s, $%s, $%s", BINARY_PARAMS);
        emit_to_bool(res);
      } else if(type == "&&"){
        emit_to_bool("t1", r1);
        emit_to_bool("t2", r2);
        emitf("and $%s, $t1, $t2", res.c_str());
      } else if(type == "||"){
        emitf("or $%s, $%s, $%s", BINARY_PARAMS);
        emit_to_bool(res);
      } else if(type == "<"){
        emitf("slt $%s, $%s, $%s", BINARY_PARAMS);
      } else if(type == ">"){
        emitf("slt $%s, $%s, $%s", res.c_str(), r2.c_str(), r1.c_str());
      } else if(type == "<="){
        emitf("slt $%s, $%s, $%s", res.c_str(), r2.c_str(), r1.c_str());
        emit_not(res);
      } else if(type == ">="){
        emitf("slt $%s, $%s, $%s", BINARY_PARAMS);
        emit_not(res);
      } else {
        throw std::runtime_error(
          "invalid type of binary operator during code generation");
      }

      #undef BINARY_PARAMS
  }

  void emit_binary_operation(std::string res, std::string reg, std::string type){
    emit_binary_operation(res, res, reg, type);
  }

  void emit_unary_operation(std::string res, std::string reg, std::string type){
    if(type == "-"){
      emitf("not $%s, $%s", res.c_str(), reg.c_str());
      emitf("addiu $%s, $%s, 1", res.c_str(), res.c_str());
    } else if(type == "!"){
      emit_to_bool(res, reg);
      emit_not(res);
    } else{
      throw std::runtime_error(
        "invalid type of unary operator during code generation");
    }
  }

  void emit_unary_operation(std::string reg, std::string type){
    emit_unary_operation(reg, reg, type);
  }
  //

  void emit_print_code(){
    emit_label("print");
    emitf("li $v0, 1");
    emitf("lw $a0, %d($sp)", WORD);
    emitf("syscall");
    emitf("li $v0, 11");
    emitf("li $a0, 0x%02x", (int)'\n');
    emitf("syscall");
    emitf("jr $ra");
  }

  Code & operator+=(const Code & rhs) {
    this->buf += rhs.buf;
    return *this;
  }

  void print() const {
    puts(buf.c_str());
  }
};
