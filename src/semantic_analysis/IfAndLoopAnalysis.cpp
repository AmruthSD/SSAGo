#include <SemanticAnalyser.hpp>

void SemanticAnalyser::analyseIfElse(IfStmt *stmt) {
  Type *type = stmt->condition->analyse(*this);

  stmt->thenBranch->analyse(*this);
  stmt->elseBranch->analyse(*this);
}

void SemanticAnalyser::analyseWhile(WhileStmt *stmt) {
  loop_number++;
  Type *type = stmt->condition->analyse(*this);

  stmt->thenBranch->analyse(*this);
  loop_number--;
}

void SemanticAnalyser::analyseBreak(BreakStmt *stmt) {
  if (loop_number == 0) {
    throw std::runtime_error("No Break Outside loops");
  }
}
void SemanticAnalyser::analyseContinue(ContinueStmt *stmt) {
  if (loop_number == 0) {
    throw std::runtime_error("No Continue Outside loops");
  }
}