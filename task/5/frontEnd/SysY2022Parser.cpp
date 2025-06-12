
// Generated from src/frontEnd/SysY2022.g4 by ANTLR 4.12.0


#include "SysY2022Visitor.h"

#include "SysY2022Parser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct SysY2022ParserStaticData final {
  SysY2022ParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SysY2022ParserStaticData(const SysY2022ParserStaticData&) = delete;
  SysY2022ParserStaticData(SysY2022ParserStaticData&&) = delete;
  SysY2022ParserStaticData& operator=(const SysY2022ParserStaticData&) = delete;
  SysY2022ParserStaticData& operator=(SysY2022ParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag sysy2022ParserOnceFlag;
SysY2022ParserStaticData *sysy2022ParserStaticData = nullptr;

void sysy2022ParserInitialize() {
  assert(sysy2022ParserStaticData == nullptr);
  auto staticData = std::make_unique<SysY2022ParserStaticData>(
    std::vector<std::string>{
      "compUnit", "decl", "constDecl", "bType", "constDef", "constInitVal", 
      "varDecl", "varDef", "initVal", "funcDef", "funcType", "funcFParams", 
      "funcFParam", "block", "blockItem", "stmt", "exp", "cond", "lVal", 
      "primaryExp", "number", "unaryExp", "unaryOp", "funcRParams", "funcRParam", 
      "mulExp", "addExp", "relExp", "eqExp", "lAndExp", "lOrExp", "constExp"
    },
    std::vector<std::string>{
      "", "'const'", "','", "';'", "'int'", "'float'", "'['", "']'", "'='", 
      "'{'", "'}'", "'('", "')'", "'void'", "'if'", "'else'", "'while'", 
      "'break'", "'continue'", "'return'", "'+'", "'-'", "'!'", "'*'", "'/'", 
      "'%'", "'<'", "'>'", "'<='", "'>='", "'=='", "'!='", "'&&'", "'||'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "FloatConst", "IntConst", "Identifier", "WS", "COMMENT", "LINE_COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,39,372,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,1,0,1,0,5,0,67,8,0,10,0,12,0,70,9,0,
  	1,1,1,1,3,1,74,8,1,1,2,1,2,1,2,1,2,1,2,5,2,81,8,2,10,2,12,2,84,9,2,1,
  	2,1,2,1,3,1,3,1,4,1,4,1,4,1,4,1,4,5,4,95,8,4,10,4,12,4,98,9,4,1,4,1,4,
  	1,4,1,5,1,5,1,5,1,5,1,5,5,5,108,8,5,10,5,12,5,111,9,5,3,5,113,8,5,1,5,
  	3,5,116,8,5,1,6,1,6,1,6,1,6,5,6,122,8,6,10,6,12,6,125,9,6,1,6,1,6,1,7,
  	1,7,1,7,1,7,1,7,5,7,134,8,7,10,7,12,7,137,9,7,1,7,1,7,1,7,1,7,1,7,5,7,
  	144,8,7,10,7,12,7,147,9,7,1,7,1,7,3,7,151,8,7,1,8,1,8,1,8,1,8,1,8,5,8,
  	158,8,8,10,8,12,8,161,9,8,3,8,163,8,8,1,8,3,8,166,8,8,1,9,1,9,1,9,1,9,
  	3,9,172,8,9,1,9,1,9,1,9,1,10,1,10,1,11,1,11,1,11,5,11,182,8,11,10,11,
  	12,11,185,9,11,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,5,12,195,8,12,
  	10,12,12,12,198,9,12,3,12,200,8,12,1,13,1,13,5,13,204,8,13,10,13,12,13,
  	207,9,13,1,13,1,13,1,14,1,14,3,14,213,8,14,1,15,1,15,1,15,1,15,1,15,1,
  	15,3,15,221,8,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,
  	15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,
  	15,1,15,1,15,1,15,3,15,251,8,15,1,15,3,15,254,8,15,1,16,1,16,1,17,1,17,
  	1,18,1,18,1,18,1,18,1,18,5,18,265,8,18,10,18,12,18,268,9,18,1,19,1,19,
  	1,19,1,19,1,19,1,19,3,19,276,8,19,1,20,1,20,1,21,1,21,1,21,1,21,3,21,
  	284,8,21,1,21,1,21,1,21,1,21,3,21,290,8,21,1,22,1,22,1,23,1,23,1,23,5,
  	23,297,8,23,10,23,12,23,300,9,23,1,24,1,24,1,25,1,25,1,25,1,25,1,25,1,
  	25,5,25,310,8,25,10,25,12,25,313,9,25,1,26,1,26,1,26,1,26,1,26,1,26,5,
  	26,321,8,26,10,26,12,26,324,9,26,1,27,1,27,1,27,1,27,1,27,1,27,5,27,332,
  	8,27,10,27,12,27,335,9,27,1,28,1,28,1,28,1,28,1,28,1,28,5,28,343,8,28,
  	10,28,12,28,346,9,28,1,29,1,29,1,29,1,29,1,29,1,29,5,29,354,8,29,10,29,
  	12,29,357,9,29,1,30,1,30,1,30,1,30,1,30,1,30,5,30,365,8,30,10,30,12,30,
  	368,9,30,1,31,1,31,1,31,0,6,50,52,54,56,58,60,32,0,2,4,6,8,10,12,14,16,
  	18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,
  	0,8,1,0,4,5,2,0,4,5,13,13,1,0,34,35,1,0,20,22,1,0,23,25,1,0,20,21,1,0,
  	26,29,1,0,30,31,383,0,68,1,0,0,0,2,73,1,0,0,0,4,75,1,0,0,0,6,87,1,0,0,
  	0,8,89,1,0,0,0,10,115,1,0,0,0,12,117,1,0,0,0,14,150,1,0,0,0,16,165,1,
  	0,0,0,18,167,1,0,0,0,20,176,1,0,0,0,22,178,1,0,0,0,24,186,1,0,0,0,26,
  	201,1,0,0,0,28,212,1,0,0,0,30,253,1,0,0,0,32,255,1,0,0,0,34,257,1,0,0,
  	0,36,259,1,0,0,0,38,275,1,0,0,0,40,277,1,0,0,0,42,289,1,0,0,0,44,291,
  	1,0,0,0,46,293,1,0,0,0,48,301,1,0,0,0,50,303,1,0,0,0,52,314,1,0,0,0,54,
  	325,1,0,0,0,56,336,1,0,0,0,58,347,1,0,0,0,60,358,1,0,0,0,62,369,1,0,0,
  	0,64,67,3,2,1,0,65,67,3,18,9,0,66,64,1,0,0,0,66,65,1,0,0,0,67,70,1,0,
  	0,0,68,66,1,0,0,0,68,69,1,0,0,0,69,1,1,0,0,0,70,68,1,0,0,0,71,74,3,4,
  	2,0,72,74,3,12,6,0,73,71,1,0,0,0,73,72,1,0,0,0,74,3,1,0,0,0,75,76,5,1,
  	0,0,76,77,3,6,3,0,77,82,3,8,4,0,78,79,5,2,0,0,79,81,3,8,4,0,80,78,1,0,
  	0,0,81,84,1,0,0,0,82,80,1,0,0,0,82,83,1,0,0,0,83,85,1,0,0,0,84,82,1,0,
  	0,0,85,86,5,3,0,0,86,5,1,0,0,0,87,88,7,0,0,0,88,7,1,0,0,0,89,96,5,36,
  	0,0,90,91,5,6,0,0,91,92,3,62,31,0,92,93,5,7,0,0,93,95,1,0,0,0,94,90,1,
  	0,0,0,95,98,1,0,0,0,96,94,1,0,0,0,96,97,1,0,0,0,97,99,1,0,0,0,98,96,1,
  	0,0,0,99,100,5,8,0,0,100,101,3,10,5,0,101,9,1,0,0,0,102,116,3,62,31,0,
  	103,112,5,9,0,0,104,109,3,10,5,0,105,106,5,2,0,0,106,108,3,10,5,0,107,
  	105,1,0,0,0,108,111,1,0,0,0,109,107,1,0,0,0,109,110,1,0,0,0,110,113,1,
  	0,0,0,111,109,1,0,0,0,112,104,1,0,0,0,112,113,1,0,0,0,113,114,1,0,0,0,
  	114,116,5,10,0,0,115,102,1,0,0,0,115,103,1,0,0,0,116,11,1,0,0,0,117,118,
  	3,6,3,0,118,123,3,14,7,0,119,120,5,2,0,0,120,122,3,14,7,0,121,119,1,0,
  	0,0,122,125,1,0,0,0,123,121,1,0,0,0,123,124,1,0,0,0,124,126,1,0,0,0,125,
  	123,1,0,0,0,126,127,5,3,0,0,127,13,1,0,0,0,128,135,5,36,0,0,129,130,5,
  	6,0,0,130,131,3,62,31,0,131,132,5,7,0,0,132,134,1,0,0,0,133,129,1,0,0,
  	0,134,137,1,0,0,0,135,133,1,0,0,0,135,136,1,0,0,0,136,151,1,0,0,0,137,
  	135,1,0,0,0,138,145,5,36,0,0,139,140,5,6,0,0,140,141,3,62,31,0,141,142,
  	5,7,0,0,142,144,1,0,0,0,143,139,1,0,0,0,144,147,1,0,0,0,145,143,1,0,0,
  	0,145,146,1,0,0,0,146,148,1,0,0,0,147,145,1,0,0,0,148,149,5,8,0,0,149,
  	151,3,16,8,0,150,128,1,0,0,0,150,138,1,0,0,0,151,15,1,0,0,0,152,166,3,
  	32,16,0,153,162,5,9,0,0,154,159,3,16,8,0,155,156,5,2,0,0,156,158,3,16,
  	8,0,157,155,1,0,0,0,158,161,1,0,0,0,159,157,1,0,0,0,159,160,1,0,0,0,160,
  	163,1,0,0,0,161,159,1,0,0,0,162,154,1,0,0,0,162,163,1,0,0,0,163,164,1,
  	0,0,0,164,166,5,10,0,0,165,152,1,0,0,0,165,153,1,0,0,0,166,17,1,0,0,0,
  	167,168,3,20,10,0,168,169,5,36,0,0,169,171,5,11,0,0,170,172,3,22,11,0,
  	171,170,1,0,0,0,171,172,1,0,0,0,172,173,1,0,0,0,173,174,5,12,0,0,174,
  	175,3,26,13,0,175,19,1,0,0,0,176,177,7,1,0,0,177,21,1,0,0,0,178,183,3,
  	24,12,0,179,180,5,2,0,0,180,182,3,24,12,0,181,179,1,0,0,0,182,185,1,0,
  	0,0,183,181,1,0,0,0,183,184,1,0,0,0,184,23,1,0,0,0,185,183,1,0,0,0,186,
  	187,3,6,3,0,187,199,5,36,0,0,188,189,5,6,0,0,189,196,5,7,0,0,190,191,
  	5,6,0,0,191,192,3,62,31,0,192,193,5,7,0,0,193,195,1,0,0,0,194,190,1,0,
  	0,0,195,198,1,0,0,0,196,194,1,0,0,0,196,197,1,0,0,0,197,200,1,0,0,0,198,
  	196,1,0,0,0,199,188,1,0,0,0,199,200,1,0,0,0,200,25,1,0,0,0,201,205,5,
  	9,0,0,202,204,3,28,14,0,203,202,1,0,0,0,204,207,1,0,0,0,205,203,1,0,0,
  	0,205,206,1,0,0,0,206,208,1,0,0,0,207,205,1,0,0,0,208,209,5,10,0,0,209,
  	27,1,0,0,0,210,213,3,2,1,0,211,213,3,30,15,0,212,210,1,0,0,0,212,211,
  	1,0,0,0,213,29,1,0,0,0,214,215,3,36,18,0,215,216,5,8,0,0,216,217,3,32,
  	16,0,217,218,5,3,0,0,218,254,1,0,0,0,219,221,3,32,16,0,220,219,1,0,0,
  	0,220,221,1,0,0,0,221,222,1,0,0,0,222,254,5,3,0,0,223,254,3,26,13,0,224,
  	225,5,14,0,0,225,226,5,11,0,0,226,227,3,34,17,0,227,228,5,12,0,0,228,
  	229,3,30,15,0,229,254,1,0,0,0,230,231,5,14,0,0,231,232,5,11,0,0,232,233,
  	3,34,17,0,233,234,5,12,0,0,234,235,3,30,15,0,235,236,5,15,0,0,236,237,
  	3,30,15,0,237,254,1,0,0,0,238,239,5,16,0,0,239,240,5,11,0,0,240,241,3,
  	34,17,0,241,242,5,12,0,0,242,243,3,30,15,0,243,254,1,0,0,0,244,245,5,
  	17,0,0,245,254,5,3,0,0,246,247,5,18,0,0,247,254,5,3,0,0,248,250,5,19,
  	0,0,249,251,3,32,16,0,250,249,1,0,0,0,250,251,1,0,0,0,251,252,1,0,0,0,
  	252,254,5,3,0,0,253,214,1,0,0,0,253,220,1,0,0,0,253,223,1,0,0,0,253,224,
  	1,0,0,0,253,230,1,0,0,0,253,238,1,0,0,0,253,244,1,0,0,0,253,246,1,0,0,
  	0,253,248,1,0,0,0,254,31,1,0,0,0,255,256,3,52,26,0,256,33,1,0,0,0,257,
  	258,3,60,30,0,258,35,1,0,0,0,259,266,5,36,0,0,260,261,5,6,0,0,261,262,
  	3,32,16,0,262,263,5,7,0,0,263,265,1,0,0,0,264,260,1,0,0,0,265,268,1,0,
  	0,0,266,264,1,0,0,0,266,267,1,0,0,0,267,37,1,0,0,0,268,266,1,0,0,0,269,
  	270,5,11,0,0,270,271,3,32,16,0,271,272,5,12,0,0,272,276,1,0,0,0,273,276,
  	3,36,18,0,274,276,3,40,20,0,275,269,1,0,0,0,275,273,1,0,0,0,275,274,1,
  	0,0,0,276,39,1,0,0,0,277,278,7,2,0,0,278,41,1,0,0,0,279,290,3,38,19,0,
  	280,281,5,36,0,0,281,283,5,11,0,0,282,284,3,46,23,0,283,282,1,0,0,0,283,
  	284,1,0,0,0,284,285,1,0,0,0,285,290,5,12,0,0,286,287,3,44,22,0,287,288,
  	3,42,21,0,288,290,1,0,0,0,289,279,1,0,0,0,289,280,1,0,0,0,289,286,1,0,
  	0,0,290,43,1,0,0,0,291,292,7,3,0,0,292,45,1,0,0,0,293,298,3,48,24,0,294,
  	295,5,2,0,0,295,297,3,48,24,0,296,294,1,0,0,0,297,300,1,0,0,0,298,296,
  	1,0,0,0,298,299,1,0,0,0,299,47,1,0,0,0,300,298,1,0,0,0,301,302,3,32,16,
  	0,302,49,1,0,0,0,303,304,6,25,-1,0,304,305,3,42,21,0,305,311,1,0,0,0,
  	306,307,10,1,0,0,307,308,7,4,0,0,308,310,3,42,21,0,309,306,1,0,0,0,310,
  	313,1,0,0,0,311,309,1,0,0,0,311,312,1,0,0,0,312,51,1,0,0,0,313,311,1,
  	0,0,0,314,315,6,26,-1,0,315,316,3,50,25,0,316,322,1,0,0,0,317,318,10,
  	1,0,0,318,319,7,5,0,0,319,321,3,50,25,0,320,317,1,0,0,0,321,324,1,0,0,
  	0,322,320,1,0,0,0,322,323,1,0,0,0,323,53,1,0,0,0,324,322,1,0,0,0,325,
  	326,6,27,-1,0,326,327,3,52,26,0,327,333,1,0,0,0,328,329,10,1,0,0,329,
  	330,7,6,0,0,330,332,3,52,26,0,331,328,1,0,0,0,332,335,1,0,0,0,333,331,
  	1,0,0,0,333,334,1,0,0,0,334,55,1,0,0,0,335,333,1,0,0,0,336,337,6,28,-1,
  	0,337,338,3,54,27,0,338,344,1,0,0,0,339,340,10,1,0,0,340,341,7,7,0,0,
  	341,343,3,54,27,0,342,339,1,0,0,0,343,346,1,0,0,0,344,342,1,0,0,0,344,
  	345,1,0,0,0,345,57,1,0,0,0,346,344,1,0,0,0,347,348,6,29,-1,0,348,349,
  	3,56,28,0,349,355,1,0,0,0,350,351,10,1,0,0,351,352,5,32,0,0,352,354,3,
  	56,28,0,353,350,1,0,0,0,354,357,1,0,0,0,355,353,1,0,0,0,355,356,1,0,0,
  	0,356,59,1,0,0,0,357,355,1,0,0,0,358,359,6,30,-1,0,359,360,3,58,29,0,
  	360,366,1,0,0,0,361,362,10,1,0,0,362,363,5,33,0,0,363,365,3,58,29,0,364,
  	361,1,0,0,0,365,368,1,0,0,0,366,364,1,0,0,0,366,367,1,0,0,0,367,61,1,
  	0,0,0,368,366,1,0,0,0,369,370,3,52,26,0,370,63,1,0,0,0,35,66,68,73,82,
  	96,109,112,115,123,135,145,150,159,162,165,171,183,196,199,205,212,220,
  	250,253,266,275,283,289,298,311,322,333,344,355,366
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  sysy2022ParserStaticData = staticData.release();
}

}

SysY2022Parser::SysY2022Parser(TokenStream *input) : SysY2022Parser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SysY2022Parser::SysY2022Parser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SysY2022Parser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *sysy2022ParserStaticData->atn, sysy2022ParserStaticData->decisionToDFA, sysy2022ParserStaticData->sharedContextCache, options);
}

