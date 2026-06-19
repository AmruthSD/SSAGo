#include <Lexer.hpp>
#include <cctype>
#include <iostream>

Lexer::Lexer(std::istream &in) : input(in), currentChar(0) { advance(); }

void Lexer::advance() { currentChar = input.get(); }

void Lexer::skipWhitespace() {
  while (std::isspace(currentChar)) {
    advance();
  }
}

Token Lexer::nextToken() {
  skipWhitespace();

  if (input.eof())
    return {"", TOKEN_TYPE::END_OF_FILE};

  if (std::isalpha(currentChar) || currentChar == '_') {
    std::string lexeme;

    while (std::isalnum(currentChar) || currentChar == '_') {
      lexeme += currentChar;
      advance();
    }

    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
      return {lexeme, it->second};
    }
    return {lexeme, TOKEN_TYPE::IDENTIFIER};
  }

  if (std::isdigit(currentChar)) {
    std::string lexeme;
    bool isFloat = false;

    while (std::isdigit(currentChar)) {
      lexeme += currentChar;
      advance();
    }

    if (currentChar == '.') {
      isFloat = true;
      lexeme += currentChar;
      advance();

      if (!std::isdigit(currentChar)) {
        throw std::runtime_error("Invalid float literal");
      }

      while (std::isdigit(currentChar)) {
        lexeme += currentChar;
        advance();
      }
    }

    if (isFloat) {
      return {lexeme, TOKEN_TYPE::FLOAT_LITERAL};
    } else {
      return {lexeme, TOKEN_TYPE::INTEGER_LITERAL};
    }
  }

  switch (currentChar) {

  case '+':
    advance();
    return {"+", TOKEN_TYPE::PLUS};

  case '-':
    advance();
    return {"-", TOKEN_TYPE::MINUS};

  case '*':
    advance();
    return {"*", TOKEN_TYPE::ASTERISK};

  case '/':
    advance();
    return {"/", TOKEN_TYPE::SLASH};

  case '=': {
    advance();
    if (currentChar == '=') {
      advance();
      return {"==", TOKEN_TYPE::EQUAL};
    }
    return {"=", TOKEN_TYPE::ASSIGN};
  }

  case '!': {
    advance();
    if (currentChar == '=') {
      advance();
      return {"!=", TOKEN_TYPE::NOT_EQUAL};
    }
    return {"!", TOKEN_TYPE::UNKNOWN};
  }

  case '<':
    advance();
    return {"<", TOKEN_TYPE::LESS};

  case '>':
    advance();
    return {">", TOKEN_TYPE::GREATER};

  case '(':
    advance();
    return {"(", TOKEN_TYPE::LPAREN};

  case ')':
    advance();
    return {")", TOKEN_TYPE::RPAREN};

  case ';':
    advance();
    return {";", TOKEN_TYPE::SEMICOLON};

  case ',':
    advance();
    return {",", TOKEN_TYPE::COMMA};

  case '{':
    advance();
    return {"{", TOKEN_TYPE::LCURLY};

  case '}':
    advance();
    return {"}", TOKEN_TYPE::RCURLY};

  case '|': {
    advance();
    if (currentChar == '|') {
      advance();
      return {"||", TOKEN_TYPE::OR};
    }
    return {"|", TOKEN_TYPE::UNKNOWN};
  }

  case '&': {
    advance();
    if (currentChar == '&') {
      advance();
      return {"&&", TOKEN_TYPE::OR};
    }
    return {"&", TOKEN_TYPE::AMPERSAND};
  }
  }

  if (currentChar == '"') {
    std::string lexeme;
    advance();

    while (!input.eof() && currentChar != '"') {

      if (currentChar == '\\') {
        advance();

        switch (currentChar) {
        case 'n':
          lexeme += '\n';
          break;

        case 't':
          lexeme += '\t';
          break;

        case 'r':
          lexeme += '\r';
          break;

        case '\\':
          lexeme += '\\';
          break;

        case '"':
          lexeme += '"';
          break;

        default:
          lexeme += currentChar;
          break;
        }

        advance();
        continue;
      }

      lexeme += currentChar;
      advance();
    }

    if (currentChar == '"') {
      advance();
      return {lexeme, TOKEN_TYPE::STRING_LITERAL};
    }

    return {lexeme, TOKEN_TYPE::UNKNOWN};
  }
  char unknownChar = currentChar;
  advance();
  return {std::string(1, unknownChar), TOKEN_TYPE::UNKNOWN};
}