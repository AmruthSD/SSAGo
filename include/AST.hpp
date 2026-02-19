#pragma once

#include <Lexer.hpp>
#include <memory>
#include <string>
#include <vector>

class ASTNode {
public:
  virtual ~ASTNode() = default;
};

class Program;
class Statement;
class Expr;

class Program : public ASTNode {
public:
  std::vector<std::unique_ptr<Statement>> statements;

  Program() = default;

  explicit Program(std::vector<std::unique_ptr<Statement>> stmts)
      : statements(std::move(stmts)) {}
};

class Statement : public ASTNode {
public:
  virtual ~Statement() = default;
};

class ExpressionStmt : public Statement {
public:
  std::unique_ptr<Expr> expression;

  explicit ExpressionStmt(std::unique_ptr<Expr> expr)
      : expression(std::move(expr)) {}
};

class Expr : public ASTNode {
public:
  virtual ~Expr() = default;
};

class BinaryExpr : public Expr {
public:
  TOKEN_TYPE op;

  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  BinaryExpr(TOKEN_TYPE oper, std::unique_ptr<Expr> lhs,
             std::unique_ptr<Expr> rhs)
      : op(oper), left(std::move(lhs)), right(std::move(rhs)) {}
};

class NumberExpr : public Expr {
public:
  std::string value;

  explicit NumberExpr(const std::string &val) : value(val) {}
};

class VariableExpr : public Expr {
public:
  std::string name;

  explicit VariableExpr(const std::string &n) : name(n) {}
};
