#include <AST.hpp>
#include <Parser.hpp>

Parser::Parser(Lexer &lexer) : lexer(lexer) { advance(); }

void Parser::advance() { currentToken = lexer.nextToken(); }

bool Parser::match(TOKEN_TYPE type) {
  if (currentToken.type == type) {
    advance();
    return true;
  }
  return false;
}

void Parser::expect(TOKEN_TYPE type, const std::string &message) {
  if (currentToken.type != type) {
    throw std::runtime_error("Parse error: " + message);
  }
  advance();
}

std::unique_ptr<Program> Parser::parse() { return parseProgram(); }

std::unique_ptr<Program> Parser::parseProgram() {
  auto program = std::make_unique<Program>();

  while (currentToken.type != TOKEN_TYPE::END_OF_FILE) {
    program->statements.push_back(parseStatement());
  }

  return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {

  if (dataTypeFromToken.find(currentToken.type) != dataTypeFromToken.end()) {
    return parseDeclarationStatement();
  } else if (currentToken.type == TOKEN_TYPE::FUNCTION) {
    return parseFunctionStatement();
  } else if (currentToken.type == TOKEN_TYPE::RETURN) {
    return parseReturnStatement();
  }

  return parseExpressionStatement();
}

std::unique_ptr<Statement> Parser::parseDeclarationStatement() {

  TOKEN_TYPE dataType = currentToken.type;
  advance();

  std::string varName = currentToken.lexeme;
  expect(TOKEN_TYPE::IDENTIFIER, "Expected variable name");

  std::unique_ptr<Expr> initializer = nullptr;

  if (match(TOKEN_TYPE::ASSIGN)) {
    initializer = parseExpression();
  }

  expect(TOKEN_TYPE::SEMICOLON, "Expected ';' after declaration");

  return std::make_unique<DeclarationStmt>(
      varName, (*(dataTypeFromToken.find(dataType))).second,
      std::move(initializer));
}
