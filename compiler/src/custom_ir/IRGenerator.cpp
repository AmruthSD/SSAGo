#include <CustomIRGenerator.hpp>
#include <DominanceAnalysis.hpp>

custom_ir::IRGenerator::IRGenerator(SemanticAnalyser &semanticAnalyser,
                                    IRBuilder &irBuilder, IRPrinter &IRPrinter)
    : semanticAnalyser(semanticAnalyser), builder(irBuilder),
      printer(IRPrinter) {
  module = new ModuleIR();
  namedValues.emplace_back();
  std::cout << "Codegeneration is starting" << std::endl;
  semanticAnalyser.ast.get()->codegen(*this);
  std::cout << "Codegeneration is done" << std::endl;
  // printer.print(module);
}

custom_ir::Value *custom_ir::IRGenerator::generateProgram(Program *prog) {
  for (auto &stmt : prog->statements) {
    stmt->codegen(*this);
  }

  return nullptr;
}

custom_ir::Value *
custom_ir::IRGenerator::generateExpressionStmt(ExpressionStmt *stmt) {
  if (namedValues.size() == 1)
    throw std::runtime_error("Unable to have expression as global");

  return stmt->expression->codegen(*this);
}

custom_ir::Value *custom_ir::IRGenerator::generateLiteral(LiteralExpr *expr) {
  DATA_TYPE dataType = expr->dataType->base;
  Type *type = new Type{dataType, nullptr};
  switch (dataType) {
  case DATA_TYPE::DATATYPE_INT:
  case DATA_TYPE::DATATYPE_FLOAT:
  case DATA_TYPE::DATATYPE_STRING:
    return new Constant(type, expr->value);
  }

  throw std::runtime_error("Unknown literal type");
}

custom_ir::Value *custom_ir::IRGenerator::generateCast(CastExpr *expr) {
  custom_ir::Value *val = expr->expr->codegen(*this);
  if (!val)
    return nullptr;

  DATA_TYPE targetType = expr->dataType->base;
  DATA_TYPE oldType = val->dataType->base;
  if (oldType == targetType)
    return val;
  if (oldType == DATA_TYPE::DATATYPE_INT &&
      targetType == DATA_TYPE::DATATYPE_FLOAT) {

    return builder.CreateCast(
        val, new TempValue(new Type{targetType, nullptr}, "intToFloatTemp"));
  }
  if (oldType == DATA_TYPE::DATATYPE_FLOAT &&
      targetType == DATA_TYPE::DATATYPE_INT) {

    return builder.CreateCast(
        val, new TempValue(new Type{targetType, nullptr}, "floatToIntTemp"));
  }

  throw std::runtime_error("Unsupported cast");
}

custom_ir::Value *custom_ir::IRGenerator::generateBinary(BinaryExpr *expr) {

  if (expr->op == TOKEN_TYPE::ASSIGN) {
    if (!expr->left->isLValue())
      throw std::runtime_error("Left side of assignment is not assignable");

    custom_ir::Value *ptr = expr->left->codegenLValue(*this);
    custom_ir::Value *value = expr->right->codegen(*this);
    if (!value)
      return nullptr;

    Type *ptrElemTy = ptr->dataType->pointee;

    if (!areTypesEqual(value->dataType, ptrElemTy)) {

      if (value->dataType->pointee != nullptr &&
          ptrElemTy->pointee != nullptr) {
        value = builder.CreateBitCast(value, ptr);
      } else if (value->dataType->base == DATA_TYPE::DATATYPE_INT &&
                 ptrElemTy->base == DATA_TYPE::DATATYPE_INT) {
        // unsigned fromBits = value->getType()->getIntegerBitWidth();
        // unsigned toBits = ptrElemTy->getIntegerBitWidth();

        // if (fromBits < toBits) {
        //   value = builder.CreateZExt(value, ptrElemTy);
        // } else if (fromBits > toBits) {
        //   value = builder.CreateTrunc(value, ptrElemTy);
        // }
      }

      else {
        throw std::runtime_error("Invalid type mismatch in assignment\n");
      }
    }
    VariableValue *varValue = dynamic_cast<VariableValue *>(ptr);
    if (varValue == nullptr)
      throw std::runtime_error("assignment allowed only to a variable");
    builder.CreateStore(varValue, value);
    return value;
  }
  custom_ir::Value *L = expr->left->codegen(*this);
  custom_ir::Value *R = expr->right->codegen(*this);

  if (L->dataType->base == DATA_TYPE::DATATYPE_POINTER) {
    std::cout << "Pointer on the left" << std::endl;
  }

  if (!L || !R)
    return nullptr;

  if (L->dataType->base == DATA_TYPE::DATATYPE_FLOAT) {

    switch (expr->op) {

    case TOKEN_TYPE::PLUS:
    case TOKEN_TYPE::MINUS:
    case TOKEN_TYPE::ASTERISK:
    case TOKEN_TYPE::SLASH:
    // ===== Comparisons (double) =====
    case TOKEN_TYPE::EQUAL:
    case TOKEN_TYPE::NOT_EQUAL:
    case TOKEN_TYPE::LESS:
    case TOKEN_TYPE::GREATER: {
      Opcode op = floatOpcodeMap.at(expr->op);
      std::string tempName = floatTempNameMap.at(expr->op);
      return builder.CreateBinary(op, L, R,
                                  new TempValue(L->dataType, tempName));
    }
    default:
      return nullptr;
    }
  }

  switch (expr->op) {

  case TOKEN_TYPE::PLUS:
  case TOKEN_TYPE::MINUS:
  case TOKEN_TYPE::ASTERISK:
  case TOKEN_TYPE::SLASH:
  // ===== Comparisons (int) =====
  case TOKEN_TYPE::EQUAL:
  case TOKEN_TYPE::NOT_EQUAL:
  case TOKEN_TYPE::LESS:
  case TOKEN_TYPE::GREATER: {
    Opcode op = intOpcodeMap.at(expr->op);
    std::string tempName = tempNameMap.at(expr->op);
    return builder.CreateBinary(op, L, R, new TempValue(L->dataType, tempName));
  }
  // ===== Logical ops =====
  case TOKEN_TYPE::AND: {
    auto *zero = builder.CreateConstant(L->dataType, "0");

    L = builder.CreateBinary(Opcode::ICmpNE, L, zero,
                             new TempValue(L->dataType, "lhsbool"));

    R = builder.CreateBinary(Opcode::ICmpNE, R, zero,
                             new TempValue(R->dataType, "rhsbool"));
    return builder.CreateBinary(Opcode::And, L, R,
                                new TempValue(L->dataType, "andtmp"));
  }

  case TOKEN_TYPE::OR: {
    auto *zero = builder.CreateConstant(L->dataType, "0");

    L = builder.CreateBinary(Opcode::ICmpNE, L, zero,
                             new TempValue(L->dataType, "lhsbool"));

    R = builder.CreateBinary(Opcode::ICmpNE, R, zero,
                             new TempValue(R->dataType, "rhsbool"));
    return builder.CreateBinary(Opcode::Or, L, R,
                                new TempValue(L->dataType, "ortmp"));
  }

  default:
    return nullptr;
  }
}

