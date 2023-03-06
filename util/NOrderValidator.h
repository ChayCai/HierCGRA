#ifndef __FASTCGRA_NORDERVALIDATOR__
#define __FASTCGRA_NORDERVALIDATOR__

#include "common/Common.h"
#include "common/HyperGraph.h"

namespace FastCGRA
{

class NOrderValidator
{
private: 
    const Graph &_DFG; 
    const Graph &_RRG; 
    const std::unordered_map<std::string, std::unordered_set<std::string>> &_compat; 

public: 
    NOrderValidator() = delete; 
    NOrderValidator(const Graph &DFG, const Graph &RRG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat) : _DFG(DFG), _RRG(RRG), _compat(compat) {} 
    NOrderValidator(const NOrderValidator &validator): _DFG(validator._DFG), _RRG(validator._RRG), _compat(validator._compat) {}
    NOrderValidator(NOrderValidator &&validator): _DFG(validator._DFG), _RRG(validator._RRG), _compat(validator._compat) {}
    
    const NOrderValidator &operator = (const NOrderValidator &validator) = delete; 
    const NOrderValidator &operator = (NOrderValidator &&validator) = delete; 

    size_t validate(const std::string &vertexDFG, const std::string &vertexRRG, size_t order = 1, const std::function<bool(const std::string &)> &used = [](const std::string &){return false; }); 
    size_t validateFast(const std::string &vertexDFG, const std::string &vertexRRG, size_t order = 1, const std::function<bool(const std::string &)> &used = [](const std::string &){return false; }); 
    std::unordered_set<std::string> validateSet(const std::string &vertexDFG, const std::string &vertexRRG, size_t order = 1, const std::function<bool(const std::string &)> &used = [](const std::string &){return false; }); 
    bool validateSlow(const std::string &vertexDFG, const std::string &vertexRRG, size_t order = 1, const std::function<bool(const std::string &)> &used = [](const std::string &){return false; }); 
    bool validateShow(const std::string &vertexDFG, const std::string &vertexRRG, size_t order = 1, const std::function<bool(const std::string &)> &used = [](const std::string &){return false; }); 
}; 

}

#endif
