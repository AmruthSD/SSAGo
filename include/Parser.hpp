#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include <AST.hpp>
#include <Lexer.hpp>

class Parser {
private:
  Lexer &lexer;
  Token currentToken;

  void advance();
  bool match(TOKEN_TYPE type);
  void expect(TOKEN_TYPE type, const std::string &message);

  std::unique_ptr<Program> parseProgram();
  std::unique_ptr<Statement> parseStatement();
  std::unique_ptr<Statement> parseExpressionStatement();
  std::unique_ptr<Expr> parseExpression(int precedence = 0);

public:
  Parser(Lexer &lexer);

  std::unique_ptr<Program> parse();
};