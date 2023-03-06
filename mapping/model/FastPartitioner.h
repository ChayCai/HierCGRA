#ifndef __FASTPARTITIONER__
#define __FASTPARTITIONER__

#include "./common/Common.h"
#include "./common/HyperGraph.h"

namespace FastCGRA
{

class FastPartitioner
{
private:
    std::unordered_map<std::string, std::string> _compat;
    std::unordered_map<std::string, double> _weights;
    std::unordered_map<std::string, size_t> _blockConstraint;
    std::unordered_map<std::string, size_t> _fuConstraint;
    std::unordered_map<std::string, size_t> _numConstraint;
    std::pair<size_t, size_t> _portConstraint;

public:
    FastPartitioner() {};
    FastPartitioner(const FastPartitioner &partitioner): _compat(partitioner._compat), _weights(partitioner._weights), _blockConstraint(partitioner._blockConstraint), _fuConstraint(partitioner._fuConstraint), _portConstraint(partitioner._portConstraint) {}
    FastPartitioner(FastPartitioner &&partitioner):  _compat(partitioner._compat), _weights(partitioner._weights), _blockConstraint(partitioner._blockConstraint), _fuConstraint(partitioner._fuConstraint), _portConstraint(partitioner._portConstraint) {}
    
    void prepare(const std::unordered_map<std::string, std::string> compat, const std::unordered_map<std::string, double> &weights,
        const std::unordered_map<std::string, size_t> &blockConstraint, const std::unordered_map<std::string, size_t> &fuConstraint, const std::pair<size_t, size_t> &portConstraint);
    size_t distance(std::vector<std::unordered_set<std::string>> P0, std::vector<std::unordered_set<std::string>> P1);
    std::unordered_map<std::string, int> score(const std::vector<std::unordered_set<std::string>> &P, const Graph &graphGlobal);
    bool better(std::unordered_map<std::string, int> score1, std::unordered_map<std::string, int> score2);

    std::vector<std::vector<std::unordered_set<std::string>>> initialPoP(const Graph &graphInit, const size_t &partNum, const size_t &popNum, const size_t &randNum = 0);
    std::vector<std::vector<std::unordered_set<std::string>>> geneOffspring(std::vector<std::vector<std::unordered_set<std::string>>> parents, size_t OffNum);

    std::vector<std::unordered_set<std::string>> spectralClustering(const Graph &graphInit, const size_t &partNum);
    std::vector<std::unordered_set<std::string>> memeticPartition(const Graph &graphInit, const Graph &graphGlobal, const size_t &partNum);
};

}

#endif