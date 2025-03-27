lexer grammar SYsULexer;
@lexer::members {
    public: std::vector<int> preprocessedLines = std::vector<int>();
    public: std::vector<std::pair<std::string, int>> fileHistory = std::vector<std::pair<std::string, int>>();
    // 解析预处理指令，更新当前文件和行号
    public: void updateFileAndLine(const std::string& text, int newLine) {
        std::istringstream stream(text);
        std::string part;
        std::vector<std::string> parts;

        while (stream >> part) {
            parts.push_back(part);
        }

        if (parts.size() >= 3) {
            int lineNumber = std::stoi(parts[1]);
            std::string fileName = parts[2];

            // 去掉引号
            if (fileName.front() == '"' && fileName.back() == '"') {
                fileName = fileName.substr(1, fileName.length() - 2);
            }

            if (!preprocessedLines.empty() && newLine <= preprocessedLines.back() + 1) {
                preprocessedLines.back() = newLine;
                fileHistory.back() = {fileName, lineNumber};
            } else {
                preprocessedLines.push_back(newLine);
                fileHistory.emplace_back(fileName, lineNumber);
            }
        }
    }
}

Int: 'int';
Char: 'char';
Void: 'void';
If: 'if';
Else: 'else';
While: 'while';
For: 'for';
Break: 'break';
Bool: 'bool';
String: 'string';
Short: 'short';
Long: 'long';
Float: 'float';
Double: 'double';
Unsigned: 'unsigned';
Signed: 'signed';
Const: 'const';
Static: 'static';
Auto: 'auto';
Register: 'register';
Extern: 'extern';
Volatile: 'volatile';
Inline: 'inline';
Restrict: 'restrict';
Size_t: 'size_t';
Typedef: 'typedef';
Union: 'union';
Do: 'do';
True: 'true';
False: 'false';
Sizeof: 'sizeof';
Typeof: 'typeof';
Struct: 'struct';
Enum: 'enum';
Switch: 'switch';
Case: 'case';
Default: 'default';
Goto: 'goto';
Continue: 'continue';
Return: 'return';

LeftParen: '(';
RightParen: ')';
LeftBracket: '[';
RightBracket: ']';
LeftBrace: '{';
RightBrace: '}';

Plus: '+';
Minus: '-';
Star: '*';
Slash: '/';
Percent: '%';
Semi: ';';
Comma: ',';
Question: '?';
Colon: ':';
Dot: '.';
Arrow: '->';
Ellipsis: '...';
And: '&&';
Or: '||';
Exclaim: '!';
Tilde: '~';
BitOr: '|';
BitXor: '^';
BitAnd: '&';
Equal: '=';
EqualEqual: '==';
NotEqual: '!=';
Less: '<';
Greater: '>';
LessEqual: '<=';
GreaterEqual: '>=';
LeftShift: '<<';
RightShift: '>>';
PlusEqual: '+=';
MinusEqual: '-=';
StarEqual: '*=';
SlashEqual: '/=';
ModEqual: '%=';
AndEqual: '&=';
OrEqual: '|=';
XorEqual: '^=';
LeftShiftEqual: '<<=';
RightShiftEqual: '>>=';
HEX_NUMBER: '0x' [0-9a-fA-F]+;
Identifier: IdentifierNondigit ( IdentifierNondigit | Digit)*;

fragment IdentifierNondigit: Nondigit;

fragment Nondigit: [a-zA-Z_];

fragment Digit: [0-9];

Constant: IntegerConstant;

fragment IntegerConstant: DecimalConstant | OctalConstant;

fragment DecimalConstant: NonzeroDigit Digit*;

fragment OctalConstant: '0' OctalDigit*;

fragment NonzeroDigit: [1-9];

fragment OctalDigit: [0-7];

// 预处理信息处理，可以从预处理信息中获得文件名以及行号 预处理信息前面的数组即行号
LineAfterPreprocessing:
	'#' Whitespace* ~[\r\n]* {
            updateFileAndLine(getText(), getLine());
        } -> skip;

Whitespace: [ \t]+ -> skip;

// 换行符号，可以利用这个信息来更新行号
Newline: ( '\r' '\n'? | '\n') -> skip;