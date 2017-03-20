#include "scope.hpp"
#include "ast.cpp"
#include <memory>

using namespace std;
typedef shared_ptr<ASTNode> PASTNode;

void do_semantics(PASTNode root){
  shared_ptr<ProgASTNode> program = dynamic_pointer_cast<ProgASTNode>(root);
  
  ScopeStack sta;
  sta.push();
  sta.declare_func("print");
  sta.get_func("print");

  program->check_semantics(sta);

  sta.get_func("main");
}
