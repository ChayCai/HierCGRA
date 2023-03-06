#ifndef __FASTPARTITION__
#define __FASTPARTITION__

#include "./common/Common.h"
#include "./common/HyperGraph.h"
#include "./util/Utils.h"
#include "./model/FastPartitioner.h"

namespace FastCGRA
{
    namespace FastPartition
    {
        bool Partition(const std::string &dfgName, const std::string &compatName, const std::string &blockFileName,
                    const std::vector<std::string> &block2Type, const std::unordered_map<std::string, size_t> &block2Num, const std::unordered_map<std::string, size_t> &fu2Num, 
                    std::pair<size_t, size_t> &portNum, const std::unordered_map<std::string, double> &weights, const size_t &partNum);
    }
}

#endif
