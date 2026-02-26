#pragma once
#include <istream>
#include <string>
#include <unordered_map>

enum class TOKEN_TYPE {
  // Identifiers & keywords
  IDENTIFIER,
  DATATYPE_INT,
  DATATYPE_FLOAT,
  DATATYPE_STRING,
  FUNCTION,

  // Literals
  INTEGER_LITERAL,
  FLOAT_LITERAL,
  STRING_LITERAL,

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
      {"int", TOKEN_TYPE::DATATYPE_INT},
      {"float", TOKEN_TYPE::DATATYPE_FLOAT},
      {"string", TOKEN_TYPE::DATATYPE_STRING},
      {"functions", TOKEN_TYPE::FUNCTION}};

  void advance();
  void skipWhitespace();

public:
  Lexer(std::istream &in);
  Token nextToken();
};