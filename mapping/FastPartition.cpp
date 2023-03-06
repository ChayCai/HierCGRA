
#include "./FastPartition.h"

using namespace std; 

namespace FastCGRA
{
    namespace FastPartition
    {
        bool Partition(const std::string &dfgName, const std::string &compatName, const std::string &blockFileName,
                    const std::vector<std::string> &block2Type, const std::unordered_map<std::string, size_t> &block2Num, const std::unordered_map<std::string, size_t> &fu2Num, 
                    std::pair<size_t, size_t> &portNum, const std::unordered_map<std::string, double> &weights, const size_t &partNum)
        {
            Graph dfgGraph(dfgName);
            unordered_map<string, unordered_set<string>> compatFUs = readSets(compatName);
            ASSERT(!compatFUs.empty(), "compat empty");
            unordered_map<string, unordered_set<string>> coarse2vertex;
            unordered_map<string, string> vertex2caorse;
            unordered_map<string, unordered_set<string>> type2coarse;
            unordered_map<string, string> coarse2type;

            unordered_map<string, string> blockFiles = Utils::parseBlock(blockFileName);
            for(const auto &block: blockFiles){
                string typeName = block2Type[str2num<size_t>(block.second)];
                if(type2coarse.find(typeName) == type2coarse.end()){
                    type2coarse[typeName] == unordered_set<string>();
                }
                string coarseName = typeName + "_" + to_string(type2coarse[typeName].size());
                coarse2vertex[coarseName] = unordered_set<string>();
                type2coarse[typeName].insert(coarseName);
                coarse2type[coarseName] = typeName;
                Graph subGraph(block.first);
                for(const auto &vertex: subGraph.vertices()){
                    coarse2vertex[coarseName].insert(vertex.first);
                    vertex2caorse[vertex.first] = coarseName;
                }
            }
            for(const auto &vertex: dfgGraph.vertices()){
                if(vertex2caorse.find(vertex.first) != vertex2caorse.end()){
                    continue;
                }
                string coarseName = getPrefix(vertex.first);
                if(coarse2vertex.find(coarseName) == coarse2vertex.end()){
                    coarse2vertex[coarseName] = unordered_set<string>();
                }
                coarse2vertex[coarseName].insert(vertex.first);
                vertex2caorse[vertex.first] = coarseName;
            }
            for(const auto &coarse: coarse2vertex){
                if(coarse2type.find(coarse.first) != coarse2type.end()){
                    continue;
                }
                string typeName = *compatFUs[coarse.first].begin();
                if(type2coarse.find(typeName) == type2coarse.end()){
                    type2coarse[typeName] == unordered_set<string>();
                }
                coarse2type[coarse.first] = typeName;
                type2coarse[typeName].insert(coarse.first);
            }

            Graph graphInit;
            Graph graphGlobal;
            for(const auto &vertex: coarse2type){
                graphGlobal.addVertex(Vertex(vertex.first));
            }
            for(const auto &vertex: dfgGraph.vertices()){
                for(const auto &edge: dfgGraph.edgesOut(vertex.first)){
                    string coarseFrom = vertex2caorse[edge.from()];
                    string coarseTo = vertex2caorse[edge.to()];
                    if(coarseFrom != coarseTo){
                        Edge edge1(coarseFrom, coarseTo);
                        edge1.setAttr("from", edge.from());
                        edge1.setAttr("to", edge.to());
                        graphGlobal.addEdge(edge1);
                    }
                }
            }
            for(const auto &vertex: graphGlobal.vertices()){
                string type = coarse2type[vertex.first];
                if(block2Num.find(type) != block2Num.end()
                ||fu2Num.find(type) != fu2Num.end()){
                    graphInit.addVertex(vertex.second);
                }
            }
            for(const auto &vertex: graphInit.vertices()){
                for(const auto &edge: graphGlobal.edgesOut(vertex.first)){
                    if(graphInit.vertices().find(edge.to()) != graphInit.vertices().end()){
                        graphInit.addEdge(edge);
                    }
                }
            }
    
            FastPartitioner partitioner;
            partitioner.prepare(coarse2type, weights, block2Num, fu2Num, portNum);
            vector<unordered_set<string>> result = partitioner.memeticPartition(graphInit, graphGlobal, partNum);
            vector<Graph> subGraphs;
            unordered_set<string> clusteredVertices;
            for(size_t idx = 0; idx < result.size(); idx++){
                Graph subGraph;
                for(const auto &coarse: result[idx]){
                    for(const auto &vertex: coarse2vertex[coarse]){
                        subGraph.addVertex(dfgGraph.vertex(vertex));
                    }
                }
                for(const auto &vertex: subGraph.vertices()){
                    for(const auto &edge: dfgGraph.edgesOut(vertex.first)){
                        if(subGraph.vertices().find(edge.to()) != subGraph.vertices().end()){
                            subGraph.addEdge(edge);
                        }
                    }
                }
                subGraphs.push_back(subGraph);
            }
            for(size_t idx = 0; idx < result.size(); idx++){
                string partName = dfgName.substr(0, dfgName.size() - 4) + "_part" + to_string(idx) + ".txt";
                subGraphs[idx].dump(partName);
                for(const auto &vertex: subGraphs[idx].vertices()){
                    clusteredVertices.insert(vertex.first);
                }
            }
            for(const auto &vertex: dfgGraph.vertices()){
                if(getPostfix(vertex.first).empty()
                &&clusteredVertices.find(vertex.first) == clusteredVertices.end()){
                    clog << "partition: unClustered vertex -> " << vertex.first << " : " << compatFUs[vertex.first] << endl;
                }
            }


            return true;
        }
    }
}