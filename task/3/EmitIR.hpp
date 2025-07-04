#include "asg.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

class EmitIR
{
public:
  Obj::Mgr& mMgr;
  llvm::Module mMod;

  EmitIR(Obj::Mgr& mgr, llvm::LLVMContext& ctx, llvm::StringRef mid = "-");

  llvm::Module& operator()(asg::TranslationUnit* tu);

private:
  llvm::LLVMContext& mCtx;

  llvm::Type* mIntTy;
  llvm::FunctionType* mCtorTy;

  llvm::Function* mCurFunc;
  std::unique_ptr<llvm::IRBuilder<>> mCurIrb;

  //============================================================================
  // 类型
  //============================================================================

  llvm::Type* operator()(const asg::Type* type);

  //============================================================================
  // 表达式
  //============================================================================

  llvm::Value* operator()(asg::Expr* obj);

  llvm::Value* operator()(asg::BinaryExpr* obj);

  llvm::Value* operator()(asg::ImplicitCastExpr* obj);

  llvm::Value* operator()(asg::DeclRefExpr* obj);

  llvm::Value* operator()(asg::UnaryExpr* obj);

  llvm::Value* operator()(asg::CallExpr* obj);

  llvm::Value* operator()(asg::InitListExpr* obj);

  void storeElement(llvm::IRBuilder<>& irb, llvm::Value* pointer, llvm::ArrayType* arrayType, llvm::Value* value, std::vector<int> indices);

  llvm::Value* loadElement(llvm::IRBuilder<>& irb, llvm::Value* pointer, llvm::ArrayType* arrayType, std::vector<int> indices);
  
  llvm::Value* operator()(asg::ParenExpr* obj);

  llvm::Constant* operator()(asg::IntegerLiteral* obj);

  // TODO: 添加表达式处理相关声明

  //============================================================================
  // 语句
  //============================================================================

  void operator()(asg::Stmt* obj);

  void operator()(asg::CompoundStmt* obj);

  void operator()(asg::ReturnStmt* obj);

  void operator()(asg::ExprStmt* obj);

  void operator()(asg::DeclStmt* obj);

  void operator()(asg::IfStmt* obj);

  void operator()(asg::WhileStmt* obj);

  void operator()(asg::NullStmt* obj);

  void operator()(asg::ContinueStmt* obj);

  void operator()(asg::BreakStmt* obj);

  // TODO: 添加语句处理相关声明

  //============================================================================
  // 声明
  //============================================================================

  void operator()(asg::Decl* obj);

  void operator()(asg::VarDecl* obj);

  void trans_init(llvm::Type* type, llvm::Value* val, asg::Expr* obj);

  void operator()(asg::FunctionDecl* obj);
  // TODO: 添加声明处理相关声明
};
