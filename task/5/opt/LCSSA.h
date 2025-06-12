#include "Module.h"
#include "Loop.h"
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
using namespace std;

void computeDefInUseOut(Loop* loop, unordered_set<InstructionPtr>& DefInUseOut);

PhiInstruction* getIncomingValAtBB(BasicBlockPtr bb, InstructionPtr instr, 
unordered_map<BasicBlockPtr, PhiInstruction*>& BBToPhi, Loop* loop);

void insertLoopClosedPhi(Loop* loop, InstructionPtr instr);
void runOnLoop(Loop* loop);
void LCSSA(FunctionPtr func);