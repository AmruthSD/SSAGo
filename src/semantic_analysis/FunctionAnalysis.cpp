#include <SemanticAnalyser.hpp>

void SemanticAnalyser::analyseFunctionStmt(FunctionStmt *func) {
  if (symbolTable.find(func->identifier) != symbolTable.end())
    throw std::runtime_error("function identifier is used " + func->identifier);

  symbolTable[func->identifier] = {func->dataType, true};
  current_function_type = func->dataType;

  for (auto param : func->arguments) {
    if (symbolTable.find(param.first) != symbolTable.end())
      throw std::runtime_error("Argument identifier is being used");
    symbolTable[param.first] = {param.second};
  }

  for (auto param : func->arguments) {
    symbolTable.erase(param.first);
  }
}

void SemanticAnalyser::analyseBlock(BlockStmt *stmt) {
  block_number++;
  for (int i = 0; i < stmt->body.size(); i++) {
    stmt->body[i]->analyse(*this);
  }

  for (auto i : symbolTable) {
    if (i.second.block_number == block_number) {
      symbolTable.erase(i.first);
    }
  }
  block_number--;
}

void SemanticAnalyser::analyseReturn(ReturnStmt *stmt) {
  if (block_number == 0)
    throw std::runtime_error("Return cant be in the global scope");

  DATA_TYPE exprDataType = stmt->expr->analyse(*this);
  if (exprDataType != current_function_type) {
    stmt->expr = std::make_unique<CastExpr>(std::move(stmt->expr),
                                            current_function_type);
  }
}