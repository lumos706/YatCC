#include "reassociate.h"
#include <unordered_map>

// #define MDEBUG

#ifdef MDEBUG
#define error(...) cerr<<__VA_ARGS__;fflush(stderr)
#else
#define error(...)
#endif


unordered_map<ValuePtr, unsigned> ValueRankMap;
unordered_map<BasicBlockPtr, unsigned> BBRankMap;
unordered_map<Instruction*, unsigned> IRankMap;

unordered_set<Instruction*> isInRedoInsts;
unordered_set<Instruction*> eraseSet;

vector<Instruction* > RedoInsts;



int NumChanged = 0;
int NumAnnihil = 0;
int NumFactor = 0;



using RepeatedValue = std::pair<ValuePtr, int>;

bool MadeChange = false;
struct PairMapValue {
    ValuePtr Value1;
    ValuePtr Value2;                                        
    unsigned Score;
    PairMapValue(){};
    PairMapValue(ValuePtr a,ValuePtr b, unsigned S):Value1(a),Value2(b),Score(S){}
    bool isValid() const { return Value1 && Value2; }
};
//目前的实现中总共12种二元操作
const int NumBinaryOps = 12;
map<std::pair<ValuePtr, ValuePtr>, PairMapValue> PairMap[NumBinaryOps];

static const unsigned GlobalReassociateLimit = 10;

struct ValueEntry {
  unsigned Rank;
  ValuePtr Op;

  ValueEntry(unsigned R, ValuePtr O) : Rank(R), Op(O) {}
};

//Base^Power
struct Factor{
    ValuePtr Base;
    unsigned Power;
    Factor(ValuePtr B,unsigned P):Base{B}, Power{P}{}
};


void insertInstruction(InstructionPtr instruction, Instruction* InsertBefore){

    auto curInstruction = InsertBefore->basicblock->instructions.begin();
    while (curInstruction != InsertBefore->basicblock->instructions.end())
    {
        if ((*curInstruction).get() == InsertBefore) break;
        curInstruction++;
    }
    // make sure next can be found
    assert(curInstruction != InsertBefore->basicblock->instructions.end());
    InsertBefore->basicblock->instructions.emplace(curInstruction, instruction);
    instruction->basicblock = InsertBefore->basicblock;
}


static  InstructionPtr CreateMul(ValuePtr S1, ValuePtr S2, Instruction* InsertBefore){
    if(S1->type->isInt()){
        InstructionPtr Mul = InstructionPtr(new BinaryInstruction(S1,S2,'*',InsertBefore));
        error("insert before\n");
        //在构造函数中插入了
        insertInstruction(Mul, InsertBefore);
        // Mul->print();
        error("insert after\n");
        return Mul;
    }
    else{
        error("Finsert before\n");
        InstructionPtr FMul = InstructionPtr(new BinaryInstruction(S1,S2,'*',InsertBefore));
        insertInstruction(FMul, InsertBefore);
        return FMul;
    }

}

static InstructionPtr CreateNeg(ValuePtr S1,string name,InstructionPtr insertBefore){
   
    if(S1->type->isInt()){
        InstructionPtr Neg = InstructionPtr(new BinaryInstruction(Const::getConst(Type::getInt(),int(0)),S1,'-',insertBefore.get()));
        insertInstruction(Neg,insertBefore.get());
        return Neg;
    }
    else if(S1->type->isFloat()){
        //因为这个指令没有实现insertbefore版本
        InstructionPtr fneg = InstructionPtr(new FnegInstruction(S1,insertBefore->basicblock));
        insertInstruction(fneg,insertBefore.get());
        return fneg;
    }
    else{
        assert(false&&"CreateNeg error");
        return nullptr;
    }

}

//name并不会使用到
static  InstructionPtr CreateAdd(ValuePtr S1, ValuePtr S2, InstructionPtr InsertBefore ,string name = ""){
    error("test::::::::::::        ");
    error(S1->name<<endl);
    error(S2->name<<endl);
    if(S2->I)error(S2->I->reg->name<<endl);
    if(S1->type->isInt()){
        error("what::::::::::::        ");
        error(S1->name<<endl);
        error(S2->name<<endl);

        if(S2->I)error(S2->I->reg->name<<endl);
        if(S2->I)error(S2->I<<endl);
        if(S2->I)error("smart   "<<S2->I->getSharedThis()<<endl);
        auto temp = new BinaryInstruction(S1,S2,'+',InsertBefore.get());
        error(temp<<endl);
        InstructionPtr Add = InstructionPtr(temp);
        error("insert before\n");
        //在构造函数中插入了
        error(Add->reg->name<<endl);
        error(S1->name<<endl);
        error(S2->name<<endl);
        if(S2->I)error(S2->I->reg->name<<endl);
        
        insertInstruction(Add, InsertBefore.get());
        // Add->print();
        error("insert after\n");
        return Add;
    }
    else{
        InstructionPtr FAdd = InstructionPtr(new BinaryInstruction(S1,S2,'+',InsertBefore.get()));
        insertInstruction(FAdd, InsertBefore.get());
        return FAdd;
    }

}


//计算函数中bb的逆后序排列
//DAG的逆后序排列一定是其拓扑排序，需要注意的是，其前序排列不一定是其拓扑排序

void dfsBB(BasicBlockPtr root, vector<BasicBlockPtr> & ret,unordered_map<BasicBlockPtr,bool>& vis){
    if(root==nullptr){
        return;
    }
    for(auto & succ : root->succBasicBlocks){
        if(vis.find(succ)==vis.end()){
            vis[succ] = true;
            dfsBB(succ,ret,vis);
        }
    }
    ret.push_back(root);
}

void getReversPostOrderTraversal(FunctionPtr func, vector<BasicBlockPtr> &rpoBB){
    assert(func->basicBlocks.size()&&"empty function");
    auto entryBlock = func->basicBlocks[0];
    unordered_map<BasicBlockPtr,bool> vis;
    vector<BasicBlockPtr> temp;
    dfsBB(entryBlock,temp,vis);
    for(int i = temp.size()-1;i>=0;i--){
        rpoBB.push_back(temp[i]);
    }
}

bool isInsLoadOrStore(InstructionPtr I){
    if(I->type == Store||I->type == Load||I->type == Phi||I->type == Return||I->type == Br||I->type == Alloca){
        return true;
    }
    else if(I->type == Call){
        auto tI = dynamic_cast<CallInstruction *>(I.get());

        return !(tI->func->isReenterable); 
    }
    return false;
}



void buildRankMap(FunctionPtr func, vector<BasicBlockPtr> & rpoBB){
    unsigned Rank = 2;
    for(auto &args: func->formArguments){
        ValueRankMap[args] = ++Rank;
    }
    for(auto& BB:rpoBB){
        unsigned BBRank = BBRankMap[BB] = ++Rank<<16;
        for(InstructionPtr & I:BB->instructions){
            if(isInsLoadOrStore(I)){
                IRankMap[I.get()] = ++BBRank;
            }
        }
    }
}

//函数参数
unsigned getRank(ValuePtr arg){

    //常数Rank为0
    if(ValueRankMap.find(arg) == ValueRankMap.end()){
        return 0;
    }
    return ValueRankMap[arg];
}
unsigned getRank(Instruction* Ins){
    error("enter getRank\n");
    // if(I)I->print();
    //常数
    if(Ins==nullptr){
        return 0;
    }

    //参数
    
    if(unsigned t = getRank(Ins->reg)){
        return t;
    }

 
    if(IRankMap.find(Ins)!=IRankMap.end()){
        return IRankMap[Ins];
    }

  
    unsigned Rank = 0, MaxRank = BBRankMap[Ins->basicblock];


    for(unsigned i = 0,e = Ins->getNumOperands();i!=e &&Rank!=MaxRank;++i){
        Rank = std::max(Rank,getRank(Ins->getOperand(i)->I));
    }


    if(!(Ins->type == Fneg)){
        ++Rank;
    }


    return IRankMap[Ins] = Rank;
}
//好像用不到
unsigned getRank(BasicBlockPtr bb){
    assert(false&&"get BB rank");
}

