#pragma once

#include <Parser.hpp>

class IRGenerator {
private:
  Parser parser;

public:
  IRGenerator(Parser &parser);
};
