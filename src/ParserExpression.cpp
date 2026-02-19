#include <Parser.hpp>

enum Precedence {
  LOWEST = 0,
  SUM,
  PRODUCT,
};

int getPrecedence(TOKEN_TYPE type) {
  switch (type) {
  case TOKEN_TYPE::PLUS:
  case TOKEN_TYPE::MINUS:
    return Precedence::SUM;

  case TOKEN_TYPE::ASTERISK:
  case TOKEN_TYPE::SLASH:
    return Precedence::PRODUCT;

  default:
    return Precedence::LOWEST;
  }
}

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
  auto expr = parseExpression();

  if (currentToken.type == TOKEN_TYPE ::SEMICOLON)
    advance();

  return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::parseExpression(int precedence) {

  std::unique_ptr<Expr> left;

  switch (currentToken.type) {

  case TOKEN_TYPE ::IDENTIFIER: {
    left = std::make_unique<VariableExpr>(currentToken.lexeme);
    advance();
    break;
  }

  case TOKEN_TYPE ::INTEGER_LITERAL: {
    left = std::make_unique<NumberExpr>((currentToken.lexeme));
    advance();
    break;
  }

  case TOKEN_TYPE ::LPAREN: {
    advance();
    left = parseExpression(Precedence::LOWEST);
    if (currentToken.type != TOKEN_TYPE ::RPAREN)
      throw std::runtime_error("Expected ')'");
    advance();
    break;
  }

  default:
    throw std::runtime_error("Unexpected token in expression");
  }

  while (precedence < getPrecedence(currentToken.type)) {

    TOKEN_TYPE op = currentToken.type;
    int opPrecedence = getPrecedence(op);

    advance();

    auto right = parseExpression(opPrecedence);

    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }

  return left;
}