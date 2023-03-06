#ifndef __FASTPACKER__
#define __FASTPACKER__

#include "./common/Common.h"
#include "./common/HyperGraph.h"
#include "./FastPlacer.h"
#include "./FastRouter.h"

namespace FastCGRA
{

class FastPacker
{

private: 
    Graph _globalRRG; 
    Graph _globalContractedRRG; 

    std::vector<Graph>  _subRRGs; 
    std::vector<Graph>  _subRRGIOs; 
    std::vector<Graph>  _subContractedRRGs; 
    std::vector<Graph>  _subContractedRRGIOs; 
    std::vector<size_t> _numberSubRRGs; 
    std::vector<std::pair<std::size_t, std::size_t>> _portConstraint;
    
    std::unordered_map<std::string, std::unordered_set<std::string>> _FUs; 
    std::unordered_set<std::string> _linkTypes; 

    double _weightSize; 

    Graph _DFG; 
    Graph _globalDFG; 
    Graph _contractedDFG; 
    std::unordered_map<std::string, std::unordered_set<std::string>> _contractedFUs;
    std::unordered_map<std::string, std::unordered_set<std::string>> _compatDevices; 

    Graph _coarseDFG; 
    std::unordered_map<std::string, Graph> _coarseDict; 
    std::unordered_map<std::string, std::unordered_set<size_t>> _coarse2Blocks;
    std::unordered_map<std::string, std::string> _fine2coarse; 
    std::unordered_set<std::string> _usedCoarseDFG; 

    std::vector<std::unordered_set<std::string>> _packsPlacible;
    std::vector<std::unordered_set<std::string>> _packsUnplacible;

public:
    FastPacker() {}
    FastPacker(const Graph &globalRRG, const std::vector<Graph> &subRRGs, const std::vector<size_t> &numberSubRRGs, const std::unordered_map<std::string, std::unordered_set<std::string>> &FUs, double weightSize=0.5); 
    
    std::unordered_map<std::string, std::unordered_set<std::string>> updateCompatTable(const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat); 
    void prepare(const Graph &DFG, const Graph &globalDFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat);
    void portConstraint(const std::vector<std::pair<std::size_t, std::size_t>> &portConstraint);
    
    Graph _packOne(const size_t &type, const std::string &seedInit);
    std::unordered_map<size_t, std::vector<Graph>> Vpack();
    std::unordered_map<size_t, std::vector<Graph>> packTabuAnnealing();

    bool validatePack(const Graph &pack, const size_t &type);
    bool validatePack(const std::unordered_set<std::string> &pack, const size_t &type);
}; 

}

#endif
