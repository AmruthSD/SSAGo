#pragma once
#include <istream>
#include <string>
#include <unordered_map>

enum TOKEN_TYPE {
  // Identifiers & keywords
  IDENTIFIER,
  DATATYPE_INT,
  DATATYPE_FLOAT,

  // Literals
  INTEGER_LITERAL,
  FLOAT_LITERAL,

  // Operators
  PLUS,     // +
  MINUS,    // -
  ASTERISK, // *
  SLASH,    // /
  ASSIGN,   // =

  EQUAL,     // ==
  NOT_EQUAL, // !=
  LESS,      // <
  GREATER,   // >

  // Delimiters
  LPAREN,    // (
  RPAREN,    // )
  SEMICOLON, // ;

  END_OF_FILE,
  UNKNOWN
};

struct Token {
  std::string lexeme;
  TOKEN_TYPE type;
};

class Lexer {
private:
  std::istream &input;
  char currentChar;

  const std::unordered_map<std::string, TOKEN_TYPE> keywords = {
      {"int", DATATYPE_INT}, {"float", DATATYPE_FLOAT}};

  void advance();
  void skipWhitespace();

public:
  Lexer(std::istream &in);
  Token nextToken();
};