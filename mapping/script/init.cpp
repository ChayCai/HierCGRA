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
    string fus = argv[1];
    string rrg = argv[2];
    string rrgpath = argv[3];
    string linkpath = argv[4];

    NetworkAnalyzerLegacy analyzerGlobal(fus, Graph(rrg));
    analyzerGlobal.dumpAnalysis(rrgpath, linkpath);


    return 0;
}