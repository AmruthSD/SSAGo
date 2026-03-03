#pragma once

#include <AST.hpp>
#include <Lexer.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

enum class DATA_TYPE {
  DATATYPE_FLOAT,
  DATATYPE_INT,
  DATATYPE_STRING,
  DATATYPE_VOID
};

class Parser {
private:
  Lexer &lexer;
  Token currentToken;

  const std::map<TOKEN_TYPE, DATA_TYPE> dataTypeFromToken = {
      {TOKEN_TYPE::DATATYPE_VOID, DATA_TYPE::DATATYPE_VOID},
      {TOKEN_TYPE::DATATYPE_FLOAT, DATA_TYPE::DATATYPE_FLOAT},
      {TOKEN_TYPE::DATATYPE_INT, DATA_TYPE::DATATYPE_INT},
      {TOKEN_TYPE::DATATYPE_STRING, DATA_TYPE::DATATYPE_STRING},
      {TOKEN_TYPE::FLOAT_LITERAL, DATA_TYPE::DATATYPE_FLOAT},
      {TOKEN_TYPE::INTEGER_LITERAL, DATA_TYPE::DATATYPE_INT},
      {TOKEN_TYPE::STRING_LITERAL, DATA_TYPE::DATATYPE_STRING}};

  void advance();
  bool match(TOKEN_TYPE type);
  void expect(TOKEN_TYPE type, const std::string &message);

  std::unique_ptr<Program> parseProgram();
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<Statement> parseExpressionStatement();
  std::unique_ptr<Statement> parseFunctionStatement();
  std::unique_ptr<BlockStmt> parseBlockStatement();
  std::unique_ptr<Expr> parseExpression(int precedence = 0);
  std::unique_ptr<Statement> parseDeclarationStatement();
  std::unique_ptr<Statement> parseReturnStatement();
  std::unique_ptr<Statement> parseIfElseStatement();
  std::unique_ptr<Statement> parseWhileStatement();
  std::unique_ptr<Statement> parseBreakStatement();
  std::unique_ptr<Statement> parseContinueStatement();

public:
  Parser(Lexer &lexer);

  std::unique_ptr<Program> parse();
};