//常量和Rank大的在右边，不是很理解为什么
void canonicalizeOperands(Instruction* I){
  
    assert(I->type == Binary && "Expected binary operator.");
    assert(dynamic_cast<BinaryInstruction*>(I)->isCommutative && "expecter commutative operator.");


    ValuePtr LHS = I->getOperand(0);
    ValuePtr RHS = I->getOperand(1);
 
    if(LHS == RHS || RHS->isConst){
        return;
    }

    if(LHS->isConst || getRank(RHS->I) < getRank(LHS->I)){
      dynamic_cast<BinaryInstruction*>(I)->swapOperands();

    }

}

//-x实现为0-1，这里将其降级为x*-1
// static InstructionPtr LowerNegateToMultiply(InstructionPtr Neg){
//     assert(Neg->type == Binary&& Neg->)
// }


//结合律优化，统计各个符合结合律的运算中。各个对出现的频次，可用于后续优化
//比如 a*b*c+a*b*d*f+a*d*c*b中，a*b出现了3次，将其放在一起(a*b)*c+(a*b)*d+(a*b)*c*d，且其在最底层，这样cse就能很容易地识别并优化
void buildPairMap(vector<BasicBlockPtr> & rpoBB){
    for(BasicBlockPtr bb:rpoBB){
        for(InstructionPtr I:bb->instructions){
            if(!(I->isAssociative)){
                continue;
            }
            if((I->reg&&I->reg->hasOneUse())&&(I->reg->useHead->user->type==I->type)){
                continue;
            }

           

            vector<ValuePtr> Worklist = {I->getOperand(0),I->getOperand(1)};
            vector<ValuePtr> Ops;
            while(!Worklist.empty()&&Ops.size()<=GlobalReassociateLimit){
                ValuePtr Op = Worklist.back();
              
                Worklist.pop_back();
                Instruction* OpI = Op->I;
                if(!OpI || 
                (OpI->type != I->type||
                (OpI->type == Binary&&dynamic_cast<BinaryInstruction*>(OpI)->getOpcode()!=dynamic_cast<BinaryInstruction*>(I.get())->getOpcode())) ||
                 !Op->hasOneUse()){
                    
                    Ops.push_back(Op);
                    continue;
                }

                if(OpI->getOperand(0)!=Op){
                    Worklist.push_back(OpI->getOperand(0));
                }
                if(OpI->getOperand(1)!=Op){
                    Worklist.push_back(OpI->getOperand(1));
                }
            }
            if(Ops.size()>GlobalReassociateLimit){
                continue;
            }
            unsigned BinaryIdx = dynamic_cast<BinaryInstruction*>(I.get())->getOpcode();
            set<pair<ValuePtr, ValuePtr>> Visited;
            for(int i =0 ;i<Ops.size()-1;++i){
                for(int j = i+1;j<Ops.size();++j){
                    ValuePtr Op0 = Ops[i];
                    ValuePtr Op1 = Ops[j];
                    //交换的是指针的值，使得较小的指针排在前面，跟指针指向的值没有关系
                    if(std::less<Value *>()(Op1.get(), Op0.get())){
                        std::swap(Op0, Op1);
                    }
                    if(!Visited.insert({Op0,Op1}).second){
                        continue;
                    }
                    auto res = PairMap[BinaryIdx].insert({{Op0,Op1},{Op0,Op1,1}});
                    if(!res.second){
                        assert(res.first->second.isValid()&&"ValuePtr invalidated");
                        ++res.first->second.Score;
                    }

                }
                
            }

        }
    }
}

static BinaryInstruction* isReassociableOp(Instruction* I,unsigned Opcode){
    error("enter isReassociableOp\n");
    auto BI = dynamic_cast<BinaryInstruction*> (I);
    error("isReassociableOp1\n");
    if(BI&&BI->reg->hasOneUse()&&BI->getOpcode()==Opcode){
        if(!BI->getOperand(0)->type->isFloat()){
            error("isReassociableOp2\n");
            return BI;
        }
    }
    error("isReassociableOp3\n");
    return nullptr;
}

static BinaryInstruction* isReassociableOp(Instruction* I,unsigned Opcode1, unsigned Opcode2){
    auto BI = dynamic_cast<BinaryInstruction*> (I);
    if(BI&&BI->reg->hasOneUse()&&(BI->getOpcode()==Opcode1||BI->getOpcode()==Opcode2)){
        return BI;
    }
    return nullptr;
}


static unsigned CarmichaelShift(unsigned Bitwidth) {
  if (Bitwidth < 3)
    return Bitwidth - 1;
  return Bitwidth - 2;
}


static void IncorporateWeight(int& LHS,int &RHS,unsigned Opcode){
    if(RHS==0){
        return ;
    }
    if(LHS ==0){
        LHS  = RHS;
        return;
    }

    //在本结构中不会出现这两种运算符
    // if(Opcode == And||Opcode == Or)
    if(Opcode == Xor){
        assert(LHS==1&&RHS==1&&"Weight not reduced!");
        LHS = 0;
        return;
    }

    if(Opcode == Add||Opcode == FAdd){
        LHS+=RHS;
        return;
    }

    assert((Opcode==Mul||Opcode==FMul)&&"unknown associative operation");

    //参与乘法的位数都是32位
    unsigned Bitwidth = 32;

    if(Bitwidth > 3){
        unsigned CM = 1U<<CarmichaelShift(Bitwidth);        
        unsigned threshold = CM+Bitwidth;
        assert(unsigned(LHS)<threshold&&unsigned(RHS)<threshold&&"Weights not reduced!");

        LHS+=RHS;
        while(LHS>=threshold){
            LHS-=CM;
        }

    }
    else{
        //不会发生
        // unsigned CM = 1<<CarmichaelShift(Bitwidth);
        // unsigned threshold =CM+Bitwidth;


    }




}



