#ifndef __FASTROUTER__
#define __FASTROUTER__

#include "./common/Common.h"
#include "./common/HyperGraph.h"

namespace FastCGRA
{

class FastRouter
{
private:
    Graph &_RRG;
    std::unordered_map<std::string, std::unordered_set<std::string>> _FUs;

    std::unordered_multimap<std::string, std::pair<std::string, std::string>>                  _usedNode; 
    std::unordered_map<std::string, std::string>                                               _usedSignal; 
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> _paths; 

    std::unordered_set<std::string>                                                            _forbiddened;
    std::unordered_map<std::string, std::vector<std::vector<std::string>>>                     _cache; 

public:
    FastRouter() = delete;
    FastRouter(Graph &rrg): _RRG(rrg){}
    FastRouter(Graph &rrg, const std::unordered_map<std::string, std::unordered_set<std::string>> &FUs): _RRG(rrg), _FUs(FUs){}
    FastRouter(const FastRouter &router): _RRG(router._RRG), _FUs(router._FUs), _usedNode(router._usedNode), _usedSignal(router._usedSignal), _paths(router._paths), _forbiddened(router._forbiddened), _cache(router._cache){}
    FastRouter(FastRouter &&router): _RRG(router._RRG), _FUs(router._FUs), _usedNode(std::move(router._usedNode)), _usedSignal(std::move(router._usedSignal)), _paths(std::move(router._paths)), _forbiddened(std::move(router._forbiddened)), _cache(router._cache) {}

    const FastRouter &operator = (const FastRouter &router) {_FUs = router._FUs; _usedNode = router._usedNode; _usedSignal = router._usedSignal; _paths = router._paths; _forbiddened = router._forbiddened; _cache = router._cache; return *this; } 
    const FastRouter &operator = (FastRouter &&router) {_FUs = std::move(router._FUs); _usedNode = std::move(router._usedNode); _usedSignal = std::move(router._usedSignal); _paths = std::move(router._paths); _forbiddened = router._forbiddened; _cache = router._cache; return *this; } 

    void forbid(const std::unordered_set<std::string> &forbiddend){_forbiddened = forbiddend; _cache.clear();};
    std::unordered_multimap<std::string, std::pair<std::string, std::string>>       &usedNode()       {return _usedNode; }
    const std::unordered_multimap<std::string, std::pair<std::string, std::string>> &usedNode() const {return _usedNode; }
    std::unordered_map<std::string, std::string>       &usedSignal()       {return _usedSignal; }
    const std::unordered_map<std::string, std::string> &usedSignal() const {return _usedSignal; }
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>       &paths()       {return _paths; }
    const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &paths() const {return _paths; }

    void clear() {_usedNode.clear(); _usedSignal.clear(); _paths.clear(); }
    
    bool route(const std::vector<std::pair<std::string, std::string>> &edgesToMap, const std::vector<std::string> &edgesSignal, 
                const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &pathsGiven = {});
    bool strict_route(const std::vector<std::pair<std::string, std::string>> &edgesToMap, const std::vector<std::string> &edgesSignal, 
                const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &pathsGiven = {});
    bool unroute(const std::vector<std::pair<std::string, std::string>> &edgesToMap); 

    static size_t CacheCapacity; 
};

}

#endif