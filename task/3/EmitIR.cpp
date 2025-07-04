#include "EmitIR.hpp"
#include <llvm/Transforms/Utils/ModuleUtils.h>

#define self (*this)

using namespace asg;

EmitIR::EmitIR(Obj::Mgr& mgr, llvm::LLVMContext& ctx, llvm::StringRef mid)
  : mMgr(mgr)
  , mMod(mid, ctx)
  , mCtx(ctx)
  , mIntTy(llvm::Type::getInt32Ty(ctx))
  , mCurIrb(std::make_unique<llvm::IRBuilder<>>(ctx))
  , mCtorTy(llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), false))
{
}

llvm::Module&
EmitIR::operator()(asg::TranslationUnit* tu)
{
  for (auto&& i : tu->decls)
    self(i);
  return mMod;
}

//==============================================================================
// 类型
//==============================================================================

llvm::Type*
EmitIR::operator()(const Type* type)
{
  if (type->texp == nullptr) {
    switch (type->spec) {
      case Type::Spec::kInt:
        return llvm::Type::getInt32Ty(mCtx);
      case Type::Spec::kLong:
        return llvm::Type::getInt64Ty(mCtx);
      case Type::Spec::kLongLong:
        return llvm::Type::getInt128Ty(mCtx);
      case Type::Spec::kChar:
        return llvm::Type::getInt8Ty(mCtx);
      case Type::Spec::kVoid:
        return llvm::Type::getVoidTy(mCtx);
      default:
        ABORT();
    }
  }

  Type subt;
  subt.spec = type->spec;
  subt.qual = type->qual;
  subt.texp = type->texp->sub;

  llvm::ArrayType* arrType;
  std::vector<int> dimensions;

  if (auto arrayType = type->texp->dcst<ArrayType>()) {
    // 收集维度
    dimensions.push_back(arrayType->len);
    while (arrayType->sub != nullptr) {
      if (auto subArrayType = arrayType->sub->dcst<ArrayType>()) {
        arrayType = subArrayType;
        dimensions.push_back(arrayType->len);
      }
    }

    arrType = llvm::ArrayType::get(llvm::Type::getInt32Ty(mCtx), dimensions.back());
    dimensions.pop_back();
    while (!dimensions.empty()) {
      arrType = llvm::ArrayType::get(arrType, dimensions.back());
      dimensions.pop_back();
    }
    return arrType;
  }

  if (auto pointerType = type->texp->dcst<PointerType>()) {
    return llvm::PointerType::get(llvm::Type::getInt32Ty(mCtx), 0);
  }

  if (auto functionType = type->texp->dcst<FunctionType>()) {
    std::vector<llvm::Type*> paramTypes;
    for (size_t i = 0; i < functionType->params.size(); ++i) {
      paramTypes.push_back(self(functionType->params[i]));
    }
    return llvm::FunctionType::get(self(&subt), paramTypes, false);
  }

  ABORT();
}

//==============================================================================
// 表达式
//==============================================================================

llvm::Value*
EmitIR::operator()(Expr* obj)
{
  // TODO: 在此添加对更多表达式处理的跳转
  if (auto p = obj->dcst<IntegerLiteral>())
    return self(p);

  if (auto p = obj->dcst<DeclRefExpr>())
    return self(p);

  if (auto p = obj->dcst<ImplicitCastExpr>())
    return self(p);

  if (auto p = obj->dcst<BinaryExpr>())
    return self(p);

  if (auto p = obj->dcst<UnaryExpr>())
    return self(p);
  
  if (auto p = obj->dcst<CallExpr>())
    return self(p);
  
  if (auto p = obj->dcst<InitListExpr>())
    return self(p);

  if (auto p = obj->dcst<ParenExpr>())
    return self(p);

  ABORT();
}

llvm::Value*
EmitIR::operator()(ImplicitCastExpr* obj)
{
  auto sub = self(obj->sub);

  auto& irb = *mCurIrb;
  switch (obj->kind) {
    case ImplicitCastExpr::kLValueToRValue: {
      auto ty = self(obj->sub->type);
      auto loadVal = irb.CreateLoad(ty, sub);
      return loadVal;
    }

    case ImplicitCastExpr::kFunctionToPointerDecay:{
      return sub;
    }

    case ImplicitCastExpr::kArrayToPointerDecay:{
      return sub;
    }

    default:
      ABORT();
  }
}

