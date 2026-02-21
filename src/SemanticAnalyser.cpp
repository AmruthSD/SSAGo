#include <SemanticAnalyser.hpp>

SemanticAnalyser::SemanticAnalyser(Parser &parser) : parser(parser) {
  ast = parser.parse();
}

void SemanticAnalyser::analyseAST() { analyseProgram(ast.get()); }

void SemanticAnalyser::analyseProgram(Program *program) {
  for (auto &stmt : program->statements) {
    stmt.get()->analyse(*this);
  }
}

void SemanticAnalyser::analyseExpressionStmt(ExpressionStmt *stmt) {
  stmt->expression.get()->analyse(*this);
}

DATA_TYPE SemanticAnalyser::analyseNumberExpr(NumberExpr *expr) {
  return DATA_TYPE::DATATYPE_INT;
}

DATA_TYPE SemanticAnalyser::analyseVariableExpr(VariableExpr *expr) {
  if (symbolTable.find(expr->name) == symbolTable.end())
    throw std::runtime_error("Unknown variable found " + expr->name);

  return symbolTable[expr->name].dataType;
}

DATA_TYPE SemanticAnalyser::analyseBinaryExpr(BinaryExpr *expr) {

  DATA_TYPE left_type = expr->left->analyse(*this);
  DATA_TYPE right_type = expr->right->analyse(*this);

  switch (expr->op) {

  case TOKEN_TYPE::PLUS:
  case TOKEN_TYPE::MINUS:
  case TOKEN_TYPE::ASTERISK:
  case TOKEN_TYPE::SLASH: {

    if (left_type != right_type) {
      throw std::runtime_error("Type mismatch in arithmetic expression");
    }

    if (left_type != DATA_TYPE::DATATYPE_INT &&
        left_type != DATA_TYPE::DATATYPE_FLOAT) {
      throw std::runtime_error("Invalid operand type for arithmetic operator");
    }

    return left_type;
  }

  case TOKEN_TYPE::ASSIGN: {

    if (!dynamic_cast<VariableExpr *>(expr->left.get())) {
      throw std::runtime_error("Left side of assignment must be a variable");
    }

    if (left_type != right_type) {
      throw std::runtime_error("Type mismatch in assignment");
    }

    return left_type;
  }

  case TOKEN_TYPE::EQUAL:
  case TOKEN_TYPE::NOT_EQUAL:
  case TOKEN_TYPE::LESS:
  case TOKEN_TYPE::GREATER: {

    if (left_type != right_type) {
      throw std::runtime_error("Type mismatch in comparison");
    }

    if (left_type != DATA_TYPE::DATATYPE_INT &&
        left_type != DATA_TYPE::DATATYPE_FLOAT) {
      throw std::runtime_error("Invalid operand type for comparison");
    }

    return DATA_TYPE::DATATYPE_INT;
  }

  default:
    throw std::runtime_error("Unknown binary operator");
  }
}

void SemanticAnalyser::analyseDeclarationStmt(DeclarationStmt *stmt) {
  if (symbolTable.find(stmt->identifier) != symbolTable.end())
    throw std::runtime_error("Identifier exists");

  if (stmt->expr != nullptr) {
    DATA_TYPE right_type = stmt->expr.get()->analyse(*this);
    if (stmt->dataType != right_type) {
      throw std::runtime_error("Improper Assignment Operation");
    }
  }

  symbolTable[stmt->identifier] = {stmt->dataType};
}