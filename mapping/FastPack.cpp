#include "./FastPack.h"

using namespace std;

namespace FastCGRA
{

bool FastPack::packBlock(const std::string &dfg, const std::string &compat, const std::string &dfgGlobal,
                    const std::vector<std::string> &subRRGs, const std::vector<size_t> &numberSubRRGs,
                    const std::vector<std::pair<std::size_t, std::size_t>> &portConstraint,
                    const std::string &RRG, const std::string &FUs)
{
    string path = dfg.substr(0, dfg.size() - 3) + "log.txt";
    vector<string> blockfiles;
    vector<string> blocktypes;

    vector<Graph> subRRGGraphs;
    for(const auto &name: subRRGs){
        subRRGGraphs.push_back(Graph(name));
    }
    unordered_map<string, unordered_set<string>> compats = readSets(compat);
    FastPacker packer(Graph(RRG), subRRGGraphs, numberSubRRGs, NetworkAnalyzerLegacy::parse(FUs));
    packer.prepare(Graph(dfg), Graph(dfgGlobal), compats);
    packer.portConstraint(portConstraint);
    clog << "FastPack: Begin Packing. " << endl;
    unordered_map<size_t, vector<Graph>> packs = packer.packTabuAnnealing();

    unordered_set<string> packedVertices;
    for(const auto &type: packs){ 
        string blocktype = num2str<size_t>(type.first);
        for(size_t x = 0; x < type.second.size(); x++){
            string blockfile = dfg.substr(0, dfg.size() - 3) + blocktype + "_" + to_string(x) + ".txt";
            blockfiles.push_back(blockfile);
            blocktypes.push_back(blocktype);
            type.second[x].dump(blockfile);
            for(const auto &vertex: type.second[x].vertices()){
                packedVertices.insert(vertex.first);
            }
        }
    }
    Graph dfgGraph(dfg);
    for(const auto &vertex: dfgGraph.vertices()){
        if(getPostfix(vertex.first).empty()
        &&packedVertices.find(vertex.first) == packedVertices.end()){
            clog << "packBlock: unPacked vertex -> " << vertex.first << " : " << compats[vertex.first] << endl;
        }
    }
    Utils::dumpBlock(path, blockfiles, blocktypes);

    return true;
}


}