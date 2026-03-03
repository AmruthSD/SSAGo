#include <Parser.hpp>

std::unique_ptr<Statement> Parser::parseIfElseStatement() {
  advance();
  expect(TOKEN_TYPE::LPAREN, "start of condition");
  std::unique_ptr<Expr> ifexpr = parseExpression();
  expect(TOKEN_TYPE::RPAREN, "end of condition");

  std::unique_ptr<Statement> ifstmt = parseStatement();
  std::unique_ptr<Statement> elsestmt = nullptr;
  if (match(TOKEN_TYPE::ELSE)) {
    elsestmt = parseStatement();
  }

  return std::make_unique<IfStmt>(std::move(ifexpr), std::move(ifstmt),
                                  std::move(elsestmt));
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
  advance();
  expect(TOKEN_TYPE::LPAREN, "start of condition");
  std::unique_ptr<Expr> conditionexpr = parseExpression();
  expect(TOKEN_TYPE::RPAREN, "end of condition");
  std::unique_ptr<Statement> thenblock = parseStatement();

  return std::make_unique<WhileStmt>(std::move(conditionexpr),
                                     std::move(thenblock));
}

std::unique_ptr<Statement> Parser::parseBreakStatement() {
  expect(TOKEN_TYPE::BREAK, "Expected break");
  expect(TOKEN_TYPE::SEMICOLON, "Semicolon after break");
  return std::make_unique<BreakStmt>();
}

std::unique_ptr<Statement> Parser::parseContinueStatement() {
  expect(TOKEN_TYPE::CONTINUE, "Expected break");
  expect(TOKEN_TYPE::SEMICOLON, "Semicolon after break");
  return std::make_unique<ContinueStmt>();
}