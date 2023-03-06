#include "./common/Common.h"
#include "./common/Logger.h"
#include "./common/HyperGraph.h"
#include "./common/HierGraph.h"
#include "./util/NetworkAnalyzer.h"
#include "./util/GraphSort.h"
#include "./util/Utils.h"
#include "./util/tool/genrtl/GenVerilog.h"

using namespace std; 
using namespace FastCGRA; 


int main(int argc, char **argv){
    string adl       = string(argv[1]);
    string lib       = string(argv[2]);
    string path      = string(argv[3]);

    srand(time(nullptr));

    GenVerilog::Core core(adl, lib);
    core.core2RTL(path);

    return 0;
}