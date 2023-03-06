#include "./common/Common.h"
#include "./common/Logger.h"
#include "./common/HyperGraph.h"
#include "./common/HierGraph.h"
#include "./util/NetworkAnalyzer.h"
#include "./util/GraphSort.h"
#include "./util/Utils.h"
#include "./mapping/model/FastPacker.h"
#include "./mapping/model/FastRouter.h"
#include "./mapping/model/FastPlacer.h"
#include "./mapping/FastPack.h"
#include "./mapping/FastPlace.h"

#include "./util/tool/genrtl/GenVerilog.h"

using namespace std; 
using namespace FastCGRA; 

int main(int argc, char **argv){

    string way        = string(argv[1]);
    string path       = string(argv[2]);
    srand(time(nullptr));
    bool result = false;

    // if(way == "test"){
    //     GenVerilog::Core core("./arch/arch6/Core.json", "./arch/arch6/CoreLib.ini");
    //     core.core2RTL(path);

    //     return 0;
    // }

    if(way == "placeCore"){
        assert(argc >= 5);
        string dfg        = string(argv[2]); 
        string globalDfg  = string(argv[3]); 
        string compat     = string(argv[4]);
        string archmsg     = string(argv[5]);
        string sortMode   = "STB";
        string seed       = "";
        if(argc >= 7){
            sortMode = string(argv[6]);
        } else if(argc >= 8){
            seed = string(argv[7]);
        } else if (argc >= 8) {
            // srand((unsigned)stoi(string(argv[7])));
        } else {
            // srand(time(nullptr));
        }

        unordered_map<string, unordered_map<string, vector<string>>> config = Utils::readConifgFile(archmsg);

        string rrg = config["Global"]["CoreRRG"][0];
        string fus = config["Global"]["CoreFUs"][0];
        // string rrg = "./tmp/CoreRRG.txt";
        // string fus = "./tmp/CoreFUs.txt";
        vector<string> mappedPack;
        vector<string> routedPack; 
        string mapped = dfg.substr(0, dfg.size() - 3) + "placed.txt";
        string routed = dfg.substr(0, dfg.size() - 3) + "routed.txt";

        string path = dfg.substr(0, dfg.size() - 10) + ".log.txt";
        unordered_map<string, string> blocks = Utils::parseBlock(path);
        // for(const auto &block: blocks){
        //     mappedPack.push_back(block.first.substr(0, block.first.size() - 3) + "placed.txt");
        //     routedPack.push_back(block.first.substr(0, block.first.size() - 3) + "routed.txt");
        // }
        
        // result = FastPlace::PlaceCore(dfg, compat, globalDfg, rrg, fus, 
        //                         mappedPack, routedPack, 
        //                         mapped, routed, sortMode, {}, seed);
        vector<string> blocktypes;
        unordered_map<string, vector<string>> type2block;
        for(const auto &type: config["Place"]["BlockType"]){
            blocktypes.push_back(type);
            type2block[type] = config["Place"][type];
        }

        
        // Graph test;
        // unordered_map<string, unordered_set<string>> pack2vertex;
        // unordered_map<string, string> vertex2pack;
        // size_t count = 0;
        // for(const auto &pack: blocks){
        //     string packName = "Pack"+pack.second+"_"+to_string(count++);
        //     pack2vertex[packName] = unordered_set<string>();
        //     Graph packGraph(pack.first);
        //     for(const auto &vertex: packGraph.vertices()){
        //         pack2vertex[packName].insert(vertex.first);
        //         vertex2pack[vertex.first] = packName;
        //     }
        // }
        // Graph dfgGraph(dfg);
        // clog << vertex2pack << endl;
        // unordered_set<string> packss;
        // for(const auto &vertex: dfgGraph.vertices()){
        //     clog << vertex.first << endl;
        //     if(vertex2pack.find(vertex.first) == vertex2pack.end()){
        //         clog << "11111" << endl;
        //         test.addVertex(Vertex(vertex.first));
        //     } else {
        //         packss.insert(vertex2pack[vertex.first]);
        //     }
        // }
        // for(const auto &vertex: packss){
        //     test.addVertex(Vertex(vertex));
        // }
        // for(const auto &vertex: dfgGraph.vertices()){
        //     for(const auto &edge: dfgGraph.edgesOut(vertex.first)){
        //         bool inPack1 = vertex2pack.find(edge.from()) != vertex2pack.end();
        //         bool inPack2 = vertex2pack.find(edge.to()) != vertex2pack.end();
        //         if(inPack1 && !inPack2){
        //             test.addEdge(Edge(vertex2pack[edge.from()], edge.to()));
        //         }
        //         if(!inPack1 && inPack2){
        //             test.addEdge(Edge(edge.from(), vertex2pack[edge.to()]));
        //         }
        //         if(!inPack1 && !inPack2){
        //             test.addEdge(edge);
        //         }
        //         if(inPack1 && inPack2){
        //             if(vertex2pack[edge.from()] == vertex2pack[edge.to()]){
        //                 continue;
        //             }
        //             test.addEdge(Edge(vertex2pack[edge.from()], vertex2pack[edge.to()]));
        //         }
        //     }
        // }
        // test.dump("graph.txt");
        // return 0;


        result = FastPlace::PlaceCore(dfg, compat, globalDfg, rrg, fus, 
                                blocks, mapped, routed, blocktypes, type2block , sortMode, {}, seed);

        //Utils::validate(dfg, rrg, compat, mapped, routed);
    }
    else if (way == "placeCoreII")
    {
        string dfg        = string(argv[2]); 
        string compat     = string(argv[3]);
        size_t ii         = str2num<size_t>(argv[4]);
        string sortMode   = "STB";
        string seed       = "";
        if(argc >= 6){
            sortMode = string(argv[5]);
        } else if(argc >= 7){
            seed = string(argv[6]);
        }
        
        string rrg = "./arch/arch6/Core_RRG.txt";
        string fus = "./arch/arch6/Core_FUs.txt";
        string mapped = dfg.substr(0, dfg.size() - 3) + "placed.txt";
        string routed = dfg.substr(0, dfg.size() - 3) + "routed.txt";

        result = FastPlace::PlaceCoreII(dfg, compat, rrg, fus, mapped, routed, ii, sortMode, {}, seed);

    }
    else if(way == "placeTop")
    {
        string dfg        = string(argv[2]); 
        string compat     = string(argv[3]);
        string archmsg    = string(argv[4]);
        vector<pair<string, string>> core2type;
        // clog << core2type << endl;

        unordered_map<string, unordered_map<string, vector<string>>> config = Utils::readConifgFile(archmsg);
        string TopRRG = config["Global"]["TopRRG"][0];
        string TopFUs = config["Global"]["TopFUs"][0];
        string TopRRGAnalyzed = config["Global"]["TopRRGAnalyzed"][0];
        string TopLinksAnalyzed = config["Global"]["TopLinksAnalyzed"][0];
        vector<string> top = config["Global"]["Cores"];
        NetworkAnalyzerLegacy::setDefault(TopRRGAnalyzed, TopLinksAnalyzed);

        for(size_t idx = 0; idx < top.size(); idx++)
        {
            string corefile = dfg.substr(0, dfg.size() - 4) + "_part" + num2str(idx) + ".txt";
            string coretype = "Core" + num2str(idx);
            if(FastPlace::isFileExists(corefile))
            {
                core2type.push_back(make_pair(corefile, coretype));
            }
        }
        vector<string> vecCoreplaced;
        vector<string> vecCorerouted;
        for(size_t idx = 0; idx < core2type.size(); idx++)
        {
            vecCoreplaced.push_back(core2type[idx].first.substr(0, core2type[idx].first.size() - 4) + ".placed.txt");
            vecCorerouted.push_back(core2type[idx].first.substr(0, core2type[idx].first.size() - 4) + ".routed.txt");
        }

        clog << "placeTop: inFiles:" << endl;
        clog << "\t" << vecCoreplaced << endl << "\t" << vecCorerouted << endl;
        
        string mapped = dfg.substr(0, dfg.size() - 3) + "placed.txt";
        string routed = dfg.substr(0, dfg.size() - 3) + "routed.txt";

        NetworkAnalyzerLegacy::setDefault(TopRRGAnalyzed, TopLinksAnalyzed);
        result = FastPlace::PlaceTop(dfg, compat, TopRRG, TopFUs, top, vecCoreplaced, vecCorerouted,
                                      mapped, routed, "");
        //Utils::validate(dfg, "./arch/Top_RRG.txt", compat, mapped, routed);
    }
    else if(way == "validate")
    {
        string dfg        = string(argv[2]); 
        string compat     = string(argv[3]);

        string TopRRGAnalyzed = "./arch/Top_RRG_Analyzed.txt";
        string TopLinksAnalyzed = "./arch/Top_Links_Analyzed.txt";
        NetworkAnalyzerLegacy::setDefault(TopRRGAnalyzed, TopLinksAnalyzed);

        string mapped = dfg.substr(0, dfg.size() - 3) + "placed.txt";
        string routed = dfg.substr(0, dfg.size() - 3) + "routed.txt";

        result = Utils::validate(dfg, "./arch1/Top_RRG.txt", compat, mapped, routed);

    }
    



    return !result;
}