custom_ir::Value *custom_ir::IRGenerator::generateUnaryExpr(UnaryExpr *expr) {
  switch (expr->op) {

  case TOKEN_TYPE::ASTERISK: {
    custom_ir::Value *ptr = generateUnaryExprLValue(expr);
    if (!ptr)
      return nullptr;

    Type *elementType = ptr->dataType->pointee;

    VariableValue *varValue = dynamic_cast<VariableValue *>(ptr);
    if (varValue == nullptr)
      throw std::runtime_error("Load allowed only to a variable");
    return builder.CreateLoad(
        varValue, new TempValue(elementType, varValue->value + "_deref_val"));
  }
  case TOKEN_TYPE::AMPERSAND: {
    return expr->operand->codegenLValue(*this);
  }

  default:
    throw std::runtime_error("Unsupported unary operator\n");
  }
}

custom_ir::Value *
custom_ir::IRGenerator::generateUnaryExprLValue(UnaryExpr *expr) {
  switch (expr->op) {

  case TOKEN_TYPE::ASTERISK: {
    custom_ir::Value *ptr = expr->operand->codegen(*this);
    if (!ptr)
      return nullptr;

    if (ptr->dataType->pointee == nullptr) {
      throw std::runtime_error("Cannot dereference non-pointer\n");
    }

    return ptr;
  }

  default:
    throw std::runtime_error("Unsupported unary operator for lvalue\n");
  }
}

custom_ir::Value *custom_ir::IRGenerator::generateVariable(VariableExpr *expr) {
  custom_ir::Value *ptr = generateVariableLValue(expr);
  if (!ptr)
    return nullptr;

  Type *elementType = ptr->dataType->pointee;
  VariableValue *varValue = dynamic_cast<VariableValue *>(ptr);
  if (varValue == nullptr)
    throw std::runtime_error("Load allowed only to a variable");
  return builder.CreateLoad(
      varValue, new TempValue(elementType, varValue->value + "_val"));
}

custom_ir::Value *
custom_ir::IRGenerator::generateVariableLValue(VariableExpr *expr) {
  custom_ir::Value *ptr = nullptr;
  for (auto it = namedValues.rbegin(); it != namedValues.rend(); ++it) {
    auto found = it->find(expr->name);
    if (found != it->end()) {
      ptr = found->second;
      break;
    }
  }

  if (!ptr) {
    throw std::runtime_error("Unknown variable: " + expr->name + "\n");
  }
  return ptr;
}

custom_ir::Value *
custom_ir::IRGenerator::generateDeclaration(DeclarationStmt *stmt) {
  Type *varType = stmt->dataType;
  custom_ir::Value *initValue = nullptr;
  if (stmt->expr) {
    initValue = stmt->expr->codegen(*this);
  } else {
    if (varType->pointee != nullptr) {
      initValue = builder.CreateConstant(varType, "null");
    } else if (varType->base == DATA_TYPE::DATATYPE_INT) {
      initValue = builder.CreateConstant(varType, "0");
    } else if (varType->base == DATA_TYPE::DATATYPE_FLOAT) {
      initValue = builder.CreateConstant(varType, "0.0");
    } else {
      throw std::runtime_error("Unsupported type for default initialization");
    }
  }

  if (builder.isInsertPointNull()) {

    auto *constant = dynamic_cast<custom_ir::Constant *>(initValue);

    if (!constant) {
      throw std::runtime_error("Global initializer must be constant");
    }
    VariableValue *varVal =
        new VariableValue(new Type{DATA_TYPE::DATATYPE_POINTER, varType},
                          stmt->identifier, variable_id++);
    auto *global = builder.CreateGlobalVariable(varVal, constant);

    Instruction *globalInst = dynamic_cast<Instruction *>(global);

    module->globalDeclarations->instructions.push_back(globalInst);

    namedValues.back()[stmt->identifier] = varVal;

    return global;
  }

  VariableValue *varValue =
      new VariableValue(new Type{DATA_TYPE::DATATYPE_POINTER, varType},
                        stmt->identifier, variable_id++);

  auto *alloca = builder.CreateAlloca(varValue);

  builder.CreateStore(varValue, initValue);

  namedValues.back()[stmt->identifier] = varValue;

  return alloca;
}
