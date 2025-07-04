/* 生成.output文件 */
%verbose

/* 用于调试 (yydebug) */
%define parse.trace

%code top {
int yylex (void);             // 该函数由 Flex 生成
void yyerror (char const *);	// 该函数定义在 par.cpp 中
}

%code requires {
#include "par.hpp"
#include <iostream>
}

%union {
  std::string* RawStr;
  par::Decls* Decls;
  par::Exprs* Exprs;

  asg::TranslationUnit* TranslationUnit;
  asg::Type* Type;
  asg::Expr* Expr;
  asg::Decl* Decl;
  asg::FunctionDecl* FunctionDecl;
  asg::Stmt* Stmt;
  asg::CompoundStmt* CompoundStmt;
  asg::ExprStmt* ExprStmt;
  asg::ReturnStmt* ReturnStmt;
  asg::IfStmt* IfStmt;
  asg::WhileStmt* WhileStmt;
  asg::ContinueStmt* ContinueStmt;
  asg::BreakStmt* BreakStmt;
  asg::NullStmt* NullStmt;
}

/* 在下面说明每个非终结符对应的 union 成员，以便进行编译期类型检查 */
%type <Type> declaration_specifiers type_specifier
%type <Expr> expression primary_expression postfix_expression unary_expression 
%type <Expr> multiplicative_expression additive_expression relational_expression
%type <Expr> equality_expression logical_and_expression logical_or_expression
%type <Expr> assignment_expression initializer initializer_list

%type <Stmt> block_item statement expression_statement
%type <CompoundStmt> compound_statement block_item_list
// expression_statement 不能转换为 NullStmt 但是有基类Stmt
%type <ReturnStmt> jump_statement

%type <Decls> external_declaration declaration init_declarator_list parameter_list
%type <Exprs> argument_expression_list
%type <FunctionDecl> function_definition
%type <Decl> declarator init_declarator parameter_declaration

%type <TranslationUnit> translation_unit


%token <RawStr> IDENTIFIER CONSTANT
%token <RawStr> STRING_LITERAL
%token CONST INT VOID 
%token IF ELSE WHILE DO FOR
%token BREAK CONTINUE
%token RETURN 

%token LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP

%start start

%%

// 起始符号
start
  : {
      par::Symtbl::g = new par::Symtbl();
    }
    translation_unit
    {
      par::gTranslationUnit = $2;
      delete par::Symtbl::g;
    }
  ;

translation_unit
  : external_declaration
    {
      $$ = par::gMgr.make<asg::TranslationUnit>();
      for (auto&& decl: *$1)
        $$->decls.push_back(decl);
      delete $1;
    }
  | translation_unit external_declaration
    {
      $$ = $1;
      for (auto&& decl: *$2)
        $$->decls.push_back(decl);
      delete $2;
    }
  ;

external_declaration
  : function_definition
    {
      $$ = new par::Decls();
      $$->push_back($1);
    }
  | declaration { $$ = $1; }
  ;

function_definition
  : declaration_specifiers declarator
    {
      auto funcDecl = $2->dcst<asg::FunctionDecl>();
      ASSERT(funcDecl);
      // 设置当前全局的函数作用变量
      par::gCurrentFunction = funcDecl;
      auto ty = par::gMgr.make<asg::Type>();
      if (funcDecl->type != nullptr)
        ty->texp = funcDecl->type->texp;
      ty->spec = $1->spec, ty->qual = $1->qual;
      funcDecl->type = ty;

    }
    compound_statement
    {
      $$ = par::gCurrentFunction;
      $$->name = $2->name;
      $$->body = $4;
    }
  ;

declaration
  : declaration_specifiers init_declarator_list ';'
    {
      for (auto decl: *$2) 
      {
        auto ty = par::gMgr.make<asg::Type>();
        if (decl->type != nullptr)
          ty->texp = decl->type->texp; // 保留前面 ArrayType 的texp
        ty->spec = $1->spec, ty->qual = $1->qual;
        decl->type = ty;
        auto varDecl = dynamic_cast<asg::VarDecl*>(decl);
        if (varDecl != nullptr)
        {
          if (varDecl->init != nullptr)
            varDecl->init->type = decl->type;
        }
      }
      $$ = $2;
    }
  ;

declaration_specifiers
  : type_specifier { $$ = $1; }
  | type_specifier declaration_specifiers
    {
      $$ = $2;
      $$->spec = $1->spec;
    }
    // 类型说明符const
  | CONST type_specifier
    {
      $$ = $2;
      $$->qual.const_ = true;
    }
  ;

