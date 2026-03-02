#include <Parser.hpp>

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
  auto expr = parseExpression();

  if (currentToken.type == TOKEN_TYPE ::SEMICOLON)
    advance();

  return std::make_unique<ExpressionStmt>(std::move(expr));
}

enum Precedence {
  LOWEST = 0,
  ASSIGN,
  OR,
  AND,
  EQUALITY,
  COMPARISON,
  SUM,
  PRODUCT,
  CALL,
};

int getPrecedence(TOKEN_TYPE type) {
  switch (type) {
  case TOKEN_TYPE::ASSIGN:
    return Precedence::ASSIGN;

  case TOKEN_TYPE::OR:
    return Precedence::OR;

  case TOKEN_TYPE::AND:
    return Precedence::AND;

  case TOKEN_TYPE::EQUAL:
    return Precedence::EQUALITY;

  case TOKEN_TYPE::GREATER:
  case TOKEN_TYPE::LESS:
    return Precedence::COMPARISON;

  case TOKEN_TYPE::PLUS:
  case TOKEN_TYPE::MINUS:
    return Precedence::SUM;

  case TOKEN_TYPE::ASTERISK:
  case TOKEN_TYPE::SLASH:
    return Precedence::PRODUCT;

  case TOKEN_TYPE::LPAREN:
    return Precedence::CALL;

  default:
    return Precedence::LOWEST;
  }
}

std::unique_ptr<Expr> Parser::parseExpression(int precedence) {

  std::unique_ptr<Expr> left;

  switch (currentToken.type) {

  case TOKEN_TYPE::IDENTIFIER: {
    left = std::make_unique<VariableExpr>(currentToken.lexeme);
    advance();
    break;
  }

  case TOKEN_TYPE::INTEGER_LITERAL:
  case TOKEN_TYPE::STRING_LITERAL:
  case TOKEN_TYPE::FLOAT_LITERAL: {
    left = std::make_unique<LiteralExpr>(
        currentToken.lexeme, dataTypeFromToken.at(currentToken.type));
    advance();
    break;
  }

  case TOKEN_TYPE::LPAREN: {
    advance();
    left = parseExpression(Precedence::LOWEST);
    if (currentToken.type != TOKEN_TYPE::RPAREN)
      throw std::runtime_error("Expected ')'");
    advance();
    break;
  }

  default:
    throw std::runtime_error("Unexpected token in expression " +
                             currentToken.lexeme);
  }

  while (true) {

    if (currentToken.type == TOKEN_TYPE::LPAREN) {

      int tokenPrecedence = getPrecedence(currentToken.type);
      if (precedence >= tokenPrecedence)
        break;

      advance();

      std::vector<std::unique_ptr<Expr>> args;

      if (currentToken.type != TOKEN_TYPE::RPAREN) {
        while (true) {
          args.push_back(parseExpression(Precedence::LOWEST));

          if (currentToken.type == TOKEN_TYPE::COMMA) {
            advance();
            continue;
          }
          break;
        }
      }

      if (currentToken.type != TOKEN_TYPE::RPAREN)
        throw std::runtime_error("Expected ')' in function call");

      advance();

      left = std::make_unique<CallExpr>(std::move(left), std::move(args));
      continue;
    }
    int tokenPrecedence = getPrecedence(currentToken.type);
    if (precedence >= tokenPrecedence)
      break;

    TOKEN_TYPE op = currentToken.type;
    advance();

    int nextPrecedence = tokenPrecedence;
    if (op == TOKEN_TYPE::ASSIGN)
      nextPrecedence = tokenPrecedence - 1;

    auto right = parseExpression(nextPrecedence);

    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }

  return left;
}