SysY2022Parser::~SysY2022Parser() {
  delete _interpreter;
}

const atn::ATN& SysY2022Parser::getATN() const {
  return *sysy2022ParserStaticData->atn;
}

std::string SysY2022Parser::getGrammarFileName() const {
  return "SysY2022.g4";
}

const std::vector<std::string>& SysY2022Parser::getRuleNames() const {
  return sysy2022ParserStaticData->ruleNames;
}

const dfa::Vocabulary& SysY2022Parser::getVocabulary() const {
  return sysy2022ParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SysY2022Parser::getSerializedATN() const {
  return sysy2022ParserStaticData->serializedATN;
}


//----------------- CompUnitContext ------------------------------------------------------------------

SysY2022Parser::CompUnitContext::CompUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysY2022Parser::DeclContext *> SysY2022Parser::CompUnitContext::decl() {
  return getRuleContexts<SysY2022Parser::DeclContext>();
}

SysY2022Parser::DeclContext* SysY2022Parser::CompUnitContext::decl(size_t i) {
  return getRuleContext<SysY2022Parser::DeclContext>(i);
}

std::vector<SysY2022Parser::FuncDefContext *> SysY2022Parser::CompUnitContext::funcDef() {
  return getRuleContexts<SysY2022Parser::FuncDefContext>();
}

SysY2022Parser::FuncDefContext* SysY2022Parser::CompUnitContext::funcDef(size_t i) {
  return getRuleContext<SysY2022Parser::FuncDefContext>(i);
}


size_t SysY2022Parser::CompUnitContext::getRuleIndex() const {
  return SysY2022Parser::RuleCompUnit;
}


std::any SysY2022Parser::CompUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitCompUnit(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::CompUnitContext* SysY2022Parser::compUnit() {
  CompUnitContext *_localctx = _tracker.createInstance<CompUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, SysY2022Parser::RuleCompUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(68);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 8242) != 0)) {
      setState(66);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(64);
        decl();
        break;
      }

      case 2: {
        setState(65);
        funcDef();
        break;
      }

      default:
        break;
      }
      setState(70);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclContext ------------------------------------------------------------------