type_specifier
  : VOID
    {
      $$ = par::gMgr.make<asg::Type>();
      $$->spec = asg::Type::Spec::kVoid;
    }
  | INT
    {
      $$ = par::gMgr.make<asg::Type>();
      $$->spec = asg::Type::Spec::kInt;
    }
  ;

declarator
  : IDENTIFIER
    {
      $$ = par::gMgr.make<asg::VarDecl>();
      $$->name = std::move(*$1);
      delete $1;

      // 插入符号表
      par::Symtbl::g->insert_or_assign($$->name, $$);
    }
  | declarator '[' ']' // 未知长度数组
    {
      $$ = $1;
      // 填充Type
      auto ty = par::gMgr.make<asg::Type>();
      if ($$->type != nullptr)
        ty->texp = $$->type->texp;
      auto p = par::gMgr.make<asg::ArrayType>();
      p->len = asg::ArrayType::kUnLen;
      if (ty->texp == nullptr)
      {
        ty->texp = p;
      }
      else
      {
        ty->texp->sub = p;
      }
      $$->type = ty;

      // 插入符号表
      par::Symtbl::g->insert_or_assign($$->name, $$);
    }
  | declarator '[' assignment_expression ']' // 数组定义
    {
      $$ = $1;
      // 填充Type
      auto ty = par::gMgr.make<asg::Type>();
      if ($$->type != nullptr)
        ty->texp = $$->type->texp;
      auto p = par::gMgr.make<asg::ArrayType>();
      auto integerLiteral = $3->dcst<asg::IntegerLiteral>();
      ASSERT(integerLiteral);
      p->len = integerLiteral->val;
      if (ty->texp == nullptr)
      {
        ty->texp = p;
      }
      else
      {
        ty->texp->sub = p;
      }
      $$->type = ty;

      // 插入符号表
      par::Symtbl::g->insert_or_assign($$->name, $$);
    }
  | declarator '(' ')'
    {
      $$ = par::gMgr.make<asg::FunctionDecl>();
      $$->name = $1->name;
      auto ty = par::gMgr.make<asg::Type>();
      auto p = par::gMgr.make<asg::FunctionType>();
      ty->texp = p;
      $$->type = ty;
      
      // 插入符号表
      par::Symtbl::g->insert_or_assign($$->name, $$);
    }
  // 函数列表的定义
  | declarator '(' parameter_list ')'
    {
      auto p = par::gMgr.make<asg::FunctionDecl>();
      p->name = $1->name;
      p->params = *$3;
      auto ty = par::gMgr.make<asg::Type>();
      auto functionType = par::gMgr.make<asg::FunctionType>();
      for (auto decl: *$3)
      {
        functionType->params.push_back(decl->type);
      }
      ty->texp = functionType;
      p->type = ty;
      $$ = p;

      // 插入符号表
      par::Symtbl::g->insert_or_assign($$->name, $$);
    }
  ;

parameter_list
  : parameter_declaration
    {
      $$ = new par::Decls();
      $$->push_back($1);
    }
  | parameter_list ',' parameter_declaration
    {
      $$ = $1;
      $$->push_back($3);
    }
  ;

parameter_declaration
  : declaration_specifiers declarator
    {
      // 保留之前定义的 Type
      auto ty = par::gMgr.make<asg::Type>();
      if ($2->type != nullptr)
        ty->texp = $2->type->texp;
      ty->spec = $1->spec;
      ty->qual = $1->qual;
      $2->type = ty;
      $$ = $2;
    }
  ;

compound_statement
  : { $$ = par::gMgr.make<asg::CompoundStmt>(); } // 代码块为空的情况
  |'{' '}' { $$ = par::gMgr.make<asg::CompoundStmt>(); }
  | '{'
    { new par::Symtbl(); } 		// 开启新的符号表作用域
    block_item_list
    '}'
    {
      delete par::Symtbl::g; 	// 结束符号表作用域
      $$ = $block_item_list;
    }
  ;

block_item_list
  : block_item
    {
      $$ = par::gMgr.make<asg::CompoundStmt>();
      $$->subs.push_back($1);
    }
  | block_item_list block_item
    {
      $$ = $1;
      $$->subs.push_back($2);
    }
  ;

block_item
  : declaration
    {
      auto p = par::gMgr.make<asg::DeclStmt>();
      for (auto decl: *$1)
        p->decls.push_back(decl);
      $$ = p;
    }
  | statement { $$ = $1; }
  ;