static bool LinearizeExprTree(Instruction* I, vector<RepeatedValue>& Ops){
    assert(I->type == Binary &&"expected binaryInstruction");

    error("LinearizeExprTree1\n");
   
    //risc-v中int与float为32位
    unsigned Bitwidth = 32;
    if(I->reg->type->isBool()){
        Bitwidth = 1;
    }
    else if(I->reg->type->isPtr()){
        Bitwidth = 64;
    }
    unsigned Opcode = dynamic_cast<BinaryInstruction*>(I)->getOpcode();
    assert(I->isAssociative&&I->isCommutative&&"Expected associative and commutative operation");

    error("LinearizeExprTree2\n");
    //(Op,weight)
    //weight即每个op在表达式中出现的次数
    vector<pair<Instruction* ,int>> Worklist;
    Worklist.push_back({I,1});
    error("root:   "<<I->reg->name<<endl);
    bool Changed = false;

    //LeaveNode , weight
    unordered_map<ValuePtr, int> Leaves;

    vector<ValuePtr> LeafOrder;
    unordered_set<ValuePtr> visited;
    error("LinearizeExprTree3\n");
    
    while(!Worklist.empty()){
        pair<Instruction*, int> P = Worklist.back();
        Worklist.pop_back();
        I = P.first;
        error("LinearizeExprTree31\n");
        error("name:  "<<I->reg->name<<endl);
        for(unsigned OpIdx = 0;OpIdx< I->getNumOperands();OpIdx++){
            ValuePtr Op = I->getOperand(OpIdx);
            int weight = P.second;
            error("LinearizeExprTree30\n");
            assert(!((Op->numUses)==0)&&"No used");
            error("LinearizeExprTree32\n");
            error(Op->name<<endl);
            if(Op->I) error(Op->I->reg->name<<" "<<(Op->I==I)<<endl);
            if(BinaryInstruction* BO = isReassociableOp(Op->I,Opcode)){
                error("LinearizeExprTree321\n");
                error("BBBBB "<<BO->reg->name<<endl);
                assert(visited.insert(Op).second&&"Not first visit");
                error("LinearizeExprTree323\n");
                Worklist.push_back({BO,weight});
                error("LinearizeExprTree322\n");
                continue;
            }
            error("LinearizeExprTree33\n");
            auto it = Leaves.find(Op);
    
            if(it == Leaves.end()){
                assert(visited.insert(Op).second&&"Not first visit");

                if(!Op->hasOneUse()){
                    LeafOrder.push_back(Op);
                    Leaves[Op] = weight;
                    continue;
                }
            }
           
            else{
                assert(it!=Leaves.end()&&visited.count(Op)&&"In leaf map but not visited");
                
                IncorporateWeight(it->second, weight, Opcode);

                if(!Op->hasOneUse()){
                    continue;
                }

                weight = it -> second;
                Leaves.erase(it);
            }

            error("LinearizeExprTree34\n");
            
            assert((!Op->I||Op->I->type != Binary || dynamic_cast<BinaryInstruction*>(Op->I)->getOpcode()!= Opcode)&&"Should have been handled above");

         
            assert(Op->hasOneUse()&&"Has uses outside the expression tree!");

            //不用处理，负数本来就是被处理成乘-1的
            //但是浮点数的不会,不过不安全就不处理算了
            // if(Op->I){
            //     if((Opcode == Mul&&Op->I->type == Fneg)){

            //     }
            // }
      
            assert(!isReassociableOp(Op->I,Opcode)&&"Value was morphed?");
            LeafOrder.push_back(Op);
            Leaves[Op] = weight;
        }
    }

    error("LinearizeExprTree4\n");

    for(unsigned i = 0,e = LeafOrder.size();i!=e;i++){
        ValuePtr V = LeafOrder[i];
        auto it = Leaves.find(V);
        if(it == Leaves.end()){
            continue;
        }

        assert(!isReassociableOp(V->I,Opcode)&&"should not be a leaf");

        int weight = it->second;
        if(weight == 0){
            continue;
        }
        it->second = 0;
        Ops.push_back({V,weight});
    }
    error("LinearizeExprTree5\n");
    if(Ops.empty()){
        ValuePtr newOp;
        if(Opcode == Add||Opcode == Xor||Opcode == Shl||Opcode==AShr||Opcode == FAdd){
            if(I->reg->type->isInt()){
                
                newOp = Const::getConst(Type::getInt(),0);
            }
            else if(I->reg->type->isFloat()){
                
                newOp = Const::getConst(Type::getFloat(),float(0.0));
            }
            else if(I->reg->type->isBool()){
               
                newOp = Const::getConst(Type::getBool(),false);
            }
        }
        else if(Opcode == Mul||Opcode == FMul){
            if(I->reg->type->isInt()){
           
                newOp = Const::getConst(Type::getInt(),1);
            }
            else if(I->reg->type->isFloat()){
           
                newOp = Const::getConst(Type::getFloat(),float(1.0));
            }
            else if(I->reg->type->isBool()){
              
                newOp = Const::getConst(Type::getBool(),true);
            }
        }
        assert(newOp&&"Associative operation without identity");
        Ops.emplace_back(newOp,1);
    }
    error("LinearizeExprTree6\n");

    return Changed;


}


ValuePtr computeNewConst(unsigned OpCode, ValuePtr C11,ValuePtr C22){
    Const * C1 = dynamic_cast<Const*>(C11.get());
    Const * C2 = dynamic_cast<Const*>(C22.get());
    assert(C1->type == C2->type&&"Operand types in binary constant expression should match");
    if(OpCode == Mul){
        if(C1->type->isInt()){
       
            return Const::getConst(Type::getInt(),C1->intVal*C2->intVal);
        }
        else if(C1->type->isFloat()){
      
            return Const::getConst(Type::getFloat(),C1->floatVal*C2->floatVal);
        }
        else{
            assert(false&&"computeNewConst error");
        }
    }
    else if(OpCode == Add){
        if(C1->type->isInt()){
 
            return Const::getConst(Type::getInt(),C1->intVal+C2->intVal);
        }
        else if(C1->type->isFloat()){
          
            return Const::getConst(Type::getFloat(),C1->floatVal+C2->floatVal);
        }
        else{
            assert(false&&"computeNewConst error");
        }
    }
    else{
        assert(false&&"compute New Constant!!");
        return nullptr;
    }
}
ValuePtr getBinOpIdentity(unsigned Opcode,TypePtr type){
     if(Opcode == Add||Opcode == Xor||Opcode == Shl||Opcode==AShr||Opcode == FAdd){
        if(type->isInt()){

            return Const::getConst(Type::getInt(),0);
        }
        else if(type->isFloat()){
   
            return Const::getConst(Type::getFloat(),float(0.0));
        }
        else if(type->isBool()){
       
            return Const::getConst(Type::getBool(),false);
        }
        else{
            assert(false&&"getbinOpIdentity add xor shl ashr fadd");
        }
    }
    else if(Opcode == Mul||Opcode == FMul){
        if(type->isInt()){
         
            return Const::getConst(Type::getInt(),1);
        }
        else if(type->isFloat()){
          
            return Const::getConst(Type::getFloat(),float(1.0));
        }
        else if(type->isBool()){

            return Const::getConst(Type::getBool(),true);
        }
        else{
            assert(false&&"mul fmul");
        }
    }
    else{
       
        return nullptr;
    }
}


ValuePtr getBinOpAbsorber(unsigned Opcode,TypePtr type){
     if(Opcode == Mul){
        if(type->isInt()){
     
            return Const::getConst(Type::getInt(),0);
        }
        else if(type->isFloat()){
     
            return Const::getConst(Type::getFloat(),float(0.0));
        }
        else if(type->isBool()){
           
            return Const::getConst(Type::getBool(),false);
        }
        else{
            assert(false&&"mul fmul");
        }
    }
    else{
      
        return nullptr;
    }
}




static void FindSingleUseMultiplyFactors(ValuePtr V, vector<ValuePtr>&Factors){
    error("enter FindSingleUseMultiplyFactors\n");
    BinaryInstruction * BI = isReassociableOp(V->I,Mul,FMul);
    error("enter FindSingleUseMultiplyFactors2\n");
    if(!BI){
        Factors.push_back(V);
        return;
    }
    error("enter FindSingleUseMultiplyFactors3\n");
    FindSingleUseMultiplyFactors(BI->getOperand(0), Factors);
    FindSingleUseMultiplyFactors(BI->getOperand(1), Factors);
    error("leave FindSingleUseMultiplyFactors\n");
}


