#include "VanillaMatcher.h"

using namespace std; 

namespace FastCGRA
{

bool VanillaMatcher::_match(const std::string &name, std::unordered_set<std::string> &visited)
{
    assert(_candidates.find(name) != _candidates.end()); 
    for(const auto &toTry: _candidates[name])
    {
        if(visited.find(toTry) == visited.end())
        {
            visited.insert(toTry); 
            if(_matchTableRev.find(toTry) == _matchTableRev.end() || _match(_matchTableRev[toTry], visited))
            {
                _matchTableRev[toTry] = name; 
                return true; 
            }
        }
    }

    return false; 
}

size_t VanillaMatcher::match()
{
    size_t count = 0; 

    for(const auto &toMatch: _candidates)
    {
        unordered_set<string> visited; 
        count += _match(toMatch.first, visited); 
    }

    if(count >= _candidates.size())
    {
        assert(count == _candidates.size()); 
    }

    for(const auto &match: _matchTableRev)
    {
        _matchTable[match.second] = match.first; 
    }

    return count; 
}


}