llvm::Value*
EmitIR::operator()(DeclRefExpr* obj)
{
  // 在LLVM IR层面，左值体现为返回指向值的指针
  // 在ImplicitCastExpr::kLValueToRValue中发射load指令从而变成右值
  return reinterpret_cast<llvm::Value*>(obj->decl->any);
}

llvm::Value*
EmitIR::operator()(BinaryExpr* obj)
{
  llvm::Value *lftVal = nullptr, *rhtVal = nullptr;
  auto& irb = *mCurIrb;

  if (obj->op != BinaryExpr::kAnd && obj->op != BinaryExpr::kOr) {
    lftVal = self(obj->lft);
    rhtVal = self(obj->rht);
  }

  switch (obj->op) {
    case BinaryExpr::kAdd:
      return irb.CreateAdd(lftVal, rhtVal);
    
    case BinaryExpr::kAssign:
      irb.CreateStore(rhtVal, lftVal);
      return rhtVal;

    case BinaryExpr::kSub:
      return irb.CreateSub(lftVal, rhtVal);
    
    case BinaryExpr::kMul:
      return irb.CreateMul(lftVal, rhtVal);

    case BinaryExpr::kDiv:
      return irb.CreateSDiv(lftVal, rhtVal);
    
    case BinaryExpr::kMod:
      return irb.CreateSRem(lftVal, rhtVal);
    
    case BinaryExpr::kEq:
      return irb.CreateICmpEQ(lftVal, rhtVal);

    case BinaryExpr::kNe:
      return irb.CreateICmpNE(lftVal, rhtVal);

    case BinaryExpr::kGt:
      return irb.CreateICmpSGT(lftVal, rhtVal);

    case BinaryExpr::kGe:
      return irb.CreateICmpSGE(lftVal, rhtVal);

    case BinaryExpr::kLt:
      return irb.CreateICmpSLT(lftVal, rhtVal);

    case BinaryExpr::kLe:
      return irb.CreateICmpSLE(lftVal, rhtVal);

    case BinaryExpr::kAnd: {
      llvm::BasicBlock* currentBlock = irb.GetInsertBlock();
      llvm::Function* parentFunction = currentBlock->getParent();

      llvm::BasicBlock* rhsBlock = llvm::BasicBlock::Create(mCtx, "land.rhs", parentFunction);
      llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(mCtx, "land.end", parentFunction);

      // 处理左值
      llvm::Value* leftResult = self(obj->lft);
      if (leftResult->getType()->isIntegerTy(32)) {
        leftResult = irb.CreateICmpNE(leftResult, irb.getInt32(0), "isNonZero");
      }
      irb.CreateCondBr(leftResult, rhsBlock, endBlock);
      llvm::BasicBlock* leftEndBlock = irb.GetInsertBlock();

      // 处理右值
      irb.SetInsertPoint(rhsBlock);
      llvm::Value* rightResult = self(obj->rht);
      if (rightResult->getType()->isIntegerTy(32)) {
        rightResult = irb.CreateICmpNE(rightResult, irb.getInt32(0), "isNonZero");
      }
      irb.CreateBr(endBlock);
      llvm::BasicBlock* rightEndBlock = irb.GetInsertBlock();

      irb.SetInsertPoint(endBlock);
      llvm::PHINode* phi = irb.CreatePHI(llvm::Type::getInt1Ty(mCtx), 2, "merge");
      phi->addIncoming(irb.getInt1(false), leftEndBlock);
      phi->addIncoming(rightResult, rightEndBlock);
      return phi;
    }

    case BinaryExpr::kOr: {
      llvm::BasicBlock* currentBlock = irb.GetInsertBlock();
      llvm::Function* parentFunction = currentBlock->getParent();

      llvm::BasicBlock* rhsBlock = llvm::BasicBlock::Create(mCtx, "lor.rhs", parentFunction);
      llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(mCtx, "lor.end", parentFunction);

      // 处理左值
      llvm::Value* leftResult = self(obj->lft);
      if (leftResult->getType()->isIntegerTy(32)) {
        leftResult = irb.CreateICmpNE(leftResult, irb.getInt32(0), "isNonZero");
      }
      irb.CreateCondBr(leftResult, endBlock, rhsBlock);
      llvm::BasicBlock* leftEndBlock = irb.GetInsertBlock();

      // 处理右值
      irb.SetInsertPoint(rhsBlock);
      llvm::Value* rightResult = self(obj->rht);
      if (rightResult->getType()->isIntegerTy(32)) {
        rightResult = irb.CreateICmpNE(rightResult, irb.getInt32(0), "isNonZero");
      }
      irb.CreateBr(endBlock);
      llvm::BasicBlock* rightEndBlock = irb.GetInsertBlock();

      irb.SetInsertPoint(endBlock);
      llvm::PHINode* phi = irb.CreatePHI(llvm::Type::getInt1Ty(mCtx), 2, "merge");
      phi->addIncoming(irb.getInt1(true), leftEndBlock);
      phi->addIncoming(rightResult, rightEndBlock);
      return phi;
    }

    case BinaryExpr::kIndex: {
      std::vector<llvm::Value*> indexList{rhtVal};
      return irb.CreateInBoundsGEP(self(obj->type), lftVal, indexList);
    }

    default:
      ABORT();
  }
}

