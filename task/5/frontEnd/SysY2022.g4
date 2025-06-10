grammar SysY2022;

compUnit: (decl | funcDef)*;

decl: constDecl | varDecl;

constDecl: 'const' bType constDef (',' constDef)* ';';

bType: 'int' | 'float';

constDef: Identifier ('[' constExp ']')* '=' constInitVal;

constInitVal:
	constExp										# itemConstInitVal
	| '{' (constInitVal (',' constInitVal)*)? '}'	# listConstInitVal;

varDecl: bType varDef (',' varDef)* ';';

varDef:
	Identifier ('[' constExp ']')*					# varDefWithOutInitVal
	| Identifier ('[' constExp ']')* '=' initVal	# varDefWithInitVal;

initVal:
	exp									# itemInitVal
	| '{' (initVal (',' initVal)*)? '}'	# listInitVal;

funcDef: funcType Identifier '(' (funcFParams)? ')' block;

funcType: 'void' | 'int' | 'float';

funcFParams: funcFParam (',' funcFParam)*;


// bug:第一维可以有数
funcFParam: bType Identifier ('[' ']' ('[' constExp ']')*)?;

block: '{' (blockItem)* '}';

blockItem: decl | stmt;

stmt:
	lVal '=' exp ';'						# assignStmt
	| (exp)? ';'							# expStmt
	| block									# blockStmt
	| 'if' '(' cond ')' stmt				# ifStmt
	| 'if' '(' cond ')' stmt 'else' stmt	# ifElseStmt
	| 'while' '(' cond ')' stmt				# whileStmt
	| 'break' ';'							# breakStmt
	| 'continue' ';'						# continueStmt
	| 'return' (exp)? ';'					# returnStmt;

exp: addExp;

cond: lOrExp;

lVal: Identifier ('[' exp ']')*;

primaryExp:
	'(' exp ')'	# expPrimaryExp
	| lVal		# lValPrimaryExp
	| number	# numberPrimaryExp;

number: FloatConst | IntConst;

unaryExp:
	primaryExp							# primaryUnaryExp
	| Identifier '(' (funcRParams)? ')'	# functionCallUnaryExp
	| unaryOp unaryExp					# unaryUnaryExp;

unaryOp: '+' | '-' | '!';

funcRParams: funcRParam (',' funcRParam)*;

funcRParam: exp;

mulExp:
	unaryExp							# unaryMulExp
	| mulExp ('*' | '/' | '%') unaryExp	# mulMulExp;

addExp:
	mulExp						# mulAddExp
	| addExp ('+' | '-') mulExp	# addAddExp;

relExp:
	addExp										# addRelExp
	| relExp ('<' | '>' | '<=' | '>=') addExp	# relRelExp;

eqExp: relExp # relEqExp | eqExp ('==' | '!=') relExp # eqEqExp;

lAndExp: eqExp # eqLAndExp | lAndExp '&&' eqExp # lAndLAndExp;

lOrExp: lAndExp # lAndLOrExp | lOrExp '||' lAndExp # lOrLOrExp;

constExp: addExp;

// to-do：这里可能需要精简 相关测例 95_float.sy
FloatConst:
	[0-9]+ '.' [0-9]*
	| '.' [0-9]+ ([eE][+-]? [0-9]+)?
	| [0-9]+ ('.' [0-9]*)? [eE][+-]? [0-9]+
	| '0x' [0-9a-fA-F]+ '.' [0-9a-fA-F]*
	| '0x' '.' [0-9a-fA-F]+ ([pP][+-]? [0-9a-fA-F]+)?
	| '0x' [0-9a-fA-F]+ ('.' [0-9a-fA-F]*)? [pP][+-]? [0-9a-fA-F]+;

IntConst: [0-9]+ | '0x' [0-9a-fA-F]+;

Identifier: [a-zA-Z_][a-zA-Z_0-9]*;

WS: [\f\r\t\n ]+ -> skip;

COMMENT: '/*' .*? '*/' -> skip;

LINE_COMMENT: '//' ~[\r\n]* -> skip;
