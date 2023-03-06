#ifndef __FASTCGRA_PASSMODULE2DFG__
#define __FASTCGRA_PASSMODULE2DFG__

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/ModuleSlotTracker.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Analysis/LoopInfo.h>

#include "common/Common.h"
#include "common/HyperGraph.h"

namespace FastCGRA
{

class PassModule2DFG: public llvm::ModulePass
{
private: 
    std::vector<std::string> _constants; 
    std::vector<std::string> _variables; 
    std::vector<std::string> _varAlloca; 
    std::vector<std::string> _varAllocaTypes; 
    std::vector<size_t> _varAllocaSizes; 
    std::vector<std::string> _varGlobal; 
    std::vector<std::string> _varGlobalTypes; 
    std::vector<size_t> _varGlobalSizes; 
    std::vector<std::string> _varArg; 
    std::vector<std::string> _varArgTypes; 
    std::vector<size_t> _varArgSizes; 
    std::vector<std::string> _varPhi; 
    std::vector<std::vector<std::string>> _varPhiFroms; 
    std::vector<std::vector<std::string>> _varPhiValues; 
    std::vector<std::string> _varBr; 
    std::vector<std::string> _varBrFlag; 
    std::vector<std::vector<std::string>> _varBrBlocks; 
    std::vector<std::string> _constNames; 
    std::vector<std::string> _varNames; 
    std::vector<std::string> _varAllocaNames; 
    std::vector<std::string> _varGlobalNames; 
    std::vector<std::string> _varArgNames; 
    std::vector<std::string> _varPhiNames; 
    std::vector<std::string> _varBrNames; 
    std::unordered_map<std::string, std::string> _constMap; 
    std::unordered_map<std::string, std::string> _varMap; 
    std::unordered_map<std::string, std::string> _varAllocaMap; 
    std::unordered_map<std::string, std::string> _varAllocaTypeMap; 
    std::unordered_map<std::string, size_t> _varAllocaSizeMap; 
    std::unordered_map<std::string, std::string> _varGlobalMap; 
    std::unordered_map<std::string, std::string> _varGlobalTypeMap; 
    std::unordered_map<std::string, size_t> _varGlobalSizeMap; 
    std::unordered_map<std::string, std::string> _varArgMap; 
    std::unordered_map<std::string, std::string> _varArgTypeMap; 
    std::unordered_map<std::string, size_t> _varArgSizeMap; 
    std::unordered_map<std::string, std::string> _varPhiMap; 
    std::unordered_map<std::string, std::vector<std::string>> _varPhiFromsMap; 
    std::unordered_map<std::string, std::vector<std::string>> _varPhiValuesMap; 
    std::unordered_map<std::string, std::string> _varBrMap; 
    std::unordered_map<std::string, std::string> _varBrFlagMap; 
    std::unordered_map<std::string, std::vector<std::string>> _varBrBlocksMap; 
    std::unordered_map<std::string, std::string> _constMapRev; 
    std::unordered_map<std::string, std::string> _varMapRev; 
    std::unordered_map<std::string, std::string> _varAllocaMapRev; 
    std::unordered_map<std::string, std::string> _varGlobalMapRev; 
    std::unordered_map<std::string, std::string> _varArgMapRev; 
    std::unordered_map<std::string, std::string> _varPhiMapRev; 
    std::unordered_map<std::string, std::string> _varBrMapRev; 
    std::unordered_set<std::string> _types; 
    std::unordered_map<std::string, llvm::BasicBlock *> _blocks; // Name -> Address (only basic blocks)
    std::unordered_map<std::string, std::string> _blockVarMap; 
    std::unordered_map<std::string, std::string> _blockVarMapRev; 
    std::unordered_map<std::string, std::string> _addr2name; // Name -> Type
    std::unordered_map<std::string, llvm::Value *> _slotsAddrs; // Name -> Address
    std::unordered_map<std::string, std::string> _slotsTypes; // Name -> Type
    std::unordered_map<std::string, std::string> _slotsValues; // Name -> Value (only constants)
    std::unordered_map<std::string, std::vector<std::string>> _operands; // Name -> Operands
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> _hierarchy; 
    std::unordered_map<std::string, std::string> _block2Func; 
    std::unordered_map<std::string, std::string> _op2Block; 
    std::unordered_map<std::string, std::string> _op2Func; 
    std::unordered_map<std::string, std::string> _finalname2name; 
    std::unordered_map<std::string, std::string> _name2finalname; 
    std::unordered_map<std::string, HyperGraph> _blocksOriGraphs; 
    std::unordered_map<std::string, HyperGraph> _blocksOpGraphs; 
    std::unordered_map<std::string, size_t> _name2number; 
    HyperGraph _graphControl; 
    std::vector<std::string> _markedLoops; 

public: 
    static char ID; 

    PassModule2DFG(): ModulePass(ID) {}
    virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {AU.addRequired<llvm::LoopInfoWrapperPass>(); }

    HyperGraph _passRemoveUnneeded(const HyperGraph &graph); 
    HyperGraph _passRemovePhi(const HyperGraph &graph); 
    HyperGraph _passRemoveGetelementptr(const HyperGraph &graph); 
    HyperGraph _passRemoveUnneededStore(const HyperGraph &graph); 
    HyperGraph _passRemoveInputLoad(const HyperGraph &graph); 
    HyperGraph _passRemoveExt(const HyperGraph &graph); 
    HyperGraph genGraph4BasicBlock(llvm::BasicBlock *block, const std::unordered_set<std::string> &deletedOps = {}); 
    HyperGraph genOpGraph4BlockGraph(const HyperGraph &graph); 
    HyperGraph genControlGraph(); 

    virtual bool runOnModule(llvm::Module &M) override; 

    std::string addOperand(const llvm::Use *operand); 
    std::pair<std::string, size_t> getTypeInfo(const std::string &opName, const llvm::Type::TypeID &typeID); 
    std::string getOpVertexName(const std::string &elem);  
    
};

}

#endif