llvm::Value* EmitIR::operator()(UnaryExpr* obj) 
{
  llvm::Value* subExpr = self(obj->sub);
  auto& irb = *mCurIrb;

  switch (obj->op) {
    case UnaryExpr::kNeg:
      return irb.CreateNeg(subExpr);

    case UnaryExpr::kNot: {
      llvm::Value* condition = subExpr->getType()->isIntegerTy(32)
                                   ? irb.CreateICmpNE(subExpr, irb.getInt32(0), "isNonZero")
                                   : subExpr;
      return irb.CreateNot(condition);
    }

    case UnaryExpr::kPos:
      return subExpr;

    default:
      ABORT();
  }
}

llvm::Value*
EmitIR::operator()(CallExpr* obj)
{
  auto& irb = *mCurIrb;
  llvm::Function* func = reinterpret_cast<llvm::Function*>(self(obj->head));

  std::vector<llvm::Value*> args;
  for (auto&& arg : obj->args) {
    args.push_back(self(arg));
  }

  return irb.CreateCall(func, args);
}

llvm::Value* EmitIR::operator()(InitListExpr* obj) {
  llvm::ArrayType* arrayType = reinterpret_cast<llvm::ArrayType*>(self(obj->type));
  auto& irb = *mCurIrb;
  llvm::Value* pointer = irb.CreateAlloca(arrayType, nullptr);

  for (size_t i = 0; i < obj->list.size(); ++i) {
    if (obj->list[i]->dcst<ImplicitInitExpr>()) {
      return llvm::Constant::getNullValue(arrayType);
    }

    if (self(obj->list[i]->type)->isIntegerTy()) {
      storeElement(irb, pointer, arrayType, self(obj->list[i]), {0, static_cast<int>(i)});
    } else {
      auto pointer1D = self(obj->list[i]);
      llvm::ArrayType* innerArrayType = llvm::dyn_cast<llvm::ArrayType>(arrayType->getElementType());
      for (size_t j = 0; j < innerArrayType->getNumElements(); ++j) {
        auto val = loadElement(irb, pointer1D, innerArrayType, {0, static_cast<int>(j)});
        storeElement(irb, pointer, arrayType, val, {0, static_cast<int>(i), static_cast<int>(j)});
      }
    }
  }
  return pointer;
}

void EmitIR::storeElement(llvm::IRBuilder<>& irb, llvm::Value* pointer, llvm::ArrayType* arrayType,
                          llvm::Value* value, std::vector<int> indices) {
  std::vector<llvm::Value*> idxList;
  for (int idx : indices) {
    idxList.push_back(irb.getInt64(idx));
  }
  llvm::Value* element = irb.CreateInBoundsGEP(arrayType, pointer, idxList);
  irb.CreateStore(value, element);
}

