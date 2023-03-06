#include "libPassModule2DFG.h"

using namespace std; 
using namespace llvm; 

namespace FastCGRA
{

char PassModule2DFG::ID = 0; 
static RegisterPass<PassModule2DFG> XPassModule2DFG("module2dfg", "Convert a module to a DFG. "); 

HyperGraph PassModule2DFG::genGraph4BasicBlock(llvm::BasicBlock *block, const std::unordered_set<std::string> &deletedOps)
{
    HyperGraph result(num2str(&(*(block)))); 
    unordered_set<string> insides; 
    unordered_map<string, string> outsides; 
    
    auto addPort = [&, this](const std::string &vertexName, const std::string &opType){
        if(deletedOps.find(getPrefix(opType)) != deletedOps.end())
        {
            return false; 
        }

        result.addVertex(Vertex(vertexName, {{"optype", opType}, })); 

        return true; 
    }; 
    
    auto addOpVertex = [&, this](const std::string &vertexName, const std::string &opType){
        if(deletedOps.find(opType) != deletedOps.end())
        {
            return false; 
        }
        result.addVertex(Vertex(vertexName, {{"optype", opType}, })); 
        if(_varAllocaMap.find(vertexName) != _varAllocaMap.end())
        {
            assert(_varAllocaTypeMap.find(vertexName) != _varAllocaTypeMap.end()); 
            assert(_varAllocaSizeMap.find(vertexName) != _varAllocaSizeMap.end()); 
            result.vertex(vertexName).setAttr("datatype", Attribute(_varAllocaTypeMap[vertexName])); 
            result.vertex(vertexName).setAttr("arraysize", Attribute(static_cast<int>(_varAllocaSizeMap[vertexName]))); 
        }
        else if(_varGlobalMap.find(vertexName) != _varGlobalMap.end())
        {
            assert(_varGlobalTypeMap.find(vertexName) != _varGlobalTypeMap.end()); 
            result.vertex(vertexName).setAttr("datatype", Attribute(_varGlobalTypeMap[vertexName])); 
            result.vertex(vertexName).setAttr("arraysize", Attribute(static_cast<int>(_varGlobalSizeMap[vertexName]))); 
        }
        else if(_varPhiMap.find(vertexName) != _varPhiMap.end())
        {
            assert(_varPhiFromsMap.find(vertexName) != _varPhiFromsMap.end()); 
            assert(_varPhiValuesMap.find(vertexName) != _varPhiValuesMap.end()); 
            vector<string> froms, values; 
            for(const auto &elem: _varPhiFromsMap[vertexName])
            {
                froms.push_back(this->getOpVertexName(elem)); 
            }
            for(const auto &elem: _varPhiValuesMap[vertexName])
            {
                values.push_back(this->getOpVertexName(elem)); 
            }
            result.vertex(vertexName).setAttr("blocks", Attribute(froms)); 
            result.vertex(vertexName).setAttr("values", Attribute(values)); 
        }
        else if(_varBrMap.find(vertexName) != _varBrMap.end())
        {
            assert(_varBrFlagMap.find(vertexName) != _varBrFlagMap.end()); 
            assert(_varBrBlocksMap.find(vertexName) != _varBrBlocksMap.end()); 
            string flag = _varBrFlagMap[vertexName].empty() ? "" : this->getOpVertexName(_varBrFlagMap[vertexName]); 
            vector<string> blocks; 
            for(const auto &elem: _varBrBlocksMap[vertexName])
            {
                blocks.push_back(this->getOpVertexName(elem)); 
            }
            result.vertex(vertexName).setAttr("flag", Attribute(flag)); 
            result.vertex(vertexName).setAttr("blocks", Attribute(blocks)); 
        }

        return true; 
    }; 
    
    for(BasicBlock::iterator iterInstr = block->begin(); iterInstr != block->end(); iterInstr++)
    {
        string opName = iterInstr->hasName() ? iterInstr->getName().str() : num2str(&(*(iterInstr))); 
        string vertexName = _varMapRev[opName]; 
        insides.insert(vertexName); 
    }

    for(BasicBlock::iterator iterInstr = block->begin(); iterInstr != block->end(); iterInstr++)
    {
        string opName = iterInstr->hasName() ? iterInstr->getName().str() : num2str(&(*(iterInstr))); 
        assert(_slotsTypes.find(opName) != _slotsTypes.end()); 
        assert(_varMapRev.find(opName) != _varMapRev.end()); 
        assert(_operands.find(opName) != _operands.end()); 
        string opType = _slotsTypes[opName]; 
        string vertexName = _varMapRev[opName];
        if(result.vertices().find(vertexName) == result.vertices().end())
        {
            bool added = addOpVertex(vertexName, opType); 
            if(!added)
            {
                continue; 
            }
            if(outsides.find(vertexName) != outsides.end())
            {
                result.vertex(vertexName).setAttr("original", outsides[vertexName]); 
            }
        }
        for(size_t idx = 0; idx < _operands[opName].size(); idx++)
        {
            const string &elem = _operands[opName][idx]; 
            string operandType; 
            string opVertexName; 
            Value *ptr = reinterpret_cast<Value *>(str2num<void *>(elem)); 
            if(_slotsTypes.find(elem) == _slotsTypes.end())
            {
                assert(dyn_cast<ConstantPointerNull>(ptr)); 
                opVertexName = "nullptr"; 
                operandType = "nullptr"; 
            }
            else
            {
                assert(_slotsTypes.find(elem) != _slotsTypes.end()); 
                operandType = _slotsTypes[elem]; 
                opVertexName = this->getOpVertexName(elem); 
            }
            if(insides.find(opVertexName) == insides.end())
            {
                string tmpName = string("input_") + opVertexName; 
                outsides[tmpName] = opVertexName; 
                opVertexName = tmpName; 
                operandType = "input"; 
            }
            if(result.vertices().find(opVertexName) == result.vertices().end())
            {
                bool added = addOpVertex(opVertexName, operandType); 
                if(!added)
                {
                    continue; 
                }
            }
            if(result.vertices().find(opVertexName + ".out0") == result.vertices().end())
            {
                bool added = addPort(opVertexName + ".out0", operandType + ".out0"); 
                if(added)
                {
                    result.addNet(Net(vector<string>{opVertexName, opVertexName + ".out0", })); 
                }
            }
            if(result.vertices().find(vertexName + ".in" + num2str(idx)) == result.vertices().end())
            {
                bool added = addPort(vertexName + ".in" + num2str(idx), opType + ".in" + num2str(idx)); 
                if(added)
                {
                    result.addNet(Net(vector<string>{vertexName + ".in" + num2str(idx), vertexName, })); 
                }
            }
            result.addNet(Net(vector<string>{opVertexName + ".out0", vertexName + ".in" + num2str(idx), })); 
        }
    }

    return result; 
}

HyperGraph PassModule2DFG::_passRemoveUnneeded(const HyperGraph &graph)
{
    HyperGraph result(graph.name()); 
    unordered_set<string> deletedTypes = {"br", "block", "call", }; 

    unordered_map<string, size_t> countRefs; 
    unordered_map<string, bool> deletedVertices; 
    for(const auto &vertex: graph.vertices())
    {
        countRefs[vertex.first] = 0; 
        deletedVertices[vertex.first] = false; 
    }
    for(const auto &vertex: graph.vertices())
    {
        for(const auto &netIn: graph.netsIn(vertex.first))
        {
            if(getPrefix(vertex.first) != getPrefix(netIn.source()))
            {
                countRefs[netIn.source()]++; 
            }
        }
    }
    
    function<void(const string &)> deleteTree = [&](const string &current){
        if(countRefs[current])
        {
            countRefs[current]--; 
        }
        if(!countRefs[current] && !deletedVertices[current])
        {
            deletedVertices[current] = true; 
            for(const auto &netIn: graph.netsIn(current))
            {
                deleteTree(netIn.source()); 
            }
        }
    }; 

    // Delete unneeded vertices
    for(const auto &vertex: graph.vertices())
    {
        string type = getPrefix(vertex.second.getAttr("optype").asStr()); 
        if(deletedTypes.find(type) != deletedTypes.end() && !deletedVertices[vertex.first])
        {
            deleteTree(vertex.first); 
        }
    }

    for(const auto &vertex: graph.vertices())
    {
        if(!deletedVertices[vertex.first])
        {
            result.addVertex(vertex.second); 
        }
    }
    for(const auto &vertex: graph.vertices())
    {
        for(const auto &net: graph.netsOut(vertex.first))
        {
            bool allin = true; 
            for(const auto &node: net.nodes())
            {
                if(result.vertices().find(node) == result.vertices().end())
                {
                    allin = false; 
                    break; 
                }
            }
            if(allin)
            {
                result.addNet(net); 
            }
        }
    }

    return result;
}

HyperGraph PassModule2DFG::_passRemovePhi(const HyperGraph &graph)
{
    auto getPhi = [](const HyperGraph &graph){
        string vertexToDelete = ""; 
        for(const auto &vertex: graph.vertices())
        {
            const string &type = vertex.second.getAttr("optype").asStr(); 
            if(type == "phi")
            {
                vertexToDelete = vertex.first; 
                break; 
            }
        }
        return vertexToDelete; 
    }; 

    auto deleteOnePhi = [](const HyperGraph &graph, const string &phi){
        HyperGraph result(graph.name());

        unordered_map<string, size_t> countRefs; 
        unordered_map<string, bool> deletedVertices; 
        for(const auto &vertex: graph.vertices())
        {
            countRefs[vertex.first] = 0; 
            deletedVertices[vertex.first] = false; 
        }
        for(const auto &vertex: graph.vertices())
        {
            for(const auto &netIn: graph.netsIn(vertex.first))
            {
                if(getPrefix(vertex.first) != getPrefix(netIn.source()))
                {
                    countRefs[netIn.source()]++; 
                }
            }
        }
        
        function<void(const string &)> deleteTree = [&](const string &current){
            if(countRefs[current])
            {
                countRefs[current]--; 
            }
            if(!countRefs[current] && !deletedVertices[current])
            {
                deletedVertices[current] = true; 
                for(const auto &netIn: graph.netsIn(current))
                {
                    deleteTree(netIn.source()); 
                }
            }
        }; 

        pair<string, Vertex> vertex = *graph.vertices().find(phi); 
        string nameIn0 = vertex.first + ".in0"; 
        string nameIn1 = vertex.first + ".in1"; 
        string nameOut0 = vertex.first + ".out0"; 
        // Identify the branch that comes from the same block (nameIn1)
        string blockFromIn0 = graph.vertex(vertex.first).getAttr("blocks").asArr()[0]; 
        string blockFromIn1 = graph.vertex(vertex.first).getAttr("blocks").asArr()[1]; 
        if(blockFromIn0 == string("block_") + graph.attr("__name__").asStr())
        {
            assert(blockFromIn1 != string("block_") + graph.attr("__name__").asStr()); 
            nameIn0 = vertex.first + ".in1"; 
            nameIn1 = vertex.first + ".in0"; 
        }

        vector<Net> addedNets; 
        if(graph.vertices().find(nameOut0) == graph.vertices().end())
        {
            deleteTree(nameIn0); 
            deletedVertices[vertex.first] = true; 
            deletedVertices[nameIn1] = true; 
        }
        else
        {
            for(size_t idx = 0; idx < graph.netsOut(nameOut0).size(); idx++)
            {
                const vector<string> &prevNodes = graph.netsOut(nameOut0)[idx].nodes(); 
                assert(graph.netsIn(nameIn1).size() == 1); 
                vector<string> nodes = {graph.netsIn(nameIn1)[0].source(), }; 
                for(size_t jdx = 1; jdx < prevNodes.size(); jdx++)
                {
                    nodes.push_back(prevNodes[jdx]); 
                }
                addedNets.push_back(Net(nodes)); 
            }
            deleteTree(nameIn0); 
            deletedVertices[vertex.first] = true; 
            deletedVertices[nameIn1] = true; 
            deletedVertices[nameOut0] = true; 
        }

        for(const auto &vertex: graph.vertices())
        {
            if(!deletedVertices[vertex.first])
            {
                result.addVertex(vertex.second); 
            }
        }
        for(const auto &vertex: graph.vertices())
        {
            for(const auto &net: graph.netsOut(vertex.first))
            {
                bool allin = true; 
                for(const auto &node: net.nodes())
                {
                    if(result.vertices().find(node) == result.vertices().end())
                    {
                        allin = false; 
                        break; 
                    }
                }
                if(allin)
                {
                    result.addNet(net); 
                }
            }
        }
        // Add nets related to phi nodes
        for(const auto &net: addedNets)
        {
            bool allin = true; 
            for(const auto &node: net.nodes())
            {
                if(result.vertices().find(node) == result.vertices().end())
                {
                    allin = false; 
                    break; 
                }
            }
            if(allin)
            {
                result.addNet(net); 
            }
        }

        return result; 
    }; 
    
    HyperGraph result(graph);
    
    string phi = getPhi(result); 
    while(phi.size() > 0)
    {
        result = deleteOnePhi(result, phi); 
        phi = getPhi(result); 
    }

    return result; 
}

// HyperGraph PassModule2DFG::_passRemovePhi(const HyperGraph &graph)
// {
//     HyperGraph result(graph.name());

//     unordered_map<string, size_t> countRefs; 
//     unordered_map<string, bool> deletedVertices; 
//     for(const auto &vertex: graph.vertices())
//     {
//         countRefs[vertex.first] = 0; 
//         deletedVertices[vertex.first] = false; 
//     }
//     for(const auto &vertex: graph.vertices())
//     {
//         for(const auto &netIn: graph.netsIn(vertex.first))
//         {
//             if(getPrefix(vertex.first) != getPrefix(netIn.source()))
//             {
//                 countRefs[netIn.source()]++; 
//             }
//         }
//     }
    
//     function<void(const string &)> deleteTree = [&](const string &current){
//         if(countRefs[current])
//         {
//             countRefs[current]--; 
//         }
//         if(!countRefs[current] && !deletedVertices[current])
//         {
//             deletedVertices[current] = true; 
//             for(const auto &netIn: graph.netsIn(current))
//             {
//                 deleteTree(netIn.source()); 
//             }
//         }
//     }; 

//     // Merge phi nodes, WARNING
//     // -> Only support a chain of 2 phi nodes
//     vector<Net> addedNets; 
//     for(const auto &vertex: graph.vertices())
//     {
//         const string &type = vertex.second.getAttr("optype").asStr(); 
//         if(type == "phi" && !deletedVertices[vertex.first])
//         {
//             string nameIn0 = vertex.first + ".in0"; 
//             string nameIn1 = vertex.first + ".in1"; 
//             string nameOut0 = vertex.first + ".out0"; 
//             // Identify the branch that comes from the same block
//             string blockFromIn0 = graph.vertex(vertex.first).getAttr("blocks").asArr()[0]; 
//             string blockFromIn1 = graph.vertex(vertex.first).getAttr("blocks").asArr()[1]; 
//             if(blockFromIn0 == string("block_") + graph.attr("__name__").asStr())
//             {
//                 assert(blockFromIn1 != string("block_") + graph.attr("__name__").asStr()); 
//                 nameIn0 = vertex.first + ".in1"; 
//                 nameIn1 = vertex.first + ".in0"; 
//             }
//             if(!(graph.netsOut(vertex.first).size() == 1 && graph.netsOut(vertex.first)[0].sink(0) == nameOut0))
//             {
//                 errs() << "WARNING: Cannot remove a phi command:" << vertex.first << ". Skip the phi removeing step.  \n"; 
//                 return graph; 
//             }
//             if(!(graph.netsIn(nameIn0).size() == 1 && graph.netsIn(nameIn1).size() == 1 && graph.netsIn(nameIn1)[0].source().size() > 0))
//             {
//                 errs() << "WARNING: Cannot remove a phi command:" << vertex.first << ". Skip the phi removeing step.  \n"; 
//                 return graph; 
//             }
//             // assert(graph.netsOut(vertex.first).size() == 1 && graph.netsOut(vertex.first)[0].sink(0) == nameOut0); 
//             // assert(graph.netsIn(nameIn0).size() == 1 && graph.netsIn(nameIn1).size() == 1 && graph.netsIn(nameIn1)[0].source().size() > 0); 
//             if(graph.vertex(graph.netsIn(nameIn1)[0].source()).getAttr("optype").asStr() == "phi.out0")
//             {
//                 continue; 
//             }
//             for(size_t idx = 0; idx < graph.netsOut(nameOut0).size(); idx++)
//             {
//                 const vector<string> &prevNodes = graph.netsOut(nameOut0)[idx].nodes(); 
//                 for(size_t jdx = 1; jdx < prevNodes.size(); jdx++)
//                 {
//                     if(graph.vertex(prevNodes[jdx]).getAttr("optype").asStr() != "phi.in1")
//                     {
//                         addedNets.push_back(Net(graph.netsIn(nameIn1)[0].source(), prevNodes[jdx]));   
//                     }
//                     else
//                     {
//                         assert(graph.netsOut(prevNodes[jdx]).size() == 1); 
//                         const string &phiNode = graph.netsOut(prevNodes[jdx])[0].sink(0); 
//                         assert(graph.netsOut(phiNode).size() == 1); 
//                         const string &phiNodeOut0 = graph.netsOut(phiNode)[0].sink(0); 
//                         deleteTree(phiNode + ".in0"); 
//                         deletedVertices[phiNode] = true; 
//                         deletedVertices[phiNode + ".in1"] = true; 
//                         deletedVertices[phiNode + ".out0"] = true; 
//                         for(const auto &edge: graph.netsOut(phiNodeOut0))
//                         {
//                             if(!(graph.vertex(edge.sink(0)).getAttr("optype").asStr() != "phi.in1"))
//                             {
//                                 errs() << "WARNING: Cannot remove a phi command:" << vertex.first << ". Skip the phi removeing step.  \n"; 
//                                 return graph; 
//                             }
//                             // assert(graph.vertex(edge.sink(0)).getAttr("optype").asStr() != "phi.in1"); 
//                             addedNets.push_back(Net(graph.netsIn(nameIn1)[0].source(), edge.sink(0))); 
//                         }
//                     }
//                 }
//             }
//             deleteTree(nameIn0); 
//             deletedVertices[vertex.first] = true; 
//             deletedVertices[nameIn1] = true; 
//             deletedVertices[nameOut0] = true; 
//         }
//     }

//     for(const auto &vertex: graph.vertices())
//     {
//         if(!deletedVertices[vertex.first])
//         {
//             result.addVertex(vertex.second); 
//         }
//     }
//     for(const auto &vertex: graph.vertices())
//     {
//         for(const auto &net: graph.netsOut(vertex.first))
//         {
//             bool allin = true; 
//             for(const auto &node: net.nodes())
//             {
//                 if(result.vertices().find(node) == result.vertices().end())
//                 {
//                     allin = false; 
//                     break; 
//                 }
//             }
//             if(allin)
//             {
//                 result.addNet(net); 
//             }
//         }
//     }
//     // Add nets related to phi nodes, WARNING
//     for(const auto &net: addedNets)
//     {
//         bool allin = true; 
//         for(const auto &node: net.nodes())
//         {
//             if(result.vertices().find(node) == result.vertices().end())
//             {
//                 allin = false; 
//                 break; 
//             }
//         }
//         if(allin)
//         {
//             result.addNet(net); 
//         }
//     }

//     return result; 
// }

HyperGraph PassModule2DFG::_passRemoveGetelementptr(const HyperGraph &graph)
{
    HyperGraph result(graph.name());

    unordered_map<string, size_t> countRefs; 
    unordered_map<string, bool> deletedVertices; 
    for(const auto &vertex: graph.vertices())
    {
        countRefs[vertex.first] = 0; 
        deletedVertices[vertex.first] = false; 
    }
    for(const auto &vertex: graph.vertices())
    {
        for(const auto &netIn: graph.netsIn(vertex.first))
        {
            if(getPrefix(vertex.first) != getPrefix(netIn.source()))
            {
                countRefs[netIn.source()]++; 
            }
        }
    }
    
    function<void(const string &)> deleteTree = [&](const string &current){
        if(countRefs[current])
        {
            countRefs[current]--; 
        }
        if(!countRefs[current] && !deletedVertices[current])
        {
            deletedVertices[current] = true; 
            for(const auto &netIn: graph.netsIn(current))
            {
                deleteTree(netIn.source()); 
            }
        }
    }; 

    // Remove getelementptr, WARNING
    for(const auto &vertex: graph.vertices())
    {
        if(graph.edgesIn(vertex.first).size() == 1 && 
           graph.vertex(graph.edgesIn(vertex.first)[0].source()).getAttr("optype").asStr() == "getelementptr.out0")
        {
            deleteTree(graph.edgesIn(vertex.first)[0].source()); 
        }
    }
    

    for(const auto &vertex: graph.vertices())
    {
        if(!deletedVertices[vertex.first])
        {
            result.addVertex(vertex.second); 
        }
    }
    for(const auto &vertex: graph.vertices())
    {
        for(const auto &net: graph.netsOut(vertex.first))
        {
            bool allin = true; 
            for(const auto &node: net.nodes())
            {
                if(result.vertices().find(node) == result.vertices().end())
                {
                    allin = false; 
                    break; 
                }
            }
            if(allin)
            {
                result.addNet(net); 
            }
        }
    }

    return result; 
}

HyperGraph PassModule2DFG::_passRemoveUnneededStore(const HyperGraph &graph)
{
    HyperGraph result(graph.name()); 

    unordered_map<string, size_t> countRefs; 
    unordered_map<string, bool> deletedVertices; 
    for(const auto &vertex: graph.vertices())
    {
        countRefs[vertex.first] = 0; 
        deletedVertices[vertex.first] = false; 
    }
    for(const auto &vertex: graph.vertices())
    {
        for(const auto &netIn: graph.netsIn(vertex.first))
        {
            if(getPrefix(vertex.first) != getPrefix(netIn.source()))
            {
                countRefs[netIn.source()]++; 
            }
        }
    }
    
    function<void(const string &)> deleteTree = [&](const string &current){
        if(countRefs[current])
        {
            countRefs[current]--; 
        }
        if(!countRefs[current] && !deletedVertices[current])
        {
            deletedVertices[current] = true; 
            for(const auto &netIn: graph.netsIn(current))
            {
                deleteTree(netIn.source()); 
            }
        }
    }; 

    // Delete unneeded vertices
    for(const auto &vertex: graph.vertices())
    {
        size_t count = 0; 
        vector<pair<string, string>> stores; 
        for(const auto &net: graph.netsOut(vertex.first))
        {
            for(size_t idx = 1; idx < net.nodes().size(); idx++)
            {
                const string &type = graph.vertex(net.nodes()[idx]).getAttr("optype").asStr(); 
                if(type == "store.in1")
                {
                    count++; 
                    string op = getPrefix(net.nodes()[idx]); 
                    string in0 = op + ".in0"; 
                    const string &from = graph.netsIn(in0)[0].source(); 
                    assert(graph.netsIn(in0).size() == 1); 
                    stores.push_back({getPrefix(from), op}); 
                }
            }
        }
        if(count > 1)
        {
            std::sort(stores.begin(), stores.end(), [&, this](const pair<string, string> &a, const pair<string, string> &b){
                assert(_name2number.find(_finalname2name[a.first]) != _name2number.end()); 
                assert(_name2number.find(_finalname2name[b.first]) != _name2number.end()); 
                size_t ida = _name2number[_finalname2name[a.first]]; 
                size_t idb = _name2number[_finalname2name[b.first]]; 
                return ida < idb; 
            }); 
            for(size_t idx = 0; idx < count; idx++)
            {
                errs() << " -> Store operator: " << stores[idx].second << " <- " << graph.netsIn(stores[idx].second + ".in0")[0].source() << ", " << graph.netsIn(stores[idx].second + ".in1")[0].source(); 
                if(idx < count - 1)
                {
                    errs() << " -> DELETED"; 
                }
                errs() << "\n"; 
            }
            for(size_t idx = 0; idx < count - 1; idx++)
            {
                if(graph.vertices().find(stores[idx].second + ".out0") != graph.vertices().end())
                {
                    deleteTree(stores[idx].second + ".out0");
                }
                if(graph.vertices().find(stores[idx].second) != graph.vertices().end())
                {
                    deleteTree(stores[idx].second);
                }
            }
        }
    }

    for(const auto &vertex: graph.vertices())
    {
        if(!deletedVertices[vertex.first])
        {
            result.addVertex(vertex.second); 
        }
    }
    for(const auto &vertex: graph.vertices())
    {
        for(const auto &net: graph.netsOut(vertex.first))
        {
            bool allin = true; 
            for(const auto &node: net.nodes())
            {
                if(result.vertices().find(node) == result.vertices().end())
                {
                    allin = false; 
                    break; 
                }
            }
            if(allin)
            {
                result.addNet(net); 
            }
        }
    }

    return result;
}

HyperGraph PassModule2DFG::_passRemoveInputLoad(const HyperGraph &graph)
{
    HyperGraph result(graph.name());

    unordered_map<string, size_t> countRefs; 
    unordered_map<string, bool> deletedVertices; 
    for(const auto &vertex: graph.vertices())
    {
        countRefs[vertex.first] = 0; 
        deletedVertices[vertex.first] = false; 
    }
    for(const auto &vertex: graph.vertices())
    {
        for(const auto &netIn: graph.netsIn(vertex.first))
        {
            if(getPrefix(vertex.first) != getPrefix(netIn.source()))
            {
                countRefs[netIn.source()]++; 
            }
        }
    }
    
    function<void(const string &)> deleteTree = [&](const string &current){
        if(countRefs[current])
        {
            countRefs[current]--; 
        }
        if(!countRefs[current] && !deletedVertices[current])
        {
            deletedVertices[current] = true; 
            for(const auto &netIn: graph.netsIn(current))
            {
                deleteTree(netIn.source()); 
            }
        }
    }; 

    // Remove getelementptr, WARNING
    for(const auto &vertex: graph.vertices())
    {
        const string &type = vertex.second.getAttr("optype").asStr(); 
        if(type == "load.in0")
        {
            if(graph.edgesIn(vertex.first).size() > 0)
            {
                assert(graph.edgesIn(vertex.first).size() == 1); 
                const string &from = graph.edgesIn(vertex.first)[0].source(); 
                const string &frType = graph.vertex(from).getAttr("optype").asStr(); 
                if(frType == "input.out0")
                {
                    deleteTree(from); 
                }
            }
        }
    }
    

    for(const auto &vertex: graph.vertices())
    {
        if(!deletedVertices[vertex.first])
        {
            result.addVertex(vertex.second); 
        }
    }
    for(const auto &vertex: graph.vertices())
    {
        for(const auto &net: graph.netsOut(vertex.first))
        {
            bool allin = true; 
            for(const auto &node: net.nodes())
            {
                if(result.vertices().find(node) == result.vertices().end())
                {
                    allin = false; 
                    break; 
                }
            }
            if(allin)
            {
                result.addNet(net); 
            }
        }
    }

    return result; 
}

HyperGraph PassModule2DFG::_passRemoveExt(const HyperGraph &graph)
{
    unordered_set<string> exts = {"zext", "trunc", }; 

    HyperGraph result(graph.name());
    vector<Net> addedNets; 

    // Delete unneeded vertices
    for(const auto &vertex: graph.vertices())
    {
        const string &type = vertex.second.getAttr("optype").asStr(); 
        if(exts.find(getPrefix(type)) == exts.end())
        {
            result.addVertex(vertex.second); 
        }
        else if(vertex.first == getPrefix(vertex.first))
        {
            string nameIn0 = vertex.first + ".in0"; 
            string nameOut0 = vertex.first + ".out0"; 
            assert(graph.netsIn(nameIn0).size() == 1); 
            if(graph.vertices().find(nameOut0) == graph.vertices().end())
            {
                continue; 
            }
            for(size_t idx = 0; idx < graph.netsOut(nameOut0).size(); idx++)
            {
                vector<string> nodes = {graph.netsIn(nameIn0)[0].source(), }; 
                for(size_t jdx = 0; jdx < graph.netsOut(nameOut0)[idx].sizeSinks(); jdx++)
                {
                    nodes.push_back(graph.netsOut(nameOut0)[idx].sink(jdx)); 
                }
                addedNets.push_back(Net(nodes)); 
            }
        }
    }

    for(const auto &vertex: graph.vertices())
    {
        for(const auto &net: graph.netsOut(vertex.first))
        {
            bool allin = true; 
            for(const auto &node: net.nodes())
            {
                if(result.vertices().find(node) == result.vertices().end())
                {
                    allin = false; 
                    break; 
                }
            }
            if(allin)
            {
                result.addNet(net); 
            }
        }
    }
    // Add nets related to phi nodes, WARNING
    for(const auto &net: addedNets)
    {
        bool allin = true; 
        for(const auto &node: net.nodes())
        {
            if(result.vertices().find(node) == result.vertices().end())
            {
                allin = false; 
                break; 
            }
        }
        if(allin)
        {
            result.addNet(net); 
        }
    }

    return result; 

}

HyperGraph PassModule2DFG::genOpGraph4BlockGraph(const HyperGraph &graph)
{
    HyperGraph result; 

    result = _passRemoveUnneeded(graph); 
    result = _passRemovePhi(result); 
    result = _passRemoveGetelementptr(result); 
    result = _passRemoveUnneededStore(result); 
    result = _passRemoveInputLoad(result); 
    result = _passRemoveExt(result); 
    
    return result; 
}

HyperGraph PassModule2DFG::genControlGraph()
{
    HyperGraph result; 
    for(const auto &elem: _blocksOriGraphs)
    {
        string blockName = string("block_") + elem.first; 
        result.addVertex(Vertex(blockName)); 
    }
    for(const auto &elem: _blocksOriGraphs)
    {
        string blockName = string("block_") + elem.first; 
        const HyperGraph &graph = elem.second; 
        for(const auto &vertex: graph.vertices())
        {
            if(vertex.second.getAttr("optype").asStr() == "br")
            {
                const string &flag = vertex.second.getAttr("flag").asStr(); 
                const vector<string> &blocks = vertex.second.getAttr("blocks").asArr(); 
                for(size_t idx = 0; idx < blocks.size(); idx++)
                {
                    result.addNet(Net({blockName, blocks[idx]}, {{"flag", flag}, {"index", static_cast<int>(idx)}, })); 
                }
                break; 
            }
        }
    }

    return result; 
}

bool PassModule2DFG::runOnModule(llvm::Module &M)
{
    for(Module::global_iterator iterGlobal = M.global_begin(); iterGlobal != M.global_end(); iterGlobal++)
    {
        string opName = iterGlobal->hasName() ? iterGlobal->getName().str() : num2str(&(*(iterGlobal))); 
        _name2number[opName] = _name2number.size(); 
        llvm::GlobalVariable *varPtr = &(*(iterGlobal)); 
        assert(dyn_cast<GlobalValue>(varPtr)); 
        _slotsAddrs[opName] = varPtr; 
        _addr2name[num2str(varPtr)] = opName; 
        _slotsTypes[opName] = "globalvar"; 
        _types.insert(_slotsTypes[opName]); 
        _variables.push_back(opName); 
        _operands[opName] = {}; 

        string dsoLocation = varPtr->isDSOLocal() && !varPtr->isImplicitDSOLocal() ? "dso_local" : ""; 
        string isConstGlobal = varPtr->isConstant() ? "constant" : "global"; 
        Type::TypeID typeID = dyn_cast<GlobalValue>(varPtr)->getValueType()->getTypeID(); 
        pair<string, size_t> tmp = this->getTypeInfo(opName, typeID); 
        const string &typeName = tmp.first; 
        size_t allocaNum = tmp.second; 
        _varGlobal.push_back(opName); 
        _varGlobalTypes.push_back(typeName); 
        _varGlobalSizes.push_back(allocaNum); 
    }

    for(Module::iterator iterFunc = M.begin(); iterFunc != M.end(); iterFunc++)
    {
        errs() << "Function Name: " << iterFunc->getName() << "\n"; 
        
        // Get the arguments of this function
        for(Function::arg_iterator iterArg = iterFunc->arg_begin(); iterArg != iterFunc->arg_end(); iterArg++)
        {
            string opName = iterArg->hasName() ? iterArg->getName().str() : num2str(&(*(iterArg))); 
            _name2number[opName] = _name2number.size(); 
            llvm::Argument *varPtr = &(*(iterArg)); 
            _slotsAddrs[opName] = varPtr; 
            _addr2name[num2str(varPtr)] = opName; 
            _slotsTypes[opName] = "argument"; 
            _types.insert(_slotsTypes[opName]); 
            _variables.push_back(opName); 
            _operands[opName] = {}; 

            Type::TypeID typeID = varPtr->getType()->getTypeID(); 
            pair<string, size_t> tmp = this->getTypeInfo(opName, typeID); 
            const string &typeName = tmp.first; 
            size_t allocaNum = tmp.second; 
            _varArg.push_back(opName); 
            _varArgTypes.push_back(typeName); 
            _varArgSizes.push_back(allocaNum); 
        }

        // Find all blocks
        for(Function::iterator iterBlock = iterFunc->begin(); iterBlock != iterFunc->end(); iterBlock++)
        {
            string blockName = iterBlock->hasName() ? iterBlock->getName().str() : num2str(&(*(iterBlock))); 
            string varName = string("block_") + blockName; 
            _blocks[blockName] = &(*(iterBlock)); 
            _blockVarMap[varName] = blockName; 
            _blockVarMapRev[blockName] = varName; 
            _slotsAddrs[blockName] = &(*(iterBlock)); 
            _slotsTypes[blockName] = "block"; 
            _addr2name[num2str(&(*(iterBlock)))] = blockName; 
            _types.insert(_slotsTypes[blockName]); 
        }

        // Analyze blocks
        _hierarchy[iterFunc->getName().str()] = unordered_map<string, vector<string>>();
        for(Function::iterator iterBlock = iterFunc->begin(); iterBlock != iterFunc->end(); iterBlock++)
        {
            _hierarchy[iterFunc->getName().str()][num2str(&(*(iterBlock)))] = vector<string>(); 
            _block2Func[num2str(&(*(iterBlock)))] = iterFunc->getName().str(); 
            for(BasicBlock::iterator iterInstr = iterBlock->begin(); iterInstr != iterBlock->end(); iterInstr++)
            {
                string opcode = iterInstr->getOpcodeName(); 
                size_t numOperands = iterInstr->getNumOperands(); 
                Use *listOperands = iterInstr->getOperandList();
                string opName = iterInstr->hasName() ? iterInstr->getName().str() : num2str(&(*(iterInstr))); 
                _name2number[opName] = _name2number.size(); 
                _hierarchy[iterFunc->getName().str()][num2str(&(*(iterBlock)))].push_back(opName); 
                _op2Block[opName] = num2str(&(*(iterBlock))); 
                _op2Func[opName] = iterFunc->getName().str(); 
                vector<string> namesOperands(numOperands); 
                for(Use *iterUse = listOperands; iterUse < listOperands + numOperands; iterUse++)
                {
                    // Add the operand into the information if it has not been in it
                    namesOperands[iterUse - listOperands] = this->addOperand(iterUse); 
                }
                _slotsAddrs[opName] = &(*(iterInstr)); 
                _addr2name[num2str(&(*(iterInstr)))] = opName; 
                _slotsTypes[opName] = opcode; 
                _types.insert(_slotsTypes[opName]); 
                _variables.push_back(opName); 
                _operands[opName] = namesOperands; 
                if(_slotsTypes[opName] == "alloca")
                {
                    assert(dyn_cast<AllocaInst>(_slotsAddrs[opName])); 
                    Type::TypeID typeID = dyn_cast<AllocaInst>(_slotsAddrs[opName])->getAllocatedType()->getTypeID(); 
                    pair<string, size_t> tmp = this->getTypeInfo(opName, typeID); 
                    const string &typeName = tmp.first; 
                    size_t allocaNum = tmp.second; 
                    _varAlloca.push_back(opName); 
                    _varAllocaTypes.push_back(typeName); 
                    _varAllocaSizes.push_back(allocaNum); 
                }
                else if(_slotsTypes[opName] == "globalvar")
                {
                    assert(dyn_cast<GlobalValue>(_slotsAddrs[opName])); 
                    Type::TypeID typeID = dyn_cast<GlobalValue>(_slotsAddrs[opName])->getValueType()->getTypeID(); 
                    pair<string, size_t> tmp = this->getTypeInfo(opName, typeID); 
                    const string &typeName = tmp.first; 
                    size_t allocaNum = tmp.second; 
                    _varGlobal.push_back(opName); 
                    _varGlobalTypes.push_back(typeName); 
                    _varGlobalSizes.push_back(allocaNum); 
                }
                else if(_slotsTypes[opName] == "argument")
                {
                    Type::TypeID typeID = _slotsAddrs[opName]->getType()->getTypeID(); 
                    pair<string, size_t> tmp = this->getTypeInfo(opName, typeID); 
                    const string &typeName = tmp.first; 
                    size_t allocaNum = tmp.second; 
                    _varArg.push_back(opName); 
                    _varArgTypes.push_back(typeName); 
                    _varArgSizes.push_back(allocaNum); 
                }
                else if(_slotsTypes[opName] == "phi")
                { 
                    assert(dyn_cast<PHINode>(_slotsAddrs[opName])); 
                    vector<string> froms, values; 
                    const auto &arrBlocks = dyn_cast<PHINode>(_slotsAddrs[opName])->blocks(); 
                    const auto &arrIncomings = dyn_cast<PHINode>(_slotsAddrs[opName])->incoming_values(); 
                    for(const auto &block: arrBlocks)
                    {
                        froms.push_back(num2str(block)); 
                    } 
                    for(const auto &value: arrIncomings)
                    {
                        values.push_back(num2str(&(*(value)))); 
                        this->addOperand(&value); 
                    } 
                    _varPhi.push_back(opName); 
                    _varPhiFroms.push_back(froms); 
                    _varPhiValues.push_back(values); 
                }
                else if(_slotsTypes[opName] == "br")
                { 
                    assert(dyn_cast<BranchInst>(_slotsAddrs[opName])); 
                    string flag; 
                    vector<string> blocks; 
                    if(dyn_cast<BranchInst>(_slotsAddrs[opName])->isConditional())
                    {
                        flag = num2str(dyn_cast<BranchInst>(_slotsAddrs[opName])->getCondition()); 
                    }
                    for(size_t idx = 0; idx < dyn_cast<BranchInst>(_slotsAddrs[opName])->getNumSuccessors(); idx++)
                    {
                        blocks.push_back(num2str(dyn_cast<BranchInst>(_slotsAddrs[opName])->getSuccessor(idx))); 
                    }
                    _varBr.push_back(opName); 
                    _varBrFlag.push_back(flag); 
                    _varBrBlocks.push_back(blocks); 
                }
                // else
                // {
                //     errs() << "NOTE: No additional process: " << num2str(&(*(iterInstr))) << ":" << iterInstr->getType()->getTypeID() << "\n";  
                // }
            } 
        }
    }

    unordered_map<string, size_t> constCounts; 
    unordered_map<string, size_t> varCounts; 
    for(const auto &elem: _constants)
    {
        const string &type = _slotsTypes[elem]; 
        size_t count = (constCounts.find(type) == constCounts.end() ? 0 : constCounts[type]); 
        string name = type + num2str(count); 
        constCounts[type] = count + 1; 
        _constNames.push_back(name); 
        _constMap[name] = elem; 
        _constMapRev[elem] = name; 
        _finalname2name[name] = elem; 
        _name2finalname[elem] = name; 
    }
    for(const auto &elem: _variables)
    {
        const string &type = _slotsTypes[elem]; 
        size_t count = (varCounts.find(type) == varCounts.end() ? 0 : varCounts[type]); 
        string name = type + num2str(count); 
        varCounts[type] = count + 1; 
        _varNames.push_back(name); 
        _varMap[name] = elem; 
        _varMapRev[elem] = name; 
        _finalname2name[name] = elem; 
        _name2finalname[elem] = name; 
    }
    for(size_t idx = 0; idx < _varAlloca.size(); idx++)
    {
        assert(_varMapRev.find(_varAlloca[idx]) != _varMapRev.end()); 
        string name = _varMapRev[_varAlloca[idx]]; 
        _varAllocaNames.push_back(name); 
        _varAllocaMap[name] = _varAlloca[idx]; 
        _varAllocaTypeMap[name] = _varAllocaTypes[idx]; 
        _varAllocaSizeMap[name] = _varAllocaSizes[idx]; 
        _varAllocaMapRev[_varAlloca[idx]] = name; 
    }
    for(size_t idx = 0; idx < _varGlobal.size(); idx++)
    {
        assert(_varMapRev.find(_varGlobal[idx]) != _varMapRev.end()); 
        string name = _varMapRev[_varGlobal[idx]]; 
        _varGlobalNames.push_back(name); 
        _varGlobalMap[name] = _varGlobal[idx]; 
        _varGlobalTypeMap[name] = _varGlobalTypes[idx]; 
        _varGlobalSizeMap[name] = _varGlobalSizes[idx]; 
        _varGlobalMapRev[_varGlobal[idx]] = name; 
    }
    for(size_t idx = 0; idx < _varArg.size(); idx++)
    {
        assert(_varMapRev.find(_varArg[idx]) != _varMapRev.end()); 
        string name = _varMapRev[_varArg[idx]]; 
        _varArgNames.push_back(name); 
        _varArgMap[name] = _varArg[idx]; 
        _varArgTypeMap[name] = _varArgTypes[idx]; 
        _varArgSizeMap[name] = _varArgSizes[idx]; 
        _varArgMapRev[_varArg[idx]] = name; 
    }
    for(size_t idx = 0; idx < _varPhi.size(); idx++)
    {
        assert(_varMapRev.find(_varPhi[idx]) != _varMapRev.end()); 
        string name = _varMapRev[_varPhi[idx]]; 
        _varPhiNames.push_back(name); 
        _varPhiMap[name] = _varPhi[idx]; 
        _varPhiFromsMap[name] = _varPhiFroms[idx]; 
        _varPhiValuesMap[name] = _varPhiValues[idx]; 
        _varPhiMapRev[_varPhi[idx]] = name; 
    }
    for(size_t idx = 0; idx < _varBr.size(); idx++)
    {
        assert(_varMapRev.find(_varBr[idx]) != _varMapRev.end()); 
        string name = _varMapRev[_varBr[idx]]; 
        _varBrNames.push_back(name); 
        _varBrMap[name] = _varBr[idx]; 
        _varBrFlagMap[name] = _varBrFlag[idx]; 
        _varBrBlocksMap[name] = _varBrBlocks[idx]; 
        _varBrMapRev[_varBr[idx]] = name; 
    }
    
    for(Module::iterator iterFunc = M.begin(); iterFunc != M.end(); iterFunc++)
    {
        for(Function::iterator iterBlock = iterFunc->begin(); iterBlock != iterFunc->end(); iterBlock++)
        {
            string blockName = iterBlock->hasName() ? iterBlock->getName().str() : num2str(&(*(iterBlock))); 
            // unordered_set<string> deletedOps = {"br", "block", "phi", "icmp", "call", "ret", }; 
            _blocksOriGraphs[blockName] = this->genGraph4BasicBlock(&(*(iterBlock)), {}); 
            _blocksOpGraphs[blockName] = this->genOpGraph4BlockGraph(_blocksOriGraphs[blockName]); 
        }
    }

    _graphControl = this->genControlGraph(); 

    // errs() << "Value Types: \n -> ";  
    // for(const auto &elem: _types)
    // {
    //     errs() << elem << ", "; 
    // }
    // errs() << "\n\n"; 
    // errs() << "Constants: \n";  
    // for(const auto &elem: _constNames)
    // {
    //     errs() << " -> " << elem << "(" << _constMap[elem] << ")" << ": " << (_slotsValues.find(_constMap[elem]) != _slotsValues.end() ? _slotsValues[_constMap[elem]] : "") << "\n"; 
    // }
    // errs() << "\n"; 
    // errs() << "Variables: \n";  
    // for(const auto &elem: _varNames)
    // {
    //     errs() << " -> " << elem << "(" << _varMap[elem] << ")" << "\n"; 
    // }
    // errs() << "\n"; 
    
    // for(Module::iterator iterFunc = M.begin(); iterFunc != M.end(); iterFunc++)
    // {
    //     for(Function::iterator iterBlock = iterFunc->begin(); iterBlock != iterFunc->end(); iterBlock++)
    //     {
    //         errs() << " -> Block Name: " << (iterBlock->hasName() ? iterBlock->getName().str() : num2str(&(*(iterBlock)))) << "\n"; 
    //         for(BasicBlock::iterator iterInstr = iterBlock->begin(); iterInstr != iterBlock->end(); iterInstr++)
    //         {
    //             string opName = iterInstr->hasName() ? iterInstr->getName().str() : num2str(&(*(iterInstr))); 
    //             assert(_varMapRev.find(opName) != _varMapRev.end()); 
    //             assert(_operands.find(opName) != _operands.end()); 
    //             errs() << " -> -> Operator: " << _varMapRev[opName]; 
    //             if(_varAllocaMapRev.find(opName) != _varAllocaMapRev.end())
    //             {
    //                 errs() << "(" << _varAllocaTypeMap[_varAllocaMapRev[opName]] << "," << _varAllocaSizeMap[_varAllocaMapRev[opName]] << ")" << ", "; 
    //             }
    //             errs() << ": "; 
    //             for(const auto &elem: _operands[opName])
    //             {
    //                 assert(_slotsTypes.find(elem) != _slotsTypes.end()); 
    //                 if(_constMapRev.find(elem) != _constMapRev.end())
    //                 {
    //                     errs() << _constMapRev[elem]; 
    //                 }
    //                 else if(_varMapRev.find(elem) != _varMapRev.end())
    //                 {
    //                     errs() << _varMapRev[elem]; 
    //                 }
    //                 else
    //                 {
    //                     errs() << _slotsTypes[elem] + "_" + elem; 
    //                 }
    //                 if(_constMapRev.find(elem) != _constMapRev.end())
    //                 {
    //                     errs() << ":" << _slotsValues[elem] << ", "; 
    //                 }
    //                 else
    //                 {
    //                     errs() << "(" << elem << ")" << ", "; 
    //                 }
    //             }
    //             errs() << "\n"; 
    //         }
    //     }
    // }
    // errs() << "\n"; 
    
    // size_t count = 0; 
    // for(Module::iterator iterFunc = M.begin(); iterFunc != M.end(); iterFunc++)
    // {
    //     for(Function::iterator iterBlock = iterFunc->begin(); iterBlock != iterFunc->end(); iterBlock++)
    //     {
    //         string nameGraph = num2str(&(*(iterBlock))); 
    //         errs() << "BasicBlock No." << (count++) << "\n"; 
    //         errs() << " -> Original Graph: \n"; 
    //         errs() << _blocksOriGraphs[nameGraph].toString() << "\n";  
    //         // FLOG(string("./tmp/original.") + nameGraph + ".txt") << _blocksOriGraphs[nameGraph].toString(); 
    //         errs() << " -> Operator Graph: \n"; 
    //         errs() << _blocksOpGraphs[nameGraph].toString() << "\n";  
    //         // FLOG(string("./tmp/opgraph.") + nameGraph + ".txt") << _blocksOpGraphs[nameGraph].toString(); 
    //         /* Graphs: 
    //             -> 0x8ed010
    //             -> 0x8ed8a0
    //             -> 0x8eda80
    //             -> 0x8efdf0 */
    //     }
    // }
    
    for(Module::iterator iterFunc = M.begin(); iterFunc != M.end(); iterFunc++)
    {
        for(Function::iterator iterBlock = iterFunc->begin(); iterBlock != iterFunc->end(); iterBlock++)
        {
            string nameGraph = num2str(&(*(iterBlock))); 
            const HyperGraph &origin = _blocksOriGraphs[nameGraph]; 
            for(const auto &vertex: origin.vertices())
            {
                if(vertex.first == "input_func___loop__")
                {
                    // FLOG(string("./original.") + nameGraph + ".txt") << _blocksOriGraphs[nameGraph].toString(); 
                    // FLOG(string("./opgraph.") + nameGraph + ".txt") << _blocksOpGraphs[nameGraph].toString(); 
                    FLOG("./opgraph.txt") << _blocksOpGraphs[nameGraph].toString(); 
                }
            }
        }
    }
    
    // errs() << "Graphs: \n"; 
    // for(Module::iterator iterFunc = M.begin(); iterFunc != M.end(); iterFunc++)
    // {
    //     for(Function::iterator iterBlock = iterFunc->begin(); iterBlock != iterFunc->end(); iterBlock++)
    //     {
    //         string nameGraph = num2str(&(*(iterBlock))); 
    //         errs() << " -> " << nameGraph << "\n";  
    //     }
    // }
    // errs() << "\n"; 

    // errs() << "Instruction / Variables / Constants \n"; 
    // for(const auto &elem: _finalname2name)
    // {
    //     errs() << " -> " << elem.first << ": " << elem.second << "\n"; 
    // }
    // errs() << "\n"; 

    // errs() << "Graphs: \n"; 
    // errs() << _graphControl.toString(); 
    // errs() << "\n"; 

    return false; 
}

std::string PassModule2DFG::getOpVertexName(const std::string &elem)
{
    assert(_slotsTypes.find(elem) != _slotsTypes.end()); 
    string operandType = _slotsTypes[elem]; 
    string opVertexName = operandType + "_" + replaceChar(elem, '.', '_'); 
    if(_constMapRev.find(elem) != _constMapRev.end())
    {
        opVertexName = _constMapRev[elem]; 
    }
    else if(_varMapRev.find(elem) != _varMapRev.end())
    {
        opVertexName = _varMapRev[elem]; 
    }

    return opVertexName; 
}

string PassModule2DFG::addOperand(const Use *operand)
{
    Value *valueOperand = operand->get(); 
    string operandName = valueOperand->hasName() ? valueOperand->getName().str() : num2str(valueOperand); 
    if(_slotsAddrs.find(operandName) == _slotsAddrs.end())
    {
        _slotsAddrs[operandName] = valueOperand; 
        _addr2name[num2str(valueOperand)] = operandName; 
        _name2number[operandName] = _name2number.size(); 
        if(dyn_cast<ConstantData>(valueOperand))
        {
            if(dyn_cast<ConstantFP>(valueOperand))
            {
                _slotsTypes[operandName] = "constfp"; 
                _types.insert(_slotsTypes[operandName]); 
                _slotsValues[operandName] = num2str(dyn_cast<ConstantFP>(valueOperand)->getValue().convertToDouble()); 
                _constants.push_back(operandName); 
            }
            else if(dyn_cast<ConstantInt>(valueOperand))
            {
                _slotsTypes[operandName] = "constint"; 
                _types.insert(_slotsTypes[operandName]); 
                _slotsValues[operandName] = num2str(dyn_cast<ConstantInt>(valueOperand)->getValue().getSExtValue()); 
                _constants.push_back(operandName); 
            }
            else if(dyn_cast<UndefValue>(valueOperand))
            {
                _slotsTypes[operandName] = "undefval"; 
                _types.insert(_slotsTypes[operandName]); 
                _slotsValues[operandName] = ""; 
                _constants.push_back(operandName); 
            }
            else
            {
                errs() << "WARNING: Unknown constant. \n";  
            }
        }
        else if(dyn_cast<ConstantExpr>(valueOperand))
        { 
            _slotsTypes[operandName] = dyn_cast<ConstantExpr>(valueOperand)->getOpcodeName();
            _types.insert(_slotsTypes[operandName]); 
            _constants.push_back(operandName); 
        }
        else if(dyn_cast<GlobalValue>(valueOperand))
        {
            if(dyn_cast<Function>(valueOperand))
            {
                _slotsTypes[operandName] = "func";
                _types.insert(_slotsTypes[operandName]); 
            }
            else if(dyn_cast<GlobalVariable>(valueOperand))
            {
                _slotsTypes[operandName] = "globalvar";
                _types.insert(_slotsTypes[operandName]); 
            }
            else
            {
                errs() << "WARNING: Unknown global value: " << valueOperand->getType()->getTypeID() << "\n";  
            }
        }
        else if(dyn_cast<PHINode>(operand->getUser()))
        {
            ; 
        }
        else
        {
            errs() << "WARNING: Unknown variable: " << num2str(valueOperand) << ":" << valueOperand->getType()->getTypeID() << "\n";  
        }
    }

    return operandName; 
}

std::pair<std::string, size_t> PassModule2DFG::getTypeInfo(const std::string &opName, const llvm::Type::TypeID &typeID)
{
    string typeName = "unknown"; 
    size_t allocaNum = 1; 
    if(typeID == Type::TypeID::IntegerTyID)
    {
        typeName = "int"; 
    }
    else if(typeID == Type::TypeID::FloatTyID)
    {
        typeName = "float"; 
    }
    else if(typeID == Type::TypeID::DoubleTyID)
    {
        typeName = "double"; 
    }
    else if(typeID == Type::TypeID::ArrayTyID)
    {
        typeName = "array"; 
        Type::TypeID arrayTypeID; 
        size_t arraySize; 
        if(dyn_cast<GlobalValue>(_slotsAddrs[opName]))
        {
            arrayTypeID = dyn_cast<GlobalValue>(_slotsAddrs[opName])->getValueType()->getArrayElementType()->getTypeID(); 
            arraySize = dyn_cast<GlobalValue>(_slotsAddrs[opName])->getValueType()->getArrayNumElements(); 
        }
        else if(dyn_cast<AllocaInst>(_slotsAddrs[opName]))
        {
            arrayTypeID = dyn_cast<AllocaInst>(_slotsAddrs[opName])->getAllocatedType()->getArrayElementType()->getTypeID(); 
            arraySize = dyn_cast<AllocaInst>(_slotsAddrs[opName])->getAllocatedType()->getArrayNumElements(); 
        }
        else
        {
            errs() << "WARNING: Unknown array type: " << opName << ":" << typeID << "\n";  
        }
        if(arrayTypeID == Type::TypeID::IntegerTyID)
        {
            typeName += "-int"; 
        }
        else if(arrayTypeID == Type::TypeID::FloatTyID)
        {
            typeName += "-float"; 
        }
        else if(arrayTypeID == Type::TypeID::DoubleTyID)
        {
            typeName += "-double"; 
        }
        else
        {
            typeName += "-unknown"; 
        }
        allocaNum = arraySize; 
    }
    else
    {
        errs() << "WARNING: Unknown global type: " << opName << ":" << typeID << "\n";  
    }

    return {typeName, allocaNum}; 
}

}

 
