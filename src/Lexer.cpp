#include "Lexer.hpp"
#include <cctype>

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
    return {"", END_OF_FILE};

  if (std::isalpha(currentChar) || currentChar == '_') {
    std::string lexeme;

    while (std::isalnum(currentChar) || currentChar == '_') {
      lexeme += currentChar;
      advance();
    }

    auto it = keywords.find(lexeme);
    if (it != keywords.end())
      return {lexeme, it->second};
    return {lexeme, IDENTIFIER};
  }

  if (std::isdigit(currentChar)) {
    std::string lexeme;

    while (std::isdigit(currentChar)) {
      lexeme += currentChar;
      advance();
    }

    return {lexeme, INTEGER_LITERAL};
  }

  switch (currentChar) {

  case '+':
    advance();
    return {"+", PLUS};

  case '-':
    advance();
    return {"-", MINUS};

  case '*':
    advance();
    return {"*", ASTERISK};

  case '/':
    advance();
    return {"/", SLASH};

  case '=': {
    advance();
    if (currentChar == '=') {
      advance();
      return {"==", EQUAL};
    }
    return {"=", ASSIGN};
  }

  case '!': {
    advance();
    if (currentChar == '=') {
      advance();
      return {"!=", NOT_EQUAL};
    }
    return {"!", UNKNOWN};
  }

  case '<':
    advance();
    return {"<", LESS};

  case '>':
    advance();
    return {">", GREATER};

  case '(':
    advance();
    return {"(", LPAREN};

  case ')':
    advance();
    return {")", RPAREN};

  case ';':
    advance();
    return {";", SEMICOLON};
  }

  char unknownChar = currentChar;
  advance();
  return {std::string(1, unknownChar), UNKNOWN};
}