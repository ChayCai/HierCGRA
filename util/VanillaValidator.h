#ifndef __FASTCGRA_VANILLAVALIDATOR__
#define __FASTCGRA_VANILLAVALIDATOR__

#include "common/Common.h"
#include "common/HyperGraph.h"

namespace FastCGRA
{

class VanillaValidator
{
private: 
    const Graph &_RRG; 
    const Graph &_DFG; 
    const std::unordered_map<std::string, std::unordered_set<std::string>> &_compat; 

public: 
    VanillaValidator() = delete; 
    VanillaValidator(const Graph &RRG, const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat): _RRG(RRG), _DFG(DFG), _compat(compat) {}
    
bool validate(const std::unordered_map<std::string, std::string> &vertexDFG2RRG, const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &paths); 

}; 

}

#endif
