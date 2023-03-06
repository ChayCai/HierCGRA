#include "VanillaValidator.h"

using namespace std; 

namespace FastCGRA
{

bool VanillaValidator::validate(const unordered_map<string, string> &vertexDFG2RRG, const unordered_map<string, unordered_map<string, vector<string>>> &paths)
{
    cout << "VanillaValidator: All vertices of DFG are mapped? " << endl; 
    bool allVerticesDFGMapped = true; 
    unordered_set<string> usedVertices; 
    for(const auto &vertex: _DFG.vertices())
    {
        if(vertexDFG2RRG.find(vertex.first) == vertexDFG2RRG.end())
        {
            cout << "VanillaValidator: -> " << vertex.first << " NOT mapped" << endl; 
            allVerticesDFGMapped = false; 
            continue; 
        }
        if(usedVertices.find(vertexDFG2RRG.find(vertex.first)->second) != usedVertices.end())
        {
            cout << "VanillaValidator: -> " << vertex.first << ": " << vertexDFG2RRG.find(vertex.first)->second << " occupied. " << endl; 
            allVerticesDFGMapped = false; 
            continue; 
        }
        usedVertices.insert(vertexDFG2RRG.find(vertex.first)->second); 
        string fuDFG = getPrefix(vertex.first); 
        if(vertexDFG2RRG.find(fuDFG) == vertexDFG2RRG.end())
        {
            cout << "VanillaValidator: -> " << fuDFG << " NOT mapped" << endl; 
            allVerticesDFGMapped = false; 
            continue; 
        }
        string fuRRG     = vertexDFG2RRG.find(fuDFG)->second; 
        string deviceRRG = _RRG.vertex(fuRRG).getAttr("device").getStr(); 
        assert(_compat.find(fuDFG) != _compat.end()); 
        if(_compat.find(fuDFG)->second.find(deviceRRG) == _compat.find(fuDFG)->second.end())
        {
            cout << "VanillaValidator: -> " << fuDFG << " NOT compatible with " << fuRRG << endl; 
            allVerticesDFGMapped = false; 
            continue; 
        }
    }
    if(allVerticesDFGMapped)
    {
        cout << "VanillaValidator: All vertices of DFG are mapped. " << endl; 
    }

    cout << "VanillaValidator: All edges of DFG are routed ? " << endl; 
    bool allVerticesDFGRouted = true; 
    vector<pair<string, string>> linksDFG; 
    vector<pair<string, string>> linksRRG; 
    for(const auto &vertex: _DFG.vertices())
    {
        for(const auto &edge: _DFG.edgesOut(vertex.first))
        {
            linksDFG.push_back({edge.from(), edge.to()}); 
            linksRRG.push_back({vertexDFG2RRG.find(edge.from())->second, vertexDFG2RRG.find(edge.to())->second}); 
        }
    }
    unordered_map<string, string> signals; 
    for(size_t idx = 0; idx < linksDFG.size(); idx++)
    {
        if(paths.find(linksRRG[idx].first) == paths.end() || 
           paths.find(linksRRG[idx].first)->second.find(linksRRG[idx].second) == paths.find(linksRRG[idx].first)->second.end())
        {
            cout << "VanillaValidator: -> " << linksDFG[idx].first << " -> " << linksDFG[idx].second << " : " << linksRRG[idx].first << " -> " << linksRRG[idx].second << " NOT found. " << endl; 
            continue; 
        }
        string prev = linksRRG[idx].first; 
        for(const auto &node: paths.find(linksRRG[idx].first)->second.find(linksRRG[idx].second)->second)
        {
            if(signals.find(node) == signals.end())
            {
                signals[node] = linksDFG[idx].first; 
            }
            else if(signals[node] != linksDFG[idx].first)
            {
                cout << "VanillaValidator: -> " << linksDFG[idx].first << " -> " << linksDFG[idx].second << " : " << linksRRG[idx].first << " -> " << linksRRG[idx].second << " conflicts: " << signals[node] << " - " << linksDFG[idx].first << " on " << node << endl; 
                allVerticesDFGRouted = false; 
            }
            bool found = false; 
            for(const auto &edge: _RRG.edgesOut(prev))
            {
                if(edge.to() == node)
                {
                    found = true; 
                    break; 
                }
            }
            if(!found)
            {
                allVerticesDFGRouted = false; 
                cout << "VanillaValidator: Invalid path " << prev << " -> " << node << "; (in " << linksRRG[idx].first << "->" << linksRRG[idx].second << "), inner vertices not continuous. " << endl; 
            }
            prev = node; 
        }
        bool found = false; 
        for(const auto &edge: _RRG.edgesOut(prev))
        {
            if(edge.to() == linksRRG[idx].second)
            {
                found = true; 
                break; 
            }
        }
        if(!found)
        {
            allVerticesDFGRouted = false; 
            cout << "VanillaValidator: Invalid path " << prev << " -> " << linksRRG[idx].second << "; (in " << linksRRG[idx].first << "->" << linksRRG[idx].second << "). " << endl; 
        }
    }
    if(allVerticesDFGRouted)
    {
        cout << "VanillaValidator: All edges of DFG are routed. " << endl; 
    }

    return (allVerticesDFGMapped && allVerticesDFGRouted); 
}

}

