#pragma once

enum class DATA_TYPE {
  DATATYPE_FLOAT,
  DATATYPE_INT,
  DATATYPE_STRING,
  DATATYPE_VOID,
  DATATYPE_POINTER,
  DATATYPE_WAITGROUP
};

struct Type {
  DATA_TYPE base;

  Type *pointee;
};

Type *makePointerType(Type *base);

bool areTypesEqual(Type *a, Type *b);

bool isNumeric(Type *t);

bool isPointer(Type *t);

extern Type *intType;
extern Type *floatType;