parser grammar SYsUParser;

options {
  tokenVocab=SYsULexer;
}

primaryExpression
    :   Identifier
    |   Constant
    |   LeftParen additiveExpression RightParen
    ;

postfixExpression
    :   primaryExpression 
    |   postfixExpression LeftBracket expression RightBracket // 支持数组下标 
    ;

unaryExpression
    :
    (postfixExpression
    |   unaryOperator unaryExpression
    )
    ;

unaryOperator
    :   Plus | Minus 
    ;

multiplicativeExpression
    :   unaryExpression ((Star|Slash|Percent) unaryExpression)*
    ;

additiveExpression
    :   multiplicativeExpression ((Plus|Minus) multiplicativeExpression)*
    ;

shiftExpression
    : additiveExpression ((LeftShift | RightShift) additiveExpression)*
    ;

relationalExpression
    : shiftExpression ((Less | Greater | LessEqual | GreaterEqual) shiftExpression)*
    ;

equalityExpression
    : relationalExpression ((EqualEqual | NotEqual) relationalExpression)*
    ;

andExpression
    : equalityExpression (BitAnd equalityExpression)*
    ;

exclusiveOrExpression
    : andExpression (BitXor andExpression)*
    ;

inclusiveOrExpression
    : exclusiveOrExpression (BitOr exclusiveOrExpression)*
    ;

logicalAndExpression
    : inclusiveOrExpression (And inclusiveOrExpression)*
    ;

logicalOrExpression
    : logicalAndExpression (Or logicalAndExpression)*
    ;

conditionalExpression
    : logicalOrExpression (Question expression Colon conditionalExpression)?
    ;

assignmentExpression
    :   additiveExpression
    |   unaryExpression Equal assignmentExpression
    ;

expression
    :   assignmentExpression (Comma assignmentExpression)*
    ;

constantExpression
    : conditionalExpression
    ;

declaration
    :   declarationSpecifiers initDeclaratorList? Semi
    ;

// 新增类型限定符规则
typeQualifier
    :   Const
    ;

// 扩展声明说明符
declarationSpecifier
    :   typeSpecifier
    |   typeQualifier
    ;

declarationSpecifiers
    :   (declarationSpecifier)+
    ;

initDeclaratorList
    :   initDeclarator (Comma initDeclarator)*
    ;

initDeclarator
    :   declarator (Equal initializer)?
    ;


typeSpecifier
    :   Int
    ;


declarator
    :   directDeclarator
    ;

directDeclarator
    :   Identifier
    |   directDeclarator LeftBracket assignmentExpression? RightBracket
    ;

identifierList
    :   Identifier (Comma Identifier)*
    ;

initializer
    :   assignmentExpression
    |   LeftBrace initializerList? Comma? RightBrace
    ;

initializerList
    :   designation? initializer (Comma designation? initializer)*
    ;

designation
    : designatorList Equal
    ;

designatorList
    : designator+
    ;

designator
    : LeftBracket constantExpression RightBracket
    | Dot Identifier
    ;

statement
    :   compoundStatement
    |   expressionStatement
    |   selectionStatement
    |   jumpStatement
    ;

selectionStatement
    : If LeftParen expression RightParen statement (Else statement)?
    //| Switch LeftParen expression RightParen statement
    ;

compoundStatement
    :   LeftBrace blockItemList? RightBrace
    ;

blockItemList
    :   blockItem+
    ;

blockItem
    :   statement
    |   declaration
    ;

expressionStatement
    :   expression? Semi
    ;



jumpStatement
    :   (Return expression?)
    Semi
    ;

compilationUnit
    :   translationUnit? EOF
    ;

translationUnit
    :   externalDeclaration+
    ;

externalDeclaration
    :   functionDefinition
    |   declaration
    ;

functionDefinition
    : declarationSpecifiers directDeclarator LeftParen RightParen compoundStatement
    ;