void RewriteExprTree(BinaryInstruction* I, vector<ValueEntry>&Ops){
    assert(Ops.size()>1&&"Single values should be used directly");
    error("RewriteExprTree1\n");
    vector<BinaryInstruction*> NodesToRewrite;
    unsigned Opcode = I->getOpcode();
    BinaryInstruction* Op = I;


    unordered_set<ValuePtr> NotRewritable;

    for(unsigned i = 0, e = Ops.size(); i != e; i++){
        NotRewritable.insert(Ops[i].Op);
    }
    error("RewriteExprTree2\n");
    BinaryInstruction* ExpressionChangedStart = nullptr,*ExpressionChangedEnd = nullptr;
    for(unsigned i = 0; ; i++){
        if(i+2 == Ops.size()){
            ValuePtr NewLHS = Ops[i].Op;
            ValuePtr NewRHS = Ops[i+1].Op;
            ValuePtr OldLHS = Op->getOperand(0);
            ValuePtr OldRHS = Op->getOperand(1);

            if(NewLHS == OldLHS && NewRHS == OldRHS){
                break;
            }

            if(NewLHS == OldRHS && NewRHS == OldLHS){
                Op->swapOperands();
                MadeChange = true;
                ++NumChanged;
                break;
            }
            if(NewLHS != OldLHS){
                BinaryInstruction* BI = isReassociableOp(OldLHS->I, Opcode);
                if(BI && !NotRewritable.count(BI->reg)){
                    NodesToRewrite.push_back(BI);
                }
                Op->setOperands(0, NewLHS);
            }
            if(NewRHS != OldRHS){
                BinaryInstruction* BI = isReassociableOp(OldRHS->I, Opcode);
                if(BI && !NotRewritable.count(BI->reg)){
                    NodesToRewrite.push_back(BI);
                }
                Op->setOperands(1, NewRHS);
            }
            ExpressionChangedStart = Op;
            if(!ExpressionChangedEnd){
                ExpressionChangedEnd = Op;
            }
            MadeChange = true;
            ++NumChanged;
            break;
        }
        ValuePtr NewRHS = Ops[i].Op;
        if(NewRHS != Op->getOperand(1)){
            if(NewRHS == Op->getOperand(0)){
                Op->swapOperands();
            }
            else{
                BinaryInstruction* BI = isReassociableOp(Op->getOperand(1)->I, Opcode);
                if(BI && !NotRewritable.count(BI->reg)){
                    NodesToRewrite.push_back(BI);
                }
                Op->setOperands(1,NewRHS);
                ExpressionChangedStart = Op;
                if(!ExpressionChangedEnd){
                    ExpressionChangedEnd = Op;
                }
            }
            MadeChange = true;
            ++NumChanged;
        }

        BinaryInstruction* BI = isReassociableOp(Op->getOperand(0)->I,Opcode);
        if(BI&&!NotRewritable.count(BI->reg)){
            Op = BI;
            continue;
        }

        BinaryInstruction* NewOp;
        if(NodesToRewrite.empty()){
            ValuePtr Undef = Const::getConst(Type::getInt(), 0);
            // opcode应该为与原式相同，后面才会被认为是inner node
            //先构造一下，防止之后没得用
            auto temp = InstructionPtr(new BinaryInstruction(Undef,Undef,I->op,I));
            NewOp = dynamic_cast<BinaryInstruction*>(temp.get());
    
            insertInstruction(NewOp->getSharedThis(), I);
        }
        else{
            NewOp = NodesToRewrite.back();
            NodesToRewrite.pop_back();
        }

        Op->setOperands(0, NewOp->reg);
        ExpressionChangedStart = Op;
        if(!ExpressionChangedEnd){
            ExpressionChangedEnd = Op;
        }
        MadeChange = true;
        ++NumChanged;
        Op = NewOp;

    }
    error("RewriteExprTree3\n");
    if(ExpressionChangedStart){
        do{
            if(ExpressionChangedStart == I){
                break;
            }
            // eraseInstructionFromBasicBlock(ExpressionChangedStart->getSharedThis());
            // insertInstruction(ExpressionChangedStart->getSharedThis(),I);
            error("RewriteExprTree5\n");

            ExpressionChangedStart->moveBefore(I->getSharedThis(),ExpressionChangedStart->getSharedThis());
            error("RewriteExprTree6\n");
            // ExpressionChanged->moveBefore(I->getSharedThis());
            ExpressionChangedStart = dynamic_cast<BinaryInstruction*>(ExpressionChangedStart->reg->useHead->user);
            error("RewriteExprTree7\n");
        }while(true);
    }
    error("RewriteExprTree4\n");
    for(unsigned i = 0, e = NodesToRewrite.size();i!=e;i++){
        if(isInRedoInsts.insert(NodesToRewrite[i]).second)
            RedoInsts.push_back(NodesToRewrite[i]);
    }

}


ValuePtr RemoveFactorFromExpression(ValuePtr V, ValuePtr Factor){
    BinaryInstruction * BI = isReassociableOp(V->I,Mul,FMul);
    if(!BI){
        return nullptr;
    }

    vector<RepeatedValue> Trees;
    MadeChange |= LinearizeExprTree(BI,Trees);

    vector<ValueEntry> Factors;
    
    for(unsigned i=0,e=Trees.size();i!=e;i++){
        RepeatedValue E = Trees[i];
        for(unsigned j = 0;j<E.second;j++){
            Factors.push_back(ValueEntry(getRank(E.first),E.first));
        }
    }

    bool FoundFactor = false;
    bool NeedsNegate = false;

    for(unsigned i = 0, e = Factors.size(); i != e; i++){
        if(Factors[i].Op == Factor){
            FoundFactor = true;
            Factors.erase(Factors.begin()+i);
            break;
        }
        if(Const * FC1 = dynamic_cast<Const*>(Factor.get())){
            if(Const* FC2 = dynamic_cast<Const*>(Factors[i].Op.get())){
                if(FC1->type == FC2->type){
                    if(FC1->type->isInt()&&FC1->intVal == -FC2->intVal){
                        FoundFactor = NeedsNegate = true;
                        Factors.erase(Factors.begin()+i);
                        break;
                    }
                    else if(FC1->type->isFloat()&&FC1->floatVal==FC2->floatVal){
                        FoundFactor = NeedsNegate = true;
                        Factors.erase(Factors.begin()+i);
                        break;
                    }
                }
            }
        }
    }

    if(!FoundFactor){
        RewriteExprTree(BI, Factors);
        return nullptr;
    }

    //BI下一条
    auto InsertPt = ++BI->getIterator();

    if(Factors.size()==1){
        if(isInRedoInsts.insert(BI).second)
            RedoInsts.push_back(BI);
        V = Factors[0].Op;
    }
    else{
        RewriteExprTree(BI,Factors);
    }

    if(NeedsNegate){
        V = CreateNeg(V,"",*InsertPt)->reg;
    }
    return V;


}

static ValuePtr EmitAddTreeOfValues(InstructionPtr I,vector<ValuePtr>&Ops){
    if(Ops.size() == 1) return Ops.back();
    ValuePtr V1 = Ops.back();
    Ops.pop_back();
    ValuePtr V2 = EmitAddTreeOfValues(I,Ops);
    return CreateAdd(V2,V1,I,"reass.add")->reg;
}

static bool collectMultiplyFactors(vector<ValueEntry>& Ops, vector<Factor>& Factors){
    unsigned FactorPowerSum = 0;
    for(unsigned Idx = 1, Size = Ops.size();Idx < Size;++Idx){
        ValuePtr Op = Ops[Idx-1].Op;
        
        unsigned Count = 1;
        for(;Idx<Size&&Ops[Idx].Op == Op; Idx++){
            Count++;
        }

        if(Count > 1){
            FactorPowerSum += Count;
        }
    }

    if(FactorPowerSum < 4){
        return false;
    }

    FactorPowerSum = 0;
    for(unsigned Idx = 1; Idx<Ops.size();Idx++){
        ValuePtr Op = Ops[Idx-1].Op;
        unsigned Count = 1;

        for(;Idx<Ops.size()&& Ops[Idx].Op == Op;Idx++){
            Count++;
        }
        if(Count == 1)
            continue;
        
        //3 ->2 ; 5->4
        Count &= ~1U;

        Idx-=Count;
        FactorPowerSum+=Count;
        Factors.push_back(Factor(Op,Count));
        Ops.erase(Ops.begin()+Idx,Ops.begin()+Idx+Count);
    }

    assert(FactorPowerSum>=4);

    stable_sort(Factors.begin(),Factors.end(), [](const Factor &LHS, const Factor &RHS){
        return LHS.Power>RHS.Power;
    });
    return true;
}

static ValuePtr buildMultiplyTree(vector<ValuePtr>&Ops,Instruction* InsertBefore){
    if(Ops.size()==1){
        return Ops.back();
    }

    ValuePtr LHS = Ops.back();
    Ops.pop_back();
    do{
        auto tempI = InstructionPtr(new BinaryInstruction(LHS,Ops.back(),'*',InsertBefore));
        insertInstruction(tempI,InsertBefore);
        LHS = tempI->reg;
    }while(!Ops.empty());
    return LHS;
}

