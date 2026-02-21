#pragma once

#include <Lexer.hpp>
#include <memory>
#include <string>
#include <vector>

class SemanticAnalyser;
enum class DATA_TYPE : int;

class ASTNode {
public:
  virtual ~ASTNode() = default;
};

class Statement;
class Expr;

class Program : public ASTNode {
public:
  std::vector<std::unique_ptr<Statement>> statements;

  Program();
  explicit Program(std::vector<std::unique_ptr<Statement>> stmts);

  void analyse(SemanticAnalyser &analyser);
};

class Statement : public ASTNode {
public:
  virtual ~Statement() = default;
  virtual void analyse(SemanticAnalyser &analyser) = 0;
};

class ExpressionStmt : public Statement {
public:
  std::unique_ptr<Expr> expression;

  explicit ExpressionStmt(std::unique_ptr<Expr> expr);
  void analyse(SemanticAnalyser &analyser) override;
};

class Expr : public ASTNode {
public:
  virtual ~Expr() = default;
  virtual DATA_TYPE analyse(SemanticAnalyser &analyser) = 0;
};

class BinaryExpr : public Expr {
public:
  TOKEN_TYPE op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  BinaryExpr(TOKEN_TYPE oper, std::unique_ptr<Expr> lhs,
             std::unique_ptr<Expr> rhs);

  DATA_TYPE analyse(SemanticAnalyser &analyser) override;
};

class NumberExpr : public Expr {
public:
  std::string value;

  explicit NumberExpr(const std::string &val);
  DATA_TYPE analyse(SemanticAnalyser &analyser) override;
};

class VariableExpr : public Expr {
public:
  std::string name;

  explicit VariableExpr(const std::string &n);
  DATA_TYPE analyse(SemanticAnalyser &analyser) override;
};

class DeclarationStmt : public Statement {
public:
  std::string identifier;
  DATA_TYPE dataType;
  std::unique_ptr<Expr> expr;

  DeclarationStmt(std::string identifier, DATA_TYPE dataType,
                  std::unique_ptr<Expr> expr);

  void analyse(SemanticAnalyser &analyser) override;
};