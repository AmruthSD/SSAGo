#include <SemanticAnalyser.hpp>
#include <iostream>

SemanticAnalyser::SemanticAnalyser(Parser &parser) : parser(parser) {
  ast = parser.parse();
  std::cout << "AST built\n";
  analyseAST();
  std::cout << "Semantic Analysis Done\n";
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

Type *SemanticAnalyser::analyseLiteralExpr(LiteralExpr *expr) {
  return expr->dataType;
}

Type *SemanticAnalyser::analyseVariableExpr(VariableExpr *expr) {
  if (symbolTable.find(expr->name) == symbolTable.end())
    throw std::runtime_error("Unknown variable found " + expr->name);

  return expr->dataType = symbolTable[expr->name].dataType;
}

Type *SemanticAnalyser::analyseBinaryExpr(BinaryExpr *expr) {

  Type *leftType = expr->left->analyse(*this);
  Type *rightType = expr->right->analyse(*this);

  switch (expr->op) {

  case TOKEN_TYPE::PLUS:
  case TOKEN_TYPE::MINUS:
  case TOKEN_TYPE::ASTERISK:
  case TOKEN_TYPE::SLASH: {

    if (!isNumeric(leftType) || !isNumeric(rightType)) {
      throw std::runtime_error("Invalid operand type for arithmetic operator");
    }

    if (areTypesEqual(leftType, rightType)) {
      return expr->dataType = leftType;
    }

    if (areTypesEqual(leftType, intType) &&
        areTypesEqual(rightType, floatType)) {

      expr->left = std::make_unique<CastExpr>(std::move(expr->left), floatType);
      return expr->dataType = floatType;
    }

    if (areTypesEqual(leftType, floatType) &&
        areTypesEqual(rightType, intType)) {

      expr->right =
          std::make_unique<CastExpr>(std::move(expr->right), floatType);
      return expr->dataType = floatType;
    }

    throw std::runtime_error("Type mismatch in arithmetic expression");
  }

  case TOKEN_TYPE::ASSIGN: {

    if (!expr->left->isLValue()) {
      throw std::runtime_error("Left side of assignment must be assignable");
    }

    if (areTypesEqual(leftType, rightType)) {
      return expr->dataType = leftType;
    }

    if (areTypesEqual(leftType, floatType) &&
        areTypesEqual(rightType, intType)) {

      expr->right =
          std::make_unique<CastExpr>(std::move(expr->right), floatType);
      return expr->dataType = floatType;
    }

    if (areTypesEqual(leftType, intType) &&
        areTypesEqual(rightType, floatType)) {

      expr->right = std::make_unique<CastExpr>(std::move(expr->right), intType);
      return expr->dataType = intType;
    }

    if (isPointer(leftType) && isPointer(rightType)) {
      if (!areTypesEqual(leftType, rightType)) {
        throw std::runtime_error("Incompatible pointer assignment");
      }
      return expr->dataType = leftType;
    }

    throw std::runtime_error("Type mismatch in assignment");
  }

  case TOKEN_TYPE::EQUAL:
  case TOKEN_TYPE::NOT_EQUAL:
  case TOKEN_TYPE::LESS:
  case TOKEN_TYPE::GREATER:
  case TOKEN_TYPE::OR:
  case TOKEN_TYPE::AND: {

    if (!isNumeric(leftType) || !isNumeric(rightType)) {
      throw std::runtime_error("Invalid operand type for comparison");
    }

    if (!areTypesEqual(leftType, rightType)) {

      if (areTypesEqual(leftType, intType) &&
          areTypesEqual(rightType, floatType)) {

        expr->left =
            std::make_unique<CastExpr>(std::move(expr->left), floatType);

      } else if (areTypesEqual(leftType, floatType) &&
                 areTypesEqual(rightType, intType)) {

        expr->right =
            std::make_unique<CastExpr>(std::move(expr->right), floatType);

      } else {
        throw std::runtime_error("Type mismatch in comparison");
      }
    }

    return expr->dataType = intType;
  }

  default:
    throw std::runtime_error("Unknown binary operator");
  }
}

void SemanticAnalyser::analyseDeclarationStmt(DeclarationStmt *stmt) {
  if (symbolTable.find(stmt->identifier) != symbolTable.end())
    throw std::runtime_error("Identifier exists");

  if (stmt->expr != nullptr) {
    Type *right_type = stmt->expr.get()->analyse(*this);
    if (!areTypesEqual(right_type, stmt->dataType)) {
      throw std::runtime_error("Improper Assignment Operation");
    }
  }

  symbolTable[stmt->identifier] = {stmt->dataType, block_number};
}

Type *SemanticAnalyser::analyseUnaryExpr(UnaryExpr *expr) {
  Type *dataType = expr->operand.get()->analyse(*this);
  switch (expr->op) {
  case TOKEN_TYPE::ASTERISK: {
    if (dataType->base != DATA_TYPE::DATATYPE_POINTER) {
      throw std::runtime_error("Cant use * unary on a non pointer");
    }
    return dataType->pointee;
  }
  case TOKEN_TYPE::AMPERSAND: {
    if (!expr->operand->isLValue()) {
      throw std::runtime_error("Operand of & must be an lvalue");
    }

    Type *operandType = expr->operand->analyse(*this);

    return makePointerType(operandType);
  }
  default:
    throw std::runtime_error("Unsupported unary operator");
  }
}