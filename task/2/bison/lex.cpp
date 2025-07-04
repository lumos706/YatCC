#include "lex.hpp"
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace lex {

G g;

int
come_line(const char* yytext, int yyleng, int yylineno)
{
  char name[64];
  char value[64];
  sscanf(yytext, "%s '%[^']'", name, value);

static const std::unordered_map<std::string, int> kTokenId = {
  { "identifier", IDENTIFIER },
  { "numeric_constant", CONSTANT },
  { "int", INT },
  { "void", VOID },
  { "return", RETURN },
  { "l_paren", '(' },
  { "r_paren", ')' },
  { "l_brace", '{' },
  { "r_brace", '}' },
  { "semi", ';' },
  { "equal", '=' },
  { "l_square", '[' },
  { "r_square", ']' },
  { "comma", ',' },
  { "minus", '-' },
  { "plus", '+' },
  { "star", '*' },
  { "slash", '/' }, 
  { "percent", '%' },
  { "less", '<' },
  { "greater", '>' },
  { "if", IF },
  { "else", ELSE },
  { "continue", CONTINUE },
  { "while", WHILE },
  { "do", DO },
  { "for", FOR },
  { "break", BREAK },
  { "lessequal", LE_OP },
  { "greaterequal", GE_OP },
  { "equalequal", EQ_OP },
  { "exclaimequal", NE_OP },
  { "ampamp", AND_OP },
  { "pipepipe", OR_OP },
  { "const", CONST },
  { "string_literal", STRING_LITERAL },
  { "exclaim", '!' },
  { "eof", YYEOF }
};


  auto iter = kTokenId.find(name);
  if (iter == kTokenId.end()) {
    std::cerr << "Unknown token: " << name << std::endl;
    return YYEOF; // 返回EOF作为错误处理
  }

  yylval.RawStr = new std::string(value, strlen(value));
  return iter->second;
}

int
come(int tokenId, const char* yytext, int yyleng, int yylineno)
{
  g.mId = tokenId;
  g.mText = { yytext, std::size_t(yyleng) };
  g.mLine = yylineno;

  g.mStartOfLine = false;
  g.mLeadingSpace = false;

  return tokenId;
}

} // namespace lex