ValuePtr buildMinimalMultiplyDAG(vector<Factor>& Factors, Instruction* InsertBefore){
    assert(Factors[0].Power);
    vector<ValuePtr> OuterProduct;
    for(unsigned LastIdx = 0, Idx = 1,Size = Factors.size(); Idx < Size && Factors[Idx].Power > 0;Idx++){
        if(Factors[Idx].Power != Factors[LastIdx].Power){
            LastIdx = Idx;
            continue;
        }

        vector<ValuePtr> InnerProduct;
        InnerProduct.push_back(Factors[LastIdx].Base);
        do{
            InnerProduct.push_back(Factors[Idx].Base);
            Idx++;
        }while(Idx<Size && Factors[Idx].Power == Factors[LastIdx].Power);

        ValuePtr M = Factors[LastIdx].Base = buildMultiplyTree(InnerProduct,InsertBefore);
        if(M->I){
            RedoInsts.push_back(M->I);
        }
        LastIdx = Idx;
    }
    Factors.erase(unique(Factors.begin(),Factors.end(),[](const Factor &LHS, const Factor &RHS){
        return LHS.Power == RHS.Power;
    }),Factors.end());

    for(Factor &F:Factors){
        if(F.Power &1){
            OuterProduct.push_back(F.Base);
        }
        F.Power>>=1;
    }
    if(Factors[0].Power){
        ValuePtr SquareRoot = buildMinimalMultiplyDAG(Factors,InsertBefore);
        OuterProduct.push_back(SquareRoot);
        OuterProduct.push_back(SquareRoot);
    }
    if(OuterProduct.size()==1){
        return OuterProduct.front();
    }
    ValuePtr V = buildMultiplyTree(OuterProduct,InsertBefore);
    return V;
}


ValuePtr OptimizeMul(Instruction *I,vector<ValueEntry>& Ops){
    if(Ops.size()<4){
        return nullptr;
    }

    vector<Factor> Factors;
    if(!collectMultiplyFactors(Ops,Factors)){
        return nullptr;
    }
    for(int i = 0;i<Factors.size();i++){
        cerr<<"Factor   "<<Factors[i].Base->name<<"  "<<Factors[i].Power<<endl;
    }

    ValuePtr  V = buildMinimalMultiplyDAG(Factors,I);
    if(Ops.empty()){
        return V;
    }
    ValueEntry NewEntry = ValueEntry(getRank(V->I), V);
    Ops.insert(lower_bound(Ops.begin(),Ops.end(),NewEntry,[](const ValueEntry & LHS, const ValueEntry&RHS){
        return LHS.Rank<RHS.Rank;
    }), NewEntry);
    return nullptr;
}

// ValuePtr OptimizeXor(Instruction *I,vector<ValueEntry>& Ops){

// }


ValuePtr OptimizeAdd(Instruction *I,vector<ValueEntry>& Ops){
    error("enter OptimizeAdd\n");
    for(unsigned i = 0, e = Ops.size();i!=e;i++){
        
        ValuePtr TheOp = Ops[i].Op;

        //加法变乘法    
        error("reach OptimizeAdd 0\n");
        if(i+1!=Ops.size()&&Ops[i+1].Op == TheOp){
            error("reach OptimizeAdd 1\n");
            unsigned NumFound = 0;
            do {
                error(Ops.size());
                error("\n");

                Ops.erase(Ops.begin()+i);
                ++NumFound;
            }while(i!=Ops.size()&&Ops[i].Op == TheOp);

            ++NumFactor;

            TypePtr Ty = TheOp->type;
            error("reach OptimizeAdd 2\n");
            // Const * C = Ty->isInt()? new Const(int(NumFound)):new Const(float(NumFound));
            ValuePtr C = Ty->isInt()? (Const::getConst(Type::getInt(),int(NumFound))):(Const::getConst(Type::getFloat(),float(NumFound)));
            error("OptimizeAdd insertMul0\n");
            InstructionPtr Mul = CreateMul(TheOp, C,I);
            error("OptimizeAdd insertMul0End\n");
            if(isInRedoInsts.insert(Mul.get()).second)
                RedoInsts.push_back(Mul.get());

            if(Ops.empty()){
                return Mul->reg;
            }

            Ops.insert(Ops.begin(),ValueEntry(getRank(Mul.get()),Mul->reg));
            --i;
            error("reach OptimizeAdd 3\n");
            e = Ops.size();
            continue;
        }
        //to-do
        // ValuePtr X;
    }
    error("reach OptimizeAdd 4\n");
    unordered_map<ValuePtr, unsigned> FactorOccurrences;

    unsigned MaxOcc = 0;
    ValuePtr MaxoccVal = nullptr;


    for(unsigned i = 0, e = Ops.size();i!=e;i++){
        error("reach OptimizeAdd -5\n");
        BinaryInstruction * BOp = isReassociableOp(Ops[i].Op->I, Mul,FMul);
        if(!BOp){
            continue;
        }
        error("reach OptimizeAdd 5\n");
        vector<ValuePtr> Factors;
        FindSingleUseMultiplyFactors(BOp->reg, Factors);
        error("reach OptimizeAdd 6\n");

        assert(Factors.size()>1&&"Bad linearize!");

        unordered_set<ValuePtr> Duplicates;
        error("reach OptimizeAdd 7\n");
        for(unsigned j = 0;j!=Factors.size();j++){
            error("reach OptimizeAdd 8\n");
            ValuePtr Factor = Factors[j];
            error("reach OptimizeAdd 8\n");
            if(!Duplicates.insert(Factor).second){
                continue;
            }
            error("reach OptimizeAdd 9\n");
            unsigned Occ = ++FactorOccurrences[Factor];
            if(Occ>MaxOcc){
                MaxOcc = Occ;
                MaxoccVal = Factor;
            }
            
            //负数当作整数计，应为提取公因式时一般不提取负数，但需要注意到的是，下面代码只在实现了常数的单例模式后才有效(已实现)
            error("reach OptimizeAdd 10\n");
            if(Const* C = dynamic_cast<Const*>(Factor.get())){
                if(C->type->isInt()){
                    if(C->intVal<0&&C->intVal!=INT_MIN){
                     
                        Factor = Const::getConst(Type::getInt(),-C->intVal);
                        if(!Duplicates.insert(Factor).second){
                            continue;
                        }
                        unsigned Occ = ++FactorOccurrences[Factor];
                        if(Occ>MaxOcc){
                            MaxOcc = Occ;
                            MaxoccVal = Factor;
                        }
                    }
                }
                else if(C->type->isFloat()){
                    if(C->floatVal<0){
           
                        Factor = Const::getConst(Type::getFloat(),-C->floatVal);
                        if(!Duplicates.insert(Factor).second){
                            continue;
                        }
                        unsigned Occ = ++FactorOccurrences[Factor];
                        if(Occ>MaxOcc){
                            MaxOcc = Occ;
                            MaxoccVal = Factor;
                        }
                    }
                }
            }
        }
        error("reach OptimizeAdd 11\n");
    }

    error("reach OptimizeAdd 12\n");
    if(MaxOcc>1){
        ++NumFactor;

        //llvm 中用于保存对maxoccval的use，防止其被当作死代码删除，我们的优化没那么高级(
        // InstructionPtr DummyInst = InstructionPtr(new BinaryInstruction(MaxoccVal,MaxoccVal,'+',I->basicblock));

        vector<ValuePtr> NewMulOps;
        for(unsigned i = 0;i!=Ops.size();i++){
            BinaryInstruction* BOp = isReassociableOp(Ops[i].Op->I, Mul,FMul);
            if(!BOp) continue;

            if(ValuePtr V = RemoveFactorFromExpression(Ops[i].Op, MaxoccVal)){
                //找到相同项，直接使用，节省计算时间，i其实也会被删掉，所以--i，防止跳过指令,--i再++
                for(unsigned j = Ops.size();j!=i;){
                    --j;
                    if(Ops[j].Op == Ops[i].Op){
                        NewMulOps.push_back(V);
                        Ops.erase(Ops.begin()+j);
                    }
                }
            --i;
            }
        } 


        unsigned NumAddedValues = NewMulOps.size();
        ValuePtr V = EmitAddTreeOfValues(I->getSharedThis(),NewMulOps);

        assert(NumAddedValues > 1 &&"Each occurrence should contribute a value");

        if(Instruction* VI = V->I){
            if(isInRedoInsts.insert(VI).second)
                RedoInsts.push_back(VI);
        }
        error("OptimizeAdd insertMul\n");
        Instruction* V2 = CreateMul(V,MaxoccVal,I).get();
        error("OptimizeAdd insertMulEnd\n");
        if(isInRedoInsts.insert(V2).second)
            RedoInsts.push_back(V2);

        error("OptimizeAdd insertMulEnd2\n");
        if(Ops.empty()){
            return V2->reg;
        }
        error("OptimizeAdd insertMulEnd3\n");
        Ops.insert(Ops.begin(), ValueEntry(getRank(V2),V2->reg));
        error("OptimizeAdd insertMulEnd4\n");

    }


    error("reach OptimizeAdd End\n");
    return nullptr;
}


