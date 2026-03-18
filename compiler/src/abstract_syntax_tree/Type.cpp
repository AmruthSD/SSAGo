#include <Type.hpp>

Type *intType = new Type{DATA_TYPE::DATATYPE_INT, nullptr};
Type *floatType = new Type{DATA_TYPE::DATATYPE_FLOAT, nullptr};

Type *makePointerType(Type *base) {
  return new Type{DATA_TYPE::DATATYPE_POINTER, base};
}

bool areTypesEqual(Type *a, Type *b) {
  if (!a || !b)
    return false;

  if (a->base != b->base)
    return false;

  if (a->base == DATA_TYPE::DATATYPE_POINTER) {
    return areTypesEqual(a->pointee, b->pointee);
  }

  return true;
}

bool isNumeric(Type *t) {
  return t->base == DATA_TYPE::DATATYPE_INT ||
         t->base == DATA_TYPE::DATATYPE_FLOAT;
}

bool isPointer(Type *t) { return t->base == DATA_TYPE::DATATYPE_POINTER; }
