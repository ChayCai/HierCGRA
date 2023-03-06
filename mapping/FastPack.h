#ifndef __FASTPACK__
#define __FASTPACK__


#include "./common/Common.h"
#include "./common/HyperGraph.h"
#include "./util/NOrderValidator.h"
#include "./util/NetworkAnalyzer.h"
#include "./util/GraphSort.h"
#include "./util/Utils.h"
#include "./model/FastPacker.h"
#include "./model/FastPlacer.h"
#include "./model/FastRouter.h"


namespace FastCGRA
{
    namespace FastPack
    {
        bool packBlock(const std::string &dfg, const std::string &compat, const std::string &dfgGlobal,
                    const std::vector<std::string> &subRRGs, const std::vector<size_t> &numberSubRRGs,
                    const std::vector<std::pair<std::size_t, std::size_t>> &portConstraint,
                    const std::string &RRG, const std::string &FUs);
    }
}


#endif