statement
  : compound_statement { $$ = $1; }
  | expression_statement { $$ = $1; }
  | jump_statement { $$ = $1; }
  ;
  | IF '(' expression ')' statement // if结构
    {
      auto p = par::gMgr.make<asg::IfStmt>();
      p->cond = $3;
      p->then = $5;
      $$ = p;
    }
  | IF '(' expression ')' statement ELSE statement // if-else结构
    {
      auto p = par::gMgr.make<asg::IfStmt>();
      p->cond = $3;
      p->then = $5;
      p->else_ = $7;
      $$ = p;
    }
  | WHILE '(' expression ')' statement // while结构
    {
      auto p = par::gMgr.make<asg::WhileStmt>();
      p->cond = $3;
      p->body = $5;
      $$ = p;
    }
  | DO statement WHILE '(' expression ')' ';' // do-while结构
    {
      auto p = par::gMgr.make<asg::DoStmt>();
      p->body = $2;
      p->cond = $5;
      $$ = p;
    }
  | CONTINUE ';' // continue语句
  {
    $$ = par::gMgr.make<asg::ContinueStmt>();
  }
  | BREAK ';' // break语句
    {
      $$ = par::gMgr.make<asg::BreakStmt>();
    }
  ;

expression_statement
  : ';' // 空
    {
      $$ = par::gMgr.make<asg::NullStmt>();
    }
  | expression ';'
    {
      auto stmt = par::gMgr.make<asg::ExprStmt>();
      stmt->expr = $1;
      $$ = stmt;
    }
  ;

jump_statement
  : RETURN ';'
    {
      auto p = par::gMgr.make<asg::ReturnStmt>();
      p->func = par::gCurrentFunction;
      $$ = p;
    }
  | RETURN expression ';'
    {
      auto p = par::gMgr.make<asg::ReturnStmt>();
      p->func = par::gCurrentFunction;
      p->expr = $2;
      $$ = p;
    }
  ;
expression
  : assignment_expression { $$ = $1; }
  | expression ',' assignment_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kComma;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  ;

assignment_expression
  : logical_or_expression { $$ = $1; }
  | unary_expression '=' assignment_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kAssign;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  ;

logical_or_expression
  : logical_and_expression { $$ = $1; }
  | logical_or_expression OR_OP logical_and_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kOr;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  ;

logical_and_expression
  : equality_expression { $$ = $1; }
  | logical_and_expression AND_OP equality_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kAnd;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  ;

equality_expression
  : relational_expression { $$ = $1; }
  | equality_expression EQ_OP relational_expression // 等于
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kEq;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  | equality_expression NE_OP relational_expression // 不等于
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kNe;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  ;

relational_expression
  : additive_expression { $$ = $1; }
  | relational_expression '<' additive_expression // 小于
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kLt;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  | relational_expression '>' additive_expression // 大于
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kGt;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  | relational_expression LE_OP additive_expression // 小于等于
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kLe;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  | relational_expression GE_OP additive_expression // 大于等于
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kGe;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  ;

additive_expression
  : multiplicative_expression { $$ = $1; }
  | additive_expression '+' multiplicative_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kAdd;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  | additive_expression '-' multiplicative_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kSub;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  ;

multiplicative_expression
  : unary_expression { $$ = $1; }
  | multiplicative_expression '*' unary_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kMul;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  | multiplicative_expression '/' unary_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kDiv;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  | multiplicative_expression '%' unary_expression
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kMod;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  ;

unary_expression
  : postfix_expression { $$ = $1; }
  | '-' unary_expression
    {
      auto p = par::gMgr.make<asg::UnaryExpr>();
      p->op = asg::UnaryExpr::Op::kNeg;
      p->sub = $2;
      $$ = p;
    }
  | '+' unary_expression
    {
      auto p = par::gMgr.make<asg::UnaryExpr>();
      p->op = asg::UnaryExpr::Op::kPos;
      p->sub = $2;
      $$ = p;
    }
  | '!' unary_expression
    {
      auto p = par::gMgr.make<asg::UnaryExpr>();
      p->op = asg::UnaryExpr::Op::kNot;
      p->sub = $2;
      $$ = p;
    }
  ;

postfix_expression
  : primary_expression { $$ = $1; }
  | postfix_expression '[' expression ']' // 数组下标
    {
      auto p = par::gMgr.make<asg::BinaryExpr>();
      p->op = asg::BinaryExpr::Op::kIndex;
      p->lft = $1, p->rht = $3;
      $$ = p;
    }
  | postfix_expression '(' ')' // 函数调用1
    {
      auto p = par::gMgr.make<asg::CallExpr>();
      p->head = $1;
      $$ = p;
    }
  | postfix_expression '(' argument_expression_list ')' // 函数调用2
    {
      auto p = par::gMgr.make<asg::CallExpr>();
      p->head = $1;
      p->args = std::move(*$3);
      delete $3;
      $$ = p;
    }
  ;