SysY2022Parser::DeclContext::DeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::ConstDeclContext* SysY2022Parser::DeclContext::constDecl() {
  return getRuleContext<SysY2022Parser::ConstDeclContext>(0);
}

SysY2022Parser::VarDeclContext* SysY2022Parser::DeclContext::varDecl() {
  return getRuleContext<SysY2022Parser::VarDeclContext>(0);
}


size_t SysY2022Parser::DeclContext::getRuleIndex() const {
  return SysY2022Parser::RuleDecl;
}


std::any SysY2022Parser::DeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitDecl(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::DeclContext* SysY2022Parser::decl() {
  DeclContext *_localctx = _tracker.createInstance<DeclContext>(_ctx, getState());
  enterRule(_localctx, 2, SysY2022Parser::RuleDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(73);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysY2022Parser::T__0: {
        enterOuterAlt(_localctx, 1);
        setState(71);
        constDecl();
        break;
      }

      case SysY2022Parser::T__3:
      case SysY2022Parser::T__4: {
        enterOuterAlt(_localctx, 2);
        setState(72);
        varDecl();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDeclContext ------------------------------------------------------------------

SysY2022Parser::ConstDeclContext::ConstDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::BTypeContext* SysY2022Parser::ConstDeclContext::bType() {
  return getRuleContext<SysY2022Parser::BTypeContext>(0);
}

std::vector<SysY2022Parser::ConstDefContext *> SysY2022Parser::ConstDeclContext::constDef() {
  return getRuleContexts<SysY2022Parser::ConstDefContext>();
}

SysY2022Parser::ConstDefContext* SysY2022Parser::ConstDeclContext::constDef(size_t i) {
  return getRuleContext<SysY2022Parser::ConstDefContext>(i);
}


size_t SysY2022Parser::ConstDeclContext::getRuleIndex() const {
  return SysY2022Parser::RuleConstDecl;
}


std::any SysY2022Parser::ConstDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitConstDecl(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::ConstDeclContext* SysY2022Parser::constDecl() {
  ConstDeclContext *_localctx = _tracker.createInstance<ConstDeclContext>(_ctx, getState());
  enterRule(_localctx, 4, SysY2022Parser::RuleConstDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(75);
    match(SysY2022Parser::T__0);
    setState(76);
    bType();
    setState(77);
    constDef();
    setState(82);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysY2022Parser::T__1) {
      setState(78);
      match(SysY2022Parser::T__1);
      setState(79);
      constDef();
      setState(84);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(85);
    match(SysY2022Parser::T__2);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BTypeContext ------------------------------------------------------------------

SysY2022Parser::BTypeContext::BTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::BTypeContext::getRuleIndex() const {
  return SysY2022Parser::RuleBType;
}


std::any SysY2022Parser::BTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitBType(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::BTypeContext* SysY2022Parser::bType() {
  BTypeContext *_localctx = _tracker.createInstance<BTypeContext>(_ctx, getState());
  enterRule(_localctx, 6, SysY2022Parser::RuleBType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(87);
    _la = _input->LA(1);
    if (!(_la == SysY2022Parser::T__3

    || _la == SysY2022Parser::T__4)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstDefContext ------------------------------------------------------------------

SysY2022Parser::ConstDefContext::ConstDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysY2022Parser::ConstDefContext::Identifier() {
  return getToken(SysY2022Parser::Identifier, 0);
}

SysY2022Parser::ConstInitValContext* SysY2022Parser::ConstDefContext::constInitVal() {
  return getRuleContext<SysY2022Parser::ConstInitValContext>(0);
}

std::vector<SysY2022Parser::ConstExpContext *> SysY2022Parser::ConstDefContext::constExp() {
  return getRuleContexts<SysY2022Parser::ConstExpContext>();
}

SysY2022Parser::ConstExpContext* SysY2022Parser::ConstDefContext::constExp(size_t i) {
  return getRuleContext<SysY2022Parser::ConstExpContext>(i);
}


size_t SysY2022Parser::ConstDefContext::getRuleIndex() const {
  return SysY2022Parser::RuleConstDef;
}


std::any SysY2022Parser::ConstDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitConstDef(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::ConstDefContext* SysY2022Parser::constDef() {
  ConstDefContext *_localctx = _tracker.createInstance<ConstDefContext>(_ctx, getState());
  enterRule(_localctx, 8, SysY2022Parser::RuleConstDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(89);
    match(SysY2022Parser::Identifier);
    setState(96);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysY2022Parser::T__5) {
      setState(90);
      match(SysY2022Parser::T__5);
      setState(91);
      constExp();
      setState(92);
      match(SysY2022Parser::T__6);
      setState(98);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(99);
    match(SysY2022Parser::T__7);
    setState(100);
    constInitVal();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstInitValContext ------------------------------------------------------------------

SysY2022Parser::ConstInitValContext::ConstInitValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::ConstInitValContext::getRuleIndex() const {
  return SysY2022Parser::RuleConstInitVal;
}

void SysY2022Parser::ConstInitValContext::copyFrom(ConstInitValContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ItemConstInitValContext ------------------------------------------------------------------

SysY2022Parser::ConstExpContext* SysY2022Parser::ItemConstInitValContext::constExp() {
  return getRuleContext<SysY2022Parser::ConstExpContext>(0);
}

SysY2022Parser::ItemConstInitValContext::ItemConstInitValContext(ConstInitValContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::ItemConstInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitItemConstInitVal(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ListConstInitValContext ------------------------------------------------------------------

std::vector<SysY2022Parser::ConstInitValContext *> SysY2022Parser::ListConstInitValContext::constInitVal() {
  return getRuleContexts<SysY2022Parser::ConstInitValContext>();
}

SysY2022Parser::ConstInitValContext* SysY2022Parser::ListConstInitValContext::constInitVal(size_t i) {
  return getRuleContext<SysY2022Parser::ConstInitValContext>(i);
}

SysY2022Parser::ListConstInitValContext::ListConstInitValContext(ConstInitValContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::ListConstInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitListConstInitVal(this);
  else
    return visitor->visitChildren(this);
}
SysY2022Parser::ConstInitValContext* SysY2022Parser::constInitVal() {
  ConstInitValContext *_localctx = _tracker.createInstance<ConstInitValContext>(_ctx, getState());
  enterRule(_localctx, 10, SysY2022Parser::RuleConstInitVal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(115);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysY2022Parser::T__10:
      case SysY2022Parser::T__19:
      case SysY2022Parser::T__20:
      case SysY2022Parser::T__21:
      case SysY2022Parser::FloatConst:
      case SysY2022Parser::IntConst:
      case SysY2022Parser::Identifier: {
        _localctx = _tracker.createInstance<SysY2022Parser::ItemConstInitValContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(102);
        constExp();
        break;
      }

      case SysY2022Parser::T__8: {
        _localctx = _tracker.createInstance<SysY2022Parser::ListConstInitValContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(103);
        match(SysY2022Parser::T__8);
        setState(112);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 120266426880) != 0)) {
          setState(104);
          constInitVal();
          setState(109);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysY2022Parser::T__1) {
            setState(105);
            match(SysY2022Parser::T__1);
            setState(106);
            constInitVal();
            setState(111);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(114);
        match(SysY2022Parser::T__9);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDeclContext ------------------------------------------------------------------

SysY2022Parser::VarDeclContext::VarDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::BTypeContext* SysY2022Parser::VarDeclContext::bType() {
  return getRuleContext<SysY2022Parser::BTypeContext>(0);
}

std::vector<SysY2022Parser::VarDefContext *> SysY2022Parser::VarDeclContext::varDef() {
  return getRuleContexts<SysY2022Parser::VarDefContext>();
}

SysY2022Parser::VarDefContext* SysY2022Parser::VarDeclContext::varDef(size_t i) {
  return getRuleContext<SysY2022Parser::VarDefContext>(i);
}


size_t SysY2022Parser::VarDeclContext::getRuleIndex() const {
  return SysY2022Parser::RuleVarDecl;
}


std::any SysY2022Parser::VarDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitVarDecl(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::VarDeclContext* SysY2022Parser::varDecl() {
  VarDeclContext *_localctx = _tracker.createInstance<VarDeclContext>(_ctx, getState());
  enterRule(_localctx, 12, SysY2022Parser::RuleVarDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(117);
    bType();
    setState(118);
    varDef();
    setState(123);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysY2022Parser::T__1) {
      setState(119);
      match(SysY2022Parser::T__1);
      setState(120);
      varDef();
      setState(125);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(126);
    match(SysY2022Parser::T__2);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDefContext ------------------------------------------------------------------

SysY2022Parser::VarDefContext::VarDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::VarDefContext::getRuleIndex() const {
  return SysY2022Parser::RuleVarDef;
}

void SysY2022Parser::VarDefContext::copyFrom(VarDefContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- VarDefWithInitValContext ------------------------------------------------------------------

tree::TerminalNode* SysY2022Parser::VarDefWithInitValContext::Identifier() {
  return getToken(SysY2022Parser::Identifier, 0);
}

SysY2022Parser::InitValContext* SysY2022Parser::VarDefWithInitValContext::initVal() {
  return getRuleContext<SysY2022Parser::InitValContext>(0);
}

std::vector<SysY2022Parser::ConstExpContext *> SysY2022Parser::VarDefWithInitValContext::constExp() {
  return getRuleContexts<SysY2022Parser::ConstExpContext>();
}

SysY2022Parser::ConstExpContext* SysY2022Parser::VarDefWithInitValContext::constExp(size_t i) {
  return getRuleContext<SysY2022Parser::ConstExpContext>(i);
}

SysY2022Parser::VarDefWithInitValContext::VarDefWithInitValContext(VarDefContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::VarDefWithInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitVarDefWithInitVal(this);
  else
    return visitor->visitChildren(this);
}
//----------------- VarDefWithOutInitValContext ------------------------------------------------------------------

tree::TerminalNode* SysY2022Parser::VarDefWithOutInitValContext::Identifier() {
  return getToken(SysY2022Parser::Identifier, 0);
}

std::vector<SysY2022Parser::ConstExpContext *> SysY2022Parser::VarDefWithOutInitValContext::constExp() {
  return getRuleContexts<SysY2022Parser::ConstExpContext>();
}

SysY2022Parser::ConstExpContext* SysY2022Parser::VarDefWithOutInitValContext::constExp(size_t i) {
  return getRuleContext<SysY2022Parser::ConstExpContext>(i);
}

SysY2022Parser::VarDefWithOutInitValContext::VarDefWithOutInitValContext(VarDefContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::VarDefWithOutInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitVarDefWithOutInitVal(this);
  else
    return visitor->visitChildren(this);
}
SysY2022Parser::VarDefContext* SysY2022Parser::varDef() {
  VarDefContext *_localctx = _tracker.createInstance<VarDefContext>(_ctx, getState());
  enterRule(_localctx, 14, SysY2022Parser::RuleVarDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(150);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<SysY2022Parser::VarDefWithOutInitValContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(128);
      match(SysY2022Parser::Identifier);
      setState(135);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysY2022Parser::T__5) {
        setState(129);
        match(SysY2022Parser::T__5);
        setState(130);
        constExp();
        setState(131);
        match(SysY2022Parser::T__6);
        setState(137);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<SysY2022Parser::VarDefWithInitValContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(138);
      match(SysY2022Parser::Identifier);
      setState(145);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysY2022Parser::T__5) {
        setState(139);
        match(SysY2022Parser::T__5);
        setState(140);
        constExp();
        setState(141);
        match(SysY2022Parser::T__6);
        setState(147);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(148);
      match(SysY2022Parser::T__7);
      setState(149);
      initVal();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitValContext ------------------------------------------------------------------

SysY2022Parser::InitValContext::InitValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::InitValContext::getRuleIndex() const {
  return SysY2022Parser::RuleInitVal;
}

void SysY2022Parser::InitValContext::copyFrom(InitValContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ItemInitValContext ------------------------------------------------------------------

SysY2022Parser::ExpContext* SysY2022Parser::ItemInitValContext::exp() {
  return getRuleContext<SysY2022Parser::ExpContext>(0);
}

SysY2022Parser::ItemInitValContext::ItemInitValContext(InitValContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::ItemInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitItemInitVal(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ListInitValContext ------------------------------------------------------------------

std::vector<SysY2022Parser::InitValContext *> SysY2022Parser::ListInitValContext::initVal() {
  return getRuleContexts<SysY2022Parser::InitValContext>();
}

SysY2022Parser::InitValContext* SysY2022Parser::ListInitValContext::initVal(size_t i) {
  return getRuleContext<SysY2022Parser::InitValContext>(i);
}

SysY2022Parser::ListInitValContext::ListInitValContext(InitValContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::ListInitValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitListInitVal(this);
  else
    return visitor->visitChildren(this);
}
SysY2022Parser::InitValContext* SysY2022Parser::initVal() {
  InitValContext *_localctx = _tracker.createInstance<InitValContext>(_ctx, getState());
  enterRule(_localctx, 16, SysY2022Parser::RuleInitVal);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(165);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysY2022Parser::T__10:
      case SysY2022Parser::T__19:
      case SysY2022Parser::T__20:
      case SysY2022Parser::T__21:
      case SysY2022Parser::FloatConst:
      case SysY2022Parser::IntConst:
      case SysY2022Parser::Identifier: {
        _localctx = _tracker.createInstance<SysY2022Parser::ItemInitValContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(152);
        exp();
        break;
      }

      case SysY2022Parser::T__8: {
        _localctx = _tracker.createInstance<SysY2022Parser::ListInitValContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(153);
        match(SysY2022Parser::T__8);
        setState(162);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 120266426880) != 0)) {
          setState(154);
          initVal();
          setState(159);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SysY2022Parser::T__1) {
            setState(155);
            match(SysY2022Parser::T__1);
            setState(156);
            initVal();
            setState(161);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(164);
        match(SysY2022Parser::T__9);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncDefContext ------------------------------------------------------------------

SysY2022Parser::FuncDefContext::FuncDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::FuncTypeContext* SysY2022Parser::FuncDefContext::funcType() {
  return getRuleContext<SysY2022Parser::FuncTypeContext>(0);
}

tree::TerminalNode* SysY2022Parser::FuncDefContext::Identifier() {
  return getToken(SysY2022Parser::Identifier, 0);
}

SysY2022Parser::BlockContext* SysY2022Parser::FuncDefContext::block() {
  return getRuleContext<SysY2022Parser::BlockContext>(0);
}

SysY2022Parser::FuncFParamsContext* SysY2022Parser::FuncDefContext::funcFParams() {
  return getRuleContext<SysY2022Parser::FuncFParamsContext>(0);
}


size_t SysY2022Parser::FuncDefContext::getRuleIndex() const {
  return SysY2022Parser::RuleFuncDef;
}


std::any SysY2022Parser::FuncDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitFuncDef(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::FuncDefContext* SysY2022Parser::funcDef() {
  FuncDefContext *_localctx = _tracker.createInstance<FuncDefContext>(_ctx, getState());
  enterRule(_localctx, 18, SysY2022Parser::RuleFuncDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(167);
    funcType();
    setState(168);
    match(SysY2022Parser::Identifier);
    setState(169);
    match(SysY2022Parser::T__10);
    setState(171);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysY2022Parser::T__3

    || _la == SysY2022Parser::T__4) {
      setState(170);
      funcFParams();
    }
    setState(173);
    match(SysY2022Parser::T__11);
    setState(174);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncTypeContext ------------------------------------------------------------------

SysY2022Parser::FuncTypeContext::FuncTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::FuncTypeContext::getRuleIndex() const {
  return SysY2022Parser::RuleFuncType;
}


std::any SysY2022Parser::FuncTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitFuncType(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::FuncTypeContext* SysY2022Parser::funcType() {
  FuncTypeContext *_localctx = _tracker.createInstance<FuncTypeContext>(_ctx, getState());
  enterRule(_localctx, 20, SysY2022Parser::RuleFuncType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(176);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 8240) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFParamsContext ------------------------------------------------------------------

SysY2022Parser::FuncFParamsContext::FuncFParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysY2022Parser::FuncFParamContext *> SysY2022Parser::FuncFParamsContext::funcFParam() {
  return getRuleContexts<SysY2022Parser::FuncFParamContext>();
}

SysY2022Parser::FuncFParamContext* SysY2022Parser::FuncFParamsContext::funcFParam(size_t i) {
  return getRuleContext<SysY2022Parser::FuncFParamContext>(i);
}


size_t SysY2022Parser::FuncFParamsContext::getRuleIndex() const {
  return SysY2022Parser::RuleFuncFParams;
}


std::any SysY2022Parser::FuncFParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitFuncFParams(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::FuncFParamsContext* SysY2022Parser::funcFParams() {
  FuncFParamsContext *_localctx = _tracker.createInstance<FuncFParamsContext>(_ctx, getState());
  enterRule(_localctx, 22, SysY2022Parser::RuleFuncFParams);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(178);
    funcFParam();
    setState(183);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysY2022Parser::T__1) {
      setState(179);
      match(SysY2022Parser::T__1);
      setState(180);
      funcFParam();
      setState(185);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFParamContext ------------------------------------------------------------------

SysY2022Parser::FuncFParamContext::FuncFParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::BTypeContext* SysY2022Parser::FuncFParamContext::bType() {
  return getRuleContext<SysY2022Parser::BTypeContext>(0);
}

tree::TerminalNode* SysY2022Parser::FuncFParamContext::Identifier() {
  return getToken(SysY2022Parser::Identifier, 0);
}

std::vector<SysY2022Parser::ConstExpContext *> SysY2022Parser::FuncFParamContext::constExp() {
  return getRuleContexts<SysY2022Parser::ConstExpContext>();
}

SysY2022Parser::ConstExpContext* SysY2022Parser::FuncFParamContext::constExp(size_t i) {
  return getRuleContext<SysY2022Parser::ConstExpContext>(i);
}


size_t SysY2022Parser::FuncFParamContext::getRuleIndex() const {
  return SysY2022Parser::RuleFuncFParam;
}


std::any SysY2022Parser::FuncFParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitFuncFParam(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::FuncFParamContext* SysY2022Parser::funcFParam() {
  FuncFParamContext *_localctx = _tracker.createInstance<FuncFParamContext>(_ctx, getState());
  enterRule(_localctx, 24, SysY2022Parser::RuleFuncFParam);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(186);
    bType();
    setState(187);
    match(SysY2022Parser::Identifier);
    setState(199);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysY2022Parser::T__5) {
      setState(188);
      match(SysY2022Parser::T__5);
      setState(189);
      match(SysY2022Parser::T__6);
      setState(196);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SysY2022Parser::T__5) {
        setState(190);
        match(SysY2022Parser::T__5);
        setState(191);
        constExp();
        setState(192);
        match(SysY2022Parser::T__6);
        setState(198);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

SysY2022Parser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysY2022Parser::BlockItemContext *> SysY2022Parser::BlockContext::blockItem() {
  return getRuleContexts<SysY2022Parser::BlockItemContext>();
}

SysY2022Parser::BlockItemContext* SysY2022Parser::BlockContext::blockItem(size_t i) {
  return getRuleContext<SysY2022Parser::BlockItemContext>(i);
}


size_t SysY2022Parser::BlockContext::getRuleIndex() const {
  return SysY2022Parser::RuleBlock;
}


std::any SysY2022Parser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::BlockContext* SysY2022Parser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 26, SysY2022Parser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(201);
    match(SysY2022Parser::T__8);
    setState(205);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 120267426362) != 0)) {
      setState(202);
      blockItem();
      setState(207);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(208);
    match(SysY2022Parser::T__9);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockItemContext ------------------------------------------------------------------

SysY2022Parser::BlockItemContext::BlockItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::DeclContext* SysY2022Parser::BlockItemContext::decl() {
  return getRuleContext<SysY2022Parser::DeclContext>(0);
}

SysY2022Parser::StmtContext* SysY2022Parser::BlockItemContext::stmt() {
  return getRuleContext<SysY2022Parser::StmtContext>(0);
}


size_t SysY2022Parser::BlockItemContext::getRuleIndex() const {
  return SysY2022Parser::RuleBlockItem;
}


std::any SysY2022Parser::BlockItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitBlockItem(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::BlockItemContext* SysY2022Parser::blockItem() {
  BlockItemContext *_localctx = _tracker.createInstance<BlockItemContext>(_ctx, getState());
  enterRule(_localctx, 28, SysY2022Parser::RuleBlockItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(212);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysY2022Parser::T__0:
      case SysY2022Parser::T__3:
      case SysY2022Parser::T__4: {
        enterOuterAlt(_localctx, 1);
        setState(210);
        decl();
        break;
      }

      case SysY2022Parser::T__2:
      case SysY2022Parser::T__8:
      case SysY2022Parser::T__10:
      case SysY2022Parser::T__13:
      case SysY2022Parser::T__15:
      case SysY2022Parser::T__16:
      case SysY2022Parser::T__17:
      case SysY2022Parser::T__18:
      case SysY2022Parser::T__19:
      case SysY2022Parser::T__20:
      case SysY2022Parser::T__21:
      case SysY2022Parser::FloatConst:
      case SysY2022Parser::IntConst:
      case SysY2022Parser::Identifier: {
        enterOuterAlt(_localctx, 2);
        setState(211);
        stmt();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StmtContext ------------------------------------------------------------------

SysY2022Parser::StmtContext::StmtContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::StmtContext::getRuleIndex() const {
  return SysY2022Parser::RuleStmt;
}

void SysY2022Parser::StmtContext::copyFrom(StmtContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- WhileStmtContext ------------------------------------------------------------------

SysY2022Parser::CondContext* SysY2022Parser::WhileStmtContext::cond() {
  return getRuleContext<SysY2022Parser::CondContext>(0);
}

SysY2022Parser::StmtContext* SysY2022Parser::WhileStmtContext::stmt() {
  return getRuleContext<SysY2022Parser::StmtContext>(0);
}

SysY2022Parser::WhileStmtContext::WhileStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::WhileStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitWhileStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IfStmtContext ------------------------------------------------------------------

SysY2022Parser::CondContext* SysY2022Parser::IfStmtContext::cond() {
  return getRuleContext<SysY2022Parser::CondContext>(0);
}

SysY2022Parser::StmtContext* SysY2022Parser::IfStmtContext::stmt() {
  return getRuleContext<SysY2022Parser::StmtContext>(0);
}

SysY2022Parser::IfStmtContext::IfStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::IfStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitIfStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BlockStmtContext ------------------------------------------------------------------

SysY2022Parser::BlockContext* SysY2022Parser::BlockStmtContext::block() {
  return getRuleContext<SysY2022Parser::BlockContext>(0);
}

SysY2022Parser::BlockStmtContext::BlockStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::BlockStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitBlockStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IfElseStmtContext ------------------------------------------------------------------

SysY2022Parser::CondContext* SysY2022Parser::IfElseStmtContext::cond() {
  return getRuleContext<SysY2022Parser::CondContext>(0);
}

std::vector<SysY2022Parser::StmtContext *> SysY2022Parser::IfElseStmtContext::stmt() {
  return getRuleContexts<SysY2022Parser::StmtContext>();
}

SysY2022Parser::StmtContext* SysY2022Parser::IfElseStmtContext::stmt(size_t i) {
  return getRuleContext<SysY2022Parser::StmtContext>(i);
}

SysY2022Parser::IfElseStmtContext::IfElseStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::IfElseStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitIfElseStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AssignStmtContext ------------------------------------------------------------------

SysY2022Parser::LValContext* SysY2022Parser::AssignStmtContext::lVal() {
  return getRuleContext<SysY2022Parser::LValContext>(0);
}

SysY2022Parser::ExpContext* SysY2022Parser::AssignStmtContext::exp() {
  return getRuleContext<SysY2022Parser::ExpContext>(0);
}

SysY2022Parser::AssignStmtContext::AssignStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::AssignStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitAssignStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BreakStmtContext ------------------------------------------------------------------

SysY2022Parser::BreakStmtContext::BreakStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::BreakStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitBreakStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExpStmtContext ------------------------------------------------------------------

SysY2022Parser::ExpContext* SysY2022Parser::ExpStmtContext::exp() {
  return getRuleContext<SysY2022Parser::ExpContext>(0);
}

SysY2022Parser::ExpStmtContext::ExpStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::ExpStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitExpStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ReturnStmtContext ------------------------------------------------------------------

SysY2022Parser::ExpContext* SysY2022Parser::ReturnStmtContext::exp() {
  return getRuleContext<SysY2022Parser::ExpContext>(0);
}

SysY2022Parser::ReturnStmtContext::ReturnStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::ReturnStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitReturnStmt(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ContinueStmtContext ------------------------------------------------------------------

SysY2022Parser::ContinueStmtContext::ContinueStmtContext(StmtContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::ContinueStmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitContinueStmt(this);
  else
    return visitor->visitChildren(this);
}
SysY2022Parser::StmtContext* SysY2022Parser::stmt() {
  StmtContext *_localctx = _tracker.createInstance<StmtContext>(_ctx, getState());
  enterRule(_localctx, 30, SysY2022Parser::RuleStmt);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(253);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<SysY2022Parser::AssignStmtContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(214);
      lVal();
      setState(215);
      match(SysY2022Parser::T__7);
      setState(216);
      exp();
      setState(217);
      match(SysY2022Parser::T__2);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<SysY2022Parser::ExpStmtContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(220);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120266426368) != 0)) {
        setState(219);
        exp();
      }
      setState(222);
      match(SysY2022Parser::T__2);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SysY2022Parser::BlockStmtContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(223);
      block();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<SysY2022Parser::IfStmtContext>(_localctx);
      enterOuterAlt(_localctx, 4);
      setState(224);
      match(SysY2022Parser::T__13);
      setState(225);
      match(SysY2022Parser::T__10);
      setState(226);
      cond();
      setState(227);
      match(SysY2022Parser::T__11);
      setState(228);
      stmt();
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<SysY2022Parser::IfElseStmtContext>(_localctx);
      enterOuterAlt(_localctx, 5);
      setState(230);
      match(SysY2022Parser::T__13);
      setState(231);
      match(SysY2022Parser::T__10);
      setState(232);
      cond();
      setState(233);
      match(SysY2022Parser::T__11);
      setState(234);
      stmt();
      setState(235);
      match(SysY2022Parser::T__14);
      setState(236);
      stmt();
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<SysY2022Parser::WhileStmtContext>(_localctx);
      enterOuterAlt(_localctx, 6);
      setState(238);
      match(SysY2022Parser::T__15);
      setState(239);
      match(SysY2022Parser::T__10);
      setState(240);
      cond();
      setState(241);
      match(SysY2022Parser::T__11);
      setState(242);
      stmt();
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<SysY2022Parser::BreakStmtContext>(_localctx);
      enterOuterAlt(_localctx, 7);
      setState(244);
      match(SysY2022Parser::T__16);
      setState(245);
      match(SysY2022Parser::T__2);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<SysY2022Parser::ContinueStmtContext>(_localctx);
      enterOuterAlt(_localctx, 8);
      setState(246);
      match(SysY2022Parser::T__17);
      setState(247);
      match(SysY2022Parser::T__2);
      break;
    }

    case 9: {
      _localctx = _tracker.createInstance<SysY2022Parser::ReturnStmtContext>(_localctx);
      enterOuterAlt(_localctx, 9);
      setState(248);
      match(SysY2022Parser::T__18);
      setState(250);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120266426368) != 0)) {
        setState(249);
        exp();
      }
      setState(252);
      match(SysY2022Parser::T__2);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpContext ------------------------------------------------------------------

SysY2022Parser::ExpContext::ExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::AddExpContext* SysY2022Parser::ExpContext::addExp() {
  return getRuleContext<SysY2022Parser::AddExpContext>(0);
}


size_t SysY2022Parser::ExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleExp;
}


std::any SysY2022Parser::ExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitExp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::ExpContext* SysY2022Parser::exp() {
  ExpContext *_localctx = _tracker.createInstance<ExpContext>(_ctx, getState());
  enterRule(_localctx, 32, SysY2022Parser::RuleExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(255);
    addExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CondContext ------------------------------------------------------------------

SysY2022Parser::CondContext::CondContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::LOrExpContext* SysY2022Parser::CondContext::lOrExp() {
  return getRuleContext<SysY2022Parser::LOrExpContext>(0);
}


size_t SysY2022Parser::CondContext::getRuleIndex() const {
  return SysY2022Parser::RuleCond;
}


std::any SysY2022Parser::CondContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitCond(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::CondContext* SysY2022Parser::cond() {
  CondContext *_localctx = _tracker.createInstance<CondContext>(_ctx, getState());
  enterRule(_localctx, 34, SysY2022Parser::RuleCond);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(257);
    lOrExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LValContext ------------------------------------------------------------------

SysY2022Parser::LValContext::LValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysY2022Parser::LValContext::Identifier() {
  return getToken(SysY2022Parser::Identifier, 0);
}

std::vector<SysY2022Parser::ExpContext *> SysY2022Parser::LValContext::exp() {
  return getRuleContexts<SysY2022Parser::ExpContext>();
}

SysY2022Parser::ExpContext* SysY2022Parser::LValContext::exp(size_t i) {
  return getRuleContext<SysY2022Parser::ExpContext>(i);
}


size_t SysY2022Parser::LValContext::getRuleIndex() const {
  return SysY2022Parser::RuleLVal;
}


std::any SysY2022Parser::LValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitLVal(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::LValContext* SysY2022Parser::lVal() {
  LValContext *_localctx = _tracker.createInstance<LValContext>(_ctx, getState());
  enterRule(_localctx, 36, SysY2022Parser::RuleLVal);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(259);
    match(SysY2022Parser::Identifier);
    setState(266);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(260);
        match(SysY2022Parser::T__5);
        setState(261);
        exp();
        setState(262);
        match(SysY2022Parser::T__6); 
      }
      setState(268);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExpContext ------------------------------------------------------------------

SysY2022Parser::PrimaryExpContext::PrimaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::PrimaryExpContext::getRuleIndex() const {
  return SysY2022Parser::RulePrimaryExp;
}

void SysY2022Parser::PrimaryExpContext::copyFrom(PrimaryExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ExpPrimaryExpContext ------------------------------------------------------------------

SysY2022Parser::ExpContext* SysY2022Parser::ExpPrimaryExpContext::exp() {
  return getRuleContext<SysY2022Parser::ExpContext>(0);
}

SysY2022Parser::ExpPrimaryExpContext::ExpPrimaryExpContext(PrimaryExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::ExpPrimaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitExpPrimaryExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LValPrimaryExpContext ------------------------------------------------------------------

SysY2022Parser::LValContext* SysY2022Parser::LValPrimaryExpContext::lVal() {
  return getRuleContext<SysY2022Parser::LValContext>(0);
}

SysY2022Parser::LValPrimaryExpContext::LValPrimaryExpContext(PrimaryExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::LValPrimaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitLValPrimaryExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NumberPrimaryExpContext ------------------------------------------------------------------

SysY2022Parser::NumberContext* SysY2022Parser::NumberPrimaryExpContext::number() {
  return getRuleContext<SysY2022Parser::NumberContext>(0);
}

SysY2022Parser::NumberPrimaryExpContext::NumberPrimaryExpContext(PrimaryExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::NumberPrimaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitNumberPrimaryExp(this);
  else
    return visitor->visitChildren(this);
}
SysY2022Parser::PrimaryExpContext* SysY2022Parser::primaryExp() {
  PrimaryExpContext *_localctx = _tracker.createInstance<PrimaryExpContext>(_ctx, getState());
  enterRule(_localctx, 38, SysY2022Parser::RulePrimaryExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(275);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysY2022Parser::T__10: {
        _localctx = _tracker.createInstance<SysY2022Parser::ExpPrimaryExpContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(269);
        match(SysY2022Parser::T__10);
        setState(270);
        exp();
        setState(271);
        match(SysY2022Parser::T__11);
        break;
      }

      case SysY2022Parser::Identifier: {
        _localctx = _tracker.createInstance<SysY2022Parser::LValPrimaryExpContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(273);
        lVal();
        break;
      }

      case SysY2022Parser::FloatConst:
      case SysY2022Parser::IntConst: {
        _localctx = _tracker.createInstance<SysY2022Parser::NumberPrimaryExpContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(274);
        number();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NumberContext ------------------------------------------------------------------

SysY2022Parser::NumberContext::NumberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysY2022Parser::NumberContext::FloatConst() {
  return getToken(SysY2022Parser::FloatConst, 0);
}

tree::TerminalNode* SysY2022Parser::NumberContext::IntConst() {
  return getToken(SysY2022Parser::IntConst, 0);
}


size_t SysY2022Parser::NumberContext::getRuleIndex() const {
  return SysY2022Parser::RuleNumber;
}


std::any SysY2022Parser::NumberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitNumber(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::NumberContext* SysY2022Parser::number() {
  NumberContext *_localctx = _tracker.createInstance<NumberContext>(_ctx, getState());
  enterRule(_localctx, 40, SysY2022Parser::RuleNumber);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(277);
    _la = _input->LA(1);
    if (!(_la == SysY2022Parser::FloatConst

    || _la == SysY2022Parser::IntConst)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryExpContext ------------------------------------------------------------------

SysY2022Parser::UnaryExpContext::UnaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::UnaryExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleUnaryExp;
}

void SysY2022Parser::UnaryExpContext::copyFrom(UnaryExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- FunctionCallUnaryExpContext ------------------------------------------------------------------

tree::TerminalNode* SysY2022Parser::FunctionCallUnaryExpContext::Identifier() {
  return getToken(SysY2022Parser::Identifier, 0);
}

SysY2022Parser::FuncRParamsContext* SysY2022Parser::FunctionCallUnaryExpContext::funcRParams() {
  return getRuleContext<SysY2022Parser::FuncRParamsContext>(0);
}

SysY2022Parser::FunctionCallUnaryExpContext::FunctionCallUnaryExpContext(UnaryExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::FunctionCallUnaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitFunctionCallUnaryExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnaryUnaryExpContext ------------------------------------------------------------------

SysY2022Parser::UnaryOpContext* SysY2022Parser::UnaryUnaryExpContext::unaryOp() {
  return getRuleContext<SysY2022Parser::UnaryOpContext>(0);
}

SysY2022Parser::UnaryExpContext* SysY2022Parser::UnaryUnaryExpContext::unaryExp() {
  return getRuleContext<SysY2022Parser::UnaryExpContext>(0);
}

SysY2022Parser::UnaryUnaryExpContext::UnaryUnaryExpContext(UnaryExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::UnaryUnaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitUnaryUnaryExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- PrimaryUnaryExpContext ------------------------------------------------------------------

SysY2022Parser::PrimaryExpContext* SysY2022Parser::PrimaryUnaryExpContext::primaryExp() {
  return getRuleContext<SysY2022Parser::PrimaryExpContext>(0);
}

SysY2022Parser::PrimaryUnaryExpContext::PrimaryUnaryExpContext(UnaryExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::PrimaryUnaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitPrimaryUnaryExp(this);
  else
    return visitor->visitChildren(this);
}
SysY2022Parser::UnaryExpContext* SysY2022Parser::unaryExp() {
  UnaryExpContext *_localctx = _tracker.createInstance<UnaryExpContext>(_ctx, getState());
  enterRule(_localctx, 42, SysY2022Parser::RuleUnaryExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(289);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<SysY2022Parser::PrimaryUnaryExpContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(279);
      primaryExp();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<SysY2022Parser::FunctionCallUnaryExpContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(280);
      match(SysY2022Parser::Identifier);
      setState(281);
      match(SysY2022Parser::T__10);
      setState(283);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 120266426368) != 0)) {
        setState(282);
        funcRParams();
      }
      setState(285);
      match(SysY2022Parser::T__11);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SysY2022Parser::UnaryUnaryExpContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(286);
      unaryOp();
      setState(287);
      unaryExp();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryOpContext ------------------------------------------------------------------

SysY2022Parser::UnaryOpContext::UnaryOpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::UnaryOpContext::getRuleIndex() const {
  return SysY2022Parser::RuleUnaryOp;
}


std::any SysY2022Parser::UnaryOpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitUnaryOp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::UnaryOpContext* SysY2022Parser::unaryOp() {
  UnaryOpContext *_localctx = _tracker.createInstance<UnaryOpContext>(_ctx, getState());
  enterRule(_localctx, 44, SysY2022Parser::RuleUnaryOp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(291);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 7340032) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncRParamsContext ------------------------------------------------------------------

SysY2022Parser::FuncRParamsContext::FuncRParamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysY2022Parser::FuncRParamContext *> SysY2022Parser::FuncRParamsContext::funcRParam() {
  return getRuleContexts<SysY2022Parser::FuncRParamContext>();
}

SysY2022Parser::FuncRParamContext* SysY2022Parser::FuncRParamsContext::funcRParam(size_t i) {
  return getRuleContext<SysY2022Parser::FuncRParamContext>(i);
}


size_t SysY2022Parser::FuncRParamsContext::getRuleIndex() const {
  return SysY2022Parser::RuleFuncRParams;
}


std::any SysY2022Parser::FuncRParamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitFuncRParams(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::FuncRParamsContext* SysY2022Parser::funcRParams() {
  FuncRParamsContext *_localctx = _tracker.createInstance<FuncRParamsContext>(_ctx, getState());
  enterRule(_localctx, 46, SysY2022Parser::RuleFuncRParams);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(293);
    funcRParam();
    setState(298);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysY2022Parser::T__1) {
      setState(294);
      match(SysY2022Parser::T__1);
      setState(295);
      funcRParam();
      setState(300);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncRParamContext ------------------------------------------------------------------

SysY2022Parser::FuncRParamContext::FuncRParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::ExpContext* SysY2022Parser::FuncRParamContext::exp() {
  return getRuleContext<SysY2022Parser::ExpContext>(0);
}


size_t SysY2022Parser::FuncRParamContext::getRuleIndex() const {
  return SysY2022Parser::RuleFuncRParam;
}


std::any SysY2022Parser::FuncRParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitFuncRParam(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::FuncRParamContext* SysY2022Parser::funcRParam() {
  FuncRParamContext *_localctx = _tracker.createInstance<FuncRParamContext>(_ctx, getState());
  enterRule(_localctx, 48, SysY2022Parser::RuleFuncRParam);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(301);
    exp();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MulExpContext ------------------------------------------------------------------

SysY2022Parser::MulExpContext::MulExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::MulExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleMulExp;
}

void SysY2022Parser::MulExpContext::copyFrom(MulExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- MulMulExpContext ------------------------------------------------------------------

SysY2022Parser::MulExpContext* SysY2022Parser::MulMulExpContext::mulExp() {
  return getRuleContext<SysY2022Parser::MulExpContext>(0);
}

SysY2022Parser::UnaryExpContext* SysY2022Parser::MulMulExpContext::unaryExp() {
  return getRuleContext<SysY2022Parser::UnaryExpContext>(0);
}

SysY2022Parser::MulMulExpContext::MulMulExpContext(MulExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::MulMulExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitMulMulExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnaryMulExpContext ------------------------------------------------------------------

SysY2022Parser::UnaryExpContext* SysY2022Parser::UnaryMulExpContext::unaryExp() {
  return getRuleContext<SysY2022Parser::UnaryExpContext>(0);
}

SysY2022Parser::UnaryMulExpContext::UnaryMulExpContext(MulExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::UnaryMulExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitUnaryMulExp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::MulExpContext* SysY2022Parser::mulExp() {
   return mulExp(0);
}

SysY2022Parser::MulExpContext* SysY2022Parser::mulExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysY2022Parser::MulExpContext *_localctx = _tracker.createInstance<MulExpContext>(_ctx, parentState);
  SysY2022Parser::MulExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 50;
  enterRecursionRule(_localctx, 50, SysY2022Parser::RuleMulExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<UnaryMulExpContext>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(304);
    unaryExp();
    _ctx->stop = _input->LT(-1);
    setState(311);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<MulMulExpContext>(_tracker.createInstance<MulExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleMulExp);
        setState(306);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(307);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 58720256) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(308);
        unaryExp(); 
      }
      setState(313);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- AddExpContext ------------------------------------------------------------------

SysY2022Parser::AddExpContext::AddExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::AddExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleAddExp;
}

void SysY2022Parser::AddExpContext::copyFrom(AddExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- AddAddExpContext ------------------------------------------------------------------

SysY2022Parser::AddExpContext* SysY2022Parser::AddAddExpContext::addExp() {
  return getRuleContext<SysY2022Parser::AddExpContext>(0);
}

SysY2022Parser::MulExpContext* SysY2022Parser::AddAddExpContext::mulExp() {
  return getRuleContext<SysY2022Parser::MulExpContext>(0);
}

SysY2022Parser::AddAddExpContext::AddAddExpContext(AddExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::AddAddExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitAddAddExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulAddExpContext ------------------------------------------------------------------

SysY2022Parser::MulExpContext* SysY2022Parser::MulAddExpContext::mulExp() {
  return getRuleContext<SysY2022Parser::MulExpContext>(0);
}

SysY2022Parser::MulAddExpContext::MulAddExpContext(AddExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::MulAddExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitMulAddExp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::AddExpContext* SysY2022Parser::addExp() {
   return addExp(0);
}

SysY2022Parser::AddExpContext* SysY2022Parser::addExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysY2022Parser::AddExpContext *_localctx = _tracker.createInstance<AddExpContext>(_ctx, parentState);
  SysY2022Parser::AddExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 52;
  enterRecursionRule(_localctx, 52, SysY2022Parser::RuleAddExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<MulAddExpContext>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(315);
    mulExp(0);
    _ctx->stop = _input->LT(-1);
    setState(322);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<AddAddExpContext>(_tracker.createInstance<AddExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleAddExp);
        setState(317);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(318);
        _la = _input->LA(1);
        if (!(_la == SysY2022Parser::T__19

        || _la == SysY2022Parser::T__20)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(319);
        mulExp(0); 
      }
      setState(324);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- RelExpContext ------------------------------------------------------------------

SysY2022Parser::RelExpContext::RelExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::RelExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleRelExp;
}

void SysY2022Parser::RelExpContext::copyFrom(RelExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- RelRelExpContext ------------------------------------------------------------------

SysY2022Parser::RelExpContext* SysY2022Parser::RelRelExpContext::relExp() {
  return getRuleContext<SysY2022Parser::RelExpContext>(0);
}

SysY2022Parser::AddExpContext* SysY2022Parser::RelRelExpContext::addExp() {
  return getRuleContext<SysY2022Parser::AddExpContext>(0);
}

SysY2022Parser::RelRelExpContext::RelRelExpContext(RelExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::RelRelExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitRelRelExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AddRelExpContext ------------------------------------------------------------------

SysY2022Parser::AddExpContext* SysY2022Parser::AddRelExpContext::addExp() {
  return getRuleContext<SysY2022Parser::AddExpContext>(0);
}

SysY2022Parser::AddRelExpContext::AddRelExpContext(RelExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::AddRelExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitAddRelExp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::RelExpContext* SysY2022Parser::relExp() {
   return relExp(0);
}

SysY2022Parser::RelExpContext* SysY2022Parser::relExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysY2022Parser::RelExpContext *_localctx = _tracker.createInstance<RelExpContext>(_ctx, parentState);
  SysY2022Parser::RelExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 54;
  enterRecursionRule(_localctx, 54, SysY2022Parser::RuleRelExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<AddRelExpContext>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(326);
    addExp(0);
    _ctx->stop = _input->LT(-1);
    setState(333);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<RelRelExpContext>(_tracker.createInstance<RelExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleRelExp);
        setState(328);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(329);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 1006632960) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(330);
        addExp(0); 
      }
      setState(335);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- EqExpContext ------------------------------------------------------------------

SysY2022Parser::EqExpContext::EqExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::EqExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleEqExp;
}

void SysY2022Parser::EqExpContext::copyFrom(EqExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- EqEqExpContext ------------------------------------------------------------------

SysY2022Parser::EqExpContext* SysY2022Parser::EqEqExpContext::eqExp() {
  return getRuleContext<SysY2022Parser::EqExpContext>(0);
}

SysY2022Parser::RelExpContext* SysY2022Parser::EqEqExpContext::relExp() {
  return getRuleContext<SysY2022Parser::RelExpContext>(0);
}

SysY2022Parser::EqEqExpContext::EqEqExpContext(EqExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::EqEqExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitEqEqExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- RelEqExpContext ------------------------------------------------------------------

SysY2022Parser::RelExpContext* SysY2022Parser::RelEqExpContext::relExp() {
  return getRuleContext<SysY2022Parser::RelExpContext>(0);
}

SysY2022Parser::RelEqExpContext::RelEqExpContext(EqExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::RelEqExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitRelEqExp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::EqExpContext* SysY2022Parser::eqExp() {
   return eqExp(0);
}

SysY2022Parser::EqExpContext* SysY2022Parser::eqExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysY2022Parser::EqExpContext *_localctx = _tracker.createInstance<EqExpContext>(_ctx, parentState);
  SysY2022Parser::EqExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 56;
  enterRecursionRule(_localctx, 56, SysY2022Parser::RuleEqExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<RelEqExpContext>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(337);
    relExp(0);
    _ctx->stop = _input->LT(-1);
    setState(344);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<EqEqExpContext>(_tracker.createInstance<EqExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleEqExp);
        setState(339);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(340);
        _la = _input->LA(1);
        if (!(_la == SysY2022Parser::T__29

        || _la == SysY2022Parser::T__30)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(341);
        relExp(0); 
      }
      setState(346);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LAndExpContext ------------------------------------------------------------------

SysY2022Parser::LAndExpContext::LAndExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::LAndExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleLAndExp;
}

void SysY2022Parser::LAndExpContext::copyFrom(LAndExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- EqLAndExpContext ------------------------------------------------------------------

SysY2022Parser::EqExpContext* SysY2022Parser::EqLAndExpContext::eqExp() {
  return getRuleContext<SysY2022Parser::EqExpContext>(0);
}

SysY2022Parser::EqLAndExpContext::EqLAndExpContext(LAndExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::EqLAndExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitEqLAndExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LAndLAndExpContext ------------------------------------------------------------------

SysY2022Parser::LAndExpContext* SysY2022Parser::LAndLAndExpContext::lAndExp() {
  return getRuleContext<SysY2022Parser::LAndExpContext>(0);
}

SysY2022Parser::EqExpContext* SysY2022Parser::LAndLAndExpContext::eqExp() {
  return getRuleContext<SysY2022Parser::EqExpContext>(0);
}

SysY2022Parser::LAndLAndExpContext::LAndLAndExpContext(LAndExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::LAndLAndExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitLAndLAndExp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::LAndExpContext* SysY2022Parser::lAndExp() {
   return lAndExp(0);
}

SysY2022Parser::LAndExpContext* SysY2022Parser::lAndExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysY2022Parser::LAndExpContext *_localctx = _tracker.createInstance<LAndExpContext>(_ctx, parentState);
  SysY2022Parser::LAndExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 58;
  enterRecursionRule(_localctx, 58, SysY2022Parser::RuleLAndExp, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<EqLAndExpContext>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(348);
    eqExp(0);
    _ctx->stop = _input->LT(-1);
    setState(355);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<LAndLAndExpContext>(_tracker.createInstance<LAndExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleLAndExp);
        setState(350);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(351);
        match(SysY2022Parser::T__31);
        setState(352);
        eqExp(0); 
      }
      setState(357);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LOrExpContext ------------------------------------------------------------------

SysY2022Parser::LOrExpContext::LOrExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SysY2022Parser::LOrExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleLOrExp;
}

void SysY2022Parser::LOrExpContext::copyFrom(LOrExpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- LAndLOrExpContext ------------------------------------------------------------------

SysY2022Parser::LAndExpContext* SysY2022Parser::LAndLOrExpContext::lAndExp() {
  return getRuleContext<SysY2022Parser::LAndExpContext>(0);
}

SysY2022Parser::LAndLOrExpContext::LAndLOrExpContext(LOrExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::LAndLOrExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitLAndLOrExp(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LOrLOrExpContext ------------------------------------------------------------------

SysY2022Parser::LOrExpContext* SysY2022Parser::LOrLOrExpContext::lOrExp() {
  return getRuleContext<SysY2022Parser::LOrExpContext>(0);
}

SysY2022Parser::LAndExpContext* SysY2022Parser::LOrLOrExpContext::lAndExp() {
  return getRuleContext<SysY2022Parser::LAndExpContext>(0);
}

SysY2022Parser::LOrLOrExpContext::LOrLOrExpContext(LOrExpContext *ctx) { copyFrom(ctx); }


std::any SysY2022Parser::LOrLOrExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitLOrLOrExp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::LOrExpContext* SysY2022Parser::lOrExp() {
   return lOrExp(0);
}

SysY2022Parser::LOrExpContext* SysY2022Parser::lOrExp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysY2022Parser::LOrExpContext *_localctx = _tracker.createInstance<LOrExpContext>(_ctx, parentState);
  SysY2022Parser::LOrExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 60;
  enterRecursionRule(_localctx, 60, SysY2022Parser::RuleLOrExp, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    _localctx = _tracker.createInstance<LAndLOrExpContext>(_localctx);
    _ctx = _localctx;
    previousContext = _localctx;

    setState(359);
    lAndExp(0);
    _ctx->stop = _input->LT(-1);
    setState(366);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        auto newContext = _tracker.createInstance<LOrLOrExpContext>(_tracker.createInstance<LOrExpContext>(parentContext, parentState));
        _localctx = newContext;
        pushNewRecursionContext(newContext, startState, RuleLOrExp);
        setState(361);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(362);
        match(SysY2022Parser::T__32);
        setState(363);
        lAndExp(0); 
      }
      setState(368);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- ConstExpContext ------------------------------------------------------------------

SysY2022Parser::ConstExpContext::ConstExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysY2022Parser::AddExpContext* SysY2022Parser::ConstExpContext::addExp() {
  return getRuleContext<SysY2022Parser::AddExpContext>(0);
}


size_t SysY2022Parser::ConstExpContext::getRuleIndex() const {
  return SysY2022Parser::RuleConstExp;
}


std::any SysY2022Parser::ConstExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SysY2022Visitor*>(visitor))
    return parserVisitor->visitConstExp(this);
  else
    return visitor->visitChildren(this);
}

SysY2022Parser::ConstExpContext* SysY2022Parser::constExp() {
  ConstExpContext *_localctx = _tracker.createInstance<ConstExpContext>(_ctx, getState());
  enterRule(_localctx, 62, SysY2022Parser::RuleConstExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(369);
    addExp(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool SysY2022Parser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 25: return mulExpSempred(antlrcpp::downCast<MulExpContext *>(context), predicateIndex);
    case 26: return addExpSempred(antlrcpp::downCast<AddExpContext *>(context), predicateIndex);
    case 27: return relExpSempred(antlrcpp::downCast<RelExpContext *>(context), predicateIndex);
    case 28: return eqExpSempred(antlrcpp::downCast<EqExpContext *>(context), predicateIndex);
    case 29: return lAndExpSempred(antlrcpp::downCast<LAndExpContext *>(context), predicateIndex);
    case 30: return lOrExpSempred(antlrcpp::downCast<LOrExpContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SysY2022Parser::mulExpSempred(MulExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysY2022Parser::addExpSempred(AddExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 1: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysY2022Parser::relExpSempred(RelExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 2: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysY2022Parser::eqExpSempred(EqExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 3: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysY2022Parser::lAndExpSempred(LAndExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 4: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SysY2022Parser::lOrExpSempred(LOrExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 5: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void SysY2022Parser::initialize() {
  ::antlr4::internal::call_once(sysy2022ParserOnceFlag, sysy2022ParserInitialize);
}
