#pragma once

#include <iostream>
#include <stack>
#include "antlr4-runtime.h"
#include "SysY2022Lexer.h"
#include "SysY2022BaseVisitor.h"
#include "Module.h"
#include "createCFG.h"
#include "mem2reg.h"
#include "reg2mem.h"
#include "constReplace.h"
#include "dce.h"
#include "cse.h"
#include "loopAnalysis.h"
#include "reassociate.h"
#include "strengthReduction.h"
#include "DSE.h"
#include "LICM.h"
#include "constPro.h"
#include "LCSSA.h"
#include "inliner.h"
#include "loopUnroll.h"
#include "computeUse.h"
#include "simplifyCFG.h"

class MyVisitor : public SysY2022BaseVisitor
{
public:
  Module irModule;

  void print();
  void opt();

  virtual antlrcpp::Any visitCompUnit(SysY2022Parser::CompUnitContext *ctx) override;

  virtual antlrcpp::Any visitDecl(SysY2022Parser::DeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstDecl(SysY2022Parser::ConstDeclContext *ctx) override;

  virtual antlrcpp::Any visitBType(SysY2022Parser::BTypeContext *ctx) override;
  
  virtual antlrcpp::Any visitConstDef(SysY2022Parser::ConstDefContext *ctx) override;

  virtual antlrcpp::Any visitItemConstInitVal(SysY2022Parser::ItemConstInitValContext *ctx) override;

  // const数组会单独处理
  // virtual antlrcpp::Any visitListConstInitVal(SysY2022Parser::ListConstInitValContext *ctx) override {
  //   return visitChildren(ctx);
  // }

  virtual antlrcpp::Any visitVarDecl(SysY2022Parser::VarDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarDefWithOutInitVal(SysY2022Parser::VarDefWithOutInitValContext *ctx) override;

  int constArr = 0;
  VariablePtr arrInitItem(SysY2022Parser::ItemConstInitValContext *ctx, TypePtr type);
  shared_ptr<Arr> arrInitList(SysY2022Parser::ListConstInitValContext *ctx, TypePtr type);
  VariablePtr arrInitItem(SysY2022Parser::ItemInitValContext *ctx, TypePtr type);
  shared_ptr<Arr> arrInitList(SysY2022Parser::ListInitValContext *ctx, TypePtr type);
  void arrSetList(SysY2022Parser::ListInitValContext *ctx, ValuePtr value);
  
  virtual antlrcpp::Any visitVarDefWithInitVal(SysY2022Parser::VarDefWithInitValContext *ctx) override;

  bool dfsInitVal(SysY2022Parser::InitValContext *ctx);

  virtual antlrcpp::Any visitItemInitVal(SysY2022Parser::ItemInitValContext *ctx) override;

  virtual antlrcpp::Any visitListInitVal(SysY2022Parser::ListInitValContext *ctx) override;

  virtual antlrcpp::Any visitFuncDef(SysY2022Parser::FuncDefContext *ctx) override;

  virtual antlrcpp::Any visitFuncType(SysY2022Parser::FuncTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncFParams(SysY2022Parser::FuncFParamsContext *ctx) override;

  virtual antlrcpp::Any visitFuncFParam(SysY2022Parser::FuncFParamContext *ctx) override;

  virtual antlrcpp::Any visitBlock(SysY2022Parser::BlockContext *ctx) override;

  virtual antlrcpp::Any visitBlockItem(SysY2022Parser::BlockItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAssignStmt(SysY2022Parser::AssignStmtContext *ctx) override;

  virtual antlrcpp::Any visitExpStmt(SysY2022Parser::ExpStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlockStmt(SysY2022Parser::BlockStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIfStmt(SysY2022Parser::IfStmtContext *ctx) override;

  virtual antlrcpp::Any visitIfElseStmt(SysY2022Parser::IfElseStmtContext *ctx) override;

  virtual antlrcpp::Any visitWhileStmt(SysY2022Parser::WhileStmtContext *ctx) override;

  virtual antlrcpp::Any visitBreakStmt(SysY2022Parser::BreakStmtContext *ctx) override;

  virtual antlrcpp::Any visitContinueStmt(SysY2022Parser::ContinueStmtContext *ctx) override;

  virtual antlrcpp::Any visitReturnStmt(SysY2022Parser::ReturnStmtContext *ctx) override;

  virtual antlrcpp::Any visitExp(SysY2022Parser::ExpContext *ctx) override;

  virtual antlrcpp::Any visitCond(SysY2022Parser::CondContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLVal(SysY2022Parser::LValContext *ctx) override;

  virtual antlrcpp::Any visitExpPrimaryExp(SysY2022Parser::ExpPrimaryExpContext *ctx) override;

  virtual antlrcpp::Any visitLValPrimaryExp(SysY2022Parser::LValPrimaryExpContext *ctx) override;

  virtual antlrcpp::Any visitNumberPrimaryExp(SysY2022Parser::NumberPrimaryExpContext *ctx) override;

  virtual antlrcpp::Any visitNumber(SysY2022Parser::NumberContext *ctx) override;

  virtual antlrcpp::Any visitPrimaryUnaryExp(SysY2022Parser::PrimaryUnaryExpContext *ctx) override;

  virtual antlrcpp::Any visitFunctionCallUnaryExp(SysY2022Parser::FunctionCallUnaryExpContext *ctx) override;

  virtual antlrcpp::Any visitUnaryUnaryExp(SysY2022Parser::UnaryUnaryExpContext *ctx) override;

  virtual antlrcpp::Any visitUnaryOp(SysY2022Parser::UnaryOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncRParams(SysY2022Parser::FuncRParamsContext *ctx) override;

  virtual antlrcpp::Any visitFuncRParam(SysY2022Parser::FuncRParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMulMulExp(SysY2022Parser::MulMulExpContext *ctx) override;

  virtual antlrcpp::Any visitUnaryMulExp(SysY2022Parser::UnaryMulExpContext *ctx) override;

  virtual antlrcpp::Any visitAddAddExp(SysY2022Parser::AddAddExpContext *ctx) override;

  virtual antlrcpp::Any visitMulAddExp(SysY2022Parser::MulAddExpContext *ctx) override;

  virtual antlrcpp::Any visitRelRelExp(SysY2022Parser::RelRelExpContext *ctx) override;

  virtual antlrcpp::Any visitAddRelExp(SysY2022Parser::AddRelExpContext *ctx) override;

  virtual antlrcpp::Any visitEqEqExp(SysY2022Parser::EqEqExpContext *ctx) override;

  virtual antlrcpp::Any visitRelEqExp(SysY2022Parser::RelEqExpContext *ctx) override;

  virtual antlrcpp::Any visitEqLAndExp(SysY2022Parser::EqLAndExpContext *ctx) override;

  virtual antlrcpp::Any visitLAndLAndExp(SysY2022Parser::LAndLAndExpContext *ctx) override;

  virtual antlrcpp::Any visitLAndLOrExp(SysY2022Parser::LAndLOrExpContext *ctx) override;

  virtual antlrcpp::Any visitLOrLOrExp(SysY2022Parser::LOrLOrExpContext *ctx) override;

  virtual antlrcpp::Any visitConstExp(SysY2022Parser::ConstExpContext *ctx) override;

};
