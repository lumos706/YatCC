
// Generated from src/frontEnd/SysY2022.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"
#include "SysY2022Parser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SysY2022Parser.
 */
class  SysY2022Visitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SysY2022Parser.
   */
    virtual std::any visitCompUnit(SysY2022Parser::CompUnitContext *context) = 0;

    virtual std::any visitDecl(SysY2022Parser::DeclContext *context) = 0;

    virtual std::any visitConstDecl(SysY2022Parser::ConstDeclContext *context) = 0;

    virtual std::any visitBType(SysY2022Parser::BTypeContext *context) = 0;

    virtual std::any visitConstDef(SysY2022Parser::ConstDefContext *context) = 0;

    virtual std::any visitItemConstInitVal(SysY2022Parser::ItemConstInitValContext *context) = 0;

    virtual std::any visitListConstInitVal(SysY2022Parser::ListConstInitValContext *context) = 0;

    virtual std::any visitVarDecl(SysY2022Parser::VarDeclContext *context) = 0;

    virtual std::any visitVarDefWithOutInitVal(SysY2022Parser::VarDefWithOutInitValContext *context) = 0;

    virtual std::any visitVarDefWithInitVal(SysY2022Parser::VarDefWithInitValContext *context) = 0;

    virtual std::any visitItemInitVal(SysY2022Parser::ItemInitValContext *context) = 0;

    virtual std::any visitListInitVal(SysY2022Parser::ListInitValContext *context) = 0;

    virtual std::any visitFuncDef(SysY2022Parser::FuncDefContext *context) = 0;

    virtual std::any visitFuncType(SysY2022Parser::FuncTypeContext *context) = 0;

    virtual std::any visitFuncFParams(SysY2022Parser::FuncFParamsContext *context) = 0;

    virtual std::any visitFuncFParam(SysY2022Parser::FuncFParamContext *context) = 0;

    virtual std::any visitBlock(SysY2022Parser::BlockContext *context) = 0;

    virtual std::any visitBlockItem(SysY2022Parser::BlockItemContext *context) = 0;

    virtual std::any visitAssignStmt(SysY2022Parser::AssignStmtContext *context) = 0;

    virtual std::any visitExpStmt(SysY2022Parser::ExpStmtContext *context) = 0;

    virtual std::any visitBlockStmt(SysY2022Parser::BlockStmtContext *context) = 0;

    virtual std::any visitIfStmt(SysY2022Parser::IfStmtContext *context) = 0;

    virtual std::any visitIfElseStmt(SysY2022Parser::IfElseStmtContext *context) = 0;

    virtual std::any visitWhileStmt(SysY2022Parser::WhileStmtContext *context) = 0;

    virtual std::any visitBreakStmt(SysY2022Parser::BreakStmtContext *context) = 0;

    virtual std::any visitContinueStmt(SysY2022Parser::ContinueStmtContext *context) = 0;

    virtual std::any visitReturnStmt(SysY2022Parser::ReturnStmtContext *context) = 0;

    virtual std::any visitExp(SysY2022Parser::ExpContext *context) = 0;

    virtual std::any visitCond(SysY2022Parser::CondContext *context) = 0;

    virtual std::any visitLVal(SysY2022Parser::LValContext *context) = 0;

    virtual std::any visitExpPrimaryExp(SysY2022Parser::ExpPrimaryExpContext *context) = 0;

    virtual std::any visitLValPrimaryExp(SysY2022Parser::LValPrimaryExpContext *context) = 0;

    virtual std::any visitNumberPrimaryExp(SysY2022Parser::NumberPrimaryExpContext *context) = 0;

    virtual std::any visitNumber(SysY2022Parser::NumberContext *context) = 0;

    virtual std::any visitPrimaryUnaryExp(SysY2022Parser::PrimaryUnaryExpContext *context) = 0;

    virtual std::any visitFunctionCallUnaryExp(SysY2022Parser::FunctionCallUnaryExpContext *context) = 0;

    virtual std::any visitUnaryUnaryExp(SysY2022Parser::UnaryUnaryExpContext *context) = 0;

    virtual std::any visitUnaryOp(SysY2022Parser::UnaryOpContext *context) = 0;

    virtual std::any visitFuncRParams(SysY2022Parser::FuncRParamsContext *context) = 0;

    virtual std::any visitFuncRParam(SysY2022Parser::FuncRParamContext *context) = 0;

    virtual std::any visitMulMulExp(SysY2022Parser::MulMulExpContext *context) = 0;

    virtual std::any visitUnaryMulExp(SysY2022Parser::UnaryMulExpContext *context) = 0;

    virtual std::any visitAddAddExp(SysY2022Parser::AddAddExpContext *context) = 0;

    virtual std::any visitMulAddExp(SysY2022Parser::MulAddExpContext *context) = 0;

    virtual std::any visitRelRelExp(SysY2022Parser::RelRelExpContext *context) = 0;

    virtual std::any visitAddRelExp(SysY2022Parser::AddRelExpContext *context) = 0;

    virtual std::any visitEqEqExp(SysY2022Parser::EqEqExpContext *context) = 0;

    virtual std::any visitRelEqExp(SysY2022Parser::RelEqExpContext *context) = 0;

    virtual std::any visitEqLAndExp(SysY2022Parser::EqLAndExpContext *context) = 0;

    virtual std::any visitLAndLAndExp(SysY2022Parser::LAndLAndExpContext *context) = 0;

    virtual std::any visitLAndLOrExp(SysY2022Parser::LAndLOrExpContext *context) = 0;

    virtual std::any visitLOrLOrExp(SysY2022Parser::LOrLOrExpContext *context) = 0;

    virtual std::any visitConstExp(SysY2022Parser::ConstExpContext *context) = 0;


};

