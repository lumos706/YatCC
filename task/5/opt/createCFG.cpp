#include "createCFG.h"
//对BasicBlock连边
void addEdgeInCFG(shared_ptr<BasicBlock> pred,shared_ptr<BasicBlock> succ){
    if (pred->succBasicBlocks.find(succ) != pred->succBasicBlocks.end()) {
        cerr<<"error: Adding Exist Edge to CFG\n";
        return;
    }
    assert(succ != nullptr && pred != nullptr);
    succ->addPredBasicBlock(pred);
    pred->addSuccBasicBlock(succ);
}



//重新计算CFG，即可能会删掉某些块，需要重新计算CFG
void computeCFG(FunctionPtr func) {
    for(int i=0;i<func->basicBlocks.size();i++){
        func->basicBlocks[i]->predBasicBlocks.clear();
        func->basicBlocks[i]->succBasicBlocks.clear();
    }
    func->LabelBBMap.clear();
    func->getLabelBBMap();
    for(int i=0;i<func->basicBlocks.size();i++){
        if(func->basicBlocks[i]->instructions.back()->type==Br){
            addEdgeInCFG(func->basicBlocks[i],
                        func->LabelBBMap[(dynamic_cast<BrInstruction*>(func->basicBlocks[i]->instructions.back().get()))->label_true]);
            if((dynamic_cast<BrInstruction*>(func->basicBlocks[i]->instructions.back().get()))->label_false!=nullptr){
                addEdgeInCFG(func->basicBlocks[i],
                        func->LabelBBMap[(dynamic_cast<BrInstruction*>(func->basicBlocks[i]->instructions.back().get()))->label_false]);
            }
        }
    }
}