#include "NetworkAnalyzer.h"

using namespace std; 

namespace FastCGRA
{
unordered_map<string, unordered_set<string>> NetworkAnalyzer::parse(const string &filename)
// FU type(device) -> FU ports
{
    clog << endl << "BEGIN: NetworkAnalyzer: Reading FUs infomation from " << filename << endl; 

    unordered_map<string, unordered_set<string>> fus; 
    ifstream fin(filename); 
    size_t countFUs     = 0; 
    size_t countInputs  = 0; 
    size_t countOutputs = 0; 
    while(!fin.eof())
    {
        string line; 
        getline(fin, line); 
        if(line.empty())
        {
            continue; 
        }
        istringstream sin(line); 
        string tmp; 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        tmp.clear(); 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        tmp.clear(); 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        fus[tmp] = unordered_set<string>(); 
        countFUs++; 

        string tmp2; 
        sin >> tmp2; 
        if(tmp2.empty())
        {
            continue; 
        }
        assert(tmp2 == "inputs"); 
        while(!tmp2.empty())
        {
            tmp2.clear(); 
            sin >> tmp2; 
            if(tmp2.empty() || tmp2 == "outputs")
            {
                break; 
            }
            fus[tmp].insert(tmp2); 
            countInputs++; 
        }
        while(!tmp2.empty())
        {
            tmp2.clear(); 
            sin >> tmp2; 
            if(tmp2.empty())
            {
                break; 
            }
            fus[tmp].insert(tmp2); 
            countOutputs++; 
        }
    }
    fin.close(); 

    clog << "NetworkAnalyzer: Found FUs:"     << countFUs << endl; 
    clog << "NetworkAnalyzer: Found Inputs:"  << countInputs << endl; 
    clog << "NetworkAnalyzer: Found Outputs:" << countOutputs << endl; 
    clog << "NetworkAnalyzer: Found Ports:"   << (countInputs + countOutputs) << endl; 

    clog << "END: NetworkAnalyzer: Reading FUs infomation. " << endl << endl; 

    return fus; 
}

bool NetworkAnalyzer::analyze(const Graph &rrg)
{
    clog << endl << "BEGIN: NetworkAnalyzer: Analyzing networks. " << endl; 

    _RRG = rrg; 
    if(_fus.empty())
    {
        genFUs(_RRG); 
    }

    Graph contractedGraph; 
    unordered_map<string, unordered_map<string, vector<string>>> connectionsTable; 
    unordered_map<string, vector<string>>                        link2path; 

    unordered_map<string, string>                fu2type; 
    unordered_map<string, string>                port2fu; 
    unordered_map<string, string>                input2fu; 
    unordered_map<string, string>                output2fu; 
    unordered_map<string, unordered_set<string>> fuInputs; 
    unordered_map<string, unordered_set<string>> fuOutputs; 
    // Find FUs
    for(const auto &vertex: rrg.vertices())
    {
        const string &name = vertex.first;  
        const string &type = vertex.second.getAttr("device").getStr(); 
        if(_fus.find(type) != _fus.end())
        {
            fu2type[name]   = type; 
            fuInputs[name]  = unordered_set<string>(); 
            fuOutputs[name] = unordered_set<string>(); 
        }
    }
    // Find FUs inputs and outputs
    for(const auto &vertex: rrg.vertices())
    {
        const string &name = vertex.first;  
        const string &type = vertex.second.getAttr("device").getStr(); 
        // Get the prefix
        size_t posDot = type.find("."); 
        if(posDot == string::npos)
        {
            continue; 
        }
        string fuType = type.substr(0, posDot); 
        if(_fus.find(fuType) == _fus.end())
        {
            continue; 
        }
        // Is input port or output port?
        bool isInput = true; 
        for(const auto &edge: rrg.edgesIn(name))
        {
            auto iter = fu2type.find(edge.from()); 
            if(iter != fu2type.end())
            {
                assert(iter->second == fuType); 
                isInput = false; 
            }
        }
        // if(vertex.second.getAttr("type").getStr() == "__TOP_INPUT_PORT__")
        // {
        //     isInput = false; 
        // }
        // else if(vertex.second.getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
        // {
        //     isInput = true; 
        // }
        if(isInput)
        {
            // Make sure that every input port only connect to one FU
            assert(rrg.edgesOut(name).size() <= 1); 
            if(rrg.edgesOut(name).size() > 0)
            {
                for(const auto &edge: rrg.edgesOut(name)) 
                {
                    auto iter = fu2type.find(edge.to()); 
                    if(iter == fu2type.end() || iter->second != fuType)
                    {
                        if(iter == fu2type.end())
                        {
                            clog << "NetworkAnalyzer: ERROR: " << edge.to() << " not found. " << endl; 
                        }
                        else if(iter->second != fuType)
                        {
                            clog << "NetworkAnalyzer: ERROR: " << iter->second << " does not match " << fuType << endl; 
                        }
                    }
                    assert(iter != fu2type.end() && iter->second == fuType); 
                }
                string fuName = rrg.edgesOut(name)[0].to(); 
                port2fu[name] = fuName; 
                input2fu[name] = fuName; 
                fuInputs[fuName].insert(name); 
                if(connectionsTable.find(name) == connectionsTable.end())
                {
                    connectionsTable[name] = unordered_map<string, vector<string>>(); 
                }
                connectionsTable[name][fuName] = vector<string>(); 
                string nameLink = name + "->" + fuName + "->" + num2str(0); 
                connectionsTable[name][fuName].push_back(nameLink); 
                link2path[nameLink] = vector<string>(); 
            }
            else
            {
                clog << "NetworkAnalyzer: Warning: input port without FU: " << name << endl; 
                string fuName = "__GLOBAL__"; 
                if(fu2type.find(fuName) == fu2type.end())
                {
                    fu2type[fuName]   = "__GLOBAL__"; 
                    fuInputs[fuName]  = unordered_set<string>(); 
                    fuOutputs[fuName] = unordered_set<string>(); 
                }
                port2fu[name] = fuName; 
                input2fu[name] = fuName; 
                fuInputs[fuName].insert(name); 
            }
        }
        else
        {
            // Make sure that every output port only be connected from one FU
            assert(rrg.edgesIn(name).size() <= 1); 
            if(rrg.edgesIn(name).size() > 0)
            {
                for(const auto &edge: rrg.edgesIn(name))
                {
                    auto iter = fu2type.find(edge.from()); 
                    assert(iter != fu2type.end() && iter->second == fuType); 
                }
                string fuName = rrg.edgesIn(name)[0].from(); 
                port2fu[name] = fuName; 
                output2fu[name] = fuName; 
                fuOutputs[fuName].insert(name); 
                if(connectionsTable.find(fuName) == connectionsTable.end())
                {
                    connectionsTable[fuName] = unordered_map<string, vector<string>>(); 
                }
                connectionsTable[fuName][name] = vector<string>(); 
                string nameLink = fuName + "->" + name + "->" + num2str(0); 
                connectionsTable[fuName][name].push_back(nameLink); 
                link2path[nameLink] = vector<string>(); 
            }
            else
            {
                clog << "NetworkAnalyzer: Warning: output port without FU: " << name << endl; 
                string fuName = "__GLOBAL__"; 
                if(fu2type.find(fuName) == fu2type.end())
                {
                    fu2type[fuName]   = "__GLOBAL__"; 
                    fuInputs[fuName]  = unordered_set<string>(); 
                    fuOutputs[fuName] = unordered_set<string>(); 
                }
                port2fu[name] = fuName; 
                output2fu[name] = fuName; 
                fuOutputs[fuName].insert(name); 
                if(connectionsTable.find(name) == connectionsTable.end())
                {
                    connectionsTable[name] = unordered_map<string, vector<string>>(); 
                }
            }
            
        }
    }
    // Functions for finding valid connections
    auto funcPath = [&](const Vertex &vertex){
        const string &type   = vertex.getAttr("type").getStr(); 
        const string &device = vertex.getAttr("device").getStr(); 
        string devicePrefix = getPrefix(device); 
        if(type == "MUX" || 
           ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
           type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__")
        {
            return true; 
        }
        else if(type == "__ELEMENT_INPUT_PORT__"  || type == "__ELEMENT_OUTPUT_PORT__")
        {
            size_t posDot = device.find("."); 
            assert(posDot != device.npos);
            string belongsTo = device.substr(0, posDot); 
            if(belongsTo.substr(0, 3) == "MUX")
            {
                return true; 
            } 
        }
        return false; 
    }; 
    auto funcSink = [&](const Vertex &vertex){
        return !funcPath(vertex); 
    }; 
    // Find valid connections
    size_t countdown = output2fu.size(); 
    for(const auto &source: output2fu)
    {
        unordered_map<string, vector<vector<string>>> reachables = rrg.reachableVertices(source.first, funcPath, funcSink); 
        size_t countLinks = 0; 
        connectionsTable[source.first] = unordered_map<string, vector<string>>(); 
        for(const auto &sink: reachables)
        {
            string basenameLink = source.first + "->" + sink.first + "->"; 
            connectionsTable[source.first][sink.first] = vector<string>(); 
            for(size_t idx = 0; idx < sink.second.size(); idx++)
            {
                string nameLink = basenameLink + num2str(idx); 
                connectionsTable[source.first][sink.first].push_back(nameLink); 
                link2path[nameLink] = sink.second[idx]; 
            }
            countLinks++; 
        }
        clog << "\r -> found the sinks of " << source.first << ": " << countLinks << " links; " << (--countdown) << " remained...... "; 
    }
    clog << endl; 
    clog << "NetworkAnalyzer: finished finding reachable elements. " << endl; 
    clog << "NetworkAnalyzer: " << link2path.size() << " links. " << endl; 

    for(const auto &fu: fu2type)
    {
        const string &name = fu.first; 
        if(fu.first != "__GLOBAL__")
        {
            contractedGraph.addVertex(rrg.vertex(name)); 
        }
        for(const auto &port: fuInputs[name])
        {
            contractedGraph.addVertex(rrg.vertex(port)); 
        }
        for(const auto &port: fuOutputs[name])
        {
            contractedGraph.addVertex(rrg.vertex(port)); 
        }
    }
    for(const auto &source: connectionsTable)
    {
        for(const auto &sink: connectionsTable.find(source.first)->second)
        {
            if(contractedGraph.vertices().find(source.first) == contractedGraph.vertices().end())
            {
                clog << "NetworkAnalyzer: ERROR: " << source.first << " not found. " << endl; 
            }
            if(contractedGraph.vertices().find(sink.first) == contractedGraph.vertices().end())
            {
                clog << "NetworkAnalyzer: ERROR: " << sink.first << " not found. " << endl; 
            }
            contractedGraph.addEdge(Edge(source.first, sink.first)); 
        }
    }
    
    clog << "END: NetworkAnalyzer: Analyzing networks. " << endl << endl; 

    _RRG        = move(contractedGraph); 
    _linksTable = move(connectionsTable); 
    _pathsTable = move(link2path); 

    return true; 
}

bool NetworkAnalyzer::compatible(const vector<string> &path1, const vector<string> &path2)
{
    unordered_set<string> used;
    for(const auto &node: path1)
    {
        used.insert(node); 
    }
    for(const auto &node: path2)
    {
        if(used.find(node) != used.end())
        {
            return false; 
        }
    }

    return true; 
}

bool NetworkAnalyzer::dumpAnalysis(const string &graphFilename, const std::string &linksFilename, const string &pathsFilename) const
{
    const Graph &graph = _RRG; 
    const unordered_map<string, unordered_map<string, vector<string>>> &links = _linksTable; 
    const unordered_map<string, vector<string>> &paths = _pathsTable; 
    bool okay = graph.dump(graphFilename); 

    ofstream foutLinks(linksFilename); 
    if(!foutLinks)
    {
        okay = false; 
    }
    for(const auto &froms: links)
    {
        for(const auto &tos: froms.second)
        {
            for(const auto &name: tos.second)
            {
                foutLinks << froms.first << " " << tos.first << " " << name << endl; 
            }
        }
    }
    foutLinks.close(); 

    ofstream foutPaths(pathsFilename); 
    if(!foutPaths)
    {
        okay = false; 
    }
    for(const auto &path: paths)
    {
        foutPaths << path.first << " "; 
        for(const auto &node: path.second)
        {
            foutPaths << node << " "; 
        }
        foutPaths << endl; 
    }
    foutPaths.close(); 

    return okay; 
}

bool NetworkAnalyzer::loadAnalysis(const string &graphFilename, const string &linksFilename, const string &pathsFilename)
{
    Graph graph(graphFilename); 
    unordered_map<string, unordered_map<string, vector<string>>> links; 
    unordered_map<string, vector<string>> paths;

    ifstream finLinks(linksFilename); 
    if(finLinks)
    {
        while(!finLinks.eof())
        {
            string line; 
            getline(finLinks, line); 
            if(line.empty())
            {
                continue; 
            }
            istringstream sin(line); 
            string from, to, name; 
            sin >> from >> to >> name; 
            if(from.empty())
            {
                continue; 
            }
            if(links.find(from) == links.end())
            {
                links[from] = unordered_map<string, vector<string>>(); 
            }
            if(links[from].find(to) == links[from].end())
            {
                links[from][to] = vector<string>(); 
            }
            links[from][to].push_back(name); 
        }
    }
    else
    {
        return false;
    }
    finLinks.close(); 

    ifstream finPaths(pathsFilename); 
    if(finPaths)
    {
        while(!finPaths.eof())
        {
            string line; 
            getline(finPaths, line); 
            if(line.empty())
            {
                continue; 
            }
            istringstream sin(line); 
            string name; 
            sin >> name;  
            if(name.empty())
            {
                continue; 
            }
            assert(paths.find(name) == paths.end()); 
            paths[name] = vector<string>(); 
            while(!sin.eof())
            {
                string tmp; 
                sin >> tmp; 
                if(!tmp.empty())
                {
                    paths[name].push_back(tmp); 
                }
            }
        }
    }
    else
    {
        return false;
    }
    finPaths.close(); 

    _RRG = move(graph); 
    _linksTable = move(links); 
    _pathsTable = move(paths); 
    return true; 
}

// void NetworkAnalyzer::genFUs(const Graph &rrg)
// {
//     _fus.clear(); 
//     for(const auto &vertex: rrg.vertices())
//     {
//         string type   = vertex.second.getAttr("type").getStr(); 
//         string device = vertex.second.getAttr("device").getStr(); 
//         string devicePrefix = getPrefix(device); 
//         string devicePostfix = getPostfix(device); 
//         if(type == "MUX" || 
//            ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
//            type == "__FB_INPUT_PORT__"  || type == "__FB_OUTPUT_PORT__" ||
//            type == "__FC_INPUT_PORT__"  || type == "__FC_OUTPUT_PORT__" ||
//            type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__" ||
//            type == "__ELEMENT_INPUT_PORT__" || type == "__ELEMENT_OUTPUT_PORT__")
//         {
//             continue; 
//         }
//         if(_fus.find(devicePrefix) == _fus.end())
//         {
//             _fus[devicePrefix] = unordered_set<string>(); 
//             // clog << "NetworkAnalyzer: " << "found FU: " << devicePrefix << endl; 
//         }
//         if(device != devicePrefix)
//         {
//             _fus[devicePrefix].insert(devicePostfix); 
//         }
//     }
// }

void NetworkAnalyzer::genFUs(const Graph &rrg)
{
    _fus.clear(); 
    for(const auto &vertex: rrg.vertices())
    {
        string type   = vertex.second.getAttr("type").getStr(); 
        string device = vertex.second.getAttr("device").getStr(); 
        string devicePrefix = getPrefix(device); 
        string devicePostfix = getPostfix(device); 
        if(type == "MUX" || devicePrefix.substr(0, 3) == "MUX" || 
           ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
//            type == "__FB_INPUT_PORT__"  || type == "__FB_OUTPUT_PORT__" ||
//            type == "__FC_INPUT_PORT__"  || type == "__FC_OUTPUT_PORT__" ||
           type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__")
        {
            continue; 
        }
        if(_fus.find(devicePrefix) == _fus.end())
        {
            _fus[devicePrefix] = unordered_set<string>(); 
            // clog << "NetworkAnalyzer: " << "found FU: " << devicePrefix << endl; 
        }
        if(device != devicePrefix)
        {
            _fus[devicePrefix].insert(devicePostfix); 
        }
    }
}


string NetworkAnalyzerLegacy::_graphFilename = ""; 
string NetworkAnalyzerLegacy::_linksFilename = ""; 

unordered_map<string, unordered_set<string>> NetworkAnalyzerLegacy::parse(const string &filename)
// FU type(device) -> FU ports
{
    clog << endl << "BEGIN: NetworkAnalyzerLegacy: Reading FUs infomation from " << filename << endl; 

    unordered_map<string, unordered_set<string>> fus; 
    ifstream fin(filename); 
    size_t countFUs     = 0; 
    size_t countInputs  = 0; 
    size_t countOutputs = 0; 
    while(!fin.eof())
    {
        string line; 
        getline(fin, line); 
        if(line.empty())
        {
            continue; 
        }
        istringstream sin(line); 
        string tmp; 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        tmp.clear(); 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        tmp.clear(); 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        fus[tmp] = unordered_set<string>(); 
        countFUs++; 

        string tmp2; 
        sin >> tmp2; 
        if(tmp2.empty())
        {
            continue; 
        }
        assert(tmp2 == "inputs"); 
        while(!tmp2.empty())
        {
            tmp2.clear(); 
            sin >> tmp2; 
            if(tmp2.empty() || tmp2 == "outputs")
            {
                break; 
            }
            fus[tmp].insert(tmp2); 
            countInputs++; 
        }
        while(!tmp2.empty())
        {
            tmp2.clear(); 
            sin >> tmp2; 
            if(tmp2.empty())
            {
                break; 
            }
            fus[tmp].insert(tmp2); 
            countOutputs++; 
        }
    }
    fin.close(); 

    clog << "NetworkAnalyzerLegacy: Found FUs:"     << countFUs << endl; 
    clog << "NetworkAnalyzerLegacy: Found Inputs:"  << countInputs << endl; 
    clog << "NetworkAnalyzerLegacy: Found Outputs:" << countOutputs << endl; 
    clog << "NetworkAnalyzerLegacy: Found Ports:"   << (countInputs + countOutputs) << endl; 

    clog << "END: NetworkAnalyzerLegacy: Reading FUs infomation. " << endl << endl; 

    return fus; 
}

bool NetworkAnalyzerLegacy::analyze(const Graph &rrg)
//TODO: make a parallel version to accelerate the analyzing procedure
{
    clog << endl << "BEGIN: NetworkAnalyzerLegacy: Analyzing networks. " << endl; 
    if(!_graphFilename.empty() && !_linksFilename.empty())
    {
        loadAnalysis(_graphFilename, _linksFilename); 
        _graphFilename = ""; 
        _linksFilename = ""; 
        return true; 
    }
    
    _RRG = rrg; 
    if(_fus.empty())
    {
        clog << "NetworkAnalyzerLegacy: Generating FUs. " << endl; 
        genFUs(_RRG); 
    }

    Graph contractedGraph; 
    unordered_map<string, unordered_set<string>> connectionsTable; 

    unordered_map<string, string>                fu2type; 
    unordered_map<string, string>                port2fu; 
    unordered_map<string, string>                input2fu; 
    unordered_map<string, string>                output2fu; 
    unordered_map<string, unordered_set<string>> fuInputs; 
    unordered_map<string, unordered_set<string>> fuOutputs; 
    // Find FUs
    for(const auto &vertex: rrg.vertices())
    {
        const string &name = vertex.first;  
        const string &type = vertex.second.getAttr("device").getStr(); 
        if(_fus.find(type) != _fus.end())
        {
            fu2type[name]   = type; 
            fuInputs[name]  = unordered_set<string>(); 
            fuOutputs[name] = unordered_set<string>(); 
        }
    }
    // Find FUs inputs and outputs
    for(const auto &vertex: rrg.vertices())
    {
        const string &name = vertex.first;  
        const string &type = vertex.second.getAttr("device").getStr(); 
        // Get the prefix
        size_t posDot = type.find("."); 
        if(posDot == string::npos)
        {
            continue; 
        }
        string fuType = type.substr(0, posDot); 
        if(_fus.find(fuType) == _fus.end())
        {
            continue; 
        }
        // Is input port or output port?
        bool isInput = true; 
        for(const auto &edge: rrg.edgesIn(name))
        {
            auto iter = fu2type.find(edge.from()); 
            if(iter != fu2type.end())
            {
                assert(iter->second == fuType); 
                isInput = false; 
            }
        }
        // if(vertex.second.getAttr("type").getStr() == "__TOP_INPUT_PORT__")
        // {
        //     isInput = false; 
        // }
        // else if(vertex.second.getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
        // {
        //     isInput = true; 
        // }
        if(isInput)
        {
            // Make sure that every input port only connect to one FU
            assert(rrg.edgesOut(name).size() <= 1); 
            if(rrg.edgesOut(name).size() > 0)
            {
                for(const auto &edge: rrg.edgesOut(name)) 
                {
                    auto iter = fu2type.find(edge.to()); 
                    if(iter == fu2type.end() || iter->second != fuType)
                    {
                        if(iter == fu2type.end())
                        {
                            clog << "NetworkAnalyzerLegacy: ERROR: " << edge.to() << " not found. " << endl; 
                        }
                        else if(iter->second != fuType)
                        {
                            clog << "NetworkAnalyzerLegacy: ERROR: " << iter->second << " does not match " << fuType << endl; 
                        }
                    }
                    assert(iter != fu2type.end() && iter->second == fuType); 
                }
                string fuName = rrg.edgesOut(name)[0].to(); 
                port2fu[name] = fuName; 
                input2fu[name] = fuName; 
                fuInputs[fuName].insert(name); 
                if(connectionsTable.find(name) == connectionsTable.end())
                {
                    connectionsTable[name] = unordered_set<string>(); 
                }
                connectionsTable[name].insert(fuName); 
            }
            else
            {
                clog << "NetworkAnalyzerLegacy: Warning: input port without FU: " << name << endl; 
                string fuName = "__GLOBAL__"; 
                if(fu2type.find(fuName) == fu2type.end())
                {
                    fu2type[fuName]   = "__GLOBAL__"; 
                    fuInputs[fuName]  = unordered_set<string>(); 
                    fuOutputs[fuName] = unordered_set<string>(); 
                }
                port2fu[name] = fuName; 
                input2fu[name] = fuName; 
                fuInputs[fuName].insert(name); 
            }
        }
        else
        {
            // Make sure that every output port only be connected from one FU
            assert(rrg.edgesIn(name).size() <= 1); 
            if(rrg.edgesIn(name).size() > 0)
            {
                for(const auto &edge: rrg.edgesIn(name))
                {
                    auto iter = fu2type.find(edge.from()); 
                    assert(iter != fu2type.end() && iter->second == fuType); 
                }
                string fuName = rrg.edgesIn(name)[0].from(); 
                port2fu[name] = fuName; 
                output2fu[name] = fuName; 
                fuOutputs[fuName].insert(name); 
                if(connectionsTable.find(fuName) == connectionsTable.end())
                {
                    connectionsTable[fuName] = unordered_set<string>(); 
                }
                connectionsTable[fuName].insert(name); 
            }
            else
            {
                clog << "NetworkAnalyzerLegacy: Warning: output port without FU: " << name << endl; 
                string fuName = "__GLOBAL__"; 
                if(fu2type.find(fuName) == fu2type.end())
                {
                    fu2type[fuName]   = "__GLOBAL__"; 
                    fuInputs[fuName]  = unordered_set<string>(); 
                    fuOutputs[fuName] = unordered_set<string>(); 
                }
                port2fu[name] = fuName; 
                output2fu[name] = fuName; 
                fuOutputs[fuName].insert(name); 
                if(connectionsTable.find(name) == connectionsTable.end())
                {
                    connectionsTable[name] = unordered_set<string>(); 
                }
            }
            
        }
    }
    // Functions for finding valid connections
    auto funcPath = [&](const Vertex &vertex){
        const string &type   = vertex.getAttr("type").getStr(); 
        const string &device = vertex.getAttr("device").getStr(); 
        string devicePrefix = getPrefix(device); 
        if(type == "MUX" || 
           ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
           type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__")
        {
            return true; 
        }
        else if(type == "__ELEMENT_INPUT_PORT__"  || type == "__ELEMENT_OUTPUT_PORT__")
        {
            size_t posDot = device.find("."); 
            assert(posDot != device.npos);
            string belongsTo = device.substr(0, posDot); 
            if(belongsTo.substr(0, 3) == "MUX")
            {
                return true; 
            } 
        }
        return false; 
    }; 
    auto funcSink = [&](const Vertex &vertex){
        return !funcPath(vertex); 
    }; 
    // Find valid connections
    size_t countAllLinks = 0; 
    size_t countdown = output2fu.size(); 
    for(const auto &source: output2fu)
    {
        unordered_set<string> reachables = rrg.reachableFrom(source.first, funcPath, funcSink); 
        size_t countLinks = reachables.size(); 
        countAllLinks += countLinks; 
        connectionsTable[source.first] = reachables; 
        clog << "\r -> found the sinks of " << source.first << ": " << countLinks << " links; " << (--countdown) << " remained...... "; 
    }
    clog << endl; 
    clog << "NetworkAnalyzerLegacy: finished finding reachable elements. " << endl; 
    clog << "NetworkAnalyzerLegacy: analyzed links: " << countAllLinks << endl; 

    for(const auto &fu: fu2type)
    {
        const string &name = fu.first; 
        if(fu.first != "__GLOBAL__")
        {
            contractedGraph.addVertex(rrg.vertex(name)); 
        }
        for(const auto &port: fuInputs[name])
        {
            contractedGraph.addVertex(rrg.vertex(port)); 
        }
        for(const auto &port: fuOutputs[name])
        {
            contractedGraph.addVertex(rrg.vertex(port)); 
        }
    }
    for(const auto &source: connectionsTable)
    {
        for(const auto &sink: connectionsTable.find(source.first)->second)
        {
            if(contractedGraph.vertices().find(source.first) == contractedGraph.vertices().end())
            {
                clog << "NetworkAnalyzerLegacy: ERROR: " << source.first << " not found. " << endl; 
            }
            if(contractedGraph.vertices().find(sink) == contractedGraph.vertices().end())
            {
                clog << "NetworkAnalyzerLegacy: ERROR: " << sink << " not found. " << endl; 
            }
            contractedGraph.addEdge(Edge(source.first, sink)); 
        }
    }
    size_t countRRGLinks = 0; 
    for(const auto &edges: contractedGraph.edgesOut())
    {
        countRRGLinks += edges.second.size(); 
    }
    clog << "NetworkAnalyzerLegacy: Vertices: " << contractedGraph.vertices().size() << endl; 
    clog << "NetworkAnalyzerLegacy: Links: " << countRRGLinks << endl; 
    
    clog << "END: NetworkAnalyzerLegacy: Analyzing networks. " << endl << endl; 

    _RRG        = move(contractedGraph); 
    _linksTable = move(connectionsTable); 
    
    return true; 
}

bool NetworkAnalyzerLegacy::dumpAnalysis(const string &graphFilename, const std::string &linksFilename) const
{
    const Graph &graph = _RRG; 
    const unordered_map<string, unordered_set<string>> &links = _linksTable; 
    bool okay = graph.dump(graphFilename); 

    ofstream foutLinks(linksFilename); 
    if(!foutLinks)
    {
        okay = false; 
    }
    for(const auto &froms: links)
    {
        for(const auto &tos: froms.second)
        {
            foutLinks << froms.first << " " << tos << " " << endl; 
        }
    }
    foutLinks.close(); 

    return okay; 
}

bool NetworkAnalyzerLegacy::loadAnalysis(const string &graphFilename, const string &linksFilename)
{
    Graph graph(graphFilename); 
    unordered_map<string, unordered_set<string>> links; 

    ifstream finLinks(linksFilename); 
    if(finLinks)
    {
        while(!finLinks.eof())
        {
            string line; 
            getline(finLinks, line); 
            if(line.empty())
            {
                continue; 
            }
            istringstream sin(line); 
            string from, to; 
            sin >> from >> to; 
            if(from.empty())
            {
                continue; 
            }
            if(links.find(from) == links.end())
            {
                links[from] = unordered_set<string>(); 
            }
            links[from].insert(to); 
        }
    }
    else
    {
        return false;
    }
    finLinks.close(); 

    _RRG = move(graph); 
    _linksTable = move(links); 
    return true; 
}

// void NetworkAnalyzerLegacy::genFUs(const Graph &rrg)
// {
//     _fus.clear(); 
//     for(const auto &vertex: rrg.vertices())
//     {
//         string type   = vertex.second.getAttr("type").getStr(); 
//         string device = vertex.second.getAttr("device").getStr(); 
//         string devicePrefix = getPrefix(device); 
//         string devicePostfix = getPostfix(device); 
//         if(type == "MUX" || 
//            ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
//            type == "__FB_INPUT_PORT__"  || type == "__FB_OUTPUT_PORT__" ||
//            type == "__FC_INPUT_PORT__"  || type == "__FC_OUTPUT_PORT__" ||
//            type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__" ||
//            type == "__ELEMENT_INPUT_PORT__" || type == "__ELEMENT_OUTPUT_PORT__")
//         {
//             continue; 
//         }
//         if(_fus.find(devicePrefix) == _fus.end())
//         {
//             _fus[devicePrefix] = unordered_set<string>(); 
//             // clog << "NetworkAnalyzer: " << "found FU: " << devicePrefix << endl; 
//         }
//         if(device != devicePrefix)
//         {
//             _fus[devicePrefix].insert(devicePostfix); 
//         }
//     }
// }

void NetworkAnalyzerLegacy::genFUs(const Graph &rrg)
{
    _fus.clear(); 
    for(const auto &vertex: rrg.vertices())
    {
        string type   = vertex.second.getAttr("type").getStr(); 
        string device = vertex.second.getAttr("device").getStr(); 
        string devicePrefix = getPrefix(device); 
        string devicePostfix = getPostfix(device); 
        if(type == "MUX" || devicePrefix.substr(0, 3) == "MUX" || 
           ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
//            type == "__FB_INPUT_PORT__"  || type == "__FB_OUTPUT_PORT__" ||
//            type == "__FC_INPUT_PORT__"  || type == "__FC_OUTPUT_PORT__" ||
           type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__")
        {
            continue; 
        }
        if(_fus.find(devicePrefix) == _fus.end())
        {
            _fus[devicePrefix] = unordered_set<string>(); 
            // clog << "NetworkAnalyzer: " << "found FU: " << devicePrefix << endl; 
        }
        if(device != devicePrefix)
        {
            _fus[devicePrefix].insert(devicePostfix); 
        }
    }
}


string NetworkAnalyzerLegacySilent::_graphFilename = ""; 
string NetworkAnalyzerLegacySilent::_linksFilename = ""; 

unordered_map<string, unordered_set<string>> NetworkAnalyzerLegacySilent::parse(const string &filename)
// FU type(device) -> FU ports
{

    unordered_map<string, unordered_set<string>> fus; 
    ifstream fin(filename); 
    size_t countFUs     = 0; 
    size_t countInputs  = 0; 
    size_t countOutputs = 0; 
    while(!fin.eof())
    {
        string line; 
        getline(fin, line); 
        if(line.empty())
        {
            continue; 
        }
        istringstream sin(line); 
        string tmp; 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        tmp.clear(); 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        tmp.clear(); 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        fus[tmp] = unordered_set<string>(); 
        countFUs++; 

        string tmp2; 
        sin >> tmp2; 
        if(tmp2.empty())
        {
            continue; 
        }
        assert(tmp2 == "inputs"); 
        while(!tmp2.empty())
        {
            tmp2.clear(); 
            sin >> tmp2; 
            if(tmp2.empty() || tmp2 == "outputs")
            {
                break; 
            }
            fus[tmp].insert(tmp2); 
            countInputs++; 
        }
        while(!tmp2.empty())
        {
            tmp2.clear(); 
            sin >> tmp2; 
            if(tmp2.empty())
            {
                break; 
            }
            fus[tmp].insert(tmp2); 
            countOutputs++; 
        }
    }
    fin.close(); 

    return fus; 
}

bool NetworkAnalyzerLegacySilent::analyze(const Graph &rrg)
//TODO: make a parallel version to accelerate the analyzing procedure
{

    if(!_graphFilename.empty() && !_linksFilename.empty())
    {
        loadAnalysis(_graphFilename, _linksFilename); 
        _graphFilename = ""; 
        _linksFilename = ""; 
        return true; 
    }

    _RRG = rrg; 
    if(_fus.empty())
    {
        genFUs(_RRG); 
    }

    Graph contractedGraph; 
    unordered_map<string, unordered_set<string>> connectionsTable; 

    unordered_map<string, string>                fu2type; 
    unordered_map<string, string>                port2fu; 
    unordered_map<string, string>                input2fu; 
    unordered_map<string, string>                output2fu; 
    unordered_map<string, unordered_set<string>> fuInputs; 
    unordered_map<string, unordered_set<string>> fuOutputs; 
    // Find FUs
    for(const auto &vertex: rrg.vertices())
    {
        const string &name = vertex.first;  
        const string &type = vertex.second.getAttr("device").getStr(); 
        if(_fus.find(type) != _fus.end())
        {
            fu2type[name]   = type; 
            fuInputs[name]  = unordered_set<string>(); 
            fuOutputs[name] = unordered_set<string>(); 
        }
    }
    // Find FUs inputs and outputs
    for(const auto &vertex: rrg.vertices())
    {
        const string &name = vertex.first;  
        const string &type = vertex.second.getAttr("device").getStr(); 
        // Get the prefix
        size_t posDot = type.find("."); 
        if(posDot == string::npos)
        {
            continue; 
        }
        string fuType = type.substr(0, posDot); 
        if(_fus.find(fuType) == _fus.end())
        {
            continue; 
        }
        // Is input port or output port?
        bool isInput = true; 
        for(const auto &edge: rrg.edgesIn(name))
        {
            auto iter = fu2type.find(edge.from()); 
            if(iter != fu2type.end())
            {
                assert(iter->second == fuType); 
                isInput = false; 
            }
        }
        // if(vertex.second.getAttr("type").getStr() == "__TOP_INPUT_PORT__")
        // {
        //     isInput = false; 
        // }
        // else if(vertex.second.getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
        // {
        //     isInput = true; 
        // }
        if(isInput)
        {
            // Make sure that every input port only connect to one FU
            assert(rrg.edgesOut(name).size() <= 1); 
            if(rrg.edgesOut(name).size() > 0)
            {
                for(const auto &edge: rrg.edgesOut(name)) 
                {
                    auto iter = fu2type.find(edge.to()); 
                    if(iter == fu2type.end() || iter->second != fuType)
                    {
                        if(iter == fu2type.end())
                        {
                            clog << "NetworkAnalyzerLegacySilent: ERROR: " << edge.to() << " not found. " << endl; 
                        }
                        else if(iter->second != fuType)
                        {
                            clog << "NetworkAnalyzerLegacySilent: ERROR: " << iter->second << " does not match " << fuType << endl; 
                        }
                    }
                    assert(iter != fu2type.end() && iter->second == fuType); 
                }
                string fuName = rrg.edgesOut(name)[0].to(); 
                port2fu[name] = fuName; 
                input2fu[name] = fuName; 
                fuInputs[fuName].insert(name); 
                if(connectionsTable.find(name) == connectionsTable.end())
                {
                    connectionsTable[name] = unordered_set<string>(); 
                }
                connectionsTable[name].insert(fuName); 
            }
            else
            {
                clog << "NetworkAnalyzerLegacySilent: Warning: input port without FU: " << name << endl; 
                string fuName = "__GLOBAL__"; 
                if(fu2type.find(fuName) == fu2type.end())
                {
                    fu2type[fuName]   = "__GLOBAL__"; 
                    fuInputs[fuName]  = unordered_set<string>(); 
                    fuOutputs[fuName] = unordered_set<string>(); 
                }
                port2fu[name] = fuName; 
                input2fu[name] = fuName; 
                fuInputs[fuName].insert(name); 
            }
        }
        else
        {
            // Make sure that every output port only be connected from one FU
            assert(rrg.edgesIn(name).size() <= 1); 
            if(rrg.edgesIn(name).size() > 0)
            {
                for(const auto &edge: rrg.edgesIn(name))
                {
                    auto iter = fu2type.find(edge.from()); 
                    assert(iter != fu2type.end() && iter->second == fuType); 
                }
                string fuName = rrg.edgesIn(name)[0].from(); 
                port2fu[name] = fuName; 
                output2fu[name] = fuName; 
                fuOutputs[fuName].insert(name); 
                if(connectionsTable.find(fuName) == connectionsTable.end())
                {
                    connectionsTable[fuName] = unordered_set<string>(); 
                }
                connectionsTable[fuName].insert(name); 
            }
            else
            {
                clog << "NetworkAnalyzerLegacySilent: Warning: output port without FU: " << name << endl; 
                string fuName = "__GLOBAL__"; 
                if(fu2type.find(fuName) == fu2type.end())
                {
                    fu2type[fuName]   = "__GLOBAL__"; 
                    fuInputs[fuName]  = unordered_set<string>(); 
                    fuOutputs[fuName] = unordered_set<string>(); 
                }
                port2fu[name] = fuName; 
                output2fu[name] = fuName; 
                fuOutputs[fuName].insert(name); 
                if(connectionsTable.find(name) == connectionsTable.end())
                {
                    connectionsTable[name] = unordered_set<string>(); 
                }
            }
            
        }
    }
    // Functions for finding valid connections
    auto funcPath = [&](const Vertex &vertex){
        const string &type   = vertex.getAttr("type").getStr(); 
        const string &device = vertex.getAttr("device").getStr(); 
        string devicePrefix = getPrefix(device); 
        if(type == "MUX" || 
           ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
           type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__")
        {
            return true; 
        }
        else if(type == "__ELEMENT_INPUT_PORT__"  || type == "__ELEMENT_OUTPUT_PORT__")
        {
            size_t posDot = device.find("."); 
            assert(posDot != device.npos);
            string belongsTo = device.substr(0, posDot); 
            if(belongsTo.substr(0, 3) == "MUX")
            {
                return true; 
            } 
        }
        return false; 
    }; 
    auto funcSink = [&](const Vertex &vertex){
        return !funcPath(vertex); 
    }; 
    // Find valid connections
    size_t countAllLinks = 0; 
    for(const auto &source: output2fu)
    {
        unordered_set<string> reachables = rrg.reachableFrom(source.first, funcPath, funcSink); 
        size_t countLinks = reachables.size(); 
        countAllLinks += countLinks; 
        connectionsTable[source.first] = reachables; 
    }

    for(const auto &fu: fu2type)
    {
        const string &name = fu.first; 
        if(fu.first != "__GLOBAL__")
        {
            contractedGraph.addVertex(rrg.vertex(name)); 
        }
        for(const auto &port: fuInputs[name])
        {
            contractedGraph.addVertex(rrg.vertex(port)); 
        }
        for(const auto &port: fuOutputs[name])
        {
            contractedGraph.addVertex(rrg.vertex(port)); 
        }
    }
    for(const auto &source: connectionsTable)
    {
        for(const auto &sink: connectionsTable.find(source.first)->second)
        {
            if(contractedGraph.vertices().find(source.first) == contractedGraph.vertices().end())
            {
                clog << "NetworkAnalyzerLegacySilent: ERROR: " << source.first << " not found. " << endl; 
            }
            if(contractedGraph.vertices().find(sink) == contractedGraph.vertices().end())
            {
                clog << "NetworkAnalyzerLegacySilent: ERROR: " << sink << " not found. " << endl; 
            }
            contractedGraph.addEdge(Edge(source.first, sink)); 
        }
    }
    size_t countRRGLinks = 0; 
    for(const auto &edges: contractedGraph.edgesOut())
    {
        countRRGLinks += edges.second.size(); 
    }

    _RRG        = move(contractedGraph); 
    _linksTable = move(connectionsTable); 

    return true; 
}

bool NetworkAnalyzerLegacySilent::dumpAnalysis(const string &graphFilename, const std::string &linksFilename) const
{
    const Graph &graph = _RRG; 
    const unordered_map<string, unordered_set<string>> &links = _linksTable; 
    bool okay = graph.dump(graphFilename); 

    ofstream foutLinks(linksFilename); 
    if(!foutLinks)
    {
        okay = false; 
    }
    for(const auto &froms: links)
    {
        for(const auto &tos: froms.second)
        {
            foutLinks << froms.first << " " << tos << " " << endl; 
        }
    }
    foutLinks.close(); 

    return okay; 
}

bool NetworkAnalyzerLegacySilent::loadAnalysis(const string &graphFilename, const string &linksFilename)
{
    Graph graph(graphFilename); 
    unordered_map<string, unordered_set<string>> links; 

    ifstream finLinks(linksFilename); 
    if(finLinks)
    {
        while(!finLinks.eof())
        {
            string line; 
            getline(finLinks, line); 
            if(line.empty())
            {
                continue; 
            }
            istringstream sin(line); 
            string from, to; 
            sin >> from >> to; 
            if(from.empty())
            {
                continue; 
            }
            if(links.find(from) == links.end())
            {
                links[from] = unordered_set<string>(); 
            }
            links[from].insert(to); 
        }
    }
    else
    {
        return false;
    }
    finLinks.close(); 

    _RRG = move(graph); 
    _linksTable = move(links); 
    return true; 
}

// void NetworkAnalyzerLegacySilent::genFUs(const Graph &rrg)
// {
//     _fus.clear(); 
//     for(const auto &vertex: rrg.vertices())
//     {
//         string type   = vertex.second.getAttr("type").getStr(); 
//         string device = vertex.second.getAttr("device").getStr(); 
//         string devicePrefix = getPrefix(device); 
//         string devicePostfix = getPostfix(device); 
//         if(type == "MUX" || 
//            ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
//            type == "__FB_INPUT_PORT__"  || type == "__FB_OUTPUT_PORT__" ||
//            type == "__FC_INPUT_PORT__"  || type == "__FC_OUTPUT_PORT__" ||
//            type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__" ||
//            type == "__ELEMENT_INPUT_PORT__" || type == "__ELEMENT_OUTPUT_PORT__")
//         {
//             continue; 
//         }
//         if(_fus.find(devicePrefix) == _fus.end())
//         {
//             _fus[devicePrefix] = unordered_set<string>(); 
//             // clog << "NetworkAnalyzer: " << "found FU: " << devicePrefix << endl; 
//         }
//         if(device != devicePrefix)
//         {
//             _fus[devicePrefix].insert(devicePostfix); 
//         }
//     }
// }

void NetworkAnalyzerLegacySilent::genFUs(const Graph &rrg)
{
    _fus.clear(); 
    for(const auto &vertex: rrg.vertices())
    {
        string type   = vertex.second.getAttr("type").getStr(); 
        string device = vertex.second.getAttr("device").getStr(); 
        string devicePrefix = getPrefix(device); 
        string devicePostfix = getPostfix(device); 
        if(type == "MUX" || devicePrefix.substr(0, 3) == "MUX" || 
           ((type == "__TOP_INPUT_PORT__"  || type == "__TOP_OUTPUT_PORT__") && !(devicePrefix == "__INPUT_FU__"  || devicePrefix == "__OUTPUT_FU__")) || 
//            type == "__FB_INPUT_PORT__"  || type == "__FB_OUTPUT_PORT__" ||
//            type == "__FC_INPUT_PORT__"  || type == "__FC_OUTPUT_PORT__" ||
           type == "__MODULE_INPUT_PORT__"  || type == "__MODULE_OUTPUT_PORT__")
        {
            continue; 
        }
        if(_fus.find(devicePrefix) == _fus.end())
        {
            _fus[devicePrefix] = unordered_set<string>(); 
            // clog << "NetworkAnalyzer: " << "found FU: " << devicePrefix << endl; 
        }
        if(device != devicePrefix)
        {
            _fus[devicePrefix].insert(devicePostfix); 
        }
    }
}

}
