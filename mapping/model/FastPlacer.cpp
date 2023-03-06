#include "./FastPlacer.h"

using namespace std;

namespace FastCGRA
{

std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::place(const Graph &coarseDFG, const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                        const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &pack2mapped, const unordered_map<string, unordered_map<string, unordered_map<string, vector<string>>>> &pack2routed,
                                                        const std::unordered_map<std::string, std::unordered_set<std::string>> &coarse2DFG, const std::vector<std::string> &order)
{
    const size_t Max_Failed_Times = 16;

    // Clear the previous status
    _failedVertices.clear();
    // Data
    FastRouter router(routerInit);

    unordered_map<string, string> vertexDFG2RRG;
    unordered_map<string, string> vertexRRG2DFG;

    unordered_map<string, string> coarse2RRG;
    unordered_map<string, string> RRG2coarse;

    stack<vector<pair<string, string>>> stackEdgesToMap;
    stack<unordered_set<string>> stackVerticesToTry;

    //Check
    bool isCheckPass = true;
    for(const auto &vertex: order){
        if(coarseDFG.vertices().find(vertex) == coarseDFG.vertices().end()){
            WARN << "FastPlacer: " << vertex << " not Found in coarseDFG.";
            isCheckPass = false;
        }
    }
    for(const auto &vertex: coarseDFG.vertices()){
        if(!vertex.second.hasAttr("type")){
            WARN << "FastPlacer: No type info found in " << vertex.first << ".";
            isCheckPass = false;
        }
        const string &type = vertex.second.getAttr("type").getStr();
        if(type != "Coin" && type != "Pack"){
            WARN << "FastPlacer: Unsupported type found in " << vertex.first << " : " << type <<  ".";
            isCheckPass = false;
        }
        if(coarse2DFG.find(vertex.first) == coarse2DFG.end()){
            WARN << "FastPlacer: Coarse2DFG info not found for " << vertex.first << ".";
            isCheckPass = false;
        }
        for(const auto &fine: coarse2DFG.find(vertex.first)->second){
            if(DFG.vertices().find(fine) == DFG.vertices().end()){
                WARN << "FastPlacer: vertex " << fine << " in " << vertex.first  << " did not found in dfg.";
                isCheckPass = false;
            }
        }
    }
    if(!isCheckPass){
        return {vertexDFG2RRG, router};
    } else {
        clog << endl << endl << "FastPlacer: Check Passed, start placing." << endl;
    }

    size_t furthest = 0;
    size_t coarseIter = 0;
    size_t failureCount = 0;

    string toMap = order[coarseIter];
    unordered_set<string> verticesToTry = compatible.find(toMap)->second;
    while(coarseIter < order.size()){
        furthest = max(furthest, coarseIter);
        clog << "FastPlacer: New iteration, size of stack: " << coarse2RRG.size() << " / " << order.size() << "; furthest: " << furthest << endl;
        bool failed = false;
        string toTry = "";
        unordered_set<string> toDelete;
        const string &toMapType = coarseDFG.vertex(toMap).getAttr("type").getStr();
        // Prepare to delete used RRG vertices
        for(const auto &vertexToTry: verticesToTry){
            if(RRG2coarse.find(vertexToTry) != RRG2coarse.end()){
                toDelete.insert(vertexToTry);
            }
        }
        // Prepare to unconnectable RRG vertice
        for(const auto &vertexToTry: verticesToTry){
            if(toDelete.find(vertexToTry) != toDelete.end()){
                continue;
            }
            bool available = true;
            unordered_multimap<string, string> linksToValidate;
            for(const auto &edge: coarseDFG.edgesIn(toMap)){
                const string &fromDFG = edge.getAttr("from").getStr();
                const string &toDFG = edge.getAttr("to").getStr();
                if(vertexDFG2RRG.find(fromDFG) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &fromRRG = vertexDFG2RRG[fromDFG];
                if(toMapType == "Coin"){
                    const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                    linksToValidate.insert({fromRRG, toRRG});
                } else {
                    const string &toRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(toDFG)->second;
                    linksToValidate.insert({fromRRG, toRRG});
                }
            }
            for(const auto &edge: coarseDFG.edgesOut(toMap)){
                const string &fromDFG = edge.getAttr("from").getStr();
                const string &toDFG = edge.getAttr("to").getStr();
                if(vertexDFG2RRG.find(toDFG) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &toRRG = vertexDFG2RRG[toDFG];
                if(toMapType == "Coin"){
                    const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                    linksToValidate.insert({fromRRG, toRRG});
                } else {
                    const string &fromRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(fromDFG)->second;
                    linksToValidate.insert({fromRRG, toRRG});
                }
            }
            for(const auto &link: linksToValidate){
                bool found = false;
                for(const auto &edge: _RRGAnalyzed.edgesOut(link.first)){
                    if(edge.to() == link.second){
                        found = true;
                        break;
                    }
                }
                if(!found){
                    available = false;
                    break;
                }
            }
            if(!available){
                toDelete.insert(vertexToTry);
            }
        }
        //Delete
        for (const auto &vertex : toDelete) {
            verticesToTry.erase(vertex);
        }
        
        clog << "FastPlacer: -> toMap: " << toMap << "; Candidates After Purge: " << verticesToTry.size() << endl;
        vector<string> verticesToTryRanked(verticesToTry.begin(), verticesToTry.end());
        const unordered_set<string> &toMaps = coarse2DFG.find(toMap)->second;
        vector<pair<string, string>> edgesToMap;
        vector<string> edgesSignal;
        if (verticesToTry.empty()) {
            failed = true;
            clog << "FastPlacer: Failed. Nothing to try for " << toMap << endl;
        } else {
            //  sort the candidates by sharedNet
            unordered_map<string, size_t> sharedNet;
            for(const auto &vertexToTry: verticesToTry){
                unordered_set<string> inPortRRG;
                unordered_set<string> outPortRRG;
                for(const auto &edge: coarseDFG.edgesIn(toMap)){
                    const string &toDFG = edge.getAttr("to").getStr();
                    if(toMapType == "Coin"){
                        const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                        inPortRRG.insert(toRRG);
                    } else {
                        const string &toRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(toDFG)->second;
                        inPortRRG.insert(toRRG);
                    }
                }
                for(const auto &edge: coarseDFG.edgesOut(toMap)){
                    const string &fromDFG = edge.getAttr("from").getStr();
                    if(toMapType == "Coin"){
                        const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                        outPortRRG.insert(fromRRG);
                    } else {
                        const string &fromRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(fromDFG)->second;
                        inPortRRG.insert(fromRRG);
                    }
                }
                for(const auto &inport: inPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesIn(inport)){
                        if(vertexRRG2DFG.find(edge.from()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
                for(const auto &outport: outPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesOut(outport)){
                        if(vertexRRG2DFG.find(edge.to()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
            }
            random_shuffle(verticesToTryRanked.end(), verticesToTryRanked.end());
            sort(verticesToTryRanked.begin(), verticesToTryRanked.end(), [&](const string &a,const string &b){return sharedNet[a] > sharedNet[b];});
            
            // try map vertex
            bool isSuccess = false;
            size_t IterVertextoTry = 0;
            while(!isSuccess && IterVertextoTry < verticesToTryRanked.size()){
                toTry = verticesToTryRanked[IterVertextoTry++];
                clog << "FastPlacer: try to map " << toMap << " to " << toTry << endl;
                // -> Find edges that need to be mapped
                edgesToMap.clear();
                edgesSignal.clear();
                unordered_map<string, unordered_map<string, vector<string>>> pathsGiven;
                // if(toMapType == "Pack"){
                //     for(const auto &from: pack2routed.find(toMap)->second){
                //         string fromRRG = toTry + "." + from.first;
                //         if(pathsGiven.find(fromRRG) == pathsGiven.end()){
                //             pathsGiven[fromRRG] = unordered_map<string, vector<string>>();
                //         }
                //         for(const auto &to: from.second){
                //             string toRRG = toTry + "." + to.first;
                //             vector<string> pathRRG;
                //             for(const auto &node: to.second){
                //                 pathRRG.push_back(toTry + "." + node);
                //             }
                //             pathsGiven[fromRRG][toRRG] = pathRRG;
                //         }
                //     }
                // }
                for(const auto &vertex: DFG.vertices()){
                    bool fromisused = vertexDFG2RRG.find(vertex.first) != vertexDFG2RRG.end();
                    bool fromistoMap = toMaps.find(vertex.first) != toMaps.end();
                    for(const auto &edge: DFG.edgesOut(vertex.first)){
                        bool toisused = vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end();
                        bool toistoMap = toMaps.find(edge.to()) != toMaps.end();
                        bool toRoute = (fromisused && toistoMap) || (fromistoMap && toisused) || (fromistoMap && toistoMap);
                        if(!toRoute){
                            continue;
                        }
                        string fromRRG;
                        string toRRG;
                        if(fromisused){
                            fromRRG = vertexDFG2RRG[edge.from()];
                        } else if(toMapType == "Coin"){
                            string portName = getPostfix(edge.from());
                            if(portName.empty()){
                                fromRRG = toTry;
                            } else {
                                fromRRG = toTry + "." + portName;
                            }
                        } else {
                            fromRRG = toTry + "." + pack2mapped.find(toMap)->second.find(edge.from())->second;
                        }
                        if(toisused){
                            toRRG = vertexDFG2RRG[edge.to()];
                        } else if(toMapType == "Coin"){
                            string portName = getPostfix(edge.to());
                            if(portName.empty()){
                                toRRG = toTry;
                            } else {
                                toRRG = toTry + "." + portName;
                            }
                        } else {
                            toRRG = toTry + "." + pack2mapped.find(toMap)->second.find(edge.to())->second;
                        }
                        edgesToMap.push_back(pair<string, string>(fromRRG, toRRG));
                        edgesSignal.push_back(edge.from());
                    }
                }
                clog << "FastPlacer: edgesToMap: " << edgesToMap.size() << " for " << toTry << endl;
                verticesToTry.erase(toTry);
                if(edgesToMap.size() > 0){
                    isSuccess = router.strict_route(edgesToMap, edgesSignal, pathsGiven);//pack route
                } else {
                    isSuccess = true;
                }
            }
            if(isSuccess){
                clog << "FastPlacer: Map " << toMap << " to " << toTry << ", Tried " << IterVertextoTry << " Time." << endl;
            } else {
                clog << "FastPlacer: Fail to map " << toMap << endl;
                failed = true;
            }
        }

        if(!failed){
            if(toMapType == "Coin"){
                for(const auto &vertexDFG: toMaps){
                    string vertexRRG;
                    string port = getPostfix(vertexDFG);
                    if(port.empty()){
                        vertexRRG = toTry;
                    } else {
                        vertexRRG = toTry + "." + port;
                    }
                    vertexDFG2RRG[vertexDFG] = vertexRRG;
                    vertexRRG2DFG[vertexRRG] = vertexDFG;
                }
            } else {
                for(const auto &vertexDFG: toMaps){
                    string vertexRRG = pack2mapped.find(toMap)->second.find(vertexDFG)->second;
                    vertexDFG2RRG[vertexDFG] = toTry + "." + vertexRRG;
                    vertexRRG2DFG[vertexRRG] = toTry + "." + vertexDFG;
                }
            }
            coarse2RRG[toMap] = toTry;
            RRG2coarse[toTry] = toMap;
            stackVerticesToTry.push(verticesToTry);
            stackEdgesToMap.push(edgesToMap);

            if(++coarseIter < order.size()){
                toMap = order[coarseIter];
                verticesToTry = compatible.find(toMap)->second;
            } else {
                break;
            }
        } else {
            failureCount++;
            if (_failedVertices.find(toMap) == _failedVertices.end()) {
                _failedVertices[toMap] = 0;
            }
            _failedVertices[toMap]++;
            if( _failedVertices[toMap] > Max_Failed_Times || failureCount > 32 * Max_Failed_Times){
                clog << "FastPlacer: FAILED. Too many failure. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            if(coarseIter == 0){
                clog << "FastPlacer: ALL FAILED. Nothing to try. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            toMap = order[--coarseIter];
            verticesToTry = stackVerticesToTry.top();
            stackVerticesToTry.pop();
            clog << "FastPlacer: Roll back to: " << toMap << "; Candidates: " << verticesToTry.size() << endl;
            vector<pair<string, string>> edgesToDelte = stackEdgesToMap.top();
            stackEdgesToMap.pop();
            router.unroute(edgesToDelte);
            for(const auto &vertexDFG: coarse2DFG.find(toMap)->second){
                vertexRRG2DFG.erase(vertexDFG2RRG[vertexDFG]);
                vertexDFG2RRG.erase(vertexDFG);
            }
            RRG2coarse.erase(coarse2RRG[toMap]);
            coarse2RRG.erase(toMap);
        }
        clog << endl << endl;
    }

    clog << "FastPlacer: finished placing the DFG. Failure count: " << failureCount << "." << endl
         << endl;

    return{vertexDFG2RRG, router};

}


std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::placeHard(const Graph &coarseDFG, const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                        const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &pack2mapped, const unordered_map<string, unordered_map<string, unordered_map<string, vector<string>>>> &pack2routed,
                                                        const std::unordered_map<std::string, std::unordered_set<std::string>> &coarse2DFG, const std::vector<std::string> &order)
{
    const size_t Max_Failed_Times = 256;

    // Clear the previous status
    _failedVertices.clear();
    // Data
    FastRouter router(routerInit);

    unordered_map<string, string> vertexDFG2RRG;
    unordered_map<string, string> vertexRRG2DFG;

    unordered_map<string, string> coarse2RRG;
    unordered_map<string, string> RRG2coarse;

    stack<vector<pair<string, string>>> stackEdgesToMap;
    stack<unordered_set<string>> stackVerticesToTry;

    //Check
    bool isCheckPass = true;
    for(const auto &vertex: order){
        if(coarseDFG.vertices().find(vertex) == coarseDFG.vertices().end()){
            WARN << "FastPlacer: " << vertex << " not Found in coarseDFG.";
            isCheckPass = false;
        }
    }
    for(const auto &vertex: coarseDFG.vertices()){
        if(!vertex.second.hasAttr("type")){
            WARN << "FastPlacer: No type info found in " << vertex.first << ".";
            isCheckPass = false;
        }
        const string &type = vertex.second.getAttr("type").getStr();
        if(type != "Coin" && type != "Pack"){
            WARN << "FastPlacer: Unsupported type found in " << vertex.first << " : " << type <<  ".";
            isCheckPass = false;
        }
        if(coarse2DFG.find(vertex.first) == coarse2DFG.end()){
            WARN << "FastPlacer: Coarse2DFG info not found for " << vertex.first << ".";
            isCheckPass = false;
        }
        for(const auto &fine: coarse2DFG.find(vertex.first)->second){
            if(DFG.vertices().find(fine) == DFG.vertices().end()){
                WARN << "FastPlacer: vertex " << fine << " in " << vertex.first  << " did not found in dfg.";
                isCheckPass = false;
            }
        }
    }
    if(!isCheckPass){
        return {vertexDFG2RRG, router};
    } else {
        clog << endl << endl << "FastPlacer: Check Passed, start placing." << endl;
    }

    size_t furthest = 0;
    size_t coarseIter = 0;
    size_t failureCount = 0;

    string toMap = order[coarseIter];
    unordered_set<string> verticesToTry = compatible.find(toMap)->second;
    while(coarseIter < order.size()){
        furthest = max(furthest, coarseIter);
        clog << "FastPlacer: New iteration, size of stack: " << coarse2RRG.size() << " / " << order.size() << "; furthest: " << furthest << endl;
        bool failed = false;
        string toTry = "";
        unordered_set<string> toDelete;
        const string &toMapType = coarseDFG.vertex(toMap).getAttr("type").getStr();
        // Prepare to delete used RRG vertices
        for(const auto &vertexToTry: verticesToTry){
            if(RRG2coarse.find(vertexToTry) != RRG2coarse.end()){
                toDelete.insert(vertexToTry);
            }
        }
        // Prepare to unconnectable RRG vertice
        for(const auto &vertexToTry: verticesToTry){
            if(toDelete.find(vertexToTry) != toDelete.end()){
                continue;
            }
            bool available = true;
            unordered_multimap<string, string> linksToValidate;
            for(const auto &edge: coarseDFG.edgesIn(toMap)){
                const string &fromDFG = edge.getAttr("from").getStr();
                const string &toDFG = edge.getAttr("to").getStr();
                if(vertexDFG2RRG.find(fromDFG) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &fromRRG = vertexDFG2RRG[fromDFG];
                if(toMapType == "Coin"){
                    const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                    linksToValidate.insert({fromRRG, toRRG});
                } else {
                    const string &toRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(toDFG)->second;
                    linksToValidate.insert({fromRRG, toRRG});
                }
            }
            for(const auto &edge: coarseDFG.edgesOut(toMap)){
                const string &fromDFG = edge.getAttr("from").getStr();
                const string &toDFG = edge.getAttr("to").getStr();
                if(vertexDFG2RRG.find(toDFG) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &toRRG = vertexDFG2RRG[toDFG];
                if(toMapType == "Coin"){
                    const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                    linksToValidate.insert({fromRRG, toRRG});
                } else {
                    const string &fromRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(fromDFG)->second;
                    linksToValidate.insert({fromRRG, toRRG});
                }
            }
            for(const auto &link: linksToValidate){
                bool found = false;
                for(const auto &edge: _RRGAnalyzed.edgesOut(link.first)){
                    if(edge.to() == link.second){
                        found = true;
                        break;
                    }
                }
                if(!found){
                    available = false;
                    break;
                }
            }
            if(!available){
                toDelete.insert(vertexToTry);
            }
        }
        //Delete
        for (const auto &vertex : toDelete) {
            verticesToTry.erase(vertex);
        }
        
        clog << "FastPlacer: -> toMap: " << toMap << "; Candidates After Purge: " << verticesToTry.size() << endl;
        vector<string> verticesToTryRanked(verticesToTry.begin(), verticesToTry.end());
        const unordered_set<string> &toMaps = coarse2DFG.find(toMap)->second;
        vector<pair<string, string>> edgesToMap;
        vector<string> edgesSignal;
        if (verticesToTry.empty()) {
            failed = true;
            clog << "FastPlacer: Failed. Nothing to try for " << toMap << endl;
        } else {
            //  sort the candidates by sharedNet
            unordered_map<string, size_t> sharedNet;
            for(const auto &vertexToTry: verticesToTry){
                unordered_set<string> inPortRRG;
                unordered_set<string> outPortRRG;
                for(const auto &edge: coarseDFG.edgesIn(toMap)){
                    const string &toDFG = edge.getAttr("to").getStr();
                    if(toMapType == "Coin"){
                        const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                        inPortRRG.insert(toRRG);
                    } else {
                        const string &toRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(toDFG)->second;
                        inPortRRG.insert(toRRG);
                    }
                }
                for(const auto &edge: coarseDFG.edgesOut(toMap)){
                    const string &fromDFG = edge.getAttr("from").getStr();
                    if(toMapType == "Coin"){
                        const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                        outPortRRG.insert(fromRRG);
                    } else {
                        const string &fromRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(fromDFG)->second;
                        inPortRRG.insert(fromRRG);
                    }
                }
                for(const auto &inport: inPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesIn(inport)){
                        if(vertexRRG2DFG.find(edge.from()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
                for(const auto &outport: outPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesOut(outport)){
                        if(vertexRRG2DFG.find(edge.to()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
            }
            random_shuffle(verticesToTryRanked.end(), verticesToTryRanked.end());
            sort(verticesToTryRanked.begin(), verticesToTryRanked.end(), [&](const string &a,const string &b){return sharedNet[a] > sharedNet[b];});
            
            // try map vertex
            bool isSuccess = false;
            size_t IterVertextoTry = 0;
            while(!isSuccess && IterVertextoTry < verticesToTryRanked.size()){
                toTry = verticesToTryRanked[IterVertextoTry++];
                clog << "FastPlacer: try to map " << toMap << " to " << toTry << endl;
                // -> Find edges that need to be mapped
                edgesToMap.clear();
                edgesSignal.clear();
                unordered_map<string, unordered_map<string, vector<string>>> pathsGiven;
                // if(toMapType == "Pack"){
                //     for(const auto &from: pack2routed.find(toMap)->second){
                //         string fromRRG = toTry + "." + from.first;
                //         if(pathsGiven.find(fromRRG) == pathsGiven.end()){
                //             pathsGiven[fromRRG] = unordered_map<string, vector<string>>();
                //         }
                //         for(const auto &to: from.second){
                //             string toRRG = toTry + "." + to.first;
                //             vector<string> pathRRG;
                //             for(const auto &node: to.second){
                //                 pathRRG.push_back(toTry + "." + node);
                //             }
                //             pathsGiven[fromRRG][toRRG] = pathRRG;
                //         }
                //     }
                // }
                for(const auto &vertex: DFG.vertices()){
                    bool fromisused = vertexDFG2RRG.find(vertex.first) != vertexDFG2RRG.end();
                    bool fromistoMap = toMaps.find(vertex.first) != toMaps.end();
                    for(const auto &edge: DFG.edgesOut(vertex.first)){
                        bool toisused = vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end();
                        bool toistoMap = toMaps.find(edge.to()) != toMaps.end();
                        bool toRoute = (fromisused && toistoMap) || (fromistoMap && toisused) || (fromistoMap && toistoMap);
                        if(!toRoute){
                            continue;
                        }
                        string fromRRG;
                        string toRRG;
                        if(fromisused){
                            fromRRG = vertexDFG2RRG[edge.from()];
                        } else if(toMapType == "Coin"){
                            string portName = getPostfix(edge.from());
                            if(portName.empty()){
                                fromRRG = toTry;
                            } else {
                                fromRRG = toTry + "." + portName;
                            }
                        } else {
                            fromRRG = toTry + "." + pack2mapped.find(toMap)->second.find(edge.from())->second;
                        }
                        if(toisused){
                            toRRG = vertexDFG2RRG[edge.to()];
                        } else if(toMapType == "Coin"){
                            string portName = getPostfix(edge.to());
                            if(portName.empty()){
                                toRRG = toTry;
                            } else {
                                toRRG = toTry + "." + portName;
                            }
                        } else {
                            toRRG = toTry + "." + pack2mapped.find(toMap)->second.find(edge.to())->second;
                        }
                        edgesToMap.push_back(pair<string, string>(fromRRG, toRRG));
                        edgesSignal.push_back(edge.from());
                    }
                }
                clog << "FastPlacer: edgesToMap: " << edgesToMap.size() << " for " << toTry << endl;
                verticesToTry.erase(toTry);
                if(edgesToMap.size() > 0){
                    isSuccess = router.strict_route(edgesToMap, edgesSignal, pathsGiven);//pack route
                } else {
                    isSuccess = true;
                }
            }
            if(isSuccess){
                clog << "FastPlacer: Map " << toMap << " to " << toTry << ", Tried " << IterVertextoTry << " Time." << endl;
            } else {
                clog << "FastPlacer: Fail to map " << toMap << endl;
                failed = true;
            }
        }

        if(!failed){
            if(toMapType == "Coin"){
                for(const auto &vertexDFG: toMaps){
                    string vertexRRG;
                    string port = getPostfix(vertexDFG);
                    if(port.empty()){
                        vertexRRG = toTry;
                    } else {
                        vertexRRG = toTry + "." + port;
                    }
                    vertexDFG2RRG[vertexDFG] = vertexRRG;
                    vertexRRG2DFG[vertexRRG] = vertexDFG;
                }
            } else {
                for(const auto &vertexDFG: toMaps){
                    string vertexRRG = pack2mapped.find(toMap)->second.find(vertexDFG)->second;
                    vertexDFG2RRG[vertexDFG] = toTry + "." + vertexRRG;
                    vertexRRG2DFG[vertexRRG] = toTry + "." + vertexDFG;
                }
            }
            coarse2RRG[toMap] = toTry;
            RRG2coarse[toTry] = toMap;
            stackVerticesToTry.push(verticesToTry);
            stackEdgesToMap.push(edgesToMap);

            if(++coarseIter < order.size()){
                toMap = order[coarseIter];
                verticesToTry = compatible.find(toMap)->second;
            } else {
                break;
            }
        } else {
            failureCount++;
            if (_failedVertices.find(toMap) == _failedVertices.end()) {
                _failedVertices[toMap] = 0;
            }
            _failedVertices[toMap]++;
            if( _failedVertices[toMap] > Max_Failed_Times || failureCount > 32 * Max_Failed_Times){
                clog << "FastPlacer: FAILED. Too many failure. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            if(coarseIter == 0){
                clog << "FastPlacer: ALL FAILED. Nothing to try. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            toMap = order[--coarseIter];
            verticesToTry = stackVerticesToTry.top();
            stackVerticesToTry.pop();
            clog << "FastPlacer: Roll back to: " << toMap << "; Candidates: " << verticesToTry.size() << endl;
            vector<pair<string, string>> edgesToDelte = stackEdgesToMap.top();
            stackEdgesToMap.pop();
            router.unroute(edgesToDelte);
            for(const auto &vertexDFG: coarse2DFG.find(toMap)->second){
                vertexRRG2DFG.erase(vertexDFG2RRG[vertexDFG]);
                vertexDFG2RRG.erase(vertexDFG);
            }
            RRG2coarse.erase(coarse2RRG[toMap]);
            coarse2RRG.erase(toMap);
        }
        clog << endl << endl;
    }

    clog << "FastPlacer: finished placing the DFG. Failure count: " << failureCount << "." << endl
         << endl;

    return{vertexDFG2RRG, router};

}


std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::placeII(const Graph &coarseDFG, const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                        const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &pack2mapped, const unordered_map<string, unordered_map<string, unordered_map<string, vector<string>>>> &pack2routed,
                                                        const std::unordered_map<std::string, std::unordered_set<std::string>> &coarse2DFG, const std::vector<std::string> &order)
{
    
    const size_t Max_Failed_Times = 128;

    // Clear the previous status
    _failedVertices.clear();
    // Data
    FastRouter router(routerInit);

    unordered_map<string, string> vertexDFG2RRG;
    unordered_map<string, string> vertexRRG2DFG;

    unordered_map<string, string> coarse2RRG;
    unordered_map<string, string> RRG2coarse;

    stack<vector<pair<string, string>>> stackEdgesToMap;
    stack<unordered_set<string>> stackVerticesToTry;


    //Check
    bool isCheckPass = true;
    for(const auto &vertex: order){
        if(coarseDFG.vertices().find(vertex) == coarseDFG.vertices().end()){
            WARN << "FastPlacer: " << vertex << " not Found in coarseDFG.";
            isCheckPass = false;
        }
    }
    for(const auto &vertex: coarseDFG.vertices()){
        if(!vertex.second.hasAttr("type")){
            WARN << "FastPlacer: No type info found in " << vertex.first << ".";
            isCheckPass = false;
        }
        const string &type = vertex.second.getAttr("type").getStr();
        if(type != "Coin" && type != "Pack"){
            WARN << "FastPlacer: Unsupported type found in " << vertex.first << " : " << type <<  ".";
            isCheckPass = false;
        }
        if(coarse2DFG.find(vertex.first) == coarse2DFG.end()){
            WARN << "FastPlacer: Coarse2DFG info not found for " << vertex.first << ".";
            isCheckPass = false;
        }
        for(const auto &fine: coarse2DFG.find(vertex.first)->second){
            if(DFG.vertices().find(fine) == DFG.vertices().end()){
                WARN << "FastPlacer: vertex " << fine << " in " << vertex.first  << " did not found in dfg.";
                isCheckPass = false;
            }
        }
    }
    if(!isCheckPass){
        return {vertexDFG2RRG, router};
    } else {
        clog << endl << endl << "FastPlacer: Check Passed, start placing." << endl;
    }

    size_t furthest = 0;
    size_t coarseIter = 0;
    size_t failureCount = 0;

    string toMap = order[coarseIter];
    unordered_set<string> verticesToTry = compatible.find(toMap)->second;
    while(coarseIter < order.size()){
        furthest = max(furthest, coarseIter);
        clog << "FastPlacer: New iteration, size of stack: " << coarse2RRG.size() << " / " << order.size() << "; furthest: " << furthest << endl;
        bool failed = false;
        string toTry = "";
        unordered_set<string> toDelete;
        // Prepare to delete used RRG vertices
        for(const auto &vertexToTry: verticesToTry){
            if(RRG2coarse.find(vertexToTry) != RRG2coarse.end()){
                toDelete.insert(vertexToTry);
            }
        }
        //Delete
        for (const auto &vertex : toDelete) {
            verticesToTry.erase(vertex);
        }
        // Prepare to unconnectable RRG vertice
        for(const auto &vertexToTry: verticesToTry){
            if(toDelete.find(vertexToTry) != toDelete.end()){
                continue;
            }
            bool available = true;
            unordered_multimap<string, string> linksToValidate;
            for(const auto &edge: coarseDFG.edgesIn(toMap)){
                const string &fromDFG = edge.getAttr("from").getStr();
                const string &toDFG = edge.getAttr("to").getStr();
                if(vertexDFG2RRG.find(fromDFG) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &fromRRG = vertexDFG2RRG[fromDFG];
                const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                linksToValidate.insert({fromRRG, toRRG});
            }
            for(const auto &edge: coarseDFG.edgesOut(toMap)){
                const string &fromDFG = edge.getAttr("from").getStr();
                const string &toDFG = edge.getAttr("to").getStr();
                if(vertexDFG2RRG.find(toDFG) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &toRRG = vertexDFG2RRG[toDFG];
                const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                linksToValidate.insert({fromRRG, toRRG});
            }
            for(const auto &link: linksToValidate){
                bool found = false;
                for(const auto &edge: _RRGAnalyzed.edgesOut(link.first)){
                    if(edge.to() == link.second){
                        found = true;
                        break;
                    }
                }
                if(!found){
                    available = false;
                    break;
                }
            }
            if(!available){
                toDelete.insert(vertexToTry);
            }
        }
        
        
        clog << "FastPlacer: -> toMap: " << toMap << "; Candidates After Purge: " << verticesToTry.size() << endl;
        vector<string> verticesToTryRanked(verticesToTry.begin(), verticesToTry.end());
        const unordered_set<string> &toMaps = coarse2DFG.find(toMap)->second;
        vector<pair<string, string>> edgesToMap;
        vector<string> edgesSignal;
        if (verticesToTry.empty()) {
            failed = true;
            clog << "FastPlacer: Failed. Nothing to try for " << toMap << endl;
        } else {
            //  sort the candidates by sharedNet
            unordered_map<string, size_t> sharedNet;
            for(const auto &vertexToTry: verticesToTry){
                unordered_set<string> inPortRRG;
                unordered_set<string> outPortRRG;
                for(const auto &edge: coarseDFG.edgesIn(toMap)){
                    const string &toDFG = edge.getAttr("to").getStr();
                    const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                    inPortRRG.insert(toRRG);
                }
                for(const auto &edge: coarseDFG.edgesOut(toMap)){
                    const string &fromDFG = edge.getAttr("from").getStr();
                    const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                    outPortRRG.insert(fromRRG);
                }
                for(const auto &inport: inPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesIn(inport)){
                        if(vertexRRG2DFG.find(edge.from()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
                for(const auto &outport: outPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesOut(outport)){
                        if(vertexRRG2DFG.find(edge.to()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
                if(toDelete.find(vertexToTry) == toDelete.end()){
                    sharedNet[vertexToTry]++;
                }
            }
            random_shuffle(verticesToTryRanked.end(), verticesToTryRanked.end());
            sort(verticesToTryRanked.begin(), verticesToTryRanked.end(), [&](const string &a,const string &b){return sharedNet[a] > sharedNet[b];});
            
            // try map vertex
            bool isSuccess = false;
            size_t IterVertextoTry = 0;
            while(!isSuccess && IterVertextoTry < verticesToTryRanked.size()){
                toTry = verticesToTryRanked[IterVertextoTry++];
                clog << "FastPlacer: try to map " << toMap << " to " << toTry << endl;
                // -> Find edges that need to be mapped
                edgesToMap.clear();
                edgesSignal.clear();
                unordered_map<string, unordered_map<string, vector<string>>> pathsGiven;
                for(const auto &vertex: DFG.vertices()){
                    bool fromisused = vertexDFG2RRG.find(vertex.first) != vertexDFG2RRG.end();
                    bool fromistoMap = toMaps.find(vertex.first) != toMaps.end();
                    for(const auto &edge: DFG.edgesOut(vertex.first)){
                        bool toisused = vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end();
                        bool toistoMap = toMaps.find(edge.to()) != toMaps.end();
                        bool toRoute = (fromisused && toistoMap) || (fromistoMap && toisused) || (fromistoMap && toistoMap);
                        if(!toRoute){
                            continue;
                        }
                        string fromRRG;
                        string toRRG;
                        if(fromisused){
                            fromRRG = vertexDFG2RRG[edge.from()];
                        } else{
                            string portName = getPostfix(edge.from());
                            if(portName.empty()){
                                fromRRG = toTry;
                            } else {
                                fromRRG = toTry + "." + portName;
                            }
                        }
                        if(toisused){
                            toRRG = vertexDFG2RRG[edge.to()];
                        } else{
                            string portName = getPostfix(edge.to());
                            if(portName.empty()){
                                toRRG = toTry;
                            } else {
                                toRRG = toTry + "." + portName;
                            }
                        }
                        edgesToMap.push_back(pair<string, string>(fromRRG, toRRG));
                        edgesSignal.push_back(edge.from());
                    }
                }
                clog << "FastPlacer: edgesToMap: " << edgesToMap.size() << " for " << toTry << endl;
                verticesToTry.erase(toTry);
                if(edgesToMap.size() > 0){
                    isSuccess = router.route(edgesToMap, edgesSignal, pathsGiven);//pack route
                } else {
                    isSuccess = true;
                }
            }
            if(isSuccess){
                clog << "FastPlacer: Map " << toMap << " to " << toTry << ", Tried " << IterVertextoTry << " Time." << endl;
            } else {
                clog << "FastPlacer: Fail to map " << toMap << endl;
                failed = true;
            }
        }

        if(!failed){
            for(const auto &vertexDFG: toMaps){
                string vertexRRG;
                string port = getPostfix(vertexDFG);
                if(port.empty()){
                    vertexRRG = toTry;
                } else {
                    vertexRRG = toTry + "." + port;
                }
                vertexDFG2RRG[vertexDFG] = vertexRRG;
                vertexRRG2DFG[vertexRRG] = vertexDFG;
            }
            coarse2RRG[toMap] = toTry;
            RRG2coarse[toTry] = toMap;
            stackVerticesToTry.push(verticesToTry);
            stackEdgesToMap.push(edgesToMap);

            if(++coarseIter < order.size()){
                toMap = order[coarseIter];
                verticesToTry = compatible.find(toMap)->second;
            } else {
                break;
            }
        } else {
            failureCount++;
            if (_failedVertices.find(toMap) == _failedVertices.end()) {
                _failedVertices[toMap] = 0;
            }
            _failedVertices[toMap]++;
            if( _failedVertices[toMap] > Max_Failed_Times || failureCount > 32 * Max_Failed_Times){
                clog << "FastPlacer: FAILED. Too many failure. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            if(coarseIter == 0){
                clog << "FastPlacer: ALL FAILED. Nothing to try. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            toMap = order[--coarseIter];
            verticesToTry = stackVerticesToTry.top();
            stackVerticesToTry.pop();
            clog << "FastPlacer: Roll back to: " << toMap << "; Candidates: " << verticesToTry.size() << endl;
            vector<pair<string, string>> edgesToDelte = stackEdgesToMap.top();
            stackEdgesToMap.pop();
            router.unroute(edgesToDelte);
            for(const auto &vertexDFG: coarse2DFG.find(toMap)->second){
                vertexRRG2DFG.erase(vertexDFG2RRG[vertexDFG]);
                vertexDFG2RRG.erase(vertexDFG);
            }
            RRG2coarse.erase(coarse2RRG[toMap]);
            coarse2RRG.erase(toMap);
        }
        clog << endl << endl;
    }

    clog << "FastPlacer: finished placing the DFG. Failure count: " << failureCount << "." << endl
         << endl;

    return{vertexDFG2RRG, router};
}

std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::place_v2(const Graph &coarseDFG, const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compatible, const FastRouter &routerInit, 
                                                        const std::unordered_map<std::string, std::string>  &usedDFG2RRGInitial,
                                                        const std::unordered_map<std::string, std::unordered_set<std::string>> &coarse2DFG, const std::vector<std::string> &order)
{
    const size_t Max_Failed_Times = 256;

    // Clear the previous status
    _failedVertices.clear();
    // Data
    FastRouter router(routerInit);
    vector<string> Goorder = order;
    unordered_map<string, string> vertexDFG2RRG = usedDFG2RRGInitial;
    for(const auto &vertex: DFG.vertices()){
        if(usedDFG2RRGInitial.find(vertex.first) != usedDFG2RRGInitial.end()){
            continue;
        }
        string fu = getPrefix(vertex.first);
        string port = getPostfix(vertex.first);
        if(compatible.find(fu) != compatible.end()
        &&compatible.find(fu)->second.size() == 1){
            if(port.empty()){
                vertexDFG2RRG[vertex.first] = *(compatible.find(fu)->second).begin();
                clog << "FastPlacer: Place" << vertex.first << " to " << *(compatible.find(fu)->second).begin() << endl;
            } else {
                vertexDFG2RRG[vertex.first] = *(compatible.find(fu)->second).begin() + "." + port;
            }
        }
    }
    unordered_map<string, string> vertexRRG2DFG;
    unordered_map<string, bool>   travesalVertexDFG;

    unordered_map<string, string> coarse2RRG;
    unordered_map<string, string> RRG2coarse;
    unordered_map<string, unordered_set<string>> coincompat = compatible;
    stack<vector<pair<string, string>>> stackEdgesToMap;
    stack<unordered_set<string>> stackVerticesToTry;
    const std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>> pack2routed;
    for(const auto &vertexDFG: DFG.vertices())
    {
        travesalVertexDFG[vertexDFG.first] = false;
    }
    for(const auto &vertex: vertexDFG2RRG)
    {
        travesalVertexDFG[vertex.first] = true;
        if(vertexRRG2DFG.find(vertex.second) != vertexRRG2DFG.end()){
            clog << "FastPlacer: Failed cause More than one node placed on " << vertex.second << endl;
            return {{}, router};
        }
        vertexRRG2DFG[vertex.second] = vertex.first;
    }
    //Check
    bool isCheckPass = true;
    // for(const auto &vertex: order){
    //     if(coarseDFG.vertices().find(vertex) == coarseDFG.vertices().end()){
    //         WARN << "FastPlacer: " << vertex << " not Found in coarseDFG.";
    //         isCheckPass = false;
    //     }
    // }
    for(const auto &vertex: coarseDFG.vertices()){
        if(!vertex.second.hasAttr("type")){
            WARN << "FastPlacer: No type info found in " << vertex.first << ".";
            isCheckPass = false;
        }
        const string &type = vertex.second.getAttr("type").getStr();
        if(type != "Coin" && type != "Pack"){
            WARN << "FastPlacer: Unsupported type found in " << vertex.first << " : " << type <<  ".";
            isCheckPass = false;
        }
        if(coarse2DFG.find(vertex.first) == coarse2DFG.end()){
            WARN << "FastPlacer: Coarse2DFG info not found for " << vertex.first << ".";
            isCheckPass = false;
        }
        for(const auto &fine: coarse2DFG.find(vertex.first)->second){
            if(DFG.vertices().find(fine) == DFG.vertices().end()){
                WARN << "FastPlacer: vertex " << fine << " in " << vertex.first  << " did not found in dfg.";
                isCheckPass = false;
            }
        }
    }
    if(!isCheckPass){
        return {vertexDFG2RRG, router};
    } else {
        clog << endl << endl << "FastPlacer: Check Passed, start placing." << endl;
    }

    //route the different block edges
    for(const auto &vertex: DFG.vertices())
    {
        if(travesalVertexDFG[vertex.first])
        {
            for(const auto &edge: DFG.edgesOut(vertex.first))
            {
                if(travesalVertexDFG[edge.to()])
                {
                    assert(vertexDFG2RRG.find(edge.from()) != vertexDFG2RRG.end() && vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end());
                    const string &fromrrg = vertexDFG2RRG[edge.from()];
                    const string &torrg   = vertexDFG2RRG[edge.to()];
                    if (router.paths().find(fromrrg) == router.paths().end() ||
                        router.paths().find(fromrrg)->second.find(torrg) == router.paths().find(fromrrg)->second.end())
                        {
                            clog << "DFG:" << edge.from() << "->" << edge.to() << endl;
                            bool routed = router.strict_route({
                                                            {fromrrg, torrg},
                                                       },
                                                       {
                                                            edge.from(),
                                                       });
                            if(!routed)
                            {
                                clog << "FastRouter: FAILED, the path " << fromrrg << " -> " << torrg << " cannot route. " << endl;
                                return {unordered_map<string, string>(), router};
                            }
                        }
                }
            }
        }
    }
    //if only one part, it is finished.
    if(vertexDFG2RRG.size() == DFG.nVertices())
    {
        return {vertexDFG2RRG, router};
    }

    //N-order validation
    // NetworkAnalyzerLegacy analyzer(analyzerInitial);
    // Graph &RRGAnalyzed = analyzer.RRG();
    // NOrderValidator validator(validatorInitial);
    // size_t unplacibleCount = 0;
    // for (const auto &vertexDFG : coarseDFG.vertices()) {
    //     if (!getPostfix(vertexDFG.first).empty() || vertexDFG.first.find("block") != string::npos) {
    //         continue;
    //     }
    //     if (coincompat.find(vertexDFG.first) == coincompat.end()) {
    //         WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
    //         return {unordered_map<string, string>(), router};
    //     }
    //     unordered_set<string> compatibles;
    //     for(const auto &vertexRRG: coincompat[vertexDFG.first]){
    //         clog << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
    //         if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) { //NorderValidate
    //             compatibles.insert(vertexRRG);
    //         }
    //     }
    //     clog << vertexDFG.first << ": " << coincompat[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
    //     coincompat[vertexDFG.first] = compatibles;
    // }
    // if(unplacibleCount > 0){
    //     clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
    //     return {unordered_map<string, string>(), router};
    // }

    //cout << coincompat << endl;
    // cout << "before order: " << Goorder << endl;
    // vector<string> temp_order;
    // for(const auto &ord: Goorder)
    // {
    //     if(ord.find("inserted") != string::npos)
    //     {
    //         temp_order.push_back(ord);
    //     }
    // }

    // Goorder.clear();
    // Goorder = temp_order;
    // cout << "after order" << Goorder << endl;

    size_t furthest = 0;
    size_t coarseIter = 0;
    size_t failureCount = 0;
    cout << endl << "FastPlacer: Begin placing. " << endl;
    vector<string> temp;
    for(const auto &vertex: order)
    {
        if(vertex.find("inport") != string::npos)
        {
            temp.push_back(vertex);
        }
    }
    for(const auto &vertex: order)
    {
        if(vertex.find("insert") == string::npos)
        {
            temp.push_back(vertex);
        }
    }
    for(const auto &vertex: order)
    {
        if(vertex.find("insert") != string::npos && vertex.find("inport") == string::npos)
        {
            temp.push_back(vertex);
        }
    }
    Goorder = temp;

    // cout << Goorder << endl;
    // cout << compatible << endl;
    // exit(0);
    string toMap = Goorder[0];
    unordered_set<string> verticesToTry = compatible.find(toMap)->second;
    while(coarseIter < Goorder.size()){
        furthest = max(furthest, coarseIter);
        clog << "FastPlacer: New iteration, size of stack: " << coarse2RRG.size() << " / " << Goorder.size() << "; furthest: " << furthest << endl;
        cout << "FastPlacer: New iteration, size of stack: " << coarse2RRG.size() << " / " << Goorder.size() << "; furthest: " << furthest << endl;
        bool failed = false;
        string toTry = "";
        unordered_set<string> toDelete;
        const string &toMapType = coarseDFG.vertex(toMap).getAttr("type").getStr();
        // Prepare to delete used RRG vertices
        for(const auto &vertexToTry: verticesToTry){
            if(RRG2coarse.find(vertexToTry) != RRG2coarse.end()){
                toDelete.insert(vertexToTry);
            }
        }
        // Prepare to unconnectable RRG vertice
        for(const auto &vertexToTry: verticesToTry){
            if(toDelete.find(vertexToTry) != toDelete.end()){
                continue;
            }
            bool available = true;
            unordered_multimap<string, string> linksToValidate;
            for(const auto &edge: coarseDFG.edgesIn(toMap)){
                const string &fromDFG = edge.getAttr("from").getStr();
                const string &toDFG = edge.getAttr("to").getStr();
                if(vertexDFG2RRG.find(fromDFG) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &fromRRG = vertexDFG2RRG[fromDFG];
                const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                linksToValidate.insert({fromRRG, toRRG});
                // if(toMapType == "Coin"){ 
                //     const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                //     linksToValidate.insert({fromRRG, toRRG});
                // } else {
                //     const string &toRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(toDFG)->second;
                //     linksToValidate.insert({fromRRG, toRRG});
                // }
            }
            for(const auto &edge: coarseDFG.edgesOut(toMap)){
                const string &fromDFG = edge.getAttr("from").getStr();
                const string &toDFG = edge.getAttr("to").getStr();
                if(vertexDFG2RRG.find(toDFG) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &toRRG = vertexDFG2RRG[toDFG];
                const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                linksToValidate.insert({fromRRG, toRRG});
                // if(toMapType == "Coin"){
                //     const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                //     linksToValidate.insert({fromRRG, toRRG});
                // } else {
                //     const string &fromRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(fromDFG)->second;
                //     linksToValidate.insert({fromRRG, toRRG});
                // }
            }
            for(const auto &link: linksToValidate){
                bool found = false;
                for(const auto &edge: _RRGAnalyzed.edgesOut(link.first)){
                    if(edge.to() == link.second){
                        found = true;
                        break;
                    }
                }
                if(!found){
                    available = false;
                    break;
                }
            }
            if(!available){
                toDelete.insert(vertexToTry);
            }
        }
        //Delete
        for (const auto &vertex : toDelete) {
            verticesToTry.erase(vertex);
        }
        
        clog << "FastPlacer: -> toMap: " << toMap << "; Candidates After Purge: " << verticesToTry.size() << endl;
        vector<string> verticesToTryRanked(verticesToTry.begin(), verticesToTry.end());
        const unordered_set<string> &toMaps = coarse2DFG.find(toMap)->second;
        cout << "FastPlacer: toMaps: " << toMaps << endl;
        vector<pair<string, string>> edgesToMap;
        vector<string> edgesSignal;
        if (verticesToTry.empty()) {
            failed = true;
            clog << "FastPlacer: Failed. Nothing to try for " << toMap << endl;
        } else {
            //  sort the candidates by sharedNet
            unordered_map<string, size_t> sharedNet;
            for(const auto &vertexToTry: verticesToTry){
                unordered_set<string> inPortRRG;
                unordered_set<string> outPortRRG;
                for(const auto &edge: coarseDFG.edgesIn(toMap)){
                    const string &toDFG = edge.getAttr("to").getStr();
                    const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                    inPortRRG.insert(toRRG);
                    // if(toMapType == "Coin"){
                    //     const string &toRRG = vertexToTry + "." + getPostfix(toDFG);
                    //     inPortRRG.insert(toRRG);
                    // } else {
                    //     const string &toRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(toDFG)->second;
                    //     inPortRRG.insert(toRRG);
                    // }
                }
                for(const auto &edge: coarseDFG.edgesOut(toMap)){
                    const string &fromDFG = edge.getAttr("from").getStr();
                    const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                    outPortRRG.insert(fromRRG);
                    // if(toMapType == "Coin"){
                    //     const string &fromRRG = vertexToTry + "." + getPostfix(fromDFG);
                    //     outPortRRG.insert(fromRRG);
                    // } else {
                    //     const string &fromRRG = vertexToTry + "." + pack2mapped.find(toMap)->second.find(fromDFG)->second;
                    //     inPortRRG.insert(fromRRG);
                    // }
                }
                for(const auto &inport: inPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesIn(inport)){
                        if(vertexRRG2DFG.find(edge.from()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
                for(const auto &outport: outPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesOut(outport)){
                        if(vertexRRG2DFG.find(edge.to()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
            }
            random_shuffle(verticesToTryRanked.end(), verticesToTryRanked.end());
            sort(verticesToTryRanked.begin(), verticesToTryRanked.end(), [&](const string &a,const string &b){return sharedNet[a] > sharedNet[b];});
            
            // try map vertex
            bool isSuccess = false;
            size_t IterVertextoTry = 0;
            while(!isSuccess && IterVertextoTry < verticesToTryRanked.size()){
                toTry = verticesToTryRanked[IterVertextoTry++];
                clog << "FastPlacer: try to map " << toMap << " to " << toTry << endl;
                // -> Find edges that need to be mapped
                edgesToMap.clear();
                edgesSignal.clear();
                unordered_map<string, unordered_map<string, vector<string>>> pathsGiven;
                if(toMapType == "Pack"){
                    for(const auto &from: pack2routed.find(toMap)->second){
                        string fromRRG = toTry + "." + from.first;
                        if(pathsGiven.find(fromRRG) == pathsGiven.end()){
                            pathsGiven[fromRRG] = unordered_map<string, vector<string>>();
                        }
                        for(const auto &to: from.second){
                            string toRRG = toTry + "." + to.first;
                            vector<string> pathRRG;
                            for(const auto &node: to.second){
                                pathRRG.push_back(toTry + "." + node);
                            }
                            pathsGiven[fromRRG][toRRG] = pathRRG;
                        }
                    }
                }
                for(const auto &vertex: DFG.vertices()){
                    bool fromisused = vertexDFG2RRG.find(vertex.first) != vertexDFG2RRG.end();
                    bool fromistoMap = toMaps.find(vertex.first) != toMaps.end();
                    for(const auto &edge: DFG.edgesOut(vertex.first)){
                        bool toisused = vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end();
                        bool toistoMap = toMaps.find(edge.to()) != toMaps.end();
                        bool toRoute = (fromisused && toistoMap) || (fromistoMap && toisused) || (fromistoMap && toistoMap);
                        if(!toRoute){
                            continue;
                        }
                        string fromRRG;
                        string toRRG;
                        if(fromisused){
                            fromRRG = vertexDFG2RRG[edge.from()];
                        } 
                        else if(toMapType == "Coin"){
                            string portName = getPostfix(edge.from());
                            if(portName.empty()){
                                fromRRG = toTry;
                            } else {
                                fromRRG = toTry + "." + portName;
                            }
                        } 
                        else {
                            ;//fromRRG = toTry + "." + pack2mapped.find(toMap)->second.find(edge.from())->second;
                        }
                        if(toisused){
                            toRRG = vertexDFG2RRG[edge.to()];
                        } else if(toMapType == "Coin"){
                            string portName = getPostfix(edge.to());
                            if(portName.empty()){
                                toRRG = toTry;
                            } else {
                                toRRG = toTry + "." + portName;
                            }
                        } else {
                            ; //toRRG = toTry + "." + pack2mapped.find(toMap)->second.find(edge.to())->second;
                        }
                        edgesToMap.push_back(pair<string, string>(fromRRG, toRRG));
                        edgesSignal.push_back(edge.from());
                    }
                }
                clog << "FastPlacer: edgesToMap: " << edgesToMap.size() << " for " << toTry << endl;
                verticesToTry.erase(toTry);
                if(edgesToMap.size() > 0){
                    isSuccess = router.strict_route(edgesToMap, edgesSignal, pathsGiven);
                    // if(!isSuccess){
                    //     clog << "FastPlacer: Choose a loose route." << endl;
                    //     isSuccess = router.route(edgesToMap, edgesSignal, pathsGiven);
                    // }
                } else {
                    isSuccess = true;
                }
            }
            if(isSuccess){
                clog << "FastPlacer: Map " << toMap << " to " << toTry << ", Tried " << IterVertextoTry << " Time." << endl;
            } else {
                clog << "FastPlacer: Fail to map " << toMap << endl;
                failed = true;
            }
        }

        if(!failed){
            if(toMapType == "Coin"){
                for(const auto &vertexDFG: toMaps){
                    string vertexRRG;
                    string port = getPostfix(vertexDFG);
                    if(port.empty()){
                        vertexRRG = toTry;
                    } else {
                        vertexRRG = toTry + "." + port;
                    }
                    vertexDFG2RRG[vertexDFG] = vertexRRG;
                    vertexRRG2DFG[vertexRRG] = vertexDFG;
                }
            }
            coarse2RRG[toMap] = toTry;
            RRG2coarse[toTry] = toMap;
            stackVerticesToTry.push(verticesToTry);
            stackEdgesToMap.push(edgesToMap);

            if(++coarseIter < Goorder.size()){
                toMap = Goorder[coarseIter];
                verticesToTry = compatible.find(toMap)->second;
            } else {
                break;
            }
        } else {
            failureCount++;
            if (_failedVertices.find(toMap) == _failedVertices.end()) {
                _failedVertices[toMap] = 0;
            }
            _failedVertices[toMap]++;
            if( _failedVertices[toMap] > Max_Failed_Times || failureCount > 32 * Max_Failed_Times){
                clog << "FastPlacer: FAILED. Too many failure. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            if(coarseIter == 0){
                clog << "FastPlacer: ALL FAILED. Nothing to try. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            toMap = Goorder[--coarseIter];
            verticesToTry = stackVerticesToTry.top();
            stackVerticesToTry.pop();
            clog << "FastPlacer: Roll back to: " << toMap << "; Candidates: " << verticesToTry.size() << endl;
            vector<pair<string, string>> edgesToDelte = stackEdgesToMap.top();
            stackEdgesToMap.pop();
            router.unroute(edgesToDelte);
            for(const auto &vertexDFG: coarse2DFG.find(toMap)->second){
                vertexRRG2DFG.erase(vertexDFG2RRG[vertexDFG]);
                vertexDFG2RRG.erase(vertexDFG);
            }
            RRG2coarse.erase(coarse2RRG[toMap]);
            coarse2RRG.erase(toMap);
        }
        clog << endl << endl;
    }

    clog << "FastPlacer: finished placing the DFG. Failure count: " << failureCount << "." << endl
         << endl;

    return{vertexDFG2RRG, router};

}

std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::placetop(const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compatFUs,
                                                                                 const FastRouter &routerInit, const std::unordered_map<std::string, std::string>  &usedDFG2RRGInitial,
                                                                                 const NetworkAnalyzerLegacy &analyzerInitial, const std::string &seed)
{
    const size_t Max_Failed_Times = 64;
    // Clear the previous status
    _failedVertices.clear();
    // Data
    FastRouter router(routerInit);

    unordered_map<string, string> vertexDFG2RRG = usedDFG2RRGInitial;
    unordered_map<string, string> vertexRRG2DFG;
    unordered_map<string, bool>   travesalVertexDFG;
    unordered_map<string, unordered_set<string>> compatNew = compatFUs;
    vector<string> unmappedOrder;
    unordered_map<string, size_t> orderSort;
    stack<vector<pair<string, string>>> stackEdgesToMap;
    stack<unordered_set<string>> stackVerticesToTry;
    for(const auto &vertexDFG: DFG.vertices())
    {
        travesalVertexDFG[vertexDFG.first] = false;
    }
    for(const auto &vertex: vertexDFG2RRG)
    {
        travesalVertexDFG[vertex.first] = true;
        vertexRRG2DFG[vertex.second] = vertex.first;
    }

    //route the different core edges
    srand(time(nullptr));
    for(const auto &vertex: DFG.vertices())
    {
        if(travesalVertexDFG[vertex.first])
        {
            for(const auto &edge: DFG.edgesOut(vertex.first))
            {
                if(travesalVertexDFG[edge.to()])
                {
                    assert(vertexDFG2RRG.find(edge.from()) != vertexDFG2RRG.end() && vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end());
                    const string &fromrrg = vertexDFG2RRG[edge.from()];
                    const string &torrg   = vertexDFG2RRG[edge.to()];
                    if(fromrrg.find("MEM") == string::npos && torrg.find("MEM") == string::npos)
                        continue;
                    if (router.paths().find(fromrrg) == router.paths().end() ||
                        router.paths().find(fromrrg)->second.find(torrg) == router.paths().find(fromrrg)->second.end())
                        {  
                            bool routed = router.strict_route({
                                                            {fromrrg, torrg},
                                                       },
                                                       {
                                                            edge.from(),
                                                       });
                            if(!routed){
                                clog << "FastPlacer: Choose a loose route." << endl;
                                routed = router.route({
                                                            {fromrrg, torrg},
                                                       },
                                                       {
                                                            edge.from(),
                                                       });
                            }
                            if(!routed)
                            {

                                clog << "FastRouter: FAILED, the path " << fromrrg << " -> " << torrg << " cannot route. " << endl;
                                //return {unordered_map<string, string>(), router};
                            }
                        }
                }
            }
        }
        else
        {
            unmappedOrder.push_back(vertex.first);
            orderSort[vertex.first] = 0;
        }
    }

    for(const auto &vertex: DFG.vertices())
    {
        if(travesalVertexDFG[vertex.first])
        {
            for(const auto &edge: DFG.edgesOut(vertex.first))
            {
                if(travesalVertexDFG[edge.to()])
                {
                    assert(vertexDFG2RRG.find(edge.from()) != vertexDFG2RRG.end() && vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end());
                    const string &fromrrg = vertexDFG2RRG[edge.from()];
                    const string &torrg   = vertexDFG2RRG[edge.to()];
                    if (router.paths().find(fromrrg) == router.paths().end() ||
                        router.paths().find(fromrrg)->second.find(torrg) == router.paths().find(fromrrg)->second.end())
                        {  
                            bool routed = router.strict_route({
                                                            {fromrrg, torrg},
                                                       },
                                                       {
                                                            edge.from(),
                                                       });
                            if(!routed){
                                clog << "FastPlacer: Choose a loose route." << endl;
                                routed = router.route({
                                                            {fromrrg, torrg},
                                                       },
                                                       {
                                                            edge.from(),
                                                       });
                            }
                            if(!routed)
                            {

                                clog << "FastRouter: FAILED, the path " << fromrrg << " -> " << torrg << " cannot route. " << endl;
                                return {unordered_map<string, string>(), router};
                            }
                        }
                }
            }
        }
        else
        {
            unmappedOrder.push_back(vertex.first);
            orderSort[vertex.first] = 0;
        }
    }

    //if only one part without I/O, it is finished.
    if(vertexDFG2RRG.size() == DFG.nVertices())
    {
        clog << "VanillaPlacer: Direct success. " << endl;
        return {vertexDFG2RRG, router};
    }
    clog << "VanillaPlacer: The number of unmapped nodes is " << unmappedOrder.size() << ". " << endl;
     //cout << compatible << endl;

    // Find compatible vertices in RRG for DFG vertices
    unordered_map<string, unordered_set<string>> device2vertexDFG;
    unordered_map<string, unordered_set<string>> compatibleVertexRRG;
    for (const auto &vertexDFG : compatNew) {
        for (const auto &deviceDFG : vertexDFG.second) {
            if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                device2vertexDFG[deviceDFG] = unordered_set<string>();
            }
            device2vertexDFG[deviceDFG].insert(vertexDFG.first);
        }
    }
    for (const auto &vertexRRG : _RRGAnalyzed.vertices()) {
        string device = vertexRRG.second.getAttr("device").getStr();
        if (device2vertexDFG.find(device) != device2vertexDFG.end()) {
            for (const auto &vertexDFG : device2vertexDFG[device]) {
                if (compatibleVertexRRG.find(vertexDFG) == compatibleVertexRRG.end()) {
                    compatibleVertexRRG[vertexDFG] = unordered_set<string>();
                }
                compatibleVertexRRG[vertexDFG].insert(vertexRRG.first);
            }
        }
    }
     // delete the placed status
    for (const auto &vertexDFG : DFG.vertices()) {
        clog <<vertexDFG.first << ": " << compatibleVertexRRG.find(vertexDFG.first)->second.size();
        if(travesalVertexDFG[vertexDFG.first]) {
            assert(vertexDFG2RRG.find(vertexDFG.first) != vertexDFG2RRG.end());
            compatibleVertexRRG[vertexDFG.first] = unordered_set<string>();
            compatibleVertexRRG[vertexDFG.first].insert(vertexDFG2RRG[vertexDFG.first]);
        } else {
            if (compatibleVertexRRG.find(vertexDFG.first) == compatibleVertexRRG.end()) {
                continue;
            }
            unordered_set<string> compatibles;
            for(const auto &deviceTmp: compatibleVertexRRG.find(vertexDFG.first)->second) {
                if(vertexRRG2DFG.find(deviceTmp) == vertexRRG2DFG.end()) {
                    compatibles.insert(deviceTmp);
                }
            }
            compatibleVertexRRG[vertexDFG.first] = compatibles;
        }
        clog << " -> " << compatibleVertexRRG.find(vertexDFG.first)->second.size() <<endl;;
    }

    //_deleteForbiddened(compatibleVertexRRG);
    for (const auto &item : compatibleVertexRRG) {
        string prefix = getPrefix(item.first);
        string postfix = getPostfix(item.first);
        if (!postfix.empty()) {
            if (item.second.size() != compatibleVertexRRG[prefix].size()) {
                unordered_set<string> tmp;
                for (const auto &port : item.second) {
                    if (compatibleVertexRRG[prefix].find(getPrefix(port)) != compatibleVertexRRG[prefix].end()) {
                        tmp.insert(port);
                    }
                }
                compatibleVertexRRG[item.first] = tmp;
            }
        }
    }

    // N-order validation
    // NOrderValidator validator(DFG, analyzerInitial.RRG(), compatNew);
    // size_t unplacibleCount = 0;
    // for (const auto &vertexDFG : DFG.vertices()) {
    //     if (!getPostfix(vertexDFG.first).empty() || travesalVertexDFG[vertexDFG.first]) {
    //         continue;
    //     }
    //     assert(compatibleVertexRRG.find(vertexDFG.first) != compatibleVertexRRG.end());
    //     unordered_set<string> compatibles;
    //     for (const auto &vertexRRG : compatibleVertexRRG[vertexDFG.first]) {
    //         clog << "\rVanillaPacker: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
    //         // if(validator.validateFast(vertexDFG.first, vertexRRG, 2) > 0)
    //         if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) {
    //             compatibles.insert(vertexRRG);
    //         }
    //     }
    //     clog << compatibleVertexRRG[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
    //     compatibleVertexRRG[vertexDFG.first] = compatibles;
    // }
    // clog << endl;

    // for (const auto &item : compatibleVertexRRG) {
    //     string prefix = getPrefix(item.first);
    //     string postfix = getPostfix(item.first);
    //     if (!postfix.empty()) {
    //         if (item.second.size() != compatibleVertexRRG[prefix].size()) {
    //             unordered_set<string> tmp;
    //             for (const auto &port : item.second) {
    //                 if (compatibleVertexRRG[prefix].find(getPrefix(port)) != compatibleVertexRRG[prefix].end()) {
    //                     tmp.insert(port);
    //                 }
    //             }
    //             compatibleVertexRRG[item.first] = tmp;
    //         }
    //     }
    // }
    // for (const auto &vertexDFG : DFG.vertices()) {
    //     if (compatibleVertexRRG.find(vertexDFG.first) == compatibleVertexRRG.end() ||
    //         compatibleVertexRRG[vertexDFG.first].empty()) {
    //         if (getPostfix(vertexDFG.first).empty()) {
    //             clog << "VanillaPlacer: Unplacible vertexDFG: " << vertexDFG.first << endl;
    //         }
    //         unplacibleCount++;
    //     }
    // }
    // if (unplacibleCount > 0) {
    //     clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
    //     return { unordered_map<string, string>(), router };
    // }

    for(const auto &node: unmappedOrder)
    {
        for(const auto &edge: DFG.edgesOut(node))
        {
            if(travesalVertexDFG[edge.to()])
            {
                orderSort[node] ++;
            }
        }
        for(const auto &edge: DFG.edgesIn(node))
        {
            if(travesalVertexDFG[edge.from()])
            {
                orderSort[node] ++;
            }
        }
    }
    cout << unmappedOrder << endl; cout << orderSort << endl;
    sort(unmappedOrder.begin(), unmappedOrder.end(), [&](const std::string &str1, const std::string &str2) {
        return orderSort[str1] > orderSort[str2];
        // if(orderSort[str1] > orderSort[str2]) {
        //     return true;
        // }
    });
    assert((unmappedOrder.size() + usedDFG2RRGInitial.size()) == DFG.nVertices());

    cout << unmappedOrder << endl;
    size_t furthest = 0;
    size_t nodeIter = 0;
    size_t failureCount = 0;
    cout << endl << "FastPlacer: Begin placing. " << endl;
    string toMap = unmappedOrder[nodeIter];
    unordered_set<string> verticesToTry = compatibleVertexRRG.find(toMap)->second;
    size_t numInit = vertexDFG2RRG.size();
    while(nodeIter < unmappedOrder.size()){
        furthest = max(furthest, nodeIter);
        clog << "FastPlacer: New iteration, size of stack: " << (vertexDFG2RRG.size() - numInit) << " / " << unmappedOrder.size() << "; furthest: " << furthest << endl;

        string toTry = "";
        unordered_set<string> toDelete;
        const string &toMapType = DFG.vertex(toMap).getAttr("optype").getStr();

        // Prepare to delete used RRG vertices
        for(const auto &vertexToTry: verticesToTry){
            if(vertexRRG2DFG.find(vertexToTry) != vertexRRG2DFG.end()){
                toDelete.insert(vertexToTry);
            }
        }
        // Prepare to unconnectable RRG vertice
        for(const auto &vertexToTry: verticesToTry){
            if(toDelete.find(vertexToTry) != toDelete.end()){
                continue;
            }
            bool available = true;
            unordered_multimap<string, string> linksToValidate;
            for(const auto &edge: DFG.edgesIn(toMap)){
                if(vertexDFG2RRG.find(edge.from()) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &fromRRG = vertexDFG2RRG[edge.from()];
                const string &toRRG = vertexToTry;// + "." + getPostfix(edge.to());
                linksToValidate.insert({fromRRG, toRRG});
            }
            for(const auto &edge: DFG.edgesOut(toMap)){
                if(vertexDFG2RRG.find(edge.to()) == vertexDFG2RRG.end()){
                    continue;
                }
                const string &toRRG = vertexDFG2RRG[edge.to()];
                const string &fromRRG = vertexToTry;// + "." + getPostfix(edge.from());
                linksToValidate.insert({fromRRG, toRRG});
            }
            for(const auto &link: linksToValidate){
                bool found = false;
                for(const auto &edge: _RRGAnalyzed.edgesOut(link.first)){
                    if(edge.to() == link.second){
                        found = true;
                        break;
                    }
                }
                if(!found){
                    available = false;
                    break;
                }
            }
            if(!available){
                toDelete.insert(vertexToTry);
            }
        }
        //Delete
        for (const auto &vertex : toDelete) {
            verticesToTry.erase(vertex);
        }
        
        clog << "FastPlacer: -> toMap: " << toMap << "; Candidates After Purge: " << verticesToTry.size() << endl;
        vector<string> verticesToTryRanked(verticesToTry.begin(), verticesToTry.end());
        vector<pair<string, string>> edgesToMap;
        vector<string> edgesSignal;
        bool failed = false;
        bool needToUnroute = failed;
        if (verticesToTry.empty()) {
            
            clog << "FastPlacer: Failed. Nothing to try for " << toMap << endl;
        } else {
            //  sort the candidates by sharedNet
            unordered_map<string, size_t> sharedNet;
            for(const auto &vertexToTry: verticesToTry){
                unordered_set<string> inPortRRG;
                unordered_set<string> outPortRRG;
                for(const auto &edge: DFG.edgesIn(toMap)){
                    const string &toRRG = vertexToTry;//+ "." + getPostfix(edge.to());
                    inPortRRG.insert(toRRG);
                }
                for(const auto &edge: DFG.edgesOut(toMap)){
                    const string &fromRRG = vertexToTry;// + "." + getPostfix(edge.from());
                    outPortRRG.insert(fromRRG);
                }
                for(const auto &inport: inPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesIn(inport)){
                        if(vertexRRG2DFG.find(edge.from()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
                for(const auto &outport: outPortRRG){
                    for(const auto &edge: _RRGAnalyzed.edgesOut(outport)){
                        if(vertexRRG2DFG.find(edge.to()) != vertexRRG2DFG.end()){
                            sharedNet[vertexToTry]++;
                        }
                    }
                }
            }
            cout << "before: " << verticesToTryRanked << endl;
            random_shuffle(verticesToTryRanked.end(), verticesToTryRanked.end());
            sort(verticesToTryRanked.begin(), verticesToTryRanked.end(), [&](const string &a,const string &b){return sharedNet[a] > sharedNet[b];});
            cout << "after: " << verticesToTryRanked << endl;
            // try map vertex
            bool isSuccess = false;
            size_t IterVertextoTry = 0;
            
            while(!isSuccess && IterVertextoTry < verticesToTryRanked.size()){
                toTry = verticesToTryRanked[IterVertextoTry++];
                clog << "FastPlacer: try to map " << toMap << " to " << toTry << endl;
                // -> Find edges that need to be mapped
                edgesToMap.clear();
                edgesSignal.clear();

                // -> Find edges that need to be mapped
                if (!isSuccess) {
                    for (const auto &edge : DFG.edgesIn(toMap)) {
                        if (travesalVertexDFG[edge.from()]) {
                            edgesToMap.push_back(pair<string, string>(vertexDFG2RRG[edge.from()], toTry));
                            edgesSignal.push_back(edge.from());
                        }
                    }
                    for (const auto &edge : DFG.edgesOut(toMap)) {
                        if (travesalVertexDFG[edge.to()]) {
                            edgesToMap.push_back(pair<string, string>(toTry, vertexDFG2RRG[edge.to()]));
                            edgesSignal.push_back(toMap);
                        }
                    }
                    clog << "FastPlacer: edgesToMap: " << edgesToMap.size() << " for " << toTry << endl;
                }

                if (!isSuccess && edgesToMap.size() > 0) {
                    isSuccess = router.route(edgesToMap, edgesSignal);
                    //failed = failed || !router.route(edgesToMap, edgesSignal);
                }
                if(isSuccess){
                    clog << "FastPlacer: Map " << toMap << " to " << toTry << ", Tried " << IterVertextoTry << " Time." << endl;
                } else {
                    clog << "FastPlacer: Fail to map " << toMap << endl;
                    needToUnroute = !failed;
                    failed = true;
                }
                

                string fuNameDFG = getPrefix(toMap);
                string portNameDFG = getPostfix(toMap);
                string fuNameRRG = portNameDFG.empty() ? toTry : getPrefix(toTry);
                unordered_set<string> fuPortsUnmappedDFG;
                unordered_set<string> fuInputsDFG;
                unordered_set<string> fuOutputsDFG;
                // bool isFirst = stackVertexDFG.empty() || getPrefix(stackVertexDFG.top()) != fuNameDFG;
                // if (!failed) {
                //     for (const auto &edge : DFG.edgesIn(fuNameDFG)) {
                //         assert(getPrefix(edge.from()) == fuNameDFG && edge.from().size() > fuNameDFG.size());
                //         string portNameTmp = getPostfix(edge.from());
                //         fuInputsDFG.insert(portNameTmp);
                //         if (edge.from() != toMap && !usedVertexDFG[edge.from()]) {
                //             fuPortsUnmappedDFG.insert(portNameTmp);
                //         }
                //     }
                //     for (const auto &edge : DFG.edgesOut(fuNameDFG)) {
                //         assert(getPrefix(edge.to()) == fuNameDFG && edge.to().size() > fuNameDFG.size());
                //         string portNameTmp = getPostfix(edge.to());
                //         fuOutputsDFG.insert(portNameTmp);
                //         if (edge.to() != toMap && !usedVertexDFG[edge.to()]) {
                //             fuPortsUnmappedDFG.insert(portNameTmp);
                //         }
                //     }
                //     if (isFirst) {
                //         clog << "VanillaPlacer: Checking unplaced port: ";
                //         for (const auto &port : fuPortsUnmappedDFG) {
                //             string vertexDFG = fuNameDFG + "." + port;
                //             string vertexRRG = fuNameRRG + "." + port;
                //             if (vertexRRG2DFG.find(vertexRRG) != vertexRRG2DFG.end()) {
                //                 clog << vertexRRG << " used by " << vertexRRG2DFG[vertexRRG] << "; ";
                //                 failed = failed || true;
                //             }
                //         }
                //         if (!portNameDFG.empty()) {
                //             string vertexDFG = fuNameDFG;
                //             string vertexRRG = fuNameRRG;
                //             if (vertexRRG2DFG.find(vertexRRG) != vertexRRG2DFG.end()) {
                //                 clog << vertexRRG << " used by " << vertexRRG2DFG[vertexRRG] << "; ";
                //                 failed = failed || true;
                //             }
                //         }
                //         clog << endl;
                //     }



                unordered_map<string, unordered_map<string, vector<string>>> pathsGiven;
                // if(toMapType == "Pack"){
                //     for(const auto &from: pack2routed.find(toMap)->second){
                //         string fromRRG = toTry + "." + from.first;
                //         if(pathsGiven.find(fromRRG) == pathsGiven.end()){
                //             pathsGiven[fromRRG] = unordered_map<string, vector<string>>();
                //         }
                //         for(const auto &to: from.second){
                //             string toRRG = toTry + "." + to.first;
                //             vector<string> pathRRG;
                //             for(const auto &node: to.second){
                //                 pathRRG.push_back(toTry + "." + node);
                //             }
                //             pathsGiven[fromRRG][toRRG] = pathRRG;
                //         }
                //     }
                // }
            //     for(const auto &vertex: DFG.vertices()){
            //         bool fromisused = vertexDFG2RRG.find(vertex.first) != vertexDFG2RRG.end();
            //         //bool fromistoMap = toMaps.find(vertex.first) != toMaps.end();
            //         for(const auto &edge: DFG.edgesOut(vertex.first)){
            //             bool toisused = vertexDFG2RRG.find(edge.to()) != vertexDFG2RRG.end();
            //             //bool toistoMap = toMaps.find(edge.to()) != toMaps.end();
            //             bool toRoute = (fromisused && toistoMap) || (fromistoMap && toisused) || (fromistoMap && toistoMap);
            //             if(!toRoute){
            //                 continue;
            //             }
            //             string fromRRG;
            //             string toRRG;
            //             if(fromisused){
            //                 fromRRG = vertexDFG2RRG[edge.from()];
            //             } else if(toMapType == "Coin"){
            //                 string portName = getPostfix(edge.from());
            //                 if(portName.empty()){
            //                     fromRRG = toTry;
            //                 } else {
            //                     fromRRG = toTry + "." + portName;
            //                 }
            //             } else {
            //                 fromRRG = toTry + "." + pack2mapped.find(toMap)->second.find(edge.from())->second;
            //             }
            //             if(toisused){
            //                 toRRG = vertexDFG2RRG[edge.to()];
            //             } else if(toMapType == "Coin"){
            //                 string portName = getPostfix(edge.to());
            //                 if(portName.empty()){
            //                     toRRG = toTry;
            //                 } else {
            //                     toRRG = toTry + "." + portName;
            //                 }
            //             } else {
            //                 toRRG = toTry + "." + pack2mapped.find(toMap)->second.find(edge.to())->second;
            //             }
            //             edgesToMap.push_back(pair<string, string>(fromRRG, toRRG));
            //             edgesSignal.push_back(edge.from());
            //         }
            //     }
            //     clog << "FastPlacer: edgesToMap: " << edgesToMap.size() << " for " << toTry << endl;
            //     verticesToTry.erase(toTry);
            //     if(edgesToMap.size() > 0){
            //         isSuccess = router.route(edgesToMap, edgesSignal, pathsGiven);
            //     } else {
            //         isSuccess = true;
            //     }
            // }
            // if(isSuccess){
            //     clog << "FastPlacer: Map " << toMap << " to " << toTry << ", Tried " << IterVertextoTry << " Time." << endl;
            // } else {
            //     clog << "FastPlacer: Fail to map " << toMap << endl;
            //     failed = true;
            }
        }

        if(!failed){
            // if(toMapType == "Coin"){
            //     for(const auto &vertexDFG: toMaps){
            //         string vertexRRG;
            //         string port = getPostfix(vertexDFG);
            //         if(port.empty()){
            //             vertexRRG = toTry;
            //         } else {
            //             vertexRRG = toTry + "." + port;
            //         }
            //         vertexDFG2RRG[vertexDFG] = vertexRRG;
            //         vertexRRG2DFG[vertexRRG] = vertexDFG;
            //     }
            // } else {
            //     for(const auto &vertexDFG: toMaps){
            //         string vertexRRG = pack2mapped.find(toMap)->second.find(vertexDFG)->second;
            //         vertexDFG2RRG[vertexDFG] = toTry + "." + vertexRRG;
            //         vertexRRG2DFG[vertexRRG] = toTry + "." + vertexDFG;
            //     }
            // }
            travesalVertexDFG[toMap] = true;
            vertexDFG2RRG[toMap] = toTry;
            vertexRRG2DFG[toTry] = toMap;
            stackVerticesToTry.push(verticesToTry);
            stackEdgesToMap.push(edgesToMap);

            if(++nodeIter < unmappedOrder.size()){
                toMap = unmappedOrder[nodeIter];
                verticesToTry = compatibleVertexRRG.find(toMap)->second;
            } else {
                break;
            }
        } else {
            failureCount++;
            if (_failedVertices.find(toMap) == _failedVertices.end()) {
                _failedVertices[toMap] = 0;
            }
            _failedVertices[toMap]++;
            if( _failedVertices[toMap] > Max_Failed_Times || failureCount > 32 * Max_Failed_Times){
                clog << "FastPlacer: FAILED. Too many failure. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            if(nodeIter == 0){
                clog << "FastPlacer: ALL FAILED. Nothing to try. " << endl;
                return {unordered_map<string, string>(), routerInit};
            }
            travesalVertexDFG[toMap] = false;
            toMap = unmappedOrder[--nodeIter];
            verticesToTry = stackVerticesToTry.top();
            stackVerticesToTry.pop();
            clog << "FastPlacer: Roll back to: " << toMap << "; Candidates: " << verticesToTry.size() << endl;
            vector<pair<string, string>> edgesToDelte = stackEdgesToMap.top();
            stackEdgesToMap.pop();
            router.unroute(edgesToDelte);
            vertexRRG2DFG.erase(vertexDFG2RRG[toMap]);
            vertexDFG2RRG.erase(toMap);
            // for(const auto &vertexDFG: coarse2DFG.find(toMap)->second){
            //     vertexRRG2DFG.erase(vertexDFG2RRG[vertexDFG]);
            //     vertexDFG2RRG.erase(vertexDFG);
            // }
            // RRG2coarse.erase(coarse2RRG[toMap]);
            // coarse2RRG.erase(toMap);
        }
        clog << endl << endl;
    }

    clog << "FastPlacer: finished placing the DFG. Failure count: " << failureCount << "." << endl
         << endl;

    return{vertexDFG2RRG, router};

}


std::pair<std::unordered_map<std::string, std::string>, FastRouter> FastPlacer::placeGivenCompat(const Graph &DFG, const unordered_map<string, unordered_set<string>> &compat)
{
    _failedVertices.clear();
    FastRouter router(_RRG, _FUs);

    unordered_map<string, unordered_set<string>> compatNew;
    for(const auto &vertex: compat){
        if(DFG.vertices().find(vertex.first) != DFG.vertices().end()){
            compatNew.insert(vertex);
        }
    }
    // N-order validation
    // Find compatible vertices in RRG for DFG vertices
    unordered_map<string, unordered_set<string>> device2vertexDFG;
    unordered_map<string, unordered_set<string>> compatibleVertexRRG;
    for (const auto &vertexDFG : compatNew) {
        for (const auto &deviceDFG : vertexDFG.second) {
            if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                device2vertexDFG[deviceDFG] = unordered_set<string>();
            }
            device2vertexDFG[deviceDFG].insert(vertexDFG.first);
        }
    }
    for (const auto &vertexRRG : _RRGAnalyzed.vertices()) {
        string device = vertexRRG.second.getAttr("device").getStr();
        if (device2vertexDFG.find(device) != device2vertexDFG.end()) {
            for (const auto &vertexDFG : device2vertexDFG[device]) {
                if (compatibleVertexRRG.find(vertexDFG) == compatibleVertexRRG.end()) {
                    compatibleVertexRRG[vertexDFG] = unordered_set<string>();
                }
                compatibleVertexRRG[vertexDFG].insert(vertexRRG.first);
            }
        }
    }
    NOrderValidator validator(DFG, _RRGAnalyzed, compatibleVertexRRG);
    size_t unplacibleCount = 0;
    for (const auto &vertexDFG : DFG.vertices()) {
        if (!getPostfix(vertexDFG.first).empty()) {
            continue;
        }
        if (compatibleVertexRRG.find(vertexDFG.first) == compatibleVertexRRG.end()) {
            clog << endl
                 << "FastPlacer: Compatible vertices NOT FOUND: " << vertexDFG.first << endl;
        }
        assert(compatibleVertexRRG.find(vertexDFG.first) != compatibleVertexRRG.end());
        unordered_set<string> compatibles;
        for (const auto &vertexRRG : compatibleVertexRRG[vertexDFG.first]) {
            // clog << "\rVanillaPlacer: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
            // if(validator.validate(vertexDFG.first, vertexRRG, 2) > 0)
            if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) {
                compatibles.insert(vertexRRG);
            }
        }
        // clog << compatibleVertexRRG[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
        compatibleVertexRRG[vertexDFG.first] = compatibles;
    }
    for (const auto &item : compatibleVertexRRG) {
        string prefix = getPrefix(item.first);
        string postfix = getPostfix(item.first);
        if (!postfix.empty()) {
            if (item.second.size() != compatibleVertexRRG[prefix].size()) {
                unordered_set<string> tmp;
                for (const auto &port : item.second) {
                    if (compatibleVertexRRG[prefix].find(getPrefix(port)) != compatibleVertexRRG[prefix].end()) {
                        tmp.insert(port);
                    }
                }
                compatibleVertexRRG[item.first] = tmp;
            }
        }
    }
    for (const auto &vertexDFG : DFG.vertices()) {
        if (compatibleVertexRRG.find(vertexDFG.first) == compatibleVertexRRG.end() ||
            compatibleVertexRRG[vertexDFG.first].empty()) {
            if (getPostfix(vertexDFG.first).empty()) {
                clog << "FastPlacer: Unplacible vertexDFG: " << vertexDFG.first << endl;
            }
            unplacibleCount++;
        }
    }
    if (unplacibleCount > 0) {
        clog << "FastPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
        return {unordered_map<string, string>(), router};
    }

    Graph coarseDFG;
    unordered_map<string, unordered_set<string>> coarse2DFG;
    vector<string> order;
    for(const auto &vertex: DFG.vertices()){
        string coarseName = getPrefix(vertex.first);
        if(coarse2DFG.find(coarseName) == coarse2DFG.end()){ 
            coarse2DFG[coarseName] = unordered_set<string>();
        }
        coarse2DFG[coarseName].insert(vertex.first);
        if(coarseName == vertex.first){
            Vertex vertex1(vertex.first);
            vertex1.setAttr("type", Attribute("Coin"));
            coarseDFG.addVertex(vertex1);
        }
    }
    for(const auto &vertex: DFG.vertices()){
        const string &fuFrom = getPrefix(vertex.first);
        for(const auto &edge: DFG.edgesOut(vertex.first)){
            const string &fuTo = getPrefix(edge.to());
            if(fuFrom != fuTo){
                Edge edge1(fuFrom, fuTo);
                edge1.setAttr("from", Attribute(edge.from()));
                edge1.setAttr("to", Attribute(edge.to()));
                coarseDFG.addEdge(edge1);
            }
        }
    }
    order = GraphSort::sortSTB(coarseDFG);
    

    return this->place(coarseDFG, DFG, compatibleVertexRRG, router, {}, {}, coarse2DFG, order);
}

}