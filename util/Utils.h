#ifndef __UTILS__
#define __UTILS__

#include "../common/Common.h"
#include "../common/HyperGraph.h"
#include "../util/NetworkAnalyzer.h"
#include "../util/NOrderValidator.h"
#include "../util/Validator.h"

namespace FastCGRA
{
    namespace Utils
    {
        std::unordered_map<std::string, std::string> readMap(const std::string &filename); 
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> readPaths(const std::string &filename); 
        bool writeMap(const std::unordered_map<std::string, std::string> &dict, const std::string &filename); 
        bool writePaths(const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &paths, const std::string &filename);
        std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> readConifgFile(const std::string &fileName);
        bool validate(const std::string &dfg, const std::string &rrg, const std::string &compat, 
                      const std::string &mapped, const std::string &routed); 
        bool validateWithoutIO(const std::string &dfg, const std::string &rrg, const std::string &compat, 
                      const std::string &mapped, const std::string &routed);  
        bool dumpBlock(const std::string &path, const std::vector<std::string> &blockfiles, const std::vector<std::string> &blocktypes);
        std::unordered_map<std::string,std::string> parseBlock(const std::string &path);
        Graph insertPortBlock(const Graph &origin);
        Graph insertPortFU(const Graph &origin);
        Graph genMRRG(Graph rrg, std::unordered_map<std::string, std::unordered_set<std::string>> FUs, size_t ii);
    }
}

#endif
