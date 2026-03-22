#include <Parser.hpp>

std::unique_ptr<Statement> Parser::parseGoStatement() {
  advance();
  std::string func = currentToken.lexeme;
  expect(TOKEN_TYPE::IDENTIFIER, "after go we need function identifier");
  expect(TOKEN_TYPE::LPAREN, "( in the function call");

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
  expect(TOKEN_TYPE::SEMICOLON, "after func call in go we need ;");
  return std::make_unique<GoStmt>(func, std::move(args));
}