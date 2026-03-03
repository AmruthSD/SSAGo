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
  DATATYPE_VOID,
  FUNCTION,
  RETURN,
  IF,
  ELSE,
  WHILE,
  BREAK,
  CONTINUE,

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
  OR,        // ||
  AND,       // &&

  // Delimiters
  LPAREN,    // (
  RPAREN,    // )
  SEMICOLON, // ;
  COMMA,     // ,
  LCURLY,    // {
  RCURLY,    // }

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
      {"function", TOKEN_TYPE::FUNCTION},
      {"void", TOKEN_TYPE::DATATYPE_VOID},
      {"return", TOKEN_TYPE::RETURN},
      {"if", TOKEN_TYPE::IF},
      {"else", TOKEN_TYPE::ELSE},
      {"while", TOKEN_TYPE::WHILE},
      {"continue", TOKEN_TYPE::CONTINUE},
      {"break", TOKEN_TYPE::BREAK}};

  void advance();
  void skipWhitespace();

public:
  Lexer(std::istream &in);
  Token nextToken();
};