ValuePtr OptimizeExpression(BinaryInstruction* I ,vector<ValueEntry>&Ops){
    error("reach OptimizeExpression\n");
    ValuePtr mConst = nullptr;
    unsigned Opcode = I->getOpcode();
    while(!Ops.empty()&&Ops.back().Op->isConst){
        ValuePtr C = Ops.back().Op;
        Ops.pop_back();
        mConst = mConst!=nullptr ? computeNewConst(Opcode,C, mConst):C;
    }

    error("reach OptimizeExpression2\n");
    if(Ops.empty()){
        return mConst;
    }
    error("reach OptimizeExpression3\n");
    if(mConst){
        if(mConst->type->isInt()){
            error("reach OptimizeExpression111\n");
            if(dynamic_cast<Const*>(mConst.get())->intVal != dynamic_cast<Const*>(getBinOpIdentity(Opcode, I->reg->type).get())->intVal){
                if(auto t = getBinOpAbsorber(Opcode,I->reg->type)){
                    // error("Absorber\n");
                    if(mConst==t)
                        return mConst;
                }
                error("reach OptimizeExpression222\n");
                Ops.push_back(ValueEntry(0,mConst));
            }
        }
        else if(mConst->type->isFloat()){
            if(dynamic_cast<Const*>(mConst.get())->floatVal != dynamic_cast<Const*>(getBinOpIdentity(Opcode, I->reg->type).get())->floatVal){
                if(auto t = getBinOpAbsorber(Opcode,I->reg->type)){
                    if(mConst==t)
                        return mConst;
                }
                Ops.push_back(ValueEntry(0,mConst));
            }
        }
        else{
            assert(false&&"bool value enter OptimizeExpression");
        }
        
    }
    error("reach OptimizeExpression4\n");
    if(Ops.size()==1) return Ops[0].Op;

    
    unsigned NumOps = Ops.size();
    error("reach OptimizeExpression5\n");

    if(Opcode == Add||Opcode == FAdd){
        if(ValuePtr Result = OptimizeAdd(I,Ops)){
            error("OptimizeAdd return\n");
            return Result;
        }
    }
    // else if(Opcode == Xor){
    //     if(ValuePtr Result = OptimizeXor(I,Ops)){
    //         return Result;
    //     }
    // }
    else if(Opcode == Mul||Opcode == FMul){
        if(ValuePtr Result = OptimizeMul(I,Ops)){
            return Result;
        }
    }
    error("reach OptimizeExpression6\n");
    if(Ops.size()!=NumOps){
        return OptimizeExpression(I,Ops);
    }
    error("reach OptimizeExpression7\n");
    return nullptr;

}



void ReassociateExpression(BinaryInstruction* I){
    error("ReassociateExpression-1\n");
    vector<RepeatedValue> Tree;


    MadeChange|= LinearizeExprTree(I,Tree);
    
    error("ReassociateExpression-2\n");
    vector<ValueEntry>  Ops;


    for(unsigned i = 0,e = Tree.size();i!=e;i++){
        RepeatedValue E = Tree[i];
        for(unsigned j = 0;j<E.second;j++){
            Ops.push_back(ValueEntry(getRank(E.first), E.first));
        }
    }
    error("ReassociateExpression-3\n");

    stable_sort(Ops.begin(),Ops.end(),[&](ValueEntry a ,ValueEntry b){return a.Rank>b.Rank;});


    if(ValuePtr V = OptimizeExpression(I,Ops)){
        error("OptimizeExpression return\n");
        if(dynamic_cast<BinaryInstruction*>(V.get()) == I){
            error("OptimizeExpression return1\n");
            return;
        }
        error("OptimizeExpression return2\n");
        // I->replaceAllUsesWith(V);
        replaceVarByVar(I->reg,V);
        error("OptimizeExpression return3\n");
        if(isInRedoInsts.insert(I).second)
            RedoInsts.push_back(I);
        ++NumAnnihil;
        return;
    }
    error("ReassociateExpression1\n");
    if(I->reg->hasOneUse()){
        if(I->getOpcode()==Mul&&I->reg->useHead->user->type==Binary&&dynamic_cast<BinaryInstruction*>(I->reg->useHead->user)->getOpcode()==Add&&
            Ops.back().Op->isConst&&Ops.back().Op==Const::getConst(Type::getInt(),int(-1))){
                ValueEntry Tmp = Ops.back();
                Ops.pop_back();
                Ops.insert(Ops.begin(),Tmp);
            }
            //浮点数随便搞搞，意思一下，因为之前已经将浮点数return了，不处理
        else if(I->getOpcode()==FMul&&I->reg->useHead->user->type==Binary&&dynamic_cast<BinaryInstruction*>(I->reg->useHead->user)->getOpcode()==FAdd&&
            Ops.back().Op->isConst&&Ops.back().Op==Const::getConst(Type::getFloat(),float(-1))){
                ValueEntry Tmp = Ops.back();
                Ops.pop_back();
                Ops.insert(Ops.begin(),Tmp);
            }
    }

    error("ReassociateExpression2\n");

    if(Ops.size()==1){
        if(Ops[0].Op->I == I){
            return;
        }
        replaceVarByVar(I->reg, Ops[0].Op);
        if(isInRedoInsts.insert(I).second)
            RedoInsts.push_back(I);
        return;
    }

    //to-do 有时间认真看看
    error("ReassociateExpression3\n");
    if(Ops.size()>2&&Ops.size()<GlobalReassociateLimit){
        error("ReassociateExpression31\n");
        unsigned Max = 1;
        unsigned BestRank = 0;
        pair<unsigned, unsigned> BestPair;
        unsigned Idx = I->getOpcode();
        unsigned LimitIdx = 0;

        bool CSELocalOpt = true;
        error("ReassociateExpression32\n");
        if(CSELocalOpt){
            BasicBlockPtr FirstSeenBB = nullptr;
            int StartIdx = Ops.size()-1;

            for(int i = StartIdx - 1;i!=-1;i--){
                ValuePtr Val = Ops[i].Op;
                auto CurrLeafInstr = Val->I;
                BasicBlockPtr SeenBB = nullptr;
                error("ReassociateExpression321\n");
                if(!CurrLeafInstr){
                    error("ReassociateExpression322\n");

                    if(!I->basicblock->belongfunc){
                        assert(false&&"block null");
                    }
                    SeenBB = I->basicblock->belongfunc->basicBlocks[0];
                    error("ReassociateExpression323\n");
                }
                else{
                    error("ReassociateExpression324\n");
                    SeenBB = CurrLeafInstr->basicblock;
                    error("ReassociateExpression325\n");
                }

                if(!FirstSeenBB){
                    FirstSeenBB = SeenBB;
                    continue;
                }

                if(FirstSeenBB != SeenBB){
                    LimitIdx = i+1;
                    break;
                }
                
            }
        }
        error("ReassociateExpression33\n");

        for(unsigned i = Ops.size()-1;i>LimitIdx;--i){
            for(int j = i-1;j>=(int)LimitIdx;--j){
                unsigned Score = 0;
                ValuePtr Op0 = Ops[i].Op;
                ValuePtr Op1 = Ops[j].Op;
                if(std::less<ValuePtr>()(Op1,Op0)){
                    std::swap(Op0,Op1);
                }
                auto it = PairMap[Idx].find({Op0,Op1});
                if(it!=PairMap[Idx].end()){
                    if(it->second.isValid()){
                        Score+=it->second.Score;
                    }
                }
                unsigned MaxRank = std::max(Ops[i].Rank,Ops[j].Rank);


                if(Score>Max ||(Score == Max&&MaxRank<BestRank)){
                    BestPair = {j,i};
                    Max = Score;
                    BestRank = MaxRank;
                }

            }
        }
        error("ReassociateExpression34\n");
        if(Max>1){
            auto Op0 = Ops[BestPair.first];
            auto Op1 = Ops[BestPair.second];
            Ops.erase(Ops.begin()+BestPair.second);
            Ops.erase(Ops.begin()+BestPair.first);
            Ops.push_back(Op0);
            Ops.push_back(Op1); 
        }
    }
    error("ReassociateExpression4\n");
    RewriteExprTree(I,Ops);
    error("ReassociateExpression5\n");

    error("OptimizeExpression return3\n");

}

