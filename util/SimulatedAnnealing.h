#ifndef __SIMULATED_ANNEALING__
#define __SIMULATED_ANNEALING__

#include "./common/Common.h"
#include "./common/HyperGraph.h"
#include "./util/Utils.h"
#include "./util/VanillaMatcher.h"

namespace FastCGRA
{

namespace SimulatedAnnealing
{

std::unordered_map<std::string, std::string> run(const std::function<double(const std::unordered_map<std::string, std::string> &vars)> &eval, const std::unordered_map<std::string, std::unordered_set<std::string>> &values, bool unique=false, const std::unordered_map<std::string, std::string> &init={}, size_t iters=1024, double temperature=1.0, double scale=0.99); 

}

}

#endif
