#include "./common/Common.h"
#include "./common/Logger.h"
#include "./common/HyperGraph.h"
#include "./common/HierGraph.h"
#include "./util/NetworkAnalyzer.h"
#include "./util/GraphSort.h"
#include "./util/Utils.h"
#include "./mapping/model/FastPacker.h"
#include "./mapping/model/FastPartitioner.h"
#include "./mapping/model/FastRouter.h"
#include "./mapping/model/FastPlacer.h"
#include "./mapping/FastPack.h"
#include "./mapping/FastPlace.h"
#include "./mapping/FastPartition.h"

using namespace std; 
using namespace FastCGRA; 

int main(int argc, char **argv){
    
    if(string(argv[1]) == "test"){
        string dfg        = string(argv[2]); 
        string compat     = string(argv[3]);
        string archmsg     = string(argv[4]);
        Graph test = Graph(dfg);
        unordered_map<string, unordered_set<string>> compats = readSets(compat);
        unordered_map<string, double> enen;
        for(const auto &vertex: test.vertices()){
            if(getPostfix(vertex.first).empty()){
                for(const auto &t: compats[vertex.first]){
                    enen[t] += 1.0/(compats[vertex.first]).size();
                }
            }
        }

        clog << enen << endl;
        return 0;
    }

    srand(time(nullptr));

    assert(argc >= 3);
    string dfg        = string(argv[1]); 
    string compat     = string(argv[2]);
    string archmsg     = string(argv[3]);

    string blockFileName = dfg.substr(0, dfg.size() - 4) + ".log.txt";
    unordered_map<string, unordered_map<string, vector<string>>> config = Utils::readConifgFile(archmsg);
    vector<string> block2type = config["Partition"]["BlockType"];
    vector<string> blockNum = config["Partition"]["BlockNum"];
    vector<string> fu2type = config["Partition"]["FuType"];
    vector<string> fuNum = config["Partition"]["FuNum"];
    size_t partNum = str2num<size_t>(config["Partition"]["PartNum"][0]);

    std::unordered_map<std::string, size_t> block2Num;
    std::unordered_map<std::string, size_t> fu2Num;

    for(size_t idx = 0; idx < block2type.size(); idx++){
        block2Num[block2type[idx]] = str2num<size_t>(blockNum[idx]);
    }
    for(size_t idx = 0; idx < fu2type.size(); idx++){
        fu2Num[fu2type[idx]] = str2num<size_t>(fuNum[idx]);
    }


    pair<size_t, size_t> portNum = {str2num<size_t>(config["Partition"]["InportNum"][0]), str2num<size_t>(config["Partition"]["OutPortNum"][0])};
    unordered_map<string, double> weights = {{"maxInportNum", 0.4},{"maxOutportNum", 0.4},{"maxUsage", 0.1},{"maxDegree", 0.1}};
    
    FastPartition::Partition(dfg, compat, blockFileName, block2type, block2Num, fu2Num, portNum, weights, partNum);

}