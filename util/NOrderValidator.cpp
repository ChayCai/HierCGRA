#include "NOrderValidator.h"

using namespace std; 

namespace FastCGRA
{

size_t NOrderValidator::validate(const std::string &vertexDFG, const std::string &vertexRRG, size_t order, const std::function<bool(const std::string &)> &used)
//WRONG
{
    if(order == 0)
    {
        return 1; 
    }
    // Go!
    size_t result = 1; 

    string fuDFG   = getPrefix(vertexDFG); 
    string portDFG = getPostfix(vertexDFG); 
    string fuRRG; 
    if(!portDFG.empty())
    {
        fuRRG = getPrefix(vertexRRG); 
    }
    else
    {
        fuRRG = vertexRRG; 
    }
    unordered_multimap<string, pair<string, string>>  inputsFU; 
    unordered_multimap<string, pair<string, string>>  outputsFU; 
    unordered_multiset<string>  inputsFUFound; 
    unordered_multiset<string>  outputsFUFound; 
    unordered_map<string, size_t> inputsScores; 
    unordered_map<string, size_t> outputsScores; 

    unordered_map<string, size_t> inputsCounts; 
    unordered_map<string, size_t> outputsCounts; 
    unordered_map<string, size_t> inputsCandidates; 
    unordered_map<string, size_t> outputsCandidates; 

    for(const auto &edge: _DFG.edgesIn(fuDFG))
    {
        assert(getPrefix(edge.from()) == fuDFG); 
        string port = getPostfix(edge.from()); 
        if(_DFG.edgesIn(edge.from()).empty())
        {
            continue; 
        }
        else
        {
            assert(_DFG.edgesIn(edge.from()).size() == 1); 
            const string &fromPort = _DFG.edgesIn(edge.from())[0].from(); 
            inputsFU.insert({fromPort, pair<string, string>(port, getPostfix(fromPort))}); 
            if(inputsCounts.find(port) == inputsCounts.end())
            {
                inputsCounts[port] = 0; 
            }
            inputsCounts[port]++; 
        }
    }
    for(const auto &edge: _DFG.edgesOut(fuDFG))
    {
        assert(getPrefix(edge.to()) == fuDFG); 
        string port = getPostfix(edge.to()); 
        if(_DFG.edgesOut(edge.to()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _DFG.edgesOut(edge.to()).size(); idx++)
            {
                const string &toPort = _DFG.edgesOut(edge.to())[idx].to(); 
                outputsFU.insert({toPort, pair<string, string>(port, getPostfix(toPort))}); 
                if(outputsCounts.find(port) == outputsCounts.end())
                {
                    outputsCounts[port] = 0; 
                }
                outputsCounts[port]++; 
            }
        }
    }
    size_t countFBInputPorts = 0; 
    size_t countFBOutputPorts = 0; 
    for(const auto &edge: _RRG.edgesIn(fuRRG))
    {
        assert(getPrefix(edge.from()) == fuRRG); 
        string port = getPostfix(edge.from()); 
        if(_RRG.edgesIn(edge.from()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _RRG.edgesIn(edge.from()).size(); idx++)
            {
                const string &fromPort = _RRG.edgesIn(edge.from())[idx].from(); 
                if(used(fromPort))
                {
                    continue; 
                }
                else if(_RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" || 
                        _RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
                {
                    countFBInputPorts++; 
                    for(const auto &vertex: inputsFU)
                    {
                        if(vertex.second.first == port)
                        {
                            if(inputsScores.find(vertex.first) == inputsScores.end())
                            {
                                inputsScores[vertex.first] = 0; 
                            }
                            inputsScores[vertex.first] = max(inputsScores[vertex.first], static_cast<size_t>(1)); 
                            if(inputsCandidates.find(port) == inputsCandidates.end())
                            {
                                inputsCandidates[port] = 0; 
                            }
                            inputsCandidates[port]++; 
                            inputsFUFound.insert(vertex.first); 
                        }
                    }
                    continue; 
                }
                string prevFU     = getPrefix(fromPort); 
                string prevPort   = getPostfix(fromPort); 
                string prevDevice = _RRG.vertex(prevFU).getAttr("device").getStr(); 
                for(const auto &vertex: inputsFU)
                {
                    auto iter = _compat.find(getPrefix(vertex.first)); 
                    if(iter == _compat.end())
                    {
                        clog << "NOrderValidator: FU " << getPrefix(vertex.first) << " not found. " << endl; 
                    }
                    assert(iter != _compat.end()); 
                    if(iter->second.find(prevDevice) == iter->second.end())
                    {
                        continue; 
                    }
                    if(vertex.second.first == port && vertex.second.second == prevPort)
                    {
                        size_t count = validate(vertex.first, fromPort, order-1, used); 
                        if(count > 0)
                        {
                            if(inputsScores.find(vertex.first) == inputsScores.end())
                            {
                                inputsScores[vertex.first] = 0; 
                            }
                            inputsScores[vertex.first] = max(inputsScores[vertex.first], count); 
                            if(inputsCandidates.find(port) == inputsCandidates.end())
                            {
                                inputsCandidates[port] = 0; 
                            }
                            inputsCandidates[port]++; 
                            inputsFUFound.insert(vertex.first); 
                        }
                    }
                }
            }
        }
    }
    for(const auto &edge: _RRG.edgesOut(fuRRG))
    {
        assert(getPrefix(edge.to()) == fuRRG); 
        string port = getPostfix(edge.to()); 
        if(_RRG.edgesOut(edge.to()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _RRG.edgesOut(edge.to()).size(); idx++)
            {
                const string &toPort = _RRG.edgesOut(edge.to())[idx].to(); 
                if(used(toPort))
                {
                    continue; 
                }
                else if(_RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" || 
                        _RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
                {
                    countFBOutputPorts++; 
                    for(const auto &vertex: outputsFU)
                    {
                        if(vertex.second.first == port)
                        {
                            if(outputsScores.find(vertex.first) == outputsScores.end())
                            {
                                outputsScores[vertex.first] = 0; 
                            }
                            outputsScores[vertex.first] = max(outputsScores[vertex.first], static_cast<size_t>(1)); 
                            if(outputsCandidates.find(port) == outputsCandidates.end())
                            {
                                outputsCandidates[port] = 0; 
                            }
                            outputsCandidates[port]++; 
                            outputsFUFound.insert(vertex.first); 
                        }
                    }
                    continue; 
                }
                string nextFU     = getPrefix(toPort); 
                string nextPort   = getPostfix(toPort); 
                string nextDevice = _RRG.vertex(nextFU).getAttr("device").getStr(); 
                for(const auto &vertex: outputsFU)
                {
                    auto iter = _compat.find(getPrefix(vertex.first)); 
                    assert(iter != _compat.end()); 
                    if(iter->second.find(nextDevice) == iter->second.end())
                    {
                        continue; 
                    }
                    if(vertex.second.first == port && vertex.second.second == nextPort)
                    {
                        size_t count = validate(vertex.first, toPort, order-1, used); 
                        if(count > 0)
                        {
                            if(outputsScores.find(vertex.first) == outputsScores.end())
                            {
                                outputsScores[vertex.first] = 0; 
                            }
                            outputsScores[vertex.first] = max(outputsScores[vertex.first], count); 
                            if(outputsCandidates.find(port) == outputsCandidates.end())
                            {
                                outputsCandidates[port] = 0; 
                            }
                            outputsCandidates[port]++; 
                            outputsFUFound.insert(vertex.first); 
                        }
                    }
                }
            }
        }
    }
    for(const auto &score: inputsScores)
    {
        result += score.second; 
    }
    for(const auto &score: outputsScores)
    {
        result += score.second; 
    }
    bool success = (inputsFU.size() <= inputsFUFound.size() && outputsFU.size() <= outputsFUFound.size()); 
    // clog << "inputsFU: " << inputsFU.size() << " vs. " << inputsFUFound.size() << endl; 
    // clog << "outputsFU: " << outputsFU.size() << " vs. " << outputsFUFound.size() << endl; 
    // clog << "inputsCounts" << endl << inputsCounts; 
    // clog << "outputsCounts" << endl << outputsCounts; 
    // clog << "inputsCandidates" << endl << inputsCandidates; 
    // clog << "outputsCandidates" << endl << outputsCandidates; 
    // if(!success)
    // {
    //     if(inputsFU.size() != inputsFUFound.size())
    //     {
    //         clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED due to unsatisfiable input ports" << endl; 
    //     }
    //     if(outputsFU.size() != outputsFUFound.size())
    //     {
    //         clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED due to unsatisfiable output ports" << endl; 
    //     }
    // }
    // if(!success)
    // {
    //     clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED; " << inputsFU.size() << " vs. " << inputsFUFound.size() << "; " << outputsFU.size() << " vs. " << outputsFUFound.size() << endl; 
    // }
    for(const auto &port: inputsCounts)
    {
        if(!success || inputsCandidates.find(port.first) == inputsCandidates.end() || inputsCandidates[port.first] < port.second)
        {
            // clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED due to insufficient input ports: " << port.first << ": " << port.second << " vs. " << inputsCandidates[port.first] << "; " << inputsCounts << "; " << inputsFU << "; " << inputsFUFound << endl; 
            success = false; 
            break; 
        }
    }
    for(const auto &port: outputsCounts)
    {
        if(!success || outputsCandidates.find(port.first) == outputsCandidates.end() || outputsCandidates[port.first] < port.second)
        {
            // clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED due to insufficient output ports: " << port.first << ": " << port.second << " vs. " << outputsCandidates[port.first] << "; " << outputsCounts << "; " << outputsFU << "; " << outputsFUFound  << endl; 
            success = false; 
            break; 
        }
    }
    return (success ? result : 0); 
}



unordered_set<string> NOrderValidator::validateSet(const std::string &vertexDFG, const std::string &vertexRRG, size_t order, const std::function<bool(const std::string &)> &used)
{
    unordered_set<string> result; 

    if(order == 0)
    {
        result.insert(vertexRRG); 
        return result; 
    }
    // Go!

    string fuDFG   = getPrefix(vertexDFG); 
    string portDFG = getPostfix(vertexDFG); 
    string fuRRG; 
    if(!portDFG.empty())
    {
        fuRRG = getPrefix(vertexRRG); 
    }
    else
    {
        fuRRG = vertexRRG; 
    }
    unordered_map<string, pair<string, string>>  inputsFU; 
    unordered_map<string, pair<string, string>>  outputsFU; 
    unordered_set<string>  inputsFUFound; 
    unordered_set<string>  outputsFUFound; 
    unordered_map<string, unordered_set<string>> inputsScores; 
    unordered_map<string, unordered_set<string>> outputsScores; 

    unordered_map<string, size_t> inputsCounts; 
    unordered_map<string, size_t> outputsCounts; 
    unordered_map<string, size_t> inputsCandidates; 
    unordered_map<string, size_t> outputsCandidates; 

    for(const auto &edge: _DFG.edgesIn(fuDFG))
    {
        assert(getPrefix(edge.from()) == fuDFG); 
        string port = getPostfix(edge.from()); 
        if(_DFG.edgesIn(edge.from()).empty())
        {
            continue; 
        }
        else
        {
            assert(_DFG.edgesIn(edge.from()).size() == 1); 
            const string &fromPort = _DFG.edgesIn(edge.from())[0].from(); 
            inputsFU[fromPort] = pair<string, string>(getPostfix(edge.from()), getPostfix(fromPort)); 
            if(inputsCounts.find(port) == inputsCounts.end())
            {
                inputsCounts[port] = 0; 
            }
            inputsCounts[port]++; 
        }
    }
    for(const auto &edge: _DFG.edgesOut(fuDFG))
    {
        assert(getPrefix(edge.to()) == fuDFG); 
        string port = getPostfix(edge.to()); 
        if(_DFG.edgesOut(edge.to()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _DFG.edgesOut(edge.to()).size(); idx++)
            {
                const string &toPort = _DFG.edgesOut(edge.to())[idx].to(); 
                outputsFU[toPort] = pair<string, string>(getPostfix(edge.to()), getPostfix(toPort)); 
                if(outputsCounts.find(port) == outputsCounts.end())
                {
                    outputsCounts[port] = 0; 
                }
                outputsCounts[port]++; 
            }
        }
    }
    size_t countFBInputPorts = 0; 
    size_t countFBOutputPorts = 0; 
    for(const auto &edge: _RRG.edgesIn(fuRRG))
    {
        assert(getPrefix(edge.from()) == fuRRG); 
        string port = getPostfix(edge.from()); 
        if(_RRG.edgesIn(edge.from()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _RRG.edgesIn(edge.from()).size(); idx++)
            {
                const string &fromPort = _RRG.edgesIn(edge.from())[idx].from(); 
                if(used(fromPort))
                {
                    continue; 
                }
                else if(_RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" || 
                        _RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
                {
                    countFBInputPorts++; 
                    for(const auto &vertex: inputsFU)
                    {
                        if(vertex.second.first == port)
                        {
                            if(inputsScores.find(vertex.first) == inputsScores.end())
                            {
                                inputsScores[vertex.first] = unordered_set<string>(); 
                            }
                            if(inputsScores[vertex.first].size() < 1)
                            {
                                inputsScores[vertex.first] = {fromPort}; 
                            }
                            if(inputsCandidates.find(port) == inputsCandidates.end())
                            {
                                inputsCandidates[port] = 0; 
                            }
                            inputsCandidates[port]++; 
                            inputsFUFound.insert(vertex.first); 
                        }
                    }
                    continue; 
                }
                string prevFU     = getPrefix(fromPort); 
                string prevPort   = getPostfix(fromPort); 
                string prevDevice = _RRG.vertex(prevFU).getAttr("device").getStr(); 
                for(const auto &vertex: inputsFU)
                {
                    auto iter = _compat.find(getPrefix(vertex.first)); 
                    if(iter == _compat.end())
                    {
                        clog << "NOrderValidator: FU " << getPrefix(vertex.first) << " not found. " << endl; 
                    }
                    assert(iter != _compat.end()); 
                    if(iter->second.find(prevDevice) == iter->second.end())
                    {
                        continue; 
                    }
                    if(vertex.second.first == port && vertex.second.second == prevPort)
                    {
                        unordered_set<string> count = validateSet(vertex.first, fromPort, order-1, used); 
                        if(count.size() > 0)
                        {
                            if(inputsScores.find(vertex.first) == inputsScores.end())
                            {
                                inputsScores[vertex.first] = unordered_set<string>(); 
                            }
                            if(inputsScores[vertex.first].size() < count.size())
                            {
                                inputsScores[vertex.first] = count; 
                            }
                            if(inputsCandidates.find(port) == inputsCandidates.end())
                            {
                                inputsCandidates[port] = 0; 
                            }
                            inputsCandidates[port]++; 
                            inputsFUFound.insert(vertex.first); 
                        }
                    }
                }
            }
        }
    }
    for(const auto &edge: _RRG.edgesOut(fuRRG))
    {
        assert(getPrefix(edge.to()) == fuRRG); 
        string port = getPostfix(edge.to()); 
        if(_RRG.edgesOut(edge.to()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _RRG.edgesOut(edge.to()).size(); idx++)
            {
                const string &toPort = _RRG.edgesOut(edge.to())[idx].to(); 
                if(used(toPort))
                {
                    continue; 
                }
                else if(_RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" || 
                        _RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
                {
                    countFBOutputPorts++; 
                    for(const auto &vertex: outputsFU)
                    {
                        if(vertex.second.first == port)
                        {
                            if(outputsScores.find(vertex.first) == outputsScores.end())
                            {
                                outputsScores[vertex.first] = unordered_set<string>(); 
                            }
                            if(outputsScores[vertex.first].size() < 1)
                            {
                                outputsScores[vertex.first] = {toPort}; 
                            }
                            if(outputsCandidates.find(port) == outputsCandidates.end())
                            {
                                outputsCandidates[port] = 0; 
                            }
                            outputsCandidates[port]++; 
                            outputsFUFound.insert(vertex.first); 
                        }
                    }
                    continue; 
                }
                string nextFU     = getPrefix(toPort); 
                string nextPort   = getPostfix(toPort); 
                string nextDevice = _RRG.vertex(nextFU).getAttr("device").getStr(); 
                for(const auto &vertex: outputsFU)
                {
                    auto iter = _compat.find(getPrefix(vertex.first)); 
                    assert(iter != _compat.end()); 
                    if(iter->second.find(nextDevice) == iter->second.end())
                    {
                        continue; 
                    }
                    if(vertex.second.first == port && vertex.second.second == nextPort)
                    {
                        unordered_set<string> count = validateSet(vertex.first, toPort, order-1, used); 
                        if(count.size() > 0)
                        {
                            if(outputsScores.find(vertex.first) == outputsScores.end())
                            {
                                outputsScores[vertex.first] = unordered_set<string>(); 
                            }
                            if(outputsScores[vertex.first].size() < count.size())
                            {
                                outputsScores[vertex.first] = count; 
                            }
                            if(outputsCandidates.find(port) == outputsCandidates.end())
                            {
                                outputsCandidates[port] = 0; 
                            }
                            outputsCandidates[port]++; 
                            outputsFUFound.insert(vertex.first); 
                        }
                    }
                }
            }
        }
    }
    for(const auto &score: inputsScores)
    {
        for(const auto &item: score.second)
        {
            result.insert(item); 
        }
    }
    for(const auto &score: outputsScores)
    {
        for(const auto &item: score.second)
        {
            result.insert(item); 
        }
    }
    bool success = (inputsFU.size() == inputsFUFound.size() && outputsFU.size() == outputsFUFound.size()); 
    // clog << "inputsFU: " << inputsFU.size() << " vs. " << inputsFUFound.size() << endl; 
    // clog << "outputsFU: " << outputsFU.size() << " vs. " << outputsFUFound.size() << endl; 
    // clog << "inputsCounts" << endl << inputsCounts; 
    // clog << "outputsCounts" << endl << outputsCounts; 
    // clog << "inputsCandidates" << endl << inputsCandidates; 
    // clog << "outputsCandidates" << endl << outputsCandidates; 
    // if(!success)
    // {
    //     if(inputsFU.size() != inputsFUFound.size())
    //     {
    //         clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED due to unsatisfiable input ports" << endl; 
    //     }
    //     if(outputsFU.size() != outputsFUFound.size())
    //     {
    //         clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED due to unsatisfiable output ports" << endl; 
    //     }
    // }
    for(const auto &port: inputsCounts)
    {
        if(!success || inputsCandidates.find(port.first) == inputsCandidates.end() || inputsCandidates[port.first] < port.second)
        {
            // clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED due to insufficient input ports" << endl; 
            success = false; 
            break; 
        }
    }
    for(const auto &port: outputsCounts)
    {
        if(!success || outputsCandidates.find(port.first) == outputsCandidates.end() || outputsCandidates[port.first] < port.second)
        {
            // clog << "NOrderValidator: " << vertexDFG << " -> " << vertexRRG << " FAILED due to insufficient output ports" << endl; 
            success = false; 
            break; 
        }
    }
    return (success ? result : unordered_set<string>()); 
}

size_t NOrderValidator::validateFast(const std::string &vertexDFG, const std::string &vertexRRG, size_t order, const std::function<bool(const std::string &)> &used)
{
    if(order == 0)
    {
        return 1; 
    }
    // Go!
    size_t result = 1; 

    string fuDFG   = getPrefix(vertexDFG); 
    string portDFG = getPostfix(vertexDFG); 
    string fuRRG; 
    if(!portDFG.empty())
    {
        fuRRG = getPrefix(vertexRRG); 
    }
    else
    {
        fuRRG = vertexRRG; 
    }
    unordered_map<string, pair<string, string>>  inputsFU; 
    unordered_map<string, pair<string, string>>  outputsFU; 
    unordered_map<string, size_t> inputsScores; 
    unordered_map<string, size_t> outputsScores; 

    unordered_map<string, size_t> inputsCounts; 
    unordered_map<string, size_t> outputsCounts; 
    unordered_map<string, size_t> inputsCandidates; 
    unordered_map<string, size_t> outputsCandidates; 

    for(const auto &edge: _DFG.edgesIn(fuDFG))
    {
        assert(getPrefix(edge.from()) == fuDFG); 
        string port = getPostfix(edge.from()); 
        if(_DFG.edgesIn(edge.from()).empty())
        {
            continue; 
        }
        else
        {
            assert(_DFG.edgesIn(edge.from()).size() == 1); 
            const string &fromPort = _DFG.edgesIn(edge.from())[0].from(); 
            inputsFU[fromPort] = pair<string, string>(getPostfix(edge.from()), getPostfix(fromPort)); 
            if(inputsCounts.find(port) == inputsCounts.end())
            {
                inputsCounts[port] = 0; 
            }
            inputsCounts[port]++; 
        }
    }
    for(const auto &edge: _DFG.edgesOut(fuDFG))
    {
        assert(getPrefix(edge.to()) == fuDFG); 
        string port = getPostfix(edge.to()); 
        if(_DFG.edgesOut(edge.to()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _DFG.edgesOut(edge.to()).size(); idx++)
            {
                const string &toPort = _DFG.edgesOut(edge.to())[idx].to(); 
                outputsFU[toPort] = pair<string, string>(getPostfix(edge.to()), getPostfix(toPort)); 
                if(outputsCounts.find(port) == outputsCounts.end())
                {
                    outputsCounts[port] = 0; 
                }
                outputsCounts[port]++; 
            }
        }
    }
    size_t countFBInputPorts = 0; 
    size_t countFBOutputPorts = 0; 
    for(const auto &edge: _RRG.edgesIn(fuRRG))
    {
        assert(getPrefix(edge.from()) == fuRRG); 
        string port = getPostfix(edge.from()); 
        if(_RRG.edgesIn(edge.from()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _RRG.edgesIn(edge.from()).size(); idx++)
            {
                const string &fromPort = _RRG.edgesIn(edge.from())[idx].from(); 
                if(used(fromPort))
                {
                    continue; 
                }
                else if(_RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" || 
                        _RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
                {
                    countFBInputPorts++; 
                    unordered_set<string> toDelete; 
                    bool added = false; 
                    for(const auto &vertex: inputsFU)
                    {
                        if(vertex.second.first == port)
                        {
                            if(inputsCandidates.find(port) == inputsCandidates.end())
                            {
                                inputsCandidates[port] = 0; 
                            }
                            inputsCandidates[port]++; 
                            toDelete.insert(vertex.first); 
                            added = true; 
                        }
                    }
                    for(const auto &vertex: toDelete)
                    {
                        inputsFU.erase(vertex); 
                    }
                    result += added; 
                    continue; 
                }
                string prevFU     = getPrefix(fromPort); 
                string prevPort   = getPostfix(fromPort); 
                string prevDevice = _RRG.vertex(prevFU).getAttr("device").getStr(); 
                unordered_set<string> toDelete; 
                for(const auto &vertex: inputsFU)
                {
                    auto iter = _compat.find(getPrefix(vertex.first)); 
                    if(iter == _compat.end())
                    {
                        clog << "NOrderValidator: FU " << getPrefix(vertex.first) << " not found. " << endl; 
                    }
                    assert(iter != _compat.end()); 
                    if(iter->second.find(prevDevice) == iter->second.end())
                    {
                        continue; 
                    }
                    if(vertex.second.first == port && vertex.second.second == prevPort)
                    {
                        size_t count = validateFast(vertex.first, fromPort, order-1, used); 
                        if(count > 0)
                        {
                            if(inputsScores.find(vertex.first) == inputsScores.end())
                            {
                                inputsScores[vertex.first] = 0; 
                            }
                            inputsScores[vertex.first] = max(inputsScores[vertex.first], count); 
                            if(inputsCandidates.find(port) == inputsCandidates.end())
                            {
                                inputsCandidates[port] = 0; 
                            }
                            inputsCandidates[port]++; 
                            toDelete.insert(vertex.first); 
                        }
                    }
                }
                for(const auto &vertex: toDelete)
                {
                    inputsFU.erase(vertex); 
                }
            }
        }
    }
    for(const auto &edge: _RRG.edgesOut(fuRRG))
    {
        assert(getPrefix(edge.to()) == fuRRG); 
        string port = getPostfix(edge.to()); 
        if(_RRG.edgesOut(edge.to()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _RRG.edgesOut(edge.to()).size(); idx++)
            {
                const string &toPort = _RRG.edgesOut(edge.to())[idx].to(); 
                if(used(toPort))
                {
                    continue; 
                }
                else if(_RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" || 
                        _RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
                {
                    countFBOutputPorts++; 
                    unordered_set<string> toDelete; 
                    bool added = false; 
                    for(const auto &vertex: outputsFU)
                    {
                        if(vertex.second.first == port)
                        {
                            if(outputsCandidates.find(port) == outputsCandidates.end())
                            {
                                outputsCandidates[port] = 0; 
                            }
                            outputsCandidates[port]++; 
                            toDelete.insert(vertex.first); 
                            added = true; 
                        }
                    }
                    for(const auto &vertex: toDelete)
                    {
                        outputsFU.erase(vertex); 
                    }
                    result += added; 
                    continue; 
                }
                string nextFU     = getPrefix(toPort); 
                string nextPort   = getPostfix(toPort); 
                string nextDevice = _RRG.vertex(nextFU).getAttr("device").getStr(); 
                unordered_set<string> toDelete; 
                for(const auto &vertex: outputsFU)
                {
                    auto iter = _compat.find(getPrefix(vertex.first)); 
                    assert(iter != _compat.end()); 
                    if(iter->second.find(nextDevice) == iter->second.end())
                    {
                        continue; 
                    }
                    if(vertex.second.first == port && vertex.second.second == nextPort)
                    {
                        size_t count = validateFast(vertex.first, toPort, order-1, used); 
                        if(count > 0)
                        {
                            if(outputsScores.find(vertex.first) == outputsScores.end())
                            {
                                outputsScores[vertex.first] = 0; 
                            }
                            outputsScores[vertex.first] = max(outputsScores[vertex.first], count); 
                            if(outputsCandidates.find(port) == outputsCandidates.end())
                            {
                                outputsCandidates[port] = 0; 
                            }
                            outputsCandidates[port]++; 
                            toDelete.insert(vertex.first); 
                        }
                    }
                }
                for(const auto &vertex: toDelete)
                {
                    outputsFU.erase(vertex); 
                }
            }
        }
    }
    for(const auto &score: inputsScores)
    {
        result += score.second; 
    }
    for(const auto &score: outputsScores)
    {
        result += score.second; 
    }
    bool success = (inputsFU.empty() && outputsFU.empty()); 
    for(const auto &port: inputsCounts)
    {
        if(!success || inputsCandidates.find(port.first) == inputsCandidates.end() || inputsCandidates[port.first] < port.second)
        {
            success = false; 
            break; 
        }
    }
    for(const auto &port: outputsCounts)
    {
        if(!success || outputsCandidates.find(port.first) == outputsCandidates.end() || outputsCandidates[port.first] < port.second)
        {
            success = false; 
            break; 
        }
    }
    return (success ? result : 0); 
}

bool NOrderValidator::validateSlow(const std::string &vertexDFG, const std::string &vertexRRG, size_t order, const std::function<bool(const std::string &)> &used)
{
    if (order == 0) {
        return true;
    }
    // Go!

    string fuDFG = getPrefix(vertexDFG);
    string portDFG = getPostfix(vertexDFG);
    string fuRRG;
    if (!portDFG.empty()) {
        fuRRG = getPrefix(vertexRRG);
    } else {
        fuRRG = vertexRRG;
    }
    unordered_map<string, pair<string, string>> inputsFU;
    unordered_map<string, pair<string, string>> outputsFU;
    unordered_map<string, unordered_set<string>> inputsCandidates;
    unordered_map<string, unordered_set<string>> outputsCandidates;
    for (const auto &edge : _DFG.edgesIn(fuDFG)) {
        assert(getPrefix(edge.from()) == fuDFG);
        if (_DFG.edgesIn(edge.from()).empty()) {
            continue;
        } else {
            assert(_DFG.edgesIn(edge.from()).size() == 1);
            const string &fromPort = _DFG.edgesIn(edge.from())[0].from();
            inputsFU[fromPort] = pair<string, string>(getPostfix(edge.from()), getPostfix(fromPort));
        }
    }
    for (const auto &edge : _DFG.edgesOut(fuDFG)) {
        assert(getPrefix(edge.to()) == fuDFG);
        if (_DFG.edgesOut(edge.to()).empty()) {
            continue;
        } else {
            for (size_t idx = 0; idx < _DFG.edgesOut(edge.to()).size(); idx++) {
                const string &toPort = _DFG.edgesOut(edge.to())[idx].to();
                outputsFU[toPort] = pair<string, string>(getPostfix(edge.to()), getPostfix(toPort));
            }
        }
    }
    size_t countFBInputPorts = 0;
    size_t countFBOutputPorts = 0;
    for (const auto &edge : _RRG.edgesIn(fuRRG)) {
        assert(getPrefix(edge.from()) == fuRRG);
        string port = getPostfix(edge.from());
        if (_RRG.edgesIn(edge.from()).empty()) {
            continue;
        } else {
            for (size_t idx = 0; idx < _RRG.edgesIn(edge.from()).size(); idx++) {
                const string &fromPort = _RRG.edgesIn(edge.from())[idx].from();
                string prevFU = getPrefix(fromPort);
                string prevPort = getPostfix(fromPort);
                // string prevDevice = _RRG.vertex(prevFU).getAttr("device").getStr();
                if (used(fromPort)) {
                    continue;
                } 
                // else if (_RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" ||
                //            _RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__") {
                //     countFBInputPorts++;
                //     for (const auto &vertex : inputsFU) {
                //         if (vertex.second.first == port) {
                //             if (inputsCandidates.find(vertex.first) == inputsCandidates.end()) {
                //                 inputsCandidates[vertex.first] = unordered_set<string>();
                //             }
                //             inputsCandidates[vertex.first].insert(fromPort);
                //         }
                //     }
                //     continue;
                // }
                for (const auto &vertex : inputsFU) {
                    auto iter = _compat.find(getPrefix(vertex.first));
                    assert(iter != _compat.end());
                    if (iter->second.find(prevFU) == iter->second.end()) {
                        continue;
                    }
                    if (vertex.second.first == port && vertex.second.second == prevPort) {
                        size_t count = validateSlow(vertex.first, fromPort, order - 1, used);
                        if (count > 0) {
                            if (inputsCandidates.find(vertex.first) == inputsCandidates.end()) {
                                inputsCandidates[vertex.first] = unordered_set<string>();
                            }
                            inputsCandidates[vertex.first].insert(fromPort);
                        }
                    }
                }
            }
        }
    }
    
    if (inputsCandidates.size() < inputsFU.size()) {
        // clog << "\nNOrderValidator: Unmatched (inputs): " << inputsFU << endl;
        // clog << " -> but " << inputsCandidates << endl;
        return false;
    }

    for (const auto &edge : _RRG.edgesOut(fuRRG)) {
        assert(getPrefix(edge.to()) == fuRRG);
        string port = getPostfix(edge.to());
        if (_RRG.edgesOut(edge.to()).empty()) {
            continue;
        } else {
            for (size_t idx = 0; idx < _RRG.edgesOut(edge.to()).size(); idx++) {
                const string &toPort = _RRG.edgesOut(edge.to())[idx].to();
                string nextFU = getPrefix(toPort);
                string nextPort = getPostfix(toPort);
                // string nextDevice = _RRG.vertex(nextFU).getAttr("device").getStr();
                if (used(toPort)) {
                    continue;
                } 
                // else if (_RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" ||
                //            _RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__") {
                //     countFBOutputPorts++;
                //     for (const auto &vertex : outputsFU) {
                //         if (vertex.second.first == port) {
                //             if (outputsCandidates.find(vertex.first) == outputsCandidates.end()) {
                //                 outputsCandidates[vertex.first] = unordered_set<string>();
                //             }
                //             outputsCandidates[vertex.first].insert(toPort);
                //         }
                //     }
                //     continue;
                // }
                for (const auto &vertex : outputsFU) {
                    auto iter = _compat.find(getPrefix(vertex.first));
                    assert(iter != _compat.end());
                    if (iter->second.find(nextFU) == iter->second.end()) {
                        continue;
                    }
                    if (vertex.second.first == port && vertex.second.second == nextPort) {
                        size_t count = validateSlow(vertex.first, toPort, order - 1, used);
                        if (count > 0) {
                            if (outputsCandidates.find(vertex.first) == outputsCandidates.end()) {
                                outputsCandidates[vertex.first] = unordered_set<string>();
                            }
                            outputsCandidates[vertex.first].insert(toPort);
                        }
                    }
                }
            }
        }
    }

    if (outputsCandidates.size() < outputsFU.size()) {
        // clog << "\nNOrderValidator: Unmatched (outputs): " << outputsFU << endl;
        // clog << " -> but " << outputsCandidates << endl;
        return false;
    }

    // Hungarian Algorithm
    unordered_map<string, string> matchTable;
    function<bool(const string &, unordered_map<string, unordered_set<string>> &, unordered_set<string> &)> match;
    match = [&](const string &name, unordered_map<string, unordered_set<string>> &candidates, unordered_set<string> &visited) -> bool {
        for (const auto &toTry : candidates[name]) {
            if (visited.find(toTry) == visited.end()) {
                visited.insert(toTry);
                if (matchTable.find(toTry) == matchTable.end() || match(matchTable[toTry], candidates, visited)) {
                    matchTable[toTry] = name;
                    return true;
                }
            }
        }
        return false;
    };
    size_t countMatched = 0;
    for (const auto &port : inputsCandidates) {
        unordered_set<string> visited;
        size_t matched = match(port.first, inputsCandidates, visited);
        countMatched += matched;
        // if(matched == 0)
        // {
        //     clog << "\nNOrderValidator: Unmatched (inputs): " << port.first << endl;
        //     clog << inputsCandidates << endl;
        // }
    }
    for (const auto &port : outputsCandidates) {
        unordered_set<string> visited;
        size_t matched = match(port.first, outputsCandidates, visited);
        countMatched += matched;
        // if(matched == 0)
        // {
        //     clog << "\nNOrderValidator: Unmatched (outputs): " << port.first << endl;
        //     clog << outputsCandidates << endl;
        // }
    }
    if (countMatched >= inputsFU.size() + outputsFU.size()) {
        assert(countMatched == inputsFU.size() + outputsFU.size());
        return true;
    }

    return false;
}

bool NOrderValidator::validateShow(const std::string &vertexDFG, const std::string &vertexRRG, size_t order, const std::function<bool(const std::string &)> &used)
{
    if(order == 0)
    {
        return true; 
    }
    // Go!

    string fuDFG   = getPrefix(vertexDFG); 
    string portDFG = getPostfix(vertexDFG); 
    string fuRRG; 
    if(!portDFG.empty())
    {
        fuRRG = getPrefix(vertexRRG); 
    }
    else
    {
        fuRRG = vertexRRG; 
    }
    unordered_map<string, pair<string, string>>  inputsFU; 
    unordered_map<string, pair<string, string>>  outputsFU; 
    unordered_map<string, unordered_set<string>> inputsCandidates; 
    unordered_map<string, unordered_set<string>> outputsCandidates; 
    for(const auto &edge: _DFG.edgesIn(fuDFG))
    {
        assert(getPrefix(edge.from()) == fuDFG); 
        if(_DFG.edgesIn(edge.from()).empty())
        {
            continue; 
        }
        else
        {
            assert(_DFG.edgesIn(edge.from()).size() == 1); 
            const string &fromPort = _DFG.edgesIn(edge.from())[0].from(); 
            inputsFU[fromPort] = pair<string, string>(getPostfix(edge.from()), getPostfix(fromPort)); 
        }
    }
    for(const auto &edge: _DFG.edgesOut(fuDFG))
    {
        assert(getPrefix(edge.to()) == fuDFG); 
        if(_DFG.edgesOut(edge.to()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _DFG.edgesOut(edge.to()).size(); idx++)
            {
                const string &toPort = _DFG.edgesOut(edge.to())[idx].to(); 
                outputsFU[toPort] = pair<string, string>(getPostfix(edge.to()), getPostfix(toPort)); 
            }
        }
    }
    size_t countFBInputPorts = 0; 
    size_t countFBOutputPorts = 0; 
    for(const auto &edge: _RRG.edgesIn(fuRRG))
    {
        assert(getPrefix(edge.from()) == fuRRG); 
        string port = getPostfix(edge.from()); 
        if(_RRG.edgesIn(edge.from()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _RRG.edgesIn(edge.from()).size(); idx++)
            {
                const string &fromPort  = _RRG.edgesIn(edge.from())[idx].from(); 
                string       prevFU     = getPrefix(fromPort); 
                string       prevPort   = getPostfix(fromPort); 
                string       prevDevice = _RRG.vertex(prevFU).getAttr("device").getStr(); 
                if(used(fromPort))
                {
                    continue; 
                }
                else if(_RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" || 
                        _RRG.vertex(fromPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
                {
                    countFBInputPorts++; 
                    for(const auto &vertex: inputsFU)
                    {
                        if(vertex.second.first == port)
                        {
                            if(inputsCandidates.find(vertex.first) == inputsCandidates.end())
                            {
                                inputsCandidates[vertex.first] = unordered_set<string>(); 
                            }
                            inputsCandidates[vertex.first].insert(fromPort); 
                        }
                    }
                    continue; 
                }
                for(const auto &vertex: inputsFU)
                {
                    auto iter = _compat.find(getPrefix(vertex.first)); 
                    assert(iter != _compat.end()); 
                    if(iter->second.find(prevDevice) == iter->second.end())
                    {
                        continue; 
                    }
                    if(vertex.second.first == port && vertex.second.second == prevPort)
                    {
                        size_t count = validateSlow(vertex.first, fromPort, order-1, used); 
                        if(count > 0)
                        {
                            if(inputsCandidates.find(vertex.first) == inputsCandidates.end())
                            {
                                inputsCandidates[vertex.first] = unordered_set<string>(); 
                            }
                            inputsCandidates[vertex.first].insert(fromPort); 
                        }
                    }
                }
            }
        }
    }
    for(const auto &edge: _RRG.edgesOut(fuRRG))
    {
        assert(getPrefix(edge.to()) == fuRRG); 
        string port = getPostfix(edge.to()); 
        if(_RRG.edgesOut(edge.to()).empty())
        {
            continue; 
        }
        else
        {
            for(size_t idx = 0; idx < _RRG.edgesOut(edge.to()).size(); idx++)
            {
                const string &toPort     = _RRG.edgesOut(edge.to())[idx].to(); 
                string        nextFU     = getPrefix(toPort); 
                string        nextPort   = getPostfix(toPort); 
                string        nextDevice = _RRG.vertex(nextFU).getAttr("device").getStr(); 
                if(used(toPort))
                {
                    continue; 
                }
                else if(_RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_INPUT_PORT__" || 
                        _RRG.vertex(toPort).getAttr("type").getStr() == "__TOP_OUTPUT_PORT__")
                {
                    countFBOutputPorts++; 
                    for(const auto &vertex: outputsFU)
                    {
                        if(vertex.second.first == port)
                        {
                            if(outputsCandidates.find(vertex.first) == outputsCandidates.end())
                            {
                                outputsCandidates[vertex.first] = unordered_set<string>(); 
                            }
                            outputsCandidates[vertex.first].insert(toPort); 
                        }
                    }
                    continue; 
                }
                for(const auto &vertex: outputsFU)
                {
                    auto iter = _compat.find(getPrefix(vertex.first)); 
                    assert(iter != _compat.end()); 
                    if(iter->second.find(nextDevice) == iter->second.end())
                    {
                        continue; 
                    }
                    if(vertex.second.first == port && vertex.second.second == nextPort)
                    {
                        size_t count = validateSlow(vertex.first, toPort, order-1, used); 
                        if(count > 0)
                        {
                            if(outputsCandidates.find(vertex.first) == outputsCandidates.end())
                            {
                                outputsCandidates[vertex.first] = unordered_set<string>(); 
                            }
                            outputsCandidates[vertex.first].insert(toPort); 
                        }
                    }
                }
            }
        }
    }

    clog << "NOrderValidator: validateShow: " << endl; 
    clog << " -> pattern inputs: " << endl; 
    clog << inputsFU  << endl; 
    clog << " -> pattern outputs: " << endl; 
    clog << outputsFU << endl; 

    if(inputsCandidates.size() < inputsFU.size())
    {
        // clog << "\nNOrderValidator: Unmatched (inputs): " << inputsFU << endl; 
        clog << " -> but too few inputs candidates " << inputsCandidates << endl; 
    }
    if(outputsCandidates.size() < outputsFU.size())
    {
        // clog << "\nNOrderValidator: Unmatched (outputs): " << outputsFU << endl; 
        clog << " -> but too few outputs candidates " << outputsCandidates << endl; 
    }
    if(inputsCandidates.size() < inputsFU.size() || outputsCandidates.size() < outputsFU.size())
    {
        return false; 
    }

    // Hungarian Algorithm
    unordered_map<string, string> matchTable; 
    function<bool(const string &, unordered_map<string, unordered_set<string>> &, unordered_set<string> &)> match;
    match = [&](const string &name, unordered_map<string, unordered_set<string>> &candidates, unordered_set<string> &visited) -> bool {
        for(const auto &toTry: candidates[name])
        {
            if(visited.find(toTry) == visited.end())
            {
                visited.insert(toTry); 
                if(matchTable.find(toTry) == matchTable.end() || match(matchTable[toTry], candidates, visited))
                {
                    matchTable[toTry] = name; 
                    return true; 
                }
            }
        }
        return false; 
    }; 
    size_t countMatched = 0; 
    for(const auto &port: inputsCandidates)
    {
        unordered_set<string> visited; 
        size_t matched = match(port.first, inputsCandidates, visited); 
        countMatched += matched; 
        // if(matched == 0)
        // {
        //     clog << "\nNOrderValidator: Unmatched (inputs): " << port.first << endl; 
        //     clog << inputsCandidates << endl; 
        // }
    }
    for(const auto &port: outputsCandidates)
    {
        unordered_set<string> visited; 
        size_t matched = match(port.first, outputsCandidates, visited); 
        countMatched += matched; 
        // if(matched == 0)
        // {
        //     clog << "\nNOrderValidator: Unmatched (outputs): " << port.first << endl; 
        //     clog << outputsCandidates << endl; 
        // }
    }

    if(countMatched >= inputsFU.size() + outputsFU.size())
    {
        assert(countMatched == inputsFU.size() + outputsFU.size()); 
        return true; 
    }

    clog << "NOrderValidator: " << "Match table: " << endl; 
    clog << matchTable; 

    return false; 
}
}
