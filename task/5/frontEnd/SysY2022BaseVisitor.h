
// Generated from src/frontEnd/SysY2022.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "SysY2022Visitor.h"


/**
 * This class provides an empty implementation of SysY2022Visitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SysY2022BaseVisitor : public SysY2022Visitor {
public:

  virtual std::any visitCompUnit(SysY2022Parser::CompUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDecl(SysY2022Parser::DeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstDecl(SysY2022Parser::ConstDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBType(SysY2022Parser::BTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstDef(SysY2022Parser::ConstDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitItemConstInitVal(SysY2022Parser::ItemConstInitValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitListConstInitVal(SysY2022Parser::ListConstInitValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarDecl(SysY2022Parser::VarDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarDefWithOutInitVal(SysY2022Parser::VarDefWithOutInitValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarDefWithInitVal(SysY2022Parser::VarDefWithInitValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitItemInitVal(SysY2022Parser::ItemInitValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitListInitVal(SysY2022Parser::ListInitValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncDef(SysY2022Parser::FuncDefContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncType(SysY2022Parser::FuncTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncFParams(SysY2022Parser::FuncFParamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncFParam(SysY2022Parser::FuncFParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlock(SysY2022Parser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockItem(SysY2022Parser::BlockItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignStmt(SysY2022Parser::AssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpStmt(SysY2022Parser::ExpStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlockStmt(SysY2022Parser::BlockStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStmt(SysY2022Parser::IfStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfElseStmt(SysY2022Parser::IfElseStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhileStmt(SysY2022Parser::WhileStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBreakStmt(SysY2022Parser::BreakStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitContinueStmt(SysY2022Parser::ContinueStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnStmt(SysY2022Parser::ReturnStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExp(SysY2022Parser::ExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCond(SysY2022Parser::CondContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLVal(SysY2022Parser::LValContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpPrimaryExp(SysY2022Parser::ExpPrimaryExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLValPrimaryExp(SysY2022Parser::LValPrimaryExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumberPrimaryExp(SysY2022Parser::NumberPrimaryExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumber(SysY2022Parser::NumberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryUnaryExp(SysY2022Parser::PrimaryUnaryExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionCallUnaryExp(SysY2022Parser::FunctionCallUnaryExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryUnaryExp(SysY2022Parser::UnaryUnaryExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryOp(SysY2022Parser::UnaryOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncRParams(SysY2022Parser::FuncRParamsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFuncRParam(SysY2022Parser::FuncRParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMulMulExp(SysY2022Parser::MulMulExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryMulExp(SysY2022Parser::UnaryMulExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddAddExp(SysY2022Parser::AddAddExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMulAddExp(SysY2022Parser::MulAddExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelRelExp(SysY2022Parser::RelRelExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddRelExp(SysY2022Parser::AddRelExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqEqExp(SysY2022Parser::EqEqExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelEqExp(SysY2022Parser::RelEqExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqLAndExp(SysY2022Parser::EqLAndExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLAndLAndExp(SysY2022Parser::LAndLAndExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLAndLOrExp(SysY2022Parser::LAndLOrExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLOrLOrExp(SysY2022Parser::LOrLOrExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstExp(SysY2022Parser::ConstExpContext *ctx) override {
    return visitChildren(ctx);
  }


};

