#include <Lexer.hpp>
#include <Parser.hpp>
#include <iostream>

std::unique_ptr<Statement> Parser::parseFunctionStatement() {
  advance();

  std::string functionName = currentToken.lexeme;
  expect(TOKEN_TYPE::IDENTIFIER, "Expected function name");

  expect(TOKEN_TYPE::LPAREN, "Expected '(' after function name");

  std::vector<std::pair<std::string, Type *>> params;
  while (currentToken.type != TOKEN_TYPE::RPAREN) {

    std::string paramName = currentToken.lexeme;
    expect(TOKEN_TYPE::IDENTIFIER, "Expected parameter name");

    TOKEN_TYPE tokenType = currentToken.type;
    if (dataTypeFromToken.find(tokenType) == dataTypeFromToken.end())
      throw std::runtime_error("Data Type of Parameter not present");

    Type *type = parseType();

    params.push_back({paramName, type});

    if (currentToken.type == TOKEN_TYPE::COMMA) {
      advance();
    }
  }

  expect(TOKEN_TYPE::RPAREN, "Expected ')' after parameters");

  TOKEN_TYPE tokenType = currentToken.type;
  if (dataTypeFromToken.find(tokenType) == dataTypeFromToken.end())
    throw std::runtime_error("Data Type of Parameter not present");
  Type *returnType = parseType();

  std::unique_ptr<BlockStmt> body = parseBlockStatement();

  return std::make_unique<FunctionStmt>(functionName, returnType,
                                        std::move(body), std::move(params));
}

std::unique_ptr<BlockStmt> Parser::parseBlockStatement() {
  expect(TOKEN_TYPE::LCURLY, "Expected { for the body of the function");

  std::vector<std::unique_ptr<Statement>> body;

  while (currentToken.type != TOKEN_TYPE::RCURLY) {
    auto stmt = parseStatement();
    body.push_back(std::move(stmt));
  }

  expect(TOKEN_TYPE::RCURLY, "Expected } for the body of the function");

  return std::make_unique<BlockStmt>(std::move(body));
}

std::unique_ptr<Statement> Parser::parseReturnStatement() {
  advance();
  auto expr = parseExpression();
  expect(TOKEN_TYPE::SEMICOLON, "Expected semicolon at end of return");
  return std::make_unique<ReturnStmt>(std::move(expr));
}