llvm::Value* EmitIR::loadElement(llvm::IRBuilder<>& irb, llvm::Value* pointer, llvm::ArrayType* arrayType,
                                 std::vector<int> indices) {
  std::vector<llvm::Value*> idxList;
  for (int idx : indices) {
    idxList.push_back(irb.getInt64(idx));
  }
  llvm::Value* element = irb.CreateInBoundsGEP(arrayType, pointer, idxList);
  return irb.CreateLoad(llvm::Type::getInt32Ty(mCtx), element);
}

llvm::Value*
EmitIR::operator()(ParenExpr* obj)
{
  return self(obj->sub);
}

llvm::Constant*
EmitIR::operator()(IntegerLiteral* obj)
{
  return llvm::ConstantInt::get(self(obj->type), obj->val);
}

// TODO: 在此添加对更多表达式类型的处理

//==============================================================================
// 语句
//==============================================================================

void
EmitIR::operator()(Stmt* obj)
{
  // TODO: 在此添加对更多Stmt类型的处理的跳转

  if (auto p = obj->dcst<CompoundStmt>())
    return self(p);

  if (auto p = obj->dcst<ReturnStmt>())
    return self(p);
  
  if (auto p = obj->dcst<ExprStmt>())
    return self(p);

  if (auto p = obj->dcst<DeclStmt>())
    return self(p);

  if (auto p = obj->dcst<IfStmt>())
    return self(p);

  if (auto p = obj->dcst<WhileStmt>())
    return self(p);

  if (auto p = obj->dcst<NullStmt>())
    return self(p);

  if (auto p = obj->dcst<ContinueStmt>())
    return self(p);

  if (auto p = obj->dcst<BreakStmt>())
    return self(p);

  ABORT();
}

// TODO: 在此添加对更多Stmt类型的处理

void
EmitIR::operator()(CompoundStmt* obj)
{
  // TODO: 可以在此添加对符号重名的处理
  for (auto&& stmt : obj->subs)
    self(stmt);
}

void
EmitIR::operator()(ReturnStmt* obj)
{
  auto& irb = *mCurIrb;

  llvm::Value* retVal;
  if (!obj->expr)
    retVal = nullptr;
  else
    retVal = self(obj->expr);

  mCurIrb->CreateRet(retVal);

  auto exitBb = llvm::BasicBlock::Create(mCtx, "return_exit", mCurFunc);
  mCurIrb->SetInsertPoint(exitBb);
}

void
EmitIR::operator()(ExprStmt* obj)
{
    self(obj->expr);
}

void
EmitIR::operator()(DeclStmt* obj)
{
  for (auto&& dec : obj->decls) self(dec);
}

void 
EmitIR::operator()(IfStmt* obj)
{
  auto& irb = *mCurIrb;
  llvm::BasicBlock* currentBlock = irb.GetInsertBlock();
  llvm::Function* parentFunction = currentBlock->getParent();

  llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(mCtx, "if.then", parentFunction);
  llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(mCtx, "if.end", parentFunction);

  auto conditionResult = self(obj->cond);
  llvm::Value* condition = conditionResult->getType()->isIntegerTy(32)
                               ? irb.CreateICmpNE(conditionResult, irb.getInt32(0), "isNonZero")
                               : conditionResult;

  if (obj->else_ != nullptr) {
    llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(mCtx, "if.else", parentFunction);

    // 条件跳转到 then 或 else
    irb.CreateCondBr(condition, thenBlock, elseBlock);

    irb.SetInsertPoint(thenBlock);
    self(obj->then);
    auto thenEndBlock = irb.GetInsertBlock();
    if (thenEndBlock->getTerminator() == nullptr) {
      irb.CreateBr(endBlock);
    }

    irb.SetInsertPoint(elseBlock);
    self(obj->else_);
    auto elseEndBlock = irb.GetInsertBlock();
    if (elseEndBlock->getTerminator() == nullptr) {
      irb.CreateBr(endBlock);
    }

    irb.SetInsertPoint(endBlock);
  } else {
    irb.CreateCondBr(condition, thenBlock, endBlock);

    irb.SetInsertPoint(thenBlock);
    self(obj->then);
    auto thenEndBlock = irb.GetInsertBlock();
    if (thenEndBlock->getTerminator() == nullptr) {
      irb.CreateBr(endBlock);
    }

    irb.SetInsertPoint(endBlock);
  }
}