static bool isNegFneg(BinaryInstruction* BI){
    error("enter isNegFneg\n");
    if(BI&&BI->getOpcode()==Sub&&
    (BI->getOperand(0)==Const::getConst(Type::getInt(),0)||
    BI->getOperand(0)==Const::getConst(Type::getFloat(),float(0)))){
        return true;    
    }
    if(BI&&BI->getOpcode()==FSub&&
    (BI->getOperand(0)==Const::getConst(Type::getInt(),0)||
    BI->getOperand(0)==Const::getConst(Type::getFloat(),float(0)))){
        return true;    
    }
    return false;
}

static bool ShouldBreakUpSubtract(BinaryInstruction* Sub){
    error("enter ShouldBreakUpSubtract\n");
    //0-x 不用再转换乘0+-x了，没有意义
    if(isNegFneg(Sub))
        return false;
    error("ShouldBreakUpSubtract1\n");
    ValuePtr V0 = Sub->getOperand(0);
    if(isReassociableOp(V0->I,Add,FAdd)||isReassociableOp(V0->I,BinaryInstructionOps::Sub,FSub)){
        return true;
    }
    error("ShouldBreakUpSubtract2\n");
    ValuePtr V1 = Sub->getOperand(1);
    if(isReassociableOp(V1->I,Add,FAdd)||isReassociableOp(V1->I,BinaryInstructionOps::Sub,FSub)){
        return true;
    }
    error("ShouldBreakUpSubtract3\n");
    if(!Sub->reg->useHead) return false;
    ValuePtr VB = Sub->reg->useHead->user->reg;
    if(Sub->reg->hasOneUse()&&VB&&(isReassociableOp(VB->I,Add,FAdd)||isReassociableOp(VB->I,BinaryInstructionOps::Sub,FSub))){
        return true;
    }
    error("end ShouldBreakUpSubtract\n");
    return false;

}
static ValuePtr NegateValue(ValuePtr V, BinaryInstruction* BI,vector<Instruction*>ToRedo){
    error("enter NegateValue\n");
    if(V->isConst){
        auto C = dynamic_cast<Const*>(V.get());
        return C->type->isFloat()?Const::getConst(Type::getFloat(),float(-1.0*C->floatVal)):Const::getConst(Type::getInt(),-1*C->intVal);
    }
    error("NegateValue1\n");
    if(BinaryInstruction* I = isReassociableOp(V->I,Add,FAdd)){
        auto t1 = NegateValue(I->getOperand(0),BI,ToRedo);
        auto t2 = NegateValue(I->getOperand(1),BI,ToRedo);

        I->setOperands(0,t1);
        I->setOperands(1,t2);

        I->moveBefore(BI->getSharedThis(),I->getSharedThis());
        I->setName(I->getName()+".neg");
        
        error(I->reg->name<<endl);
        ToRedo.push_back(I);
        return I->reg;
    }
    error("NegateValue2\n");

    //找找看有没有现成的0-V
    Use* U = V->useHead;
    while(U){
        if(!isNegFneg(dynamic_cast<BinaryInstruction*>(U->user))){
            U=U->next;
            continue;
        }
        Instruction* TheNeg = U->user;

        //属于同一个函数
        if(TheNeg->basicblock->belongfunc!=BI->basicblock->belongfunc){
            U=U->next;
            continue;
        }

        vector<InstructionPtr>::iterator InsertBefore;
        if(Instruction* InstInput = V->I){
            if(V->I->type==Phi){
                InsertBefore = InstInput->basicblock->instructions.end();
                InsertBefore--;
                if((InsertBefore!=InstInput->basicblock->instructions.begin())&&((*(InsertBefore-1))->type==Icmp||(*(InsertBefore-1))->type==Fcmp)){
                    InsertBefore--;
                }
            }
            else{
                InsertBefore = InstInput->getIterator();
                InsertBefore++;
            }
            if(InsertBefore==InstInput->basicblock->instructions.end()){
                U=U->next;
                continue;
            }
        }
        else{
            InsertBefore = TheNeg->basicblock->belongfunc->basicBlocks[0]->instructions.begin();
        }

        TheNeg->moveBefore(*InsertBefore,TheNeg->getSharedThis());
        ToRedo.push_back(TheNeg);
        return TheNeg->reg;
    }
    InstructionPtr NewNeg = CreateNeg(V,V->name+".neg", BI->getSharedThis());
    ToRedo.push_back(NewNeg.get());
    return NewNeg->reg;
    
}



static InstructionPtr BreakUpSubtract(BinaryInstruction* Sub,vector<Instruction*>&ToRedo){
    error("enter BreakUpSubtract\n");
    ValuePtr NegVal = NegateValue(Sub->getOperand(1),Sub,ToRedo);
    error((NegVal->name)<<endl);
    if(NegVal->I) error("test:            "<<NegVal->I->reg->name<<endl);
    InstructionPtr New = CreateAdd(Sub->getOperand(0),NegVal,Sub->getSharedThis(),"");

    Sub->setOperands(0,Const::getConst(Type::getInt(),0));
    Sub->setOperands(1,Const::getConst(Type::getInt(),0));
    
    replaceVarByVar(Sub->reg,New->reg);
    error("end BreakUpSubtract\n");
    return New;
}


//0-x -> x*-1  为了获得更多的优化机会
static InstructionPtr LowerNegateToMultiply(BinaryInstruction* Neg){
    error("enter LowerNegateToMultiply\n ");
    
    InstructionPtr Res = CreateMul(Neg->getOperand(1),Const::getConst(Type::getInt(),-1),Neg);
    Neg->setOperands(1,Const::getConst(Type::getInt(),0));
    replaceVarByVar(Neg->reg,Res->reg);
    error("endd LowerNegateToMultiply\n ");
    return Res;
}



void OptimizeInst(InstructionPtr I){

    if(I->type!=Binary){
        return ;
    }
    auto BI = dynamic_cast<BinaryInstruction*>(I.get());


    //将位移改为乘法，目前没必要做
    // if(BI->getOpcode()==Shl && I->getOperand(1)->isConst){
    //     if(isReassociableOp(BI->getOperand(0)->I, Mul)||
    //      (BI->reg->hasOneUse()&&
    //       (isReassociableOp(BI->reg->useHead->user, Mul)||
    //        isReassociableOp(BI->reg->useHead->user,Add)))){
    //         InstructionPtr     
    //     }
    // }


    if(BI->isCommutative){
        canonicalizeOperands(BI);
    }

    //负浮点数先不管  因为后面直接把浮点数跳过了
 
    //do not deal with float point and bool
    if(BI->getOperand(0)->type->isFloat()||BI->getOperand(0)->type->isBool()){
        return;
    }

    //to-do
    // transfer a-b -> a+-b to find more oppotunity
    error("before Sub\n");
    //to-do 等将负数转为减法的优化完成后再说
    if(BI->getOpcode()==Sub){
        if(ShouldBreakUpSubtract(BI)){
            error("sub1\n");
            InstructionPtr NI =BreakUpSubtract(BI,RedoInsts);
            RedoInsts.push_back(BI);
            MadeChange = true;
            BI = dynamic_cast<BinaryInstruction*>(NI.get());
        }
        else if(isNegFneg(BI)){
            error("sub2\n");
            if(isReassociableOp(BI->getOperand(1)->I,Mul)&&
                (!BI->reg->hasOneUse()||
                 !isReassociableOp(BI->reg->useHead->user,Mul))){
                    InstructionPtr NI = LowerNegateToMultiply(BI);
                    auto use = NI->reg->useHead;
                    while(use){
                        if(use->user->type==Binary){
                            if(isInRedoInsts.insert(use->user).second){
                                RedoInsts.push_back(use->user);
                            }
                        }
                    }
                    RedoInsts.push_back(BI);
                    MadeChange = true;
                    BI = dynamic_cast<BinaryInstruction*>(NI.get());
                 }
        }
    }


    if(!BI->isAssociative) return;


    unsigned Opcode = BI->getOpcode();
    if(BI->reg->hasOneUse()&&BI->reg->useHead->user->type == Binary&&
        dynamic_cast<BinaryInstruction*>(BI->reg->useHead->user)->getOpcode()==Opcode){
            error("goto father\n");
            if(BI->reg->useHead->user!= BI&&
                BI->basicblock == BI->reg->useHead->user->basicblock){
                    if(isInRedoInsts.insert(BI->reg->useHead->user).second)
                        RedoInsts.push_back(BI->reg->useHead->user);
            }
            return;
    }
    if(BI->reg->hasOneUse() && BI->getOpcode()==Add&&
     BI->reg->useHead->user->type==Binary&&dynamic_cast<BinaryInstruction*>(BI->reg->useHead->user)->getOpcode()==Sub){
        return;
    }
    if(BI->reg->hasOneUse() && BI->getOpcode()==FAdd&&
     BI->reg->useHead->user->type==Binary&&dynamic_cast<BinaryInstruction*>(BI->reg->useHead->user)->getOpcode()==FSub){
        return;
    }

    error("before  reaass\n");
    ReassociateExpression(BI);
    error("after  reaass\n");
}



