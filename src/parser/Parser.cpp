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

Type *Parser::parseType() {
  TOKEN_TYPE baseToken = currentToken.type;
  advance();

  Type *type = new Type{dataTypeFromToken.at(baseToken)};
  while (match(TOKEN_TYPE::ASTERISK)) {
    type = makePointerType(type);
  }

  return type;
}

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
  } else if (currentToken.type == TOKEN_TYPE::IF) {
    return parseIfElseStatement();
  } else if (currentToken.type == TOKEN_TYPE::LCURLY) {
    return parseBlockStatement();
  } else if (currentToken.type == TOKEN_TYPE::WHILE) {
    return parseWhileStatement();
  } else if (currentToken.type == TOKEN_TYPE::BREAK) {
    return parseBreakStatement();
  } else if (currentToken.type == TOKEN_TYPE::CONTINUE) {
    return parseContinueStatement();
  }

  return parseExpressionStatement();
}

std::unique_ptr<Statement> Parser::parseDeclarationStatement() {

  Type *type = parseType();

  std::string varName = currentToken.lexeme;
  expect(TOKEN_TYPE::IDENTIFIER, "Expected variable name");

  std::unique_ptr<Expr> initializer = nullptr;
  if (match(TOKEN_TYPE::ASSIGN)) {
    initializer = parseExpression();
  }

  expect(TOKEN_TYPE::SEMICOLON, "Expected ';' after declaration");

  return std::make_unique<DeclarationStmt>(varName, type,
                                           std::move(initializer));
}