llvm::BasicBlock* newest_cond_block = nullptr;
llvm::BasicBlock* newest_end_block = nullptr;

void
EmitIR::operator()(WhileStmt* obj)
{
  auto& irb = *mCurIrb;
  llvm::BasicBlock* currentBlock = irb.GetInsertBlock();
  llvm::Function* parentFunction = currentBlock->getParent();

  llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(mCtx, "while.cond", parentFunction);
  llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(mCtx, "while.body", parentFunction);
  llvm::BasicBlock* endBlock = llvm::BasicBlock::Create(mCtx, "while.end", parentFunction);

  newest_cond_block = condBlock;
  newest_end_block = endBlock;
  irb.CreateBr(condBlock);

  irb.SetInsertPoint(condBlock);

  auto conditionResult = self(obj->cond);
  llvm::Value* condition = conditionResult->getType()->isIntegerTy(32)
                               ? irb.CreateICmpNE(conditionResult, irb.getInt32(0), "isNonZero")
                               : conditionResult;

  // 条件跳转到 body 或 end
  irb.CreateCondBr(condition, bodyBlock, endBlock);

  irb.SetInsertPoint(bodyBlock);
  self(obj->body);
  
  auto bodyEndBlock = irb.GetInsertBlock();
  if (bodyEndBlock->getTerminator() == nullptr) {
    irb.CreateBr(condBlock);
  }

  irb.SetInsertPoint(endBlock);
}

void
EmitIR::operator()(NullStmt* obj)
{

}

void
EmitIR::operator()(ContinueStmt* obj)
{
  auto& irb = *mCurIrb;
  if (newest_cond_block) {
    irb.CreateBr(newest_cond_block);
  } else {
    ABORT();
  }
}

void
EmitIR::operator()(BreakStmt* obj)
{
  auto& irb = *mCurIrb;
  if (newest_end_block) {
    irb.CreateBr(newest_end_block);
  } else {
    ABORT();
  }
}
//==============================================================================
// 声明
//==============================================================================

void
EmitIR::operator()(Decl* obj)
{
  // TODO: 添加变量声明处理的跳转
  if (auto p = obj->dcst<VarDecl>())
    return self(p);

  if (auto p = obj->dcst<FunctionDecl>())
    return self(p);

  ABORT();
}


// TODO: 添加变量声明的处理

// 函数声明
void EmitIR::operator()(FunctionDecl* obj) 
{
  // 创建函数
  auto funcType = llvm::dyn_cast<llvm::FunctionType>(self(obj->type));
  auto function = llvm::Function::Create(
      funcType, llvm::GlobalVariable::ExternalLinkage, obj->name, mMod);

  obj->any = function;

  if (!obj->body) return;

  auto entryBlock = llvm::BasicBlock::Create(mCtx, "entry", function);
  mCurIrb = std::make_unique<llvm::IRBuilder<>>(entryBlock);
  auto& irBuilder = *mCurIrb;

  // 参数
  auto processFunctionParams = [&]() {
    auto argIter = function->arg_begin();
    for (size_t i = 0; i < obj->params.size(); ++i) {
      argIter->setName(obj->params[i]->name);
      auto paramType = self(obj->params[i]->type);
      auto localVar = irBuilder.CreateAlloca(paramType, nullptr, obj->params[i]->name);
      obj->params[i]->any = localVar;
      irBuilder.CreateStore(&*argIter, localVar);
      ++argIter;
    }
  };
  processFunctionParams();

  mCurFunc = function;
  self(obj->body);

  auto handleReturn = [&]() {
    auto& exitBuilder = *mCurIrb;
    if (funcType->getReturnType()->isVoidTy()) {
      exitBuilder.CreateRetVoid();
    } else {
      llvm::BasicBlock* currentBlock = exitBuilder.GetInsertBlock();
      if (!currentBlock->getTerminator()) {
        exitBuilder.CreateUnreachable();
      }
    }
  };
  handleReturn();
  mCurIrb->ClearInsertionPoint();
}

