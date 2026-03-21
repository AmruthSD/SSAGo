#pragma once

#include <Lexer.hpp>
#include <Type.hpp>
#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <vector>

class SemanticAnalyser;
class IRGenerator;

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
  llvm::Value *codegen(IRGenerator &irGen);
};

class Statement : public ASTNode {
public:
  virtual ~Statement() = default;
  virtual void analyse(SemanticAnalyser &analyser) = 0;
  virtual llvm::Value *codegen(IRGenerator &irGen) = 0;
};

class ExpressionStmt : public Statement {
public:
  std::unique_ptr<Expr> expression;

  explicit ExpressionStmt(std::unique_ptr<Expr> expr);
  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class Expr : public ASTNode {
public:
  Type *dataType;
  virtual ~Expr() = default;
  virtual Type *analyse(SemanticAnalyser &analyser) = 0;
  virtual llvm::Value *codegen(IRGenerator &irGen) = 0;
  virtual llvm::Value *codegenLValue(IRGenerator &irGen) {
    throw std::runtime_error("LValue codegen not allowed");
  };

  virtual bool isLValue() { return false; }
};

class BinaryExpr : public Expr {
public:
  TOKEN_TYPE op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  BinaryExpr(TOKEN_TYPE oper, std::unique_ptr<Expr> lhs,
             std::unique_ptr<Expr> rhs);

  Type *analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class LiteralExpr : public Expr {
public:
  std::string value;

  explicit LiteralExpr(const std::string &val, Type *dataType);
  Type *analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class CastExpr : public Expr {
public:
  std::unique_ptr<Expr> expr = nullptr;

  explicit CastExpr(std::unique_ptr<Expr> expr, Type *dataType);
  Type *analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class VariableExpr : public Expr {
public:
  std::string name;

  explicit VariableExpr(const std::string &n);
  Type *analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
  llvm::Value *codegenLValue(IRGenerator &irGen) override;
  bool isLValue() override;
};

class CallExpr : public Expr {
public:
  std::unique_ptr<Expr> callee;
  std::vector<std::unique_ptr<Expr>> arguments;

  CallExpr(std::unique_ptr<Expr> callee,
           std::vector<std::unique_ptr<Expr>> args);
  Type *analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class UnaryExpr : public Expr {
public:
  TOKEN_TYPE op;
  std::unique_ptr<Expr> operand;

  UnaryExpr(TOKEN_TYPE op, std::unique_ptr<Expr> operand);
  Type *analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
  llvm::Value *codegenLValue(IRGenerator &irGen) override;
  bool isLValue() override;
};

class SizeofExpr : public Expr {
public:
  SizeofExpr(Type *);
  Type *analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class DeclarationStmt : public Statement {
public:
  std::string identifier;
  Type *dataType;
  std::unique_ptr<Expr> expr = nullptr;

  DeclarationStmt(std::string identifier, Type *dataType,
                  std::unique_ptr<Expr> expr);

  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class BlockStmt : public Statement {
public:
  std::vector<std::unique_ptr<Statement>> body;

  BlockStmt(std::vector<std::unique_ptr<Statement>> body);

  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class FunctionStmt : public Statement {
public:
  std::string identifier;
  Type *dataType;
  std::vector<std::pair<std::string, Type *>> arguments;
  std::unique_ptr<BlockStmt> body;
  FunctionStmt(std::string identifier, Type *dataType,
               std::unique_ptr<BlockStmt> body,
               std::vector<std::pair<std::string, Type *>> args);

  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class ReturnStmt : public Statement {
public:
  std::unique_ptr<Expr> expr;

  ReturnStmt(std::unique_ptr<Expr> expr);
  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class IfStmt : public Statement {
public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Statement> thenBranch;
  std::unique_ptr<Statement> elseBranch;

  IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Statement> thenBranch,
         std::unique_ptr<Statement> elseBranch);
  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class WhileStmt : public Statement {
public:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Statement> thenBranch;

  WhileStmt(std::unique_ptr<Expr> condition,
            std::unique_ptr<Statement> thenBranch);
  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class BreakStmt : public Statement {
public:
  BreakStmt() {}
  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class ContinueStmt : public Statement {
public:
  ContinueStmt() {}
  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};

class GoStmt : public Statement {
public:
  std::string callee;
  std::vector<std::unique_ptr<Expr>> arguments;

  GoStmt(std::string callee, std::vector<std::unique_ptr<Expr>> args);
  void analyse(SemanticAnalyser &analyser) override;
  llvm::Value *codegen(IRGenerator &irGen) override;
};