primary_expression
  : IDENTIFIER
    {
      // 查找符号表, 找到对应的Decl
      auto decl = par::Symtbl::resolve(*$1);
      ASSERT(decl);
      delete $1;
      auto p = par::gMgr.make<asg::DeclRefExpr>();
      p->decl = decl;
      $$ = p;
    }
  | CONSTANT
    {
      auto p = par::gMgr.make<asg::IntegerLiteral>();
      std::string numStr = *$1;
      if (numStr.size() > 2 && numStr[0] == '0' && (numStr[1] == 'X' || numStr[1] == 'x')) 
      {
        // 十六进制
        p->val = std::stoull(numStr, nullptr, 16);
      } 
      else if (numStr.size() > 1 && numStr[0] == '0') 
      {
        // 八进制
        p->val = std::stoull(numStr, nullptr, 8);
      } 
      else 
      {
        // 十进制
        p->val = std::stoull(numStr, nullptr, 10);
      }
      delete $1;
      $$ = p;
    }
  | STRING_LITERAL // 字符串字面量
    {
      auto p = par::gMgr.make<asg::StringLiteral>();
      p->val = std::move(*$1);
      delete $1;
      $$ = p;
    }
  | '(' expression ')' // 括号表达式
    {
      auto p = par::gMgr.make<asg::ParenExpr>();
      p->sub = $2;
      $$ = p;
    }
  ;

argument_expression_list
  : assignment_expression
    {
      $$ = new par::Exprs();
      $$->push_back($1);
    }
  | argument_expression_list ',' assignment_expression
    {
      $$ = $1;
      $$->push_back($3);
    }
  ;

init_declarator_list
  : init_declarator
    {
      $$ = new par::Decls();
      $$->push_back($1);
    }
  | init_declarator_list ',' init_declarator
    {
      $$ = $1;
      $$->push_back($3);
    }
  ;

init_declarator
  : declarator { $$ = $1; }
  | declarator '=' initializer
    {
      auto varDecl = $1->dcst<asg::VarDecl>();
      ASSERT(varDecl);
      $3->type = varDecl->type;
      varDecl->init = $3;
      $$ = varDecl;
    }
  ;

 // 初始化右值
initializer
  : assignment_expression
    {
      // 检查是否是InitListExpr或CallExpr类型
      auto initListExpr = $1->dcst<asg::InitListExpr>();
      auto callExpr = $1->dcst<asg::CallExpr>();
      
      if (callExpr != nullptr) 
      {
        auto implicitCastExpr = dynamic_cast<asg::ImplicitCastExpr*>(callExpr->head);
        if (implicitCastExpr != nullptr) {
          auto declRefExpr = dynamic_cast<asg::DeclRefExpr*>(implicitCastExpr->sub);
          if (declRefExpr != nullptr) {
            // 处理 ImplicitCastExpr -> DeclRefExpr
            $$ = callExpr;
          } else {
            // 如果没有 DeclRefExpr，仍然可以直接使用 callExpr
            $$ = callExpr;
          }
        }
      } else if (initListExpr != nullptr)
      {
        // 如果是InitListExpr，直接返回
        $$ = $1;
      }
      else
      {
        // 否则，创建一个新的InitListExpr并包装它
        auto p = par::gMgr.make<asg::InitListExpr>();
        p->list.push_back($1);
        $$ = p;
      }
      $$->type = $1->type;
    }
  | '{' initializer_list '}'
    {
      $$ = $2;
    }
  | '{' '}'
    {
      auto p = par::gMgr.make<asg::InitListExpr>();
      $$ = p;
    }
  ;

// 初始化列表
initializer_list
  : initializer
    {
      auto initListExpr = $1->dcst<asg::InitListExpr>();
      if (initListExpr == nullptr) 
      {
        auto p = par::gMgr.make<asg::InitListExpr>();
        p->list.push_back($1);
        $$ = p;
      } 
      else 
      {
        $$ = $1;
      }
    }
  | initializer_list ',' initializer
    {
      auto initListExpr3 = $3->dcst<asg::InitListExpr>();
      auto initListExpr1 = $1->dcst<asg::InitListExpr>();
      if (initListExpr3 != nullptr) 
      {
        for(auto expr: initListExpr3->list)
          initListExpr1->list.push_back(expr);
      }
      else 
      {
        initListExpr1->list.push_back($3);
      }
      $$ = initListExpr1;
    }
  ;

%%