void
EmitIR::trans_init(llvm::Type* type, llvm::Value* value, asg::Expr* expr)
{
  auto& irb = *mCurIrb;

  auto storeValue = [&](llvm::Value* initVal) {
    irb.CreateStore(initVal, value);
  };

  // 常量
  if (auto intLiteral = expr->dcst<IntegerLiteral>()) {
    storeValue(self(intLiteral));
    return;
  }

  // 空数组
  if (expr->dcst<ImplicitInitExpr>()) {
    return;
  }

  // 数组
  if (auto initList = expr->dcst<InitListExpr>()) {
    if (initList->list[0]->dcst<ImplicitInitExpr>()) return;

    llvm::Value* pointer = self(initList);
    llvm::ArrayType* arrayType = reinterpret_cast<llvm::ArrayType*>(type);
    int outerSize = arrayType->getNumElements();
    llvm::Type* innerType = arrayType->getElementType();

    auto copyArrayElements = [&](int outerIndex, int innerIndex = -1) {
      std::vector<llvm::Value*> idxList{irb.getInt64(0), irb.getInt64(outerIndex)};
      if (innerIndex >= 0) idxList.push_back(irb.getInt64(innerIndex));

      llvm::Value* srcElement = irb.CreateInBoundsGEP(arrayType, pointer, idxList);
      auto initVal = irb.CreateLoad(llvm::Type::getInt32Ty(mCtx), srcElement);
      llvm::Value* destElement = irb.CreateInBoundsGEP(arrayType, value, idxList);
      irb.CreateStore(initVal, destElement);
    };

    if (auto innerArrayType = llvm::dyn_cast<llvm::ArrayType>(innerType)) {
      int innerSize = innerArrayType->getNumElements();
      for (int i = 0; i < outerSize; ++i) {
        for (int j = 0; j < innerSize; ++j) {
          copyArrayElements(i, j);
        }
      }
    } else {
      for (int i = 0; i < outerSize; ++i) {
        copyArrayElements(i);
      }
    }
    return;
  }

  if (auto castExpr = expr->dcst<ImplicitCastExpr>()) {
    storeValue(self(castExpr));
    return;
  }

  if (auto binaryExpr = expr->dcst<BinaryExpr>()) {
    storeValue(self(binaryExpr));
    return;
  }

  if (auto callExpr = expr->dcst<CallExpr>()) {
    storeValue(self(callExpr));
    return;
  }

  if (auto unaryExpr = expr->dcst<UnaryExpr>()) {
    storeValue(self(unaryExpr));
    return;
  }

  ABORT();
}

void
EmitIR::operator()(VarDecl* obj)
{ 
  auto& irb = *mCurIrb;
  llvm::BasicBlock *currentBlock = irb.GetInsertBlock();
  auto type = self(obj->type);

  if (currentBlock) { // 局部变量
    auto localVar = irb.CreateAlloca(type, nullptr, obj->name);
    obj->any = localVar;
    irb.CreateStore(llvm::Constant::getNullValue(type), localVar);

    if (obj->init) {
      trans_init(type, localVar, obj->init);
    }
  } else { // 全局变量
    auto globalVar = new llvm::GlobalVariable(
      mMod, type, false, llvm::GlobalVariable::ExternalLinkage, nullptr, obj->name);
    obj->any = globalVar;
    globalVar->setInitializer(llvm::Constant::getNullValue(type));

    if (obj->init) {
      // 创建构造函数用于初始化
      mCurFunc = llvm::Function::Create(
        mCtorTy, llvm::GlobalVariable::PrivateLinkage, "ctor_" + obj->name, mMod);
      llvm::appendToGlobalCtors(mMod, mCurFunc, 65535);

      // 创建 entry 基本块并初始化
      auto entryBlock = llvm::BasicBlock::Create(mCtx, "entry", mCurFunc);
      mCurIrb = std::make_unique<llvm::IRBuilder<>>(entryBlock);
      trans_init(type, globalVar, obj->init);
      mCurIrb->CreateRet(nullptr);
      mCurIrb->ClearInsertionPoint();
    }
  }
}

