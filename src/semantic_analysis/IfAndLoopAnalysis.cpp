#include <SemanticAnalyser.hpp>

void SemanticAnalyser::analyseIfElse(IfStmt *stmt) {
  DATA_TYPE type = stmt->condition->analyse(*this);

  stmt->thenBranch->analyse(*this);
  stmt->elseBranch->analyse(*this);
}

void SemanticAnalyser::analyseWhile(WhileStmt *stmt) {
  DATA_TYPE type = stmt->condition->analyse(*this);

  stmt->thenBranch->analyse(*this);
}