void clearAllReassociate(){
    ValueRankMap.clear();
    BBRankMap.clear();
    IRankMap.clear();
    RedoInsts.clear();

    NumChanged = 0;
    NumAnnihil = 0;
    NumFactor = 0;

    MadeChange = false;
    for(int i =0 ;i<NumBinaryOps;i++){
        PairMap[i].clear();
    }

}


//dead code elimination
static bool isDeadCode(Instruction* Ins){
    error("enter isDeadCode\n");
    if(Ins->reg)error(Ins->reg->name<<"\n");
    if(!Ins->reg||Ins->reg->useHead){
        error("enter isDeadCode2\n");
        return false;
    }
    if(Ins->type == Call &&((!dynamic_cast<CallInstruction*>(Ins)->func->isReenterable)||dynamic_cast<CallInstruction*>(Ins)->func->isLib)){
        error("enter isDeadCode4\n");
        return false;
    }
    error("enter isDeadCode3\n");
    return true;
}

void EraseInst(Instruction * I){
    error("EraseInst\n");
    assert(isDeadCode(I)&&"Only Erase Dead Inst");
    eraseSet.insert(I);

    vector<ValuePtr> Ops;
    for(unsigned i = 0;i<I->getNumOperands();i++){
        Ops.push_back(I->getOperand(i));
    }
    IRankMap.erase(I);
    for(auto it = RedoInsts.begin();it!=RedoInsts.end();it++){
        if((*it)==I){
            isInRedoInsts.erase(*it);
            RedoInsts.erase(it);
            break;
        }
    }
    error("EraseInst2\n");


    
    unordered_set<Instruction*>Visited;
    for(unsigned i = 0, e = Ops.size();i!=e;i++){
        auto use = Ops[i]->useHead;
        error("EraseInst3\n");
        while(use){
            if(use->user == I){
                use->rmUse();
                break;
            }
            use = use->next;
        }
        error("EraseInst4\n");
        if(BinaryInstruction* Op = dynamic_cast<BinaryInstruction*>(Ops[i]->I)){
            error("EraseInst41\n");
            unsigned Opcode  = Op->getOpcode();
            error("EraseInst42\n");
            while(Op->reg->hasOneUse()&&dynamic_cast<BinaryInstruction*>(Op->reg->useHead->user)&&dynamic_cast<BinaryInstruction*>(Op->reg->useHead->user)->getOpcode()==Opcode&&
            Visited.insert(Op).second){
                Op = dynamic_cast<BinaryInstruction*>(Op->reg->useHead->user);
            }
            error("EraseInst43\n");

            if(IRankMap.count(Op)){
                if(isInRedoInsts.insert(Op).second&&!eraseSet.count(Op))
                    RedoInsts.push_back(Op);
            }
        }
        error("EraseInst5\n");
    }
    error("EraseInst9\n");
    I->deleteSelfInBB();

    MadeChange = true;
}

void RecursivelyEraseDeadInsts(Instruction* I,vector<Instruction*>&Insts){
    error("enter RecursivelyRraseDeadInsest\n");
    assert(isDeadCode(I)&&"Only RecursivelyErase Dead Insts");
    eraseSet.insert(I);
    vector<ValuePtr> Ops;
    for(unsigned i = 0;i<I->getNumOperands();i++){
        Ops.push_back(I->getOperand(i));
    }
    IRankMap.erase(I);
    for(auto it = Insts.begin();it!=Insts.end();it++){
        if(*it == I){
            Insts.erase(it);
            break;
        }
    }
    error("RecursivelyRraseDeadInsest1\n");
    for(auto it = RedoInsts.begin();it!=RedoInsts.end();it++){
        if(*it == I){
            isInRedoInsts.erase(*it);
            RedoInsts.erase(it);
            break;
        }
    }
    error("RecursivelyRraseDeadInsest2\n");
    
    for(unsigned i=0,e = Ops.size();i!=e;i++){
        error("RecursivelyRraseDeadInsest7\n");
        ValuePtr Op =  Ops[i];
        auto use = Op->useHead;
        error("RecursivelyRraseDeadInsest5\n");
        while(use){
            if(use->user == I){
                use->rmUse();
                break;
            }
            use = use->next;
        }
        error("RecursivelyRraseDeadInsest6\n");
        if(Op->I){
            if(Op->getNumUses()==0&&!eraseSet.count(Op->I)){
                Insts.push_back(Op->I);
            }
        }
    }
    
    error("RecursivelyRraseDeadInsest4\n");
    I->deleteSelfInBB();
    error("RecursivelyRraseDeadInsest3\n");
    
}


void reassociate(FunctionPtr func){

    //实现初没有将其封装为类，所以需要清理一下全局变量
    clearAllReassociate();

    vector<BasicBlockPtr> rpoBB;
    getReversPostOrderTraversal(func,rpoBB);

    error("before buildRankMap\n");
    buildRankMap(func,rpoBB);
    error("after buildRankMap\n");

    error("before buildPairMap\n");
    buildPairMap(rpoBB);
    error("after buildPairMap\n");


    MadeChange = false;
    
    for(auto& BB :rpoBB){
        assert(BBRankMap.count(BB)&&"BB should be ranked.");

        for(auto it = BB->instructions.begin();it!=BB->instructions.end();){

            error("before isDeadCode\n");
            if(isDeadCode((*it).get())){
                error("before eraseInst\n");
                EraseInst((*it).get());
                error("after EraseInst\n");
            }
            else{
                error("before OptimizeInst\n");
                auto tempI = *it;
                OptimizeInst(*it);
                it = find(BB->instructions.begin(),BB->instructions.end(),tempI);
                if(it==BB->instructions.end()){
                    assert(false&&"error");
                }
                error("after OptimizeInst\n");
                assert((*it)->basicblock == BB && "Moved to a different block");
                it++;
            }
            
        }
        vector<Instruction*> ToRedo(RedoInsts);

        error("reach Toddo\n");
        while(!ToRedo.empty()){
            Instruction* I = ToRedo.back();
            ToRedo.pop_back();
            if(isDeadCode(I)){
                RecursivelyEraseDeadInsts(I,ToRedo);
                MadeChange = true;
            }
        }
        error("reach Redo\n");
        while(!RedoInsts.empty()){
            Instruction * I = RedoInsts.front();
            isInRedoInsts.erase(I);
            RedoInsts.erase(RedoInsts.begin());
            if(isDeadCode(I)){
                // EraseInst(I);
            }
            else{
                error("before Redo OptimizeInst\n");
                OptimizeInst(I->getSharedThis());
                error("after Redo OptimizeInst\n");
            }
        }
        error("redo finish\n");
        if(MadeChange){
            //那些分析要重做
        }

    }

    error("finish \n");

}