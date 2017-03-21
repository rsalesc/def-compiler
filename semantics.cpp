#include "scope.hpp"
#include "ast.cpp"
#include <memory>

using namespace std;
typedef shared_ptr<ASTNode> PASTNode;

void do_semantics(PASTNode root){
  shared_ptr<ProgASTNode> program = dynamic_pointer_cast<ProgASTNode>(root);

  Code code;
  ScopeStack sta;

  program->check_and_generate(code, sta);

  code.print();
}
