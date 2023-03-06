#include "./FastRouter.h"

using namespace std;

namespace FastCGRA
{

size_t FastRouter::CacheCapacity = 1024 * 4;

bool FastRouter::route(const std::vector<std::pair<std::string, std::string>> &edgesToMap, const std::vector<std::string> &edgesSignal, 
                const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &pathsGiven)
{
    assert(edgesToMap.size() == edgesSignal.size());
    
    std::unordered_multimap<std::string, std::pair<std::string, std::string>>                  usedNodeBackup   = _usedNode; 
    std::unordered_map<std::string, std::string>                                               usedSignalBackup = _usedSignal; 
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> pathsBackup      = _paths; 
    queue<pair<string, string>> queEdges; 
    queue<string>               queSignals;
    pair<string, string>        edge; 
    string                      signal;  
    for(size_t idx = 0; idx < edgesToMap.size(); idx++)
    {
        queEdges.push(edgesToMap[idx]); 
        queSignals.push(edgesSignal[idx]); 
    }

    auto funcPath = [&](const Vertex &vertex){
        const string &name = vertex.name();
        const string &device = vertex.getAttr("device").getStr();
        if(_forbiddened.find(name) != _forbiddened.end()
        ||_FUs.find(device) != _FUs.end()){
            return false;
        }
        return true;
    };

    auto funcPathStrict = [&](const Vertex &vertex){
        const string &name = vertex.name();
        const string &device = vertex.getAttr("device").getStr();
        if(_forbiddened.find(name) != _forbiddened.end()
        ||(_usedSignal.find(name) != _usedSignal.end() && _usedSignal[name] != signal 
        )){
        // ||_FUs.find(device) != _FUs.end()){
            return false;
        }
        return true;
    };

    auto conflicts = [&](const vector<string> &path){
        size_t result = 0;
        for(const auto &node: path){
            if(_usedNode.find(node) != _usedNode.end() && _usedSignal[node] != signal){
                result++;
            }
        }
        return result;
    };

    bool failed = false;
    size_t times = 0; 
    unordered_multiset<string> tried; 
    while(!queEdges.empty())
    {
        edge   = queEdges.front();   queEdges.pop(); 
        signal = queSignals.front(); queSignals.pop(); 
        assert(!signal.empty()); 
        if((times++) >= 32*edgesToMap.size() || tried.count(edge.first + "__->__" + edge.second) > 8) //PARAM
        {
            failed = true;
            clog << "FastRouter: Failed to route: "  << edge.first << "__->__" << edge.second << endl;
            break;
        }
        if(_paths.find(edge.first) != _paths.end() && _paths.find(edge.first)->second.find(edge.second) != _paths.find(edge.first)->second.end())
        {
            continue; 
        }
        clog << "FastRouter: Routing: "  << edge.first << "__->__" << edge.second << endl;
        if(pathsGiven.find(edge.first) != pathsGiven.end()
        &&pathsGiven.find(edge.first)->second.find(edge.second) != pathsGiven.find(edge.first)->second.end()){
            vector<string> path = pathsGiven.find(edge.first)->second.find(edge.second)->second; // Use the Given Path
            if(conflicts(path) == 0){
                clog << "FastRouter: use the Given path. " << endl;
                if(_paths.find(edge.first) == _paths.end())
                {
                    _paths[edge.first] = unordered_map<string, vector<string>>(); 
                }
                _paths[edge.first][edge.second] = path;
                for(const auto &node: path)
                {
                    _usedNode.insert({node, edge}); 
                    _usedSignal[node] = signal;
                }
                clog << "FastRouter: use path:" << path << endl; 
                clog << "FastRouter: Routed." << endl; 
                continue;
            }
        }
        pair<vector<string>, bool> pathConvenient = _RRG.findPath(edge.first, edge.second, funcPathStrict); // Find the Convenient Path
        if(pathConvenient.second)
        {   // Route Convenient
            clog << "FastRouter: Convenient path found. " << endl;
            if(_paths.find(edge.first) == _paths.end())
            {
                _paths[edge.first] = unordered_map<string, vector<string>>(); 
            }
            _paths[edge.first][edge.second] = pathConvenient.first;
            for(const auto &node: pathConvenient.first)
            {
                _usedNode.insert({node, edge}); 
                _usedSignal[node] = signal;
            }
            clog << "FastRouter: use path:" << pathConvenient.first << endl; 
            clog << "FastRouter: Routed." << endl; 
            continue; 
        }

        vector<vector<string>> pathCandidates;
        string pathname = edge.first + "__->__" + edge.second;
        auto iter = _cache.find(pathname);// Find Paths in cache
        if(iter != _cache.end())
        {
            pathCandidates = _cache[pathname];
        }
        else
        {
            pathCandidates = _RRG.findPaths(edge.first, edge.second, funcPath); //Find ALL Paths
            if(CacheCapacity > 0){
                if(_cache.size() >= CacheCapacity){
                    _cache.erase(_cache.begin());
                }
                _cache.insert({pathname, pathCandidates});
            }
        }
        clog << "FastRouter: "  << pathCandidates.size() << " paths found. " << endl;
        if(pathCandidates.empty())
        {
            failed = true;
            break;
        }

        vector<unordered_set<string>> usedNode;
        vector<size_t> pathIndex;
        for (const auto &path : pathCandidates) {
            usedNode.push_back(unordered_set<string>());
            pathIndex.push_back(pathIndex.size());
            for (const auto &vertex : path) {
                if (_usedNode.find(vertex) != _usedNode.end() && _usedSignal[vertex] != signal) {
                    assert(_usedSignal.find(vertex) != _usedSignal.end());
                    usedNode.back().insert(vertex);
                }
            }
        }
        sort(pathIndex.begin(), pathIndex.end(), [&](size_t a, size_t b) {
            if (usedNode[a].size() < usedNode[b].size()) {
                return true;
            } else if (usedNode[a].size() == usedNode[b].size()) {
                return pathCandidates[a].size() < pathCandidates[b].size();
            }
            return false;
        });

        // Delete the used vertices on the selected path
        size_t index = pathIndex[0];
        if (tried.find(edge.first + "__->__" + edge.second) != tried.end()) {
            clog << "VanillaRouter: -> Has been tried, randomly select a path. " << endl;
            index = pathIndex[rand() % pathIndex.size()];
        }
        vector<string>        &path = pathCandidates[index]; 
        unordered_set<string> used = usedNode[index];
        //unordered_set<string> used = unordered_set<string>(path.begin(), path.end());

        //Find the edges to Delete
        unordered_map<string, pair<string, string>> mapLinksToDelete; 
        unordered_map<string, string>               mapSignalsToDelete; 
        queue<pair<string, string>>                 queLinksToDelete; 
        for(const auto &vertex: used)
        { 
            for(auto iter = _usedNode.find(vertex); iter != _usedNode.end(); iter = _usedNode.find(vertex))
            {
                const pair<string, string> &link = iter->second; 
                string linkName = link.first + "__->__" + link.second; 
                if(mapLinksToDelete.find(linkName) == mapLinksToDelete.end())
                {
                    mapLinksToDelete[linkName] = link; 
                    queLinksToDelete.push(link); 
                }
                _usedNode.erase(iter); 
            }
        } 
        //Delete the edges with conflicts and prepare to reroute
        while(!queLinksToDelete.empty())
        {
            pair<string, string> link = queLinksToDelete.front(); queLinksToDelete.pop();  
            assert(_paths.find(link.first) != _paths.end() && _paths[link.first].find(link.second) != _paths[link.first].end()); 
            const vector<string> &tmppath = _paths[link.first][link.second]; 
            string linkName = link.first + "__->__" + link.second; 
            clog << "FastRouter: -> Delete link: " << linkName << endl; 
            mapSignalsToDelete[linkName] = _usedSignal[tmppath[0]]; 
            if(_usedSignal.find(tmppath[0]) == _usedSignal.end())
            {
                clog << "FastRouter: -> ERROR: vertex " << tmppath[0] << " not found on path: " << linkName << endl; 
            }
            assert(_usedSignal.find(tmppath[0]) != _usedSignal.end()); 
            for(const auto &vertex: tmppath)
            {
                if(_usedSignal[vertex] != _usedSignal[tmppath[0]])
                {
                    clog << "FastRouter: -> ERROR: inconsistent signal: " << vertex << ": " << _usedSignal[vertex] << " vs. " << tmppath[0] << ": " << _usedSignal[tmppath[0]] << endl; 
                }
                assert(_usedSignal[vertex] == _usedSignal[tmppath[0]]); 
            }
            for(const auto &vertex: tmppath)
            {
                unordered_multimap<string, pair<string, string>> addBack; 
                for(auto iter = _usedNode.find(vertex); iter != _usedNode.end(); iter = _usedNode.find(vertex))
                {
                    const pair<string, string> &link2 = iter->second; 
                    if(link2.first == link.first && link2.second == link.second)
                    {
                        ; 
                    }
                    else
                    {
                        addBack.insert(*iter); 
                    }
                    _usedNode.erase(iter); 
                }
                for(const auto &link: addBack)
                {
                    _usedNode.insert(link); 
                }
            }
            _paths[link.first].erase(link.second); 
            if(_paths[link.first].empty())
            {
                _paths.erase(link.first); 
            }
        }
        for(const auto &link: mapLinksToDelete)
        {
            clog << "FastRouter: To reroute: " << link.second.first << " -> " << link.second.second << " : " << mapSignalsToDelete[link.first] << endl; 
            queEdges.push(link.second); 
            assert(mapSignalsToDelete.find(link.first) != mapSignalsToDelete.end() && !mapSignalsToDelete[link.first].empty()); 
            queSignals.push(mapSignalsToDelete[link.first]); 
        }

        //route the selected path
        clog << "FastRouter: use path:" << path << endl; 
        clog << "FastRouter: Routed." << endl; 
        if(_paths.find(edge.first) == _paths.end())
        {
            _paths[edge.first] = unordered_map<string, vector<string>>(); 
        }
        _paths[edge.first][edge.second] = path;
        for(const auto &node: path){
            _usedNode.insert({node, edge}); 
            _usedSignal[node] = signal;
        }
        tried.insert(edge.first + "__->__" + edge.second); 
        // clog << queEdges.size() << endl;
        // clog << pathConvenient.first << endl;
        // clog << pathCandidates << endl;
        // clog << pathIndex2Conflicts << endl;
    }

    if(failed)
    {
        _usedNode   = usedNodeBackup; 
        _usedSignal = usedSignalBackup; 
        _paths      = pathsBackup; 
    }
    return !failed; 
}

bool FastRouter::unroute(const vector<pair<string, string>> &edgesToMap)
{
    for(const auto &edge: edgesToMap)
    {
        if(!(_paths.find(edge.first) != _paths.end() && _paths[edge.first].find(edge.second) != _paths[edge.first].end()))
        {
            clog << "FastRouter: -> Path " << edge.first << "->" << edge.second << " is not routed. " << endl; 
            continue; 
        }
        clog << "FastRouter: Unrouting " << edge.first << " -> " << edge.second << endl; 
        const vector<string> &path = _paths[edge.first][edge.second]; 
        for(const auto &vertex: path)
        {
            if(_usedSignal[vertex] != _usedSignal[path[0]])
            {
                clog << "FastRouter: -> ERROR: inconsistent signal: " << vertex << ": " << _usedSignal[vertex] << " vs. " << path[0] << ": " << _usedSignal[path[0]] << endl; 
            }
            assert(_usedSignal[vertex] == _usedSignal[path[0]]); 
        }
        for(const auto &vertex: path)
        {
            unordered_multimap<string, pair<string, string>> addBack; 
            for(auto iter = _usedNode.find(vertex); iter != _usedNode.end(); iter = _usedNode.find(vertex))
            {
                const pair<string, string> &edge2 = iter->second; 
                if(edge2.first == edge.first && edge2.second == edge.second)
                {
                    ; 
                }
                else
                {
                    addBack.insert(*iter); 
                }
                _usedNode.erase(iter); 
            }
            for(const auto &link: addBack)
            {
                _usedNode.insert(link); 
            }
        }
        _paths[edge.first].erase(edge.second); 
        if(_paths[edge.first].empty())
        {
            _paths.erase(edge.first); 
        }
    }

    return true; 
}

bool FastRouter::strict_route(const std::vector<std::pair<std::string, std::string>> &edgesToMap, const std::vector<std::string> &edgesSignal, 
                const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &pathsGiven)
{
    assert(edgesToMap.size() == edgesSignal.size());
    
    std::unordered_multimap<std::string, std::pair<std::string, std::string>>                  usedNodeBackup   = _usedNode; 
    std::unordered_map<std::string, std::string>                                               usedSignalBackup = _usedSignal; 
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> pathsBackup      = _paths; 
    queue<pair<string, string>> queEdges; 
    queue<string>               queSignals;
    pair<string, string>        edge; 
    string                      signal;  
    for(size_t idx = 0; idx < edgesToMap.size(); idx++)
    {
        queEdges.push(edgesToMap[idx]); 
        queSignals.push(edgesSignal[idx]); 
    }

    auto funcPath = [&](const Vertex &vertex){
        const string &name = vertex.name();
        const string &device = vertex.getAttr("device").getStr();
        if(_forbiddened.find(name) != _forbiddened.end()
        ||_FUs.find(device) != _FUs.end()){
            return false;
        }
        return true;
    };

    auto funcPathStrict = [&](const Vertex &vertex){
        const string &name = vertex.name();
        const string &device = vertex.getAttr("device").getStr();
        if(_forbiddened.find(name) != _forbiddened.end()
        ||(_usedSignal.find(name) != _usedSignal.end() && _usedSignal[name] != signal)
        ||_FUs.find(device) != _FUs.end()){
            return false;
        }
        return true;
    };

    auto conflicts = [&](const vector<string> &path){
        size_t result = 0;
        for(const auto &node: path){
            if(_usedNode.find(node) != _usedNode.end() && _usedSignal[node] != signal){
                result++;
            }
        }
        return result;
    };

    bool failed = false;
    size_t times = 0; 
    unordered_multiset<string> tried; 
    while(!queEdges.empty())
    {
        edge   = queEdges.front();   queEdges.pop(); 
        signal = queSignals.front(); queSignals.pop(); 
        assert(!signal.empty()); 
        if((times++) >= 32*edgesToMap.size() || tried.count(edge.first + "__->__" + edge.second) > 8) //PARAM
        {
            failed = true;
            clog << "FastRouter: Failed to route: "  << edge.first << "__->__" << edge.second << endl;
            break;
        }
        if(_paths.find(edge.first) != _paths.end() && _paths.find(edge.first)->second.find(edge.second) != _paths.find(edge.first)->second.end())
        {
            continue; 
        }
        clog << "FastRouter: Routing: "  << edge.first << "__->__" << edge.second << endl;
        if(pathsGiven.find(edge.first) != pathsGiven.end()
        &&pathsGiven.find(edge.first)->second.find(edge.second) != pathsGiven.find(edge.first)->second.end()){
            vector<string> path = pathsGiven.find(edge.first)->second.find(edge.second)->second; // Use the Given Path
            if(conflicts(path) == 0){
                clog << "FastRouter: use the Given path. " << endl;
                if(_paths.find(edge.first) == _paths.end())
                {
                    _paths[edge.first] = unordered_map<string, vector<string>>(); 
                }
                _paths[edge.first][edge.second] = path;
                for(const auto &node: path)
                {
                    _usedNode.insert({node, edge}); 
                    _usedSignal[node] = signal;
                }
                clog << "FastRouter: use path:" << path << endl; 
                clog << "FastRouter: Routed." << endl; 
                continue;
            }
        }
        pair<vector<string>, bool> pathConvenient = _RRG.findPath(edge.first, edge.second, funcPathStrict); // Find the Convenient Path
        if(pathConvenient.second)
        {   // Route Convenient
            clog << "FastRouter: Convenient path found. " << endl;
            if(_paths.find(edge.first) == _paths.end())
            {
                _paths[edge.first] = unordered_map<string, vector<string>>(); 
            }
            _paths[edge.first][edge.second] = pathConvenient.first;
            for(const auto &node: pathConvenient.first)
            {
                _usedNode.insert({node, edge}); 
                _usedSignal[node] = signal;
            }
            clog << "FastRouter: use path:" << pathConvenient.first << endl; 
            clog << "FastRouter: Routed." << endl; 
            continue; 
        }
        vector<vector<string>> pathCandidates;
        string pathname = edge.first + "__->__" + edge.second;
        auto iter = _cache.find(pathname);// Find Paths in cache
        if(iter != _cache.end())
        {
            pathCandidates = _cache[pathname];
        }
        else
        {
            pathCandidates = _RRG.findPaths(edge.first, edge.second, funcPath); //Find ALL Paths
            if(CacheCapacity > 0){
                if(_cache.size() >= CacheCapacity){
                    _cache.erase(_cache.begin());
                }
                _cache.insert({pathname, pathCandidates});
            }
        }
        clog << "FastRouter: "  << pathCandidates.size() << " paths found. " << endl;
        if(pathCandidates.empty())
        {
            failed = true;
            break;
        }

        vector<unordered_set<string>> usedNode;
        vector<size_t> pathIndex;
        for (const auto &path : pathCandidates) {
            usedNode.push_back(unordered_set<string>());
            pathIndex.push_back(pathIndex.size());
            for (const auto &vertex : path) {
                if (_usedNode.find(vertex) != _usedNode.end() && _usedSignal[vertex] != signal) {
                    assert(_usedSignal.find(vertex) != _usedSignal.end());
                    usedNode.back().insert(vertex);
                }
            }
        }
        sort(pathIndex.begin(), pathIndex.end(), [&](size_t a, size_t b) {
            if (usedNode[a].size() < usedNode[b].size()) {
                return true;
            } else if (usedNode[a].size() == usedNode[b].size()) {
                return pathCandidates[a].size() < pathCandidates[b].size();
            }
            return false;
        });

        // Delete the used vertices on the selected path
        size_t index = pathIndex[0];
        if (tried.find(edge.first + "__->__" + edge.second) != tried.end()) {
            clog << "VanillaRouter: -> Has been tried, randomly select a path. " << endl;
            index = pathIndex[rand() % pathIndex.size()];
        }
        vector<string>        &path = pathCandidates[index]; 
        unordered_set<string> used = usedNode[index];
        //unordered_set<string> used = unordered_set<string>(path.begin(), path.end());

        //Find the edges to Delete
        unordered_map<string, pair<string, string>> mapLinksToDelete; 
        unordered_map<string, string>               mapSignalsToDelete; 
        queue<pair<string, string>>                 queLinksToDelete; 
        for(const auto &vertex: used)
        { 
            for(auto iter = _usedNode.find(vertex); iter != _usedNode.end(); iter = _usedNode.find(vertex))
            {
                const pair<string, string> &link = iter->second; 
                string linkName = link.first + "__->__" + link.second; 
                if(mapLinksToDelete.find(linkName) == mapLinksToDelete.end())
                {
                    mapLinksToDelete[linkName] = link; 
                    queLinksToDelete.push(link); 
                }
                _usedNode.erase(iter); 
            }
        } 
        //Delete the edges with conflicts and prepare to reroute
        while(!queLinksToDelete.empty())
        {
            pair<string, string> link = queLinksToDelete.front(); queLinksToDelete.pop();  
            assert(_paths.find(link.first) != _paths.end() && _paths[link.first].find(link.second) != _paths[link.first].end()); 
            const vector<string> &tmppath = _paths[link.first][link.second]; 
            string linkName = link.first + "__->__" + link.second; 
            clog << "FastRouter: -> Delete link: " << linkName << endl; 
            mapSignalsToDelete[linkName] = _usedSignal[tmppath[0]]; 
            if(_usedSignal.find(tmppath[0]) == _usedSignal.end())
            {
                clog << "FastRouter: -> ERROR: vertex " << tmppath[0] << " not found on path: " << linkName << endl; 
            }
            assert(_usedSignal.find(tmppath[0]) != _usedSignal.end()); 
            for(const auto &vertex: tmppath)
            {
                if(_usedSignal[vertex] != _usedSignal[tmppath[0]])
                {
                    clog << "FastRouter: -> ERROR: inconsistent signal: " << vertex << ": " << _usedSignal[vertex] << " vs. " << tmppath[0] << ": " << _usedSignal[tmppath[0]] << endl; 
                }
                assert(_usedSignal[vertex] == _usedSignal[tmppath[0]]); 
            }
            for(const auto &vertex: tmppath)
            {
                unordered_multimap<string, pair<string, string>> addBack; 
                for(auto iter = _usedNode.find(vertex); iter != _usedNode.end(); iter = _usedNode.find(vertex))
                {
                    const pair<string, string> &link2 = iter->second; 
                    if(link2.first == link.first && link2.second == link.second)
                    {
                        ; 
                    }
                    else
                    {
                        addBack.insert(*iter); 
                    }
                    _usedNode.erase(iter); 
                }
                for(const auto &link: addBack)
                {
                    _usedNode.insert(link); 
                }
            }
            _paths[link.first].erase(link.second); 
            if(_paths[link.first].empty())
            {
                _paths.erase(link.first); 
            }
        }
        for(const auto &link: mapLinksToDelete)
        {
            clog << "FastRouter: To reroute: " << link.second.first << " -> " << link.second.second << " : " << mapSignalsToDelete[link.first] << endl; 
            queEdges.push(link.second); 
            assert(mapSignalsToDelete.find(link.first) != mapSignalsToDelete.end() && !mapSignalsToDelete[link.first].empty()); 
            queSignals.push(mapSignalsToDelete[link.first]); 
        }

        //route the selected path
        clog << "FastRouter: use path:" << path << endl; 
        clog << "FastRouter: Routed." << endl; 
        if(_paths.find(edge.first) == _paths.end())
        {
            _paths[edge.first] = unordered_map<string, vector<string>>(); 
        }
        _paths[edge.first][edge.second] = path;
        for(const auto &node: path){
            _usedNode.insert({node, edge}); 
            _usedSignal[node] = signal;
        }
        tried.insert(edge.first + "__->__" + edge.second); 
        // clog << queEdges.size() << endl;
        // clog << pathConvenient.first << endl;
        // clog << pathCandidates << endl;
        // clog << pathIndex2Conflicts << endl;
    }

    if(failed)
    {
        _usedNode   = usedNodeBackup; 
        _usedSignal = usedSignalBackup; 
        _paths      = pathsBackup; 
    }
    return !failed; 
}


}
