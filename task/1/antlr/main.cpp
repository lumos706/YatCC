#include "SYsULexer.h" // 确保这里的头文件名与您生成的词法分析器匹配
#include <fstream>
#include <iostream>
#include <unordered_map>

// 映射定义，将ANTLR的tokenTypeName映射到clang的格式
std::unordered_map<std::string, std::string> tokenTypeMapping = {
  { "Int", "int" },
  { "Const", "const"},
  { "Identifier", "identifier" },
  { "LeftParen", "l_paren" },
  { "RightParen", "r_paren" },
  { "RightBrace", "r_brace" },
  { "LeftBrace", "l_brace" },
  { "LeftBracket", "l_square" },
  { "RightBracket", "r_square" },
  { "Constant", "numeric_constant" },
  { "Return", "return" },
  { "Semi", "semi" },
  { "EOF", "eof" },
  { "Equal", "equal" },
  { "Plus", "plus" },
  { "Comma", "comma" },
  { "Minus", "minus" },
  { "Star", "star" },
  { "Slash", "slash" },
  { "Percent", "percent" },
  { "BitAnd", "and" },
  { "BitOr", "or" },
  { "BitXor", "xor" },
  { "Not", "not" },
  { "Less", "less" },
  { "LessEqual", "lessequal" },
  { "Greater", "greater" },
  { "GreaterEqual", "greaterequal" },
  { "EqualEqual", "equalequal" },
  { "NotEqual", "exclaimequal" },
  { "If", "if" },
  { "Else", "else" },
  { "While", "while" },
  { "For", "for" },
  { "Break", "break" },
  { "Exclaim", "exclaim"},
  { "Continue", "continue" },
  { "Void", "void" },
  { "Char", "char" },
  { "String", "string" },
  { "Struct", "struct" },
  { "Sizeof", "sizeof" },
  { "Typeof", "typeof" },
  { "True", "true" },
  { "False", "false" },
  { "Null", "null" },
  { "New", "new" },
  { "Delete", "delete" },
  { "Arrow", "arrow" },
  { "Colon", "colon" },
  { "Question", "question" },
  { "And", "ampamp" },
  { "Or", "pipepipe" },
  { "HEX_NUMBER", "numeric_constant"}

  // 在这里继续添加其他映射
};

void print_token(const antlr4::Token* token,
            const antlr4::CommonTokenStream& tokens,
            std::ofstream& outFile,
            const antlr4::Lexer& lexer,
            std::vector<std::pair<std::string, int>> fileHistory = std::vector<std::pair<std::string, int>>(),
            std::vector<int> preprocessedLines = std::vector<int>())
{
  auto& vocabulary = lexer.getVocabulary();

  auto tokenTypeName =
    std::string(vocabulary.getSymbolicName(token->getType()));

  if (tokenTypeName.empty())
    tokenTypeName = "<UNKNOWN>"; // 处理可能的空字符串情况

  if (tokenTypeMapping.find(tokenTypeName) != tokenTypeMapping.end()) {
    tokenTypeName = tokenTypeMapping[tokenTypeName];
  }

  std::string filename = "";
  int line = 0;
  int column = token->getCharPositionInLine() + 1;
  int x = 0;
  if(preprocessedLines.size() != 0 && fileHistory.size() != 0){
    while (x < preprocessedLines.size() - 1 && preprocessedLines[x + 1] <= token->getLine()) {
      x++;
    }
  }
  // 计算 line 和 filename
  line = token->getLine() - preprocessedLines[x] + fileHistory[x].second - 1;
  filename = fileHistory[x].first;

  std::string locInfo = "\t Loc=<" + filename + ":" + std::to_string(line) + ":" + std::to_string(column) + ">";

  // 判断是否在行首
  bool startOfLine = false;
  if (token->getTokenIndex() > 0) {
    // 获取当前行的所有 token
    int currentLine = token->getLine();
    int index = token->getTokenIndex() - 1;
    while (index >= 0) {
        auto prevToken = tokens.get(index);
        if (prevToken->getLine() != currentLine) {
            // 已经遍历到上一行，说明当前 token 是当前行的第一个 token
            if(token->getText() != "<EOF>")
              startOfLine = true;
            break;
        }
        if (prevToken->getType() != SYsULexer::Whitespace && prevToken->getType() != SYsULexer::Newline) {
            // 当前 token 前面有非空白字符，不是行首
            break;
        }
        index--;
    }
  } else {
      // 第一个 token 一定是行首
      startOfLine = true;
  }


  // 判断前面是否有空格
  bool leadingSpace = false;
  if (token->getTokenIndex() > 0) {
    // 获取当前行的 token
    int currentLine = token->getLine();
    int index = token->getTokenIndex() - 1;
    while (index >= 0) {
        auto prevToken = tokens.get(index);
        if (prevToken->getLine() != currentLine) {
            // 已经遍历到上一行，说明当前 token 是当前行的第一个 token
          if(token->getText() != "<EOF>" && token->getCharPositionInLine () > 0){
            leadingSpace = true;
            break;
          }
          leadingSpace = false;
          break;
        }
        if (prevToken->getText() != "<EOF>" && prevToken->getCharPositionInLine() + prevToken->getText().length() != token->getCharPositionInLine()) {
            leadingSpace = true;
            break;
        }
        leadingSpace = false;
        break;
    }
  } else {
      // 第一个 token 一定是行首
      leadingSpace = false;
  }


  if (token->getText() != "<EOF>")
    outFile << tokenTypeName << " '" << token->getText() << "'";
  else
    outFile << tokenTypeName << " '"
            << "'";


  if (startOfLine)
    outFile << "\t [StartOfLine]";
  if (leadingSpace)
    outFile << " [LeadingSpace]";

    
  outFile << locInfo << std::endl;
}

int
main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " <input> <output>\n";
    return -1;
  }

  std::ifstream inFile(argv[1]);
  if (!inFile) {
    std::cout << "Error: unable to open input file: " << argv[1] << '\n';
    return -2;
  }

  std::ofstream outFile(argv[2]);
  if (!outFile) {
    std::cout << "Error: unable to open output file: " << argv[2] << '\n';
    return -3;
  }

  std::cout << "程序 '" << argv[0] << std::endl;
  std::cout << "输入 '" << argv[1] << std::endl;
  std::cout << "输出 '" << argv[2] << std::endl;

  antlr4::ANTLRInputStream input(inFile);
  SYsULexer lexer(&input);

  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  for (auto&& token : tokens.getTokens()) {
    print_token(token, tokens, outFile, lexer, lexer.fileHistory, lexer.preprocessedLines);
  }
}
