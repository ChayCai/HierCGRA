#ifndef __FASTPLACE__
#define __FASTPLACE__

#include "./common/Common.h"
#include "./common/HyperGraph.h"
#include "./util/NOrderValidator.h"
#include "./util/NetworkAnalyzer.h"
#include "./util/GraphSort.h"
#include "./util/Utils.h"
#include "./model/FastPlacer.h"
#include "./model/FastRouter.h"
#include <time.h>

namespace FastCGRA
{
    namespace FastPlace
    {
        bool PlaceCore(const std::string &dfg, const std::string &compat,const std::string &dfgGlobal, 
                    const std::string &rrg, const std::string &fus, 
                    const std::vector<std::string> &mappedPack, const std::vector<std::string> &routedPack,
                    const std::string &mapped, const std::string &routed, 
                    const std::string &sortMode = "TVS",
                    const std::unordered_multimap<std::string, std::string> &forbidAdditional = {},const std::string &seed = "");

        bool PlaceCore(const std::string &dfg, const std::string &compat,const std::string &dfgGlobal, 
                    const std::string &rrg, const std::string &fus, 
                    const std::unordered_map<std::string, std::string> blocks,
                    const std::string &mapped, const std::string &routed, 
                    const std::vector<std::string> &blocktypes,
                    const std::unordered_map<std::string, std::vector<std::string>> &type2block,
                    const std::string &sortMode = "TVS",
                    const std::unordered_multimap<std::string, std::string> &forbidAdditional = {},const std::string &seed = "");

        bool PlaceCoreII(const std::string &dfg, const std::string &compat,
                    const std::string &rrg, const std::string &fus, 
                    const std::string &mapped, const std::string &routed, 
                    const size_t intial_interval = 1,
                    const std::string &sortMode = "TVS",
                    const std::unordered_multimap<std::string, std::string> &forbidAdditional = {},const std::string &seed = "");

        bool PlaceBlock(const std::string &dfg, const std::string &compat,const std::string &dfgGlobal, 
                    const std::string &rrg, const std::string &fus,
                    const std::string &mapped, const std::string &routed, 
                    const std::string &sortMode = "TVS",
                    const std::unordered_multimap<std::string, std::string> &forbidAdditional = {},const std::string &seed = "");
        bool PlaceTop(const std::string &dfg, const std::string &compat, const std::string &rrg, const std::string &fus, 
                                    const std::vector<std::string> &top,
                                    const std::vector<std::string> &coreplaced, const std::vector<std::string> &corerouted, 
                                    const std::string &mapped, const std::string &routed, const std::string &seed = "");

        std::unordered_map<std::string, std::unordered_set<std::string>> updateCompaTable(const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat,
                    const std::unordered_map<std::string, std::unordered_set<std::string>> &FUs);

        void deleteForbiddened(std::unordered_map<std::string, std::unordered_set<std::string>> &compat, const std::unordered_multimap<std::string, std::string> &forbid);

        std::vector<std::string> sortDFG(const Graph &dfg, const std::string &sortMode);

        std::vector<std::string> sortDFG(const Graph &dfg, const std::string &sortMode, const std::string &seed);
        inline bool isFileExists(const std::string &name)   { std::ifstream f(name.c_str()); return f.good();}
    }
}

#endif