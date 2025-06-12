#include "LCSSA.h"

void computeDefInUseOut(Loop *loop, unordered_set<InstructionPtr> &DefInUseOut)
{
    for (auto bb : loop->getBasicBlocks())
    {
        for (auto instr : bb->instructions)
        {
            if (!instr->reg)
                continue;                   // br等指令
            Use *use = instr->reg->useHead; // 用链表遍历Use集合
            while (use)
            {
                fflush(stdout);
                Instruction *UserInstr = use->user; // 枚举所有使用当前指令的指令
                fflush(stdout);
                fflush(stdout);
                auto userBB = UserInstr->basicblock;
                assert(UserInstr);
                fflush(stdout);
                auto phi2 = dynamic_cast<PhiInstruction *>(UserInstr);
                fflush(stdout);
                if (auto phi = dynamic_cast<PhiInstruction *>(UserInstr))
                {
                    fflush(stdout);
                    for (auto Pred : phi->from)
                    {
                        if (Pred.first == instr->reg) // 5
                        {
                            userBB = Pred.second;
                        }
                    }
                }
                fflush(stdout);
                use = use->next;
            }
        }
    }
}

PhiInstruction *getIncomingValAtBB(BasicBlockPtr bb, InstructionPtr instr,
                                   unordered_map<BasicBlockPtr, PhiInstruction *> &BBToPhi, Loop *loop)
{
    assert(bb);

    if (BBToPhi.count(bb)) // 如果这个块的头部已经被插入Phi指令了
    {
        return BBToPhi[bb];
    }

    auto idom = bb->directDominator;
    if (!loop->bbSet.count(idom)) // user不被这个loop里的block直接支配
    {
        // cerr << "idom for blank\n";
        auto value = getIncomingValAtBB(idom, instr, BBToPhi, loop);
        return value;
    }

    auto phi = new PhiInstruction(bb, instr->reg);
    phi->setName(phi->getName() + ".lcssa");
    bb->insertInstruction(InstructionPtr(phi), bb->instructions[0]);
    BBToPhi[bb] = phi;

    for (auto Pred : phi->from)
    {
        // cerr << "72\n";
        phi->addFrom(getIncomingValAtBB(Pred.second, instr, BBToPhi, loop)->reg, Pred.second);
    }

    return phi;
}

void insertLoopClosedPhi(Loop *loop, InstructionPtr instr)
{
    unordered_map<BasicBlockPtr, PhiInstruction *> BBToPhi;
    auto bb = instr->basicblock;

    for (auto exit : loop->getExitBlocks())
    {
        if (!BBToPhi.count(exit) && exit->getAllDoms().count(bb)) // 1 是exit块，且被当前instr支配，支配边界 + 直接支配 = 支配集合
        {
            // cerr << "here1: " << exit->label->name << endl;
            auto phi = new PhiInstruction(exit, instr->reg);
            phi->setName(phi->getName() + ".lcssa");
            exit->insertInstruction(InstructionPtr(phi), exit->instructions[0]); // 2
            BBToPhi[exit] = phi;                                                 // 如果这个块的头部被插入了phi指令代替liveout instr

            for (auto Pred : exit->predBasicBlocks)
            {
                phi->addFrom(instr->reg, Pred); // 3 第二个参数就是直接支配的边，因为这时候instr支配它，所以instr会经过所有前缀到达它
            }
        }
    }

    std::vector<pair<Instruction *, Use *>> users; // 考虑所有使用过instr的指令
    Use *use = instr->reg->useHead;
    while (use)
    {
        users.push_back(make_pair(use->user, use));
        use = use->next;
    }

    for (auto Pair : users)
    {
        // if(auto userInstr = dynamic_cast<Instruction*>(user)) // User有可能不是Instruction吗？
        //{
        auto user = Pair.first;
        auto userBB = user->basicblock;
        if (auto phi = dynamic_cast<PhiInstruction *>(user))
        {
            for (auto Pred : phi->from)
            {
                if (Pred.first == instr->reg)
                {
                    userBB = Pred.second;
                }
            }
        }

        if (userBB == bb || loop->bbSet.count(userBB)) // 保证在外边
        {
            continue;
        }
        // cerr << "128\n";
        auto value = getIncomingValAtBB(userBB, instr, BBToPhi, loop);
        replaceVarByVarForLCSSA(instr->reg, value->reg, Pair.second);
        // }
    }
}

void runOnLoop(Loop *loop)
{
    if (!loop)
        return;
    for (auto subLoop : loop->subLoops)
    {
        runOnLoop(subLoop.get());
    }

    // cerr << "runOnLoop\n";
    unordered_set<InstructionPtr> DefInUseOut;
    computeDefInUseOut(loop, DefInUseOut); // 定义在Loop里，使用在loop外的指令集合
    //cout<<"finish\n";
    fflush(stdout);
     return ;

    if (DefInUseOut.empty())
        return;

    if (loop->getExitBlocks().empty())
        return;

    for (auto instr : DefInUseOut)
    {
        insertLoopClosedPhi(loop, instr);
    }
}

void LCSSA(FunctionPtr func)
{
    int moveCount = 0;
    vector<LoopPtr> Loops = func->loops;
    int numLoops = Loops.size();
    for (int i = 0; i < numLoops; i++)
    {
        runOnLoop(Loops[i].get());
    }
}