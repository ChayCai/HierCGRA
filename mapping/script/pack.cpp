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

using namespace std; 
using namespace FastCGRA; 

int main(int argc, char **argv){

    srand(time(nullptr));

    assert(argc >= 3);
    string dfg        = string(argv[1]); 
    string globalDfg  = string(argv[2]); 
    string compat     = string(argv[3]); 
    string archmsg    = string(argv[4]);

    unordered_map<string, unordered_map<string, vector<string>>> config = Utils::readConifgFile(archmsg);
    vector<string> subRRGs = config["Pack"]["BlockRRGs"];
    vector<string> subFUs = config["Pack"]["BlockFUs"];
    vector<size_t> numberSubRRGs;
    string RRG = config["Global"]["TopRRG"][0];
    string FUs = config["Global"]["TopFUs"][0];
    size_t CoreNums = str2num<size_t>(config["Global"]["CoreNums"][0]);
    string TopRRGAnalyzed = config["Global"]["TopRRGAnalyzed"][0];
    string TopLinksAnalyzed = config["Global"]["TopLinksAnalyzed"][0];
    vector<pair<size_t, size_t>> portConstraint;
    vector<size_t> inportConstraint;
    if(config["Pack"].find("MaxInport") != config["Pack"].end()){
        for(size_t x = 0; x < config["Pack"]["MaxInport"].size(); x++){
            inportConstraint.push_back(str2num<size_t>(config["Pack"]["MaxInport"][x]));
        }
    }
    if(config["Pack"].find("MaxOutport") != config["Pack"].end()){
        for(size_t x = 0; x < config["Pack"]["MaxOutport"].size(); x++){
            if(inportConstraint.empty()){
                portConstraint.push_back({0, str2num<size_t>(config["Pack"]["MaxOutport"][x])});
            } else {
                portConstraint.push_back({inportConstraint[x], str2num<size_t>(config["Pack"]["MaxOutport"][x])});
            }
        }
    }
    

    for(const auto &numberstr: config["Pack"]["BlockNUMs"]){
        size_t number = str2num<size_t>(numberstr);
        numberSubRRGs.push_back(number * CoreNums);
    }
    ASSERT(subRRGs.size() == subFUs.size() && subRRGs.size() == numberSubRRGs.size(), "pack ERROR: Initial fail with \'arch.ini\'. FU & RRG & Number unmatch.");

    NetworkAnalyzerLegacy::setDefault(TopRRGAnalyzed, TopLinksAnalyzed);
    FastPack::packBlock(dfg, compat, globalDfg, subRRGs, numberSubRRGs, portConstraint, RRG, FUs);

    string path = dfg.substr(0, dfg.size() - 3) + "log.txt";
    unordered_map<string, string> packedBlocks = Utils::parseBlock(path);
    bool success = false;
    for(const auto &block: packedBlocks)
    {
        cout << "pack: placing " << block.first << endl;
        size_t type = str2num<size_t>(block.second);
        string subDfg = block.first;      
        string mapped = block.first.substr(0, block.first.size() - 3) + "placed.txt";
        string routed = block.first.substr(0, block.first.size() - 3) + "routed.txt";
        for(size_t count = 0; count < 8; count++){
            success = FastPlace::PlaceBlock(subDfg, compat, globalDfg, subRRGs[type], subFUs[type], mapped, routed);
            if(success){
                break;
            }
        }
        ASSERT(success, "pack: placeBlock " + block.first + " failed.");
    }

    return 0;
}