#include "./FastPlace.h"

using namespace std;

namespace FastCGRA
{
    namespace FastPlace
    {
        bool PlaceCore(const std::string &dfg, const std::string &compat,const std::string &dfgGlobal, 
                    const std::string &rrg, const std::string &fus, 
                    const std::vector<std::string> &mappedPack, const std::vector<std::string> &routedPack,
                    const std::string &mapped, const std::string &routed, 
                    const std::string &sortMode,
                    const std::unordered_multimap<std::string, std::string> &forbidAdditional, const std::string &seed)
        {
            Graph graphDFG(dfg);//Origin DFG
            Graph globalDFG(dfgGlobal);
            clog << "Placement: DFG loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 

            Graph graphDFGwithIO(graphDFG);//DFG with IO inserted
            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat);//Compat for graphDFGwithIO
            unordered_map<string, unordered_set<string>> FUs = NetworkAnalyzer::parse(fus); 
            FUs["__INPUT_FU__"]  = {"in0", "out0"}; 
            FUs["__OUTPUT_FU__"] = {"in0", "out0"}; 

            //Insert IO port
            unordered_set<string> inputPorts; 
            unordered_set<string> outputPorts; 
            unordered_map<string, string> outputToVertex; 
            for(const auto &vertex: graphDFG.vertices())
            {
                for(const auto &edge: globalDFG.edgesIn(vertex.first))
                {
                    if(graphDFG.vertices().find(edge.from()) == graphDFG.vertices().end())
                    {
                        inputPorts.insert(edge.from()); 
                    }
                }
                for(const auto &edge: globalDFG.edgesOut(vertex.first))
                {
                    if(graphDFG.vertices().find(edge.to()) == graphDFG.vertices().end())
                    {
                        outputPorts.insert(edge.from()); 
                        outputToVertex[edge.from()] = edge.to(); 
                        break; //这样子一个节点会向外输出时就会走一条core间的路径，不会走太多
                    }
                }
            }
            
            size_t countInputPorts = 0; 
            for(const auto &inport: inputPorts)
            {
                string basename = string("__inserted_inport_") + num2str(countInputPorts++); 
                Vertex port   (basename); 
                Vertex portIn (basename + ".in0"); 
                Vertex portOut(basename + ".out0"); 
                graphDFGwithIO.addVertex(port); 
                graphDFGwithIO.addVertex(portIn); 
                graphDFGwithIO.addVertex(portOut); 
                graphDFGwithIO.addEdge(Edge(basename + ".in0", basename)); 
                graphDFGwithIO.addEdge(Edge(basename,          basename + ".out0")); 
                for(const auto &vertex: graphDFG.vertices())
                {
                    for(const auto &edge: globalDFG.edgesIn(vertex.first))
                    {
                        if(edge.from() == inport)
                        {
                            graphDFGwithIO.addEdge(Edge(basename + ".out0", vertex.first)); 

                            string fuDFG = getPrefix(edge.from()); 
                            assert(compatFUs.find(fuDFG) != compatFUs.end()); 
                            string fuRRG = *compatFUs[fuDFG].begin(); 

                        }
                    }
                }
                compatFUs[basename] = {"__INPUT_FU__"}; 
            }
          
            size_t countOutputPorts = 0; 
            for(const auto &outport: outputPorts)
            {
                string basename = string("__inserted_outport_") + num2str(countOutputPorts++); 
                Vertex port   (basename); 
                Vertex portIn (basename + ".in0"); 
                Vertex portOut(basename + ".out0"); 
                graphDFGwithIO.addVertex(port); 
                graphDFGwithIO.addVertex(portIn); 
                graphDFGwithIO.addVertex(portOut); 
                graphDFGwithIO.addEdge(Edge(basename + ".in0", basename)); 
                graphDFGwithIO.addEdge(Edge(basename,          basename + ".out0")); 
                graphDFGwithIO.addEdge(Edge(outport,           basename + ".in0")); 

                assert(outputToVertex.find(outport) != outputToVertex.end()); 
                string fuDFG = getPrefix(outputToVertex[outport]); 
                assert(compatFUs.find(fuDFG) != compatFUs.end()); 
                string fuRRG = *compatFUs[fuDFG].begin(); 

                compatFUs[basename] = {"__OUTPUT_FU__"}; 
            }
            clog << "Placement: DFGwithIO loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 
            string dfgRaw = dfg + ".raw";
            graphDFGwithIO.dump(dfgRaw);

            Graph graphRRG(rrg);
            //graphRRG = Utils::insertPortFU(graphRRG); //insertPortFU for __INPUT_FU__ & __OUTPUT_FU__ //TEST
            clog << "Placement: RRG loaded. " << "Vertices: " << graphRRG.nVertices() << "; " << "Edges: " << graphRRG.nEdges() << endl; 

            //ReadPack
            vector<unordered_map<string, string>> vertexDFG2RRGAll; 
            vector<unordered_map<string, unordered_map<string, vector<string>>>> pathsAll; 
            for(const auto &filename: mappedPack)
            {
                vertexDFG2RRGAll.push_back(Utils::readMap(filename)); 
            }
            for(const auto &filename: routedPack)
            {
                pathsAll.push_back(Utils::readPaths(filename));
            }
            //Update Compatible
            unordered_map<string, unordered_set<string>> compatNew = updateCompaTable(graphDFGwithIO, compatFUs, FUs);
            if(compatNew.empty()){
                WARN << "FastPlacement::Placement: WARN: Found Uncompatible vertex.";
                return false;
            }

            //N-order validation
            NetworkAnalyzerLegacy analyzer(FUs, graphRRG); 
            Graph &RRGAnalyzed = analyzer.RRG();

            NOrderValidator validator(graphDFGwithIO, RRGAnalyzed, compatNew);
            unordered_map<string, unordered_set<string>> device2vertexDFG;
            unordered_map<string, unordered_set<string>> compatibleVertexRRG;
            for (const auto &vertexDFG: compatNew) {
                for (const auto &deviceDFG: vertexDFG.second) {
                    if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                        device2vertexDFG[deviceDFG] = unordered_set<string>();
                    }
                    device2vertexDFG[deviceDFG].insert(vertexDFG.first);
                }
            }
            for (const auto &vertexRRG: RRGAnalyzed.vertices()) {
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
            deleteForbiddened(compatibleVertexRRG, forbidAdditional);
            size_t unplacibleCount = 0;
            for (const auto &vertexDFG : graphDFGwithIO.vertices()) {
                if (!getPostfix(vertexDFG.first).empty()) {
                    continue;
                }
                if (compatibleVertexRRG.find(vertexDFG.first) == compatibleVertexRRG.end()) {
                    WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
                    return false;
                }
                unordered_set<string> compatibles;
                for(const auto &vertexRRG: compatibleVertexRRG[vertexDFG.first]){
                    clog << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
                    // if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) { //NorderValidate
                        compatibles.insert(vertexRRG);
                    // }
                }
                clog << vertexDFG.first << ": " << compatibleVertexRRG[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
                compatibleVertexRRG[vertexDFG.first] = compatibles;
            }
            if(unplacibleCount > 0){
                clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
                return false;
            }

            // Contract DFG
            Graph coarseDFG;
            unordered_map<string, unordered_set<string>> coarse2Fine;
            unordered_map<string, string> fine2Contracted;
            unordered_map<string, size_t> packNames;
            for(size_t x = 0; x < vertexDFG2RRGAll.size(); x++){//different pack placed
                string packname = "__inserted_pack_" + to_string(x);
                packNames.insert({packname, x});
                coarse2Fine[packname] = unordered_set<string>();
                for(const auto &fine: vertexDFG2RRGAll[x]){//only one pack placed
                    if(graphDFG.vertices().find(fine.first) == graphDFG.vertices().end()){ 
                        WARN << "FastPlacement::Placement: Vertex " << fine.first << " in Mapped Pack " << x << " not Found.";
                        return false;
                    }
                    fine2Contracted[fine.first] = packname;
                    coarse2Fine[packname].insert(fine.first);
                }
            }
            for(const auto &vertex: graphDFGwithIO.vertices()){
                if(fine2Contracted.find(vertex.first) != fine2Contracted.end()){
                    const string &coarse = fine2Contracted[vertex.first];
                    if(packNames.find(coarse) != packNames.end()){
                        continue;
                    }
                } 
                string coinname = getPrefix(vertex.first);
                fine2Contracted[vertex.first] = coinname;
                if(coarse2Fine.find(coinname) == coarse2Fine.end()){
                    coarse2Fine[coinname] = unordered_set<string>();
                }
                coarse2Fine[coinname].insert(vertex.first);
            }
            for(const auto &coarse: coarse2Fine){
                Vertex vertex(coarse.first);
                if(packNames.find(coarse.first) != packNames.end()){
                    vertex.setAttr("type", Attribute("Pack"));
                } else {
                    vertex.setAttr("type", Attribute("Coin"));
                }
                coarseDFG.addVertex(vertex);
            }
            for(const auto &vertex: graphDFGwithIO.vertices()){
                const string &from = vertex.first;
                string fromContracted = fine2Contracted[vertex.first];
                for(const auto &edge: graphDFGwithIO.edgesOut(vertex.first)){
                    const string &to = edge.to();
                    string toContracted = fine2Contracted[edge.to()];
                    if(fromContracted != toContracted){
                        Edge edge1(fromContracted, toContracted);
                        edge1.setAttr("from", Attribute(from));
                        edge1.setAttr("to", Attribute(to));
                        coarseDFG.addEdge(edge1);
                    }
                }
            }
            //Update coarseCompatible
            unordered_map<string, unordered_set<string>> coarseCompatible;
            unordered_map<string, unordered_map<string, string>> pack2mapped;
            unordered_map<string, unordered_map<string, unordered_map<string, vector<string>>>> pack2routed;
            for(const auto &vertex: coarseDFG.vertices()){
                if(packNames.find(vertex.first) == packNames.end()){
                    coarseCompatible[vertex.first] = compatibleVertexRRG[vertex.first];
                }
            }
            for(const auto &pack: packNames){
                unordered_set<string> compatible;
                for(auto iter = coarse2Fine[pack.first].begin(); iter != coarse2Fine[pack.first].end(); iter++){
                    const string &vertex = *iter;
                    unordered_set<string> compatible1;
                    for(const auto &device: compatibleVertexRRG[vertex]){
                        const string &packDevice = getFront(device);
                        compatible1.insert(packDevice);
                    }
                    if(iter == coarse2Fine[pack.first].begin()){
                        compatible = compatible1;
                    } else {
                        for(auto jter = compatible.begin(); jter != compatible.end();){
                            if(compatible1.find(*jter) == compatible1.end()){
                                jter = compatible.erase(jter);
                            } else {
                                jter++;
                            }
                        }
                    }
                }
                coarseCompatible[pack.first] = compatible;
            }
            //Update pack2 mapped & routed
            for(const auto &packName: packNames){
                pack2mapped[packName.first] = vertexDFG2RRGAll[packName.second];
            }
            for(const auto &packName: packNames){
                pack2routed[packName.first] = pathsAll[packName.second];
            }

            FastPlacer placer(graphRRG, analyzer);
            FastRouter router(graphRRG, FUs);

            size_t count = 0; 
            while(count++ < 16)
            {
                unordered_map<string, string>                                vertexDFG2RRG; 
                unordered_map<string, unordered_map<string, vector<string>>> paths;

                vector<string> order;
                if(seed.empty()){
                    order = sortDFG(coarseDFG, sortMode);
                } else {
                    auto iter = fine2Contracted.find(seed);
                    if(iter == fine2Contracted.end())
                    {
                        WARN << "FastPlacement::Placement: Seed " << iter->second << " not Found";
                        return false;
                    }
                    order = sortDFG(coarseDFG, sortMode, iter->second);
                }

                pair<unordered_map<string, string>, FastRouter> result = placer.place(coarseDFG, graphDFGwithIO, coarseCompatible, router,
                                                                                        pack2mapped, pack2routed, coarse2Fine, order);
                size_t countFailed = 0;
                while(countFailed++ < 16 && result.first.empty())
                {
                    vector<string> failedVertices;
                    const unordered_map<string, size_t> &failures = placer.failedVertices();
                    for(const auto &vertex: failures)
                    {
                        failedVertices.push_back(vertex.first);
                    }
                    sort(failedVertices.begin(), failedVertices.end(), [&failures](const string &a, const string &b){
                        return failures.find(a)->second  > failures.find(b)->second; 
                    });
                    if(failedVertices.empty()){
                        continue;
                    }
                    if(countFailed < 8){
                        order = sortDFG(coarseDFG, sortMode, failedVertices[0]);
                    } else if (countFailed < 12){
                        order = sortDFG(coarseDFG, sortMode);
                    } else {
                        string seedInit = coarseDFG.vertexNames()[rand() % coarseDFG.vertexNames().size()];
                        order = sortDFG(coarseDFG, sortMode, seedInit);
                    }
                    result = placer.place(coarseDFG, graphDFGwithIO, coarseCompatible, router,
                                            pack2mapped, pack2routed, coarse2Fine, order);
                }
                if(!result.first.empty()){
                    Utils::writeMap  (result.first,          mapped + ".raw"); 
                    Utils::writePaths(result.second.paths(), routed + ".raw");

                    unordered_map<string, string> vertexDFG2RRG; 
                    for(const auto &vertex: result.first)
                    {
                        if(*compatFUs[getPrefix(vertex.first)].begin() != "__INPUT_FU__" && 
                        *compatFUs[getPrefix(vertex.first)].begin() != "__OUTPUT_FU__")
                        {
                            vertexDFG2RRG.insert(vertex); 
                        }
                    }

                    unordered_map<string, unordered_map<string, vector<string>>> paths; 
                    for(const auto &from: result.second.paths())
                    {
                        if(getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                        getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                        {
                            continue; 
                        }
                        if(paths.find(from.first) == paths.end())
                        {
                            paths[from.first] = unordered_map<string, vector<string>>(); 
                        }
                        for(const auto &to: from.second)
                        {
                            if(getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                            getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                            {
                                continue; 
                            }
                            assert(paths[from.first].find(to.first) == paths[from.first].end()); 
                            paths[from.first][to.first] = to.second; 
                        }
                    }
                    Utils::writeMap(vertexDFG2RRG, mapped); 
                    Utils::writePaths(paths, routed); 
                    return true; 
                }
            }


            return false;
        }
        //jntest
        bool PlaceCore(const std::string &dfg, const std::string &compat,const std::string &dfgGlobal, 
                    const std::string &rrg, const std::string &fus, 
                    const std::unordered_map<std::string, std::string> blocks,
                    const std::string &mapped, const std::string &routed, 
                    const std::vector<std::string> &blocktypes,
                    const std::unordered_map<std::string, std::vector<std::string>> &type2block,
                    const std::string &sortMode,
                    const std::unordered_multimap<std::string, std::string> &forbidAdditional, const std::string &seed)
        {
            Graph graphDFG(dfg);//Origin DFG
            Graph globalDFG(dfgGlobal);
            clog << "Placement: DFG loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 

            Graph graphDFGwithIO(graphDFG);//DFG with IO inserted
            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat);//Compat for graphDFGwithIO
            unordered_map<string, unordered_set<string>> FUs = NetworkAnalyzer::parse(fus); 
            FUs["__INPUT_FU__"]  = {"in0", "out0"}; 
            FUs["__OUTPUT_FU__"] = {"in0", "out0"}; 

            //Insert IO port
            unordered_set<string> inputPorts; 
            unordered_set<string> outputPorts; 
            unordered_map<string, string> outputToVertex; 
            for(const auto &vertex: graphDFG.vertices())
            {
                for(const auto &edge: globalDFG.edgesIn(vertex.first))
                {
                    if(graphDFG.vertices().find(edge.from()) == graphDFG.vertices().end())
                    {
                        inputPorts.insert(edge.from()); 
                    }
                }
                for(const auto &edge: globalDFG.edgesOut(vertex.first))
                {
                    if(graphDFG.vertices().find(edge.to()) == graphDFG.vertices().end())
                    {
                        outputPorts.insert(edge.from()); 
                        outputToVertex[edge.from()] = edge.to(); 
                        break; 
                    }
                }
            }
            
            size_t countInputPorts = 0; 
            for(const auto &inport: inputPorts)
            {
                string basename = string("__inserted_inport_") + num2str(countInputPorts++); 
                Vertex port   (basename); 
                Vertex portIn (basename + ".in0"); 
                Vertex portOut(basename + ".out0"); 
                graphDFGwithIO.addVertex(port); 
                graphDFGwithIO.addVertex(portIn); 
                graphDFGwithIO.addVertex(portOut); 
                graphDFGwithIO.addEdge(Edge(basename + ".in0", basename)); 
                graphDFGwithIO.addEdge(Edge(basename,          basename + ".out0")); 
                for(const auto &vertex: graphDFG.vertices())
                {
                    for(const auto &edge: globalDFG.edgesIn(vertex.first))
                    {
                        if(edge.from() == inport)
                        {
                            graphDFGwithIO.addEdge(Edge(basename + ".out0", vertex.first)); 

                            string fuDFG = getPrefix(edge.from()); 
                            assert(compatFUs.find(fuDFG) != compatFUs.end()); 
                            string fuRRG = *compatFUs[fuDFG].begin(); 

                        }
                    }
                }
                compatFUs[basename] = {"__INPUT_FU__"}; 
            }
          
            size_t countOutputPorts = 0; 
            for(const auto &outport: outputPorts)
            {
                string basename = string("__inserted_outport_") + num2str(countOutputPorts++); 
                Vertex port   (basename); 
                Vertex portIn (basename + ".in0"); 
                Vertex portOut(basename + ".out0"); 
                graphDFGwithIO.addVertex(port); 
                graphDFGwithIO.addVertex(portIn); 
                graphDFGwithIO.addVertex(portOut); 
                graphDFGwithIO.addEdge(Edge(basename + ".in0", basename)); 
                graphDFGwithIO.addEdge(Edge(basename,          basename + ".out0")); 
                graphDFGwithIO.addEdge(Edge(outport,           basename + ".in0")); 

                assert(outputToVertex.find(outport) != outputToVertex.end()); 
                string fuDFG = getPrefix(outputToVertex[outport]); 
                assert(compatFUs.find(fuDFG) != compatFUs.end()); 
                string fuRRG = *compatFUs[fuDFG].begin(); 

                compatFUs[basename] = {"__OUTPUT_FU__"}; 
            }
            clog << "Placement: DFGwithIO loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 
            string dfgRaw = dfg + ".raw";
            graphDFGwithIO.dump(dfgRaw);

            Graph graphRRG(rrg);
            graphRRG = Utils::insertPortFU(graphRRG); //insertPortFU for __INPUT_FU__ & __OUTPUT_FU__ //TEST
            clog << "Placement: RRG loaded. " << "Vertices: " << graphRRG.nVertices() << "; " << "Edges: " << graphRRG.nEdges() << endl; 

            //ReadPack
            unordered_map<string, vector<unordered_map<string, string>>> BLOCKvertexDFG2RRGAll;
            unordered_map<string, vector<unordered_map<string, unordered_map<string, vector<string>>>>> BLOCKpathsAll;
            vector<string> packlog;
            for(const auto &block: blocks){
                string blockname = blocktypes[str2num<size_t>(block.second)];
                unordered_map<string, string> packplaced = Utils::readMap(block.first.substr(0, block.first.size() - 3) + "placed.txt");
                cout << block.first.substr(0, block.first.size() - 3) + "placed.txt" << endl;
                if(graphDFG.vertices().find(packplaced.begin()->first) != graphDFG.vertices().end())
                {
                    if(BLOCKvertexDFG2RRGAll.find(blockname) == BLOCKvertexDFG2RRGAll.end()){
                        BLOCKvertexDFG2RRGAll[blockname] = vector<unordered_map<string, string>> ();
                    }
                    cout << blockname << " : " << packplaced << endl;
                    BLOCKvertexDFG2RRGAll[blockname].push_back(packplaced); 
                    BLOCKpathsAll[blockname].push_back(Utils::readPaths(block.first.substr(0, block.first.size() - 3) + "routed.txt")); 
                    packlog.push_back(block.first);
                }
            }
            clog << "FastPlacement::Reading pack: " << endl << packlog << endl;

            //Update Compatible
            unordered_map<string, unordered_set<string>> compatNew = updateCompaTable(graphDFGwithIO, compatFUs, FUs);
            if(compatNew.empty()){
                WARN << "FastPlacement::Placement: WARN: Found Uncompatible vertex.";
                return false;
            }
            NetworkAnalyzerLegacy analyzer(FUs, graphRRG); 
            Graph &RRGAnalyzed = analyzer.RRG();
            unordered_map<string, unordered_set<string>> device2vertexDFG;
            unordered_map<string, unordered_set<string>> compatibleVertexRRG;
            for (const auto &vertexDFG: compatNew) {
                for (const auto &deviceDFG: vertexDFG.second) {
                    if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                        device2vertexDFG[deviceDFG] = unordered_set<string>();
                    }
                    device2vertexDFG[deviceDFG].insert(vertexDFG.first);
                }
            }
            for (const auto &vertexRRG: RRGAnalyzed.vertices()) {
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
            
            
            // Coarsen DFG
            Graph coarseDFG;
            unordered_map<string, unordered_set<string>> coarse2fine;
            unordered_map<string, string> fine2coarse;
            unordered_map<string, size_t> packNames;
            for(const auto &vertexDFG2RRGAll: BLOCKvertexDFG2RRGAll)
            {
                for(size_t x = 0; x < vertexDFG2RRGAll.second.size(); x++){
                    string packname = "__block" + vertexDFG2RRGAll.first + "_" + to_string(x);
                    packNames.insert({packname, x});
                    coarse2fine[packname] = unordered_set<string>();
                    for(const auto &fine: vertexDFG2RRGAll.second[x]){
                        if(graphDFG.vertices().find(fine.first) == graphDFG.vertices().end()){ 
                            WARN << "FastPlacement::Placement: Vertex " << fine.first << " in Mapped Pack " << vertexDFG2RRGAll.first << " not Found.";
                            return false;
                        }
                    fine2coarse[fine.first] = packname;
                    coarse2fine[packname].insert(fine.first);
                    }
                }
            }
            for(const auto &vertex: graphDFGwithIO.vertices()){
                if(fine2coarse.find(vertex.first) != fine2coarse.end()){ //nodes except pack nodes
                    const string &coarse = fine2coarse[vertex.first];
                    if(packNames.find(coarse) != packNames.end()){//if find pack nodes
                        continue;
                    }
                } 
                string coinname = getPrefix(vertex.first);
                fine2coarse[vertex.first] = coinname;
                if(coarse2fine.find(coinname) == coarse2fine.end()){
                    coarse2fine[coinname] = unordered_set<string>();
                }
                coarse2fine[coinname].insert(vertex.first);
            }
            for(const auto &coarse: coarse2fine){
                Vertex vertex(coarse.first);
                if(packNames.find(coarse.first) != packNames.end()){
                    vertex.setAttr("type", Attribute("Pack"));
                } else {
                    vertex.setAttr("type", Attribute("Coin"));
                }
                coarseDFG.addVertex(vertex);
            }
            for(const auto &vertex: graphDFGwithIO.vertices()){
                const string &from = vertex.first;
                string fromCoarse = fine2coarse[vertex.first];
                for(const auto &edge: graphDFGwithIO.edgesOut(vertex.first)){
                    const string &to = edge.to();
                    string toCoarse = fine2coarse[edge.to()];
                    if(fromCoarse != toCoarse){
                        Edge edge1(fromCoarse, toCoarse);
                        edge1.setAttr("from", Attribute(from));
                        edge1.setAttr("to", Attribute(to));
                        coarseDFG.addEdge(edge1);
                    }
                }
            }

            //Update coarseCompatible
            unordered_map<string, unordered_set<string>> coarseCompatible;
            for(const auto &vertex: coarseDFG.vertices()){
                if(packNames.find(vertex.first) == packNames.end()){ //other nodes
                    coarseCompatible[vertex.first] = compatibleVertexRRG[vertex.first];
                }
            }
            // pack compat
            for(const auto &pack: packNames){
                unordered_set<string> compatible;
                for(auto iter = coarse2fine[pack.first].begin(); iter != coarse2fine[pack.first].end(); iter++){
                    const string &vertex = *iter;
                    unordered_set<string> compatible1;
                    for(const auto &device: compatibleVertexRRG[vertex]){
                        const string &packDevice = getFront(device);
                        compatible1.insert(packDevice);
                    }
                    if(iter == coarse2fine[pack.first].begin()){
                        compatible = compatible1;
                    } else {
                        for(auto jter = compatible.begin(); jter != compatible.end();){
                            if(compatible1.find(*jter) == compatible1.end()){
                                jter = compatible.erase(jter);
                            } else {
                                jter++;
                            }
                        }
                    }
                }
                coarseCompatible[pack.first] = compatible;
            }

            FastPlacer placer(graphRRG, analyzer);
            FastRouter router(graphRRG, FUs);

            size_t count = 0; 
            size_t maxTry = 0;
            while(count < 32 && maxTry < 1024)
            {
                unordered_map<string, string>                                vertexDFG2RRG; 
                unordered_map<string, unordered_map<string, vector<string>>> paths;

                unordered_map<string, vector<unordered_map<string, string>>>                                vecvertexDFG2RRG; 
                unordered_map<string, vector<unordered_map<string, unordered_map<string, vector<string>>>>> vecpaths;
                for(const auto &type: type2block)
                {
                    // unordered_map<string, vector<size_t>> blocknum;
                    vector<size_t> blocknum;
                    for(size_t idx=0; idx < type.second.size(); idx++)
                    {
                        blocknum.push_back(idx);
                    }
                    random_shuffle(blocknum.begin(), blocknum.end());
                    clog << blocknum << endl;
                    while(vecvertexDFG2RRG[type.first].size() < blocknum.size())
                    {
                        vecvertexDFG2RRG[type.first].push_back(unordered_map<string, string>());
                        vecpaths[type.first].push_back(unordered_map<string, unordered_map<string, vector<string>>>());

                    }
                    for(size_t idx=0; idx < BLOCKvertexDFG2RRGAll[type.first].size(); idx++)
                    {
                        vecvertexDFG2RRG[type.first][blocknum[idx]] = BLOCKvertexDFG2RRGAll[type.first][idx];
                        vecpaths[type.first][blocknum[idx]] = BLOCKpathsAll[type.first][idx];
                    }
                }
                for(const auto &vecvertexdfg2rrg: vecvertexDFG2RRG)
                { 
                    for(size_t idx=0; idx < vecvertexdfg2rrg.second.size(); idx++)
                    {
                        for(const auto &vertex: vecvertexdfg2rrg.second[idx])
                        {
                            vertexDFG2RRG[vertex.first] = type2block.find(vecvertexdfg2rrg.first)->second[idx] + "." + vertex.second;
                        }
                        for(const auto &from: vecpaths[vecvertexdfg2rrg.first][idx])
                        {
                            string fromName = type2block.find(vecvertexdfg2rrg.first)->second[idx] + "." + from.first;
                            if(paths.find(fromName) == paths.end())
                            {
                                paths[fromName] = unordered_map<string, vector<string>>();
                            }
                            for(const auto &to: from.second)
                            {
                                string toName = type2block.find(vecvertexdfg2rrg.first)->second[idx] + "." + to.first;
                                if(paths[fromName].find(toName) == paths[fromName].end())
                                {
                                    paths[fromName][toName] = vector<string>();
                                }
                                for(const auto &node: to.second)
                                {
                                    paths[fromName][toName].push_back(type2block.find(vecvertexdfg2rrg.first)->second[idx] + "." + node);
                                }
                            }
                        }
                    }
                }

                Graph contracted;
                for(const auto &vertex: graphDFG.vertices()){
                    if(getPostfix(vertex.first).empty()){
                        contracted.addVertex(Vertex(vertex.first));
                    }
                }
                for(const auto &vertex: graphDFG.vertices()){
                    for(const auto &edge: graphDFG.edgesOut(vertex.first)){
                        string from = getPrefix(edge.from());
                        string to = getPrefix(edge.to());
                        if(from != to){
                            contracted.addEdge(Edge(from, to));
                        }
                    }
                }
                unordered_map<string, unordered_set<string>> compatibleVertexRRG1 = compatibleVertexRRG;
                for(auto iter = compatibleVertexRRG1.begin(); iter != compatibleVertexRRG1.end(); iter++){
                    string vertexDFG = iter->first;
                    if(vertexDFG2RRG.find(vertexDFG) != vertexDFG2RRG.end()){
                        iter->second = {vertexDFG2RRG[vertexDFG]};
                    }
                }
                NOrderValidator validator(graphDFGwithIO, RRGAnalyzed, compatibleVertexRRG1);
                bool unplacible = false;
                for (const auto &vertexDFG : graphDFGwithIO.vertices()) {
                    if (!getPostfix(vertexDFG.first).empty()) {
                        continue;
                    }
                    if (compatibleVertexRRG1.find(vertexDFG.first) == compatibleVertexRRG1.end()) {
                        WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
                        return false;
                    }
                    unordered_set<string> compatibles;
                    for(const auto &vertexRRG: compatibleVertexRRG1[vertexDFG.first]){
                        cout << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
                        if (validator.validateSlow(vertexDFG.first, vertexRRG, 1)) { //NorderValidate
                            compatibles.insert(vertexRRG);
                        }
                    }
                    clog << vertexDFG.first << ": " << compatibleVertexRRG1[vertexDFG.first].size() << " -> " << compatibles.size() << endl;
                    if(compatibles.empty()){
                        unplacible = true;
                        clog << compatibleVertexRRG1[vertexDFG.first] << endl;
                        for(const auto &edge: contracted.edgesIn(vertexDFG.first)){
                            clog << "  <-" << edge.from() << ":" << compatibleVertexRRG1[edge.from()] << endl;
                        }
                        for(const auto &edge: contracted.edgesOut(vertexDFG.first)){
                            clog << "  ->" << edge.to() << ":" << compatibleVertexRRG1[edge.to()] << endl;
                        }
                        break;
                    }
                    compatibleVertexRRG1[vertexDFG.first] = compatibles;
                }

                if(unplacible){
                    clog << "FastPlacement: -> Validation Fail, retry initialization." << endl;
                    cout << "FastPlacement: -> Validation Fail, retry initialization." << endl;
                    maxTry++;
                    continue;
                }
                count++;

                cout << "FastPlacement: -> Pass Validation. Try to place, " << count << " time." << endl;
                clog << "FastPlacement: -> Pass Validation. Try to place, " << count << " time." << endl;

                unordered_multimap<string, pair<string, string>> usedNode;
                unordered_map<string, string>                    usedSignal;
                unordered_map<string, string>                    vertexRRG2DFG;
                for(const auto &vertex: vertexDFG2RRG)
                {
                    vertexRRG2DFG[vertex.second] = vertex.first;
                }
                for(const auto &from: paths)
                {
                    for(const auto &to: from.second)
                    {
                        for(const auto &node: to.second)
                        {
                            usedNode.insert({node, {from.first, to.first}});
                            assert(usedSignal.find(node) == usedSignal.end() || usedSignal[node] == vertexRRG2DFG[from.first]);
                            usedSignal[node] = vertexRRG2DFG[from.first];
                        }
                    }
                }
                router.usedNode() = usedNode;
                router.usedSignal() = usedSignal;
                router.paths() = paths;

                unordered_map<string, unordered_set<string>> coarseCompatible1 = coarseCompatible;
                for(const auto &vertex: compatibleVertexRRG1){
                    if(coarseCompatible1.find(vertex.first) != coarseCompatible1.end()){
                        coarseCompatible1[vertex.first] = vertex.second;
                    }
                }

                vector<string> order;
                if(seed.empty()){
                    order = sortDFG(coarseDFG, sortMode);
                } else {
                    auto iter = fine2coarse.find(seed);
                    if(iter == fine2coarse.end())
                    {
                        WARN << "FastPlacement::Placement: Seed " << iter->second << " not Found";
                        return false;
                    }
                    order = sortDFG(coarseDFG, sortMode, iter->second);
                }
                for(const auto &packname: packNames)
                {
                    auto del = find(order.begin(),order.end(),packname.first);
                    if(del != order.end())
                        order.erase(del);
                }
                vector<string> ordInit = order;
                pair<unordered_map<string, string>, FastRouter> result = placer.place_v2(coarseDFG, graphDFGwithIO, coarseCompatible1, router, 
                                                                                        vertexDFG2RRG, coarse2fine, order);
                size_t countFailed = 0;
                while(countFailed++ < 4 && result.first.empty())
                {
                    clog << "FastPlacement::Placement: Retry " << countFailed << " time." << endl;
                    vector<string> failedVertices;
                    const unordered_map<string, size_t> &failures = placer.failedVertices();
                    for(const auto &vertex: failures)
                    {
                        failedVertices.push_back(vertex.first);
                    }
                    sort(failedVertices.begin(), failedVertices.end(), [&failures](const string &a, const string &b){
                        return failures.find(a)->second  > failures.find(b)->second; 
                    });
                    if(failedVertices.empty()){
                        continue;
                    }
                    if(countFailed < 2){
                        order = sortDFG(coarseDFG, sortMode, failedVertices[0]);
                    } else if (countFailed < 3){
                        order = sortDFG(coarseDFG, sortMode);
                    } else {
                        string seedInit = coarseDFG.vertexNames()[rand() % coarseDFG.vertexNames().size()];
                        order = sortDFG(coarseDFG, sortMode, seedInit);
                    }
                    result = placer.place_v2(coarseDFG, graphDFGwithIO, coarseCompatible1, router, 
                                                vertexDFG2RRG, coarse2fine, ordInit);
                }
                if(!result.first.empty()){
                    Utils::writeMap  (result.first,          mapped + ".raw"); 
                    Utils::writePaths(result.second.paths(), routed + ".raw");

                    unordered_map<string, string> vertexDFG2RRG; 
                    for(const auto &vertex: result.first)
                    {
                        if(*compatFUs[getPrefix(vertex.first)].begin() != "__INPUT_FU__" && 
                        *compatFUs[getPrefix(vertex.first)].begin() != "__OUTPUT_FU__")
                        {
                            vertexDFG2RRG.insert(vertex); 
                        }
                    }

                    unordered_map<string, unordered_map<string, vector<string>>> paths; 
                    for(const auto &from: result.second.paths())
                    {
                        if(getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                        getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                        {
                            continue; 
                        }
                        if(paths.find(from.first) == paths.end())
                        {
                            paths[from.first] = unordered_map<string, vector<string>>(); 
                        }
                        for(const auto &to: from.second)
                        {
                            if(getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                            getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                            {
                                continue; 
                            }
                            assert(paths[from.first].find(to.first) == paths[from.first].end()); 
                            paths[from.first][to.first] = to.second; 
                        }
                    }
                    Utils::writeMap(vertexDFG2RRG, mapped); 
                    Utils::writePaths(paths, routed); 
                    return true; 
                }
            }
            cout << dfg << " is failed. " << endl;


            return false;
        }

        bool PlaceCoreII(const std::string &dfg, const std::string &compat,
                    const std::string &rrg, const std::string &fus, 
                    const std::string &mapped, const std::string &routed, 
                    const size_t intial_interval,
                    const std::string &sortMode,
                    const std::unordered_multimap<std::string, std::string> &forbidAdditional,const std::string &seed)
        {
            time_t timeStart, timeEnd;
            time(&timeStart);

            Graph graphDFG(dfg);//Origin DFG
            clog << "Placement: DFG loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 

            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat);//Compat for graphDFGwithIO
            unordered_map<string, unordered_set<string>> FUs = NetworkAnalyzer::parse(fus); 
            FUs["__INPUT_FU__"]  = {"in0", "out0"}; 
            FUs["__OUTPUT_FU__"] = {"in0", "out0"}; 

            Graph graphRRG(rrg);
            graphRRG = Utils::insertPortBlock(graphRRG); //insertPortFU for __INPUT_FU__ & __OUTPUT_FU__ 
            clog << "Placement: RRG loaded. " << "Vertices: " << graphRRG.nVertices() << "; " << "Edges: " << graphRRG.nEdges() << endl; 

            //Update Compatible
            unordered_map<string, unordered_set<string>> compatNew = updateCompaTable(graphDFG, compatFUs, FUs);
            if(compatNew.empty()){
                WARN << "FastPlacement::Placement: WARN: Found Uncompatible vertex.";
                return false;
            }

            //gene MRRG
            Graph MRRG;
            MRRG = Utils::genMRRG(graphRRG, FUs, intial_interval);

            unordered_map<string, unordered_set<string>> device2vertexDFG;
            unordered_map<string, unordered_set<string>> compatibleVertexMRRG;
            NetworkAnalyzerLegacy analyzer(FUs, MRRG); 
            Graph &MRRGAnalyzed = analyzer.RRG();
            for (const auto &vertexDFG: compatNew) {
                for (const auto &deviceDFG: vertexDFG.second) {
                    if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                        device2vertexDFG[deviceDFG] = unordered_set<string>();
                    }
                    device2vertexDFG[deviceDFG].insert(vertexDFG.first);
                }
            }
            for (const auto &vertexRRG: MRRGAnalyzed.vertices()) {
                string device = vertexRRG.second.getAttr("device").getStr();
                if (device2vertexDFG.find(device) != device2vertexDFG.end()) {
                    for (const auto &vertexDFG : device2vertexDFG[device]) {
                        if (compatibleVertexMRRG.find(vertexDFG) == compatibleVertexMRRG.end()) {
                            compatibleVertexMRRG[vertexDFG] = unordered_set<string>();
                        }
                        compatibleVertexMRRG[vertexDFG].insert(vertexRRG.first);
                    }
                }
            }

            // Coarsen DFG
            Graph coarseDFG;
            unordered_map<string, unordered_set<string>> coarse2fine;
            unordered_map<string, string> fine2coarse;
            for(const auto &vertex: graphDFG.vertices()){
                // if(fine2coarse.find(vertex.first) != fine2coarse.end()){ //nodes except pack nodes
                //     const string &coarse = fine2coarse[vertex.first];
                // } 
                string coinname = getPrefix(vertex.first);
                fine2coarse[vertex.first] = coinname;
                if(coarse2fine.find(coinname) == coarse2fine.end()){
                    coarse2fine[coinname] = unordered_set<string>();
                }
                coarse2fine[coinname].insert(vertex.first);
            }
            for(const auto &coarse: coarse2fine){
                Vertex vertex(coarse.first);
                vertex.setAttr("type", Attribute("Coin"));
                coarseDFG.addVertex(vertex);
            }
            for(const auto &vertex: graphDFG.vertices()){
                const string &from = vertex.first;
                string fromCoarse = fine2coarse[vertex.first];
                for(const auto &edge: graphDFG.edgesOut(vertex.first)){
                    const string &to = edge.to();
                    string toCoarse = fine2coarse[edge.to()];
                    if(fromCoarse != toCoarse){
                        Edge edge1(fromCoarse, toCoarse);
                        edge1.setAttr("from", Attribute(from));
                        edge1.setAttr("to", Attribute(to));
                        coarseDFG.addEdge(edge1);
                    }
                }
            }

            //Update coarseCompatible
            unordered_map<string, unordered_set<string>> coarseCompatible;
            for(const auto &vertex: coarseDFG.vertices()){
                coarseCompatible[vertex.first] = compatibleVertexMRRG[vertex.first];
            }


            FastPlacer placer(MRRG, analyzer);
            FastRouter router(MRRG, FUs);
            size_t count = 0; 
            while(count++ < 16)
            {
                vector<string> order;
                if(seed.empty()){
                    order = sortDFG(coarseDFG, sortMode);
                } else {
                    auto iter = fine2coarse.find(seed);
                    if(iter == fine2coarse.end())
                    {
                        WARN << "FastPlacement::Placement: Seed " << iter->second << " not Found";
                        return false;
                    }
                    order = sortDFG(coarseDFG, sortMode, iter->second);
                }
                pair<unordered_map<string, string>, FastRouter> result = placer.placeII(coarseDFG, graphDFG, coarseCompatible, router, 
                                                                                        {}, {}, coarse2fine, order);
                if(!result.first.empty()){
                    time(&timeEnd);
                    double cost=difftime(timeEnd, timeStart);
                    cout << dfg << " is success. " << endl;
                    cout << "cost " << cost << " s." << endl;

                    Utils::writeMap  (result.first,          mapped); 
                    Utils::writePaths(result.second.paths(), routed);

                    return true;
                }
            }

            cout << "Failed to map. " << dfg << endl;
            return false;

        }

        bool PlaceBlock(const std::string &dfg, const std::string &compat,const std::string &dfgGlobal, 
                    const std::string &rrg, const std::string &fus,
                    const std::string &mapped, const std::string &routed, 
                    const std::string &sortMode,
                    const std::unordered_multimap<std::string, std::string> &forbidAdditional, const std::string &seed)
        {
            Graph graphDFG(dfg);//Origin DFG
            Graph globalDFG(dfgGlobal);
            clog << "Placement: DFG loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 

            Graph graphDFGwithIO(graphDFG);//DFG with IO inserted
            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat);//Compat for graphDFGwithIO
            unordered_map<string, unordered_set<string>> FUs = NetworkAnalyzer::parse(fus); 
            FUs["__INPUT_FU__"]  = {"in0", "out0"}; 
            FUs["__OUTPUT_FU__"] = {"in0", "out0"}; 

            //Insert IO port
            unordered_set<string> inputPorts; 
            unordered_set<string> outputPorts; 
            unordered_map<string, string> outputToVertex; 
            for(const auto &vertex: graphDFG.vertices())
            {
                for(const auto &edge: globalDFG.edgesIn(vertex.first))
                {
                    if(graphDFG.vertices().find(edge.from()) == graphDFG.vertices().end())
                    {
                        inputPorts.insert(edge.from()); 
                    }
                }
                for(const auto &edge: globalDFG.edgesOut(vertex.first))
                {
                    if(graphDFG.vertices().find(edge.to()) == graphDFG.vertices().end())
                    {
                        outputPorts.insert(edge.from()); 
                        outputToVertex[edge.from()] = edge.to(); 
                        break; 
                    }
                }
            }
            
            size_t countInputPorts = 0; 
            for(const auto &inport: inputPorts)
            {
                string basename = string("__inserted_inport_") + num2str(countInputPorts++); 
                Vertex port   (basename); 
                Vertex portIn (basename + ".in0"); 
                Vertex portOut(basename + ".out0"); 
                graphDFGwithIO.addVertex(port); 
                graphDFGwithIO.addVertex(portIn); 
                graphDFGwithIO.addVertex(portOut); 
                graphDFGwithIO.addEdge(Edge(basename + ".in0", basename)); 
                graphDFGwithIO.addEdge(Edge(basename,          basename + ".out0")); 
                for(const auto &vertex: graphDFG.vertices())
                {
                    for(const auto &edge: globalDFG.edgesIn(vertex.first))
                    {
                        if(edge.from() == inport)
                        {
                            graphDFGwithIO.addEdge(Edge(basename + ".out0", vertex.first)); 

                            string fuDFG = getPrefix(edge.from()); 
                            assert(compatFUs.find(fuDFG) != compatFUs.end()); 
                            string fuRRG = *compatFUs[fuDFG].begin(); 
                        }
                    }
                }
                compatFUs[basename] = {"__INPUT_FU__"}; 
            }

            size_t countOutputPorts = 0; 
            for(const auto &outport: outputPorts)
            {
                string basename = string("__inserted_outport_") + num2str(countOutputPorts++); 
                Vertex port   (basename); 
                Vertex portIn (basename + ".in0"); 
                Vertex portOut(basename + ".out0"); 
                graphDFGwithIO.addVertex(port); 
                graphDFGwithIO.addVertex(portIn); 
                graphDFGwithIO.addVertex(portOut); 
                graphDFGwithIO.addEdge(Edge(basename + ".in0", basename)); 
                graphDFGwithIO.addEdge(Edge(basename,          basename + ".out0")); 
                graphDFGwithIO.addEdge(Edge(outport,           basename + ".in0")); 

                assert(outputToVertex.find(outport) != outputToVertex.end()); 
                string fuDFG = getPrefix(outputToVertex[outport]); 
                assert(compatFUs.find(fuDFG) != compatFUs.end()); 
                string fuRRG = *compatFUs[fuDFG].begin(); 
                compatFUs[basename] = {"__OUTPUT_FU__"}; 
            }
            clog << "Placement: DFGwithIO loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 
            string dfgRaw = dfg + ".raw";
            graphDFGwithIO.dump(dfgRaw);

            Graph graphRRG(rrg);
            graphRRG = Utils::insertPortBlock(graphRRG); //insertPortFU for __INPUT_FU__ & __OUTPUT_FU__ 
            clog << "Placement: RRG loaded. " << "Vertices: " << graphRRG.nVertices() << "; " << "Edges: " << graphRRG.nEdges() << endl; 

            //Update Compatible
            unordered_map<string, unordered_set<string>> compatNew = updateCompaTable(graphDFGwithIO, compatFUs, FUs);
            if(compatNew.empty()){
                WARN << "FastPlacement::Placement: WARN: Found Uncompatible vertex.";
                return false;
            }
            //N-order validation
            NetworkAnalyzerLegacy analyzer(FUs, graphRRG); 
            Graph &RRGAnalyzed = analyzer.RRG();

            unordered_map<string, unordered_set<string>> device2vertexDFG;
            unordered_map<string, unordered_set<string>> compatibleVertexRRG;
            for (const auto &vertexDFG: compatNew) {
                for (const auto &deviceDFG: vertexDFG.second) {
                    if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                        device2vertexDFG[deviceDFG] = unordered_set<string>();
                    }
                    device2vertexDFG[deviceDFG].insert(vertexDFG.first);
                }
            }
            for (const auto &vertexRRG: RRGAnalyzed.vertices()) {
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
            deleteForbiddened(compatibleVertexRRG, forbidAdditional);
            NOrderValidator validator(graphDFGwithIO, RRGAnalyzed, compatibleVertexRRG);
            size_t unplacibleCount = 0;
            for (const auto &vertexDFG : graphDFGwithIO.vertices()) {
                if (!getPostfix(vertexDFG.first).empty()) {
                    continue;
                }
                if (compatibleVertexRRG.find(vertexDFG.first) == compatibleVertexRRG.end()) {
                    WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
                    return false;
                }
                
                unordered_set<string> compatibles;
                for(const auto &vertexRRG: compatibleVertexRRG[vertexDFG.first]){
                    clog << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
                    if (validator.validateSlow(vertexDFG.first, vertexRRG, 2)) {
                        compatibles.insert(vertexRRG);
                    }
                }
                clog << vertexDFG.first << ": " << compatibleVertexRRG[vertexDFG.first].size() << " -> " << compatibles.size() << "            ";
                compatibleVertexRRG[vertexDFG.first] = compatibles;
            }
            if(unplacibleCount > 0){
                clog << "VanillaPlacer: FAILED, uncompatible vertex found in first order validation. " << endl;
                return false;
            }
      
            // Contract DFG
            Graph coarseDFG;
            unordered_map<string, unordered_set<string>> coarse2Fine;
            unordered_map<string, string> fine2Contracted;
            unordered_map<string, size_t> packNames;
            for(const auto &vertex: graphDFGwithIO.vertices()){
                if(fine2Contracted.find(vertex.first) != fine2Contracted.end()){
                    const string &coarse = fine2Contracted[vertex.first];
                    if(packNames.find(coarse) != packNames.end()){
                        continue;
                    }
                } 
                string coinname = getPrefix(vertex.first);
                fine2Contracted[vertex.first] = coinname;
                if(coarse2Fine.find(coinname) == coarse2Fine.end()){
                    coarse2Fine[coinname] = unordered_set<string>();
                }
                coarse2Fine[coinname].insert(vertex.first);
            }
            for(const auto &coarse: coarse2Fine){
                Vertex vertex(coarse.first);
                if(packNames.find(coarse.first) != packNames.end()){
                    vertex.setAttr("type", Attribute("Pack"));
                } else {
                    vertex.setAttr("type", Attribute("Coin"));
                }
                coarseDFG.addVertex(vertex);
            }
            for(const auto &vertex: graphDFGwithIO.vertices()){
                const string &from = vertex.first;
                string fromContracted = fine2Contracted[vertex.first];
                for(const auto &edge: graphDFGwithIO.edgesOut(vertex.first)){
                    const string &to = edge.to();
                    string toContracted = fine2Contracted[edge.to()];
                    if(fromContracted != toContracted){
                        Edge edge1(fromContracted, toContracted);
                        edge1.setAttr("from", Attribute(from));
                        edge1.setAttr("to", Attribute(to));
                        coarseDFG.addEdge(edge1);
                    }
                }
            }
            //Update coarseCompatible
            unordered_map<string, unordered_set<string>> coarseCompatible;
            for(const auto &vertex: coarseDFG.vertices()){
                if(packNames.find(vertex.first) == packNames.end()){
                    coarseCompatible[vertex.first] = compatibleVertexRRG[vertex.first];
                }
            }
            for(const auto &pack: packNames){
                unordered_set<string> compatible;
                for(auto iter = coarse2Fine[pack.first].begin(); iter != coarse2Fine[pack.first].end(); iter++){
                    const string &vertex = *iter;
                    unordered_set<string> compatible1;
                    for(const auto &device: compatibleVertexRRG[vertex]){
                        const string &packDevice = getFront(device);
                        compatible1.insert(packDevice);
                    }
                    if(iter == coarse2Fine[pack.first].begin()){
                        compatible = compatible1;
                    } else {
                        for(auto jter = compatible.begin(); jter != compatible.end();){
                            if(compatible1.find(*jter) == compatible1.end()){
                                jter = compatible.erase(jter);
                            } else {
                                jter++;
                            }
                        }
                    }
                }
                coarseCompatible[pack.first] = compatible;
            }

            FastPlacer placer(graphRRG, analyzer);
            FastRouter router(graphRRG, FUs);
            size_t count = 0; 
            while(count++ < 16)
            {
                unordered_map<string, string>                                vertexDFG2RRG; 
                unordered_map<string, unordered_map<string, vector<string>>> paths;

                vector<string> order;
                if(seed.empty()){
                    order = sortDFG(coarseDFG, sortMode);
                } else {
                    auto iter = fine2Contracted.find(seed);
                    if(iter == fine2Contracted.end())
                    {
                        WARN << "FastPlacement::Placement: Seed " << iter->second << " not Found";
                        return false;
                    }
                    order = sortDFG(coarseDFG, sortMode, iter->second);
                }

                pair<unordered_map<string, string>, FastRouter> result = placer.placeHard(coarseDFG, graphDFGwithIO, coarseCompatible, router,
                                                                                        {}, {}, coarse2Fine, order);
                size_t countFailed = 0;
                while(countFailed++ < 16 && result.first.empty())
                {
                    vector<string> failedVertices;
                    const unordered_map<string, size_t> &failures = placer.failedVertices();
                    for(const auto &vertex: failures)
                    {
                        failedVertices.push_back(vertex.first);
                    }
                    sort(failedVertices.begin(), failedVertices.end(), [&failures](const string &a, const string &b){
                        return failures.find(a)->second  > failures.find(b)->second; 
                    });
                    if(failedVertices.empty()){
                        continue;
                    }
                    if(countFailed < 8){
                        order = sortDFG(coarseDFG, sortMode, failedVertices[0]);
                    } else if (countFailed < 12){
                        order = sortDFG(coarseDFG, sortMode);
                    } else {
                        string seedInit = coarseDFG.vertexNames()[rand() % coarseDFG.vertexNames().size()];
                        order = sortDFG(coarseDFG, sortMode, seedInit);
                    }
                    result = placer.place(coarseDFG, graphDFGwithIO, coarseCompatible, router,
                                            {}, {}, coarse2Fine, order);
                }
                if(!result.first.empty()){
                    Utils::writeMap  (result.first,          mapped + ".raw"); 
                    Utils::writePaths(result.second.paths(), routed + ".raw");

                    unordered_map<string, string> vertexDFG2RRG; 
                    for(const auto &vertex: result.first)
                    {
                        if(*compatFUs[getPrefix(vertex.first)].begin() != "__INPUT_FU__" && 
                        *compatFUs[getPrefix(vertex.first)].begin() != "__OUTPUT_FU__")
                        {
                            vertexDFG2RRG.insert(vertex); 
                        }
                    }

                    unordered_map<string, unordered_map<string, vector<string>>> paths; 
                    for(const auto &from: result.second.paths())
                    {
                        if(getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                        getPrefix(graphRRG.vertex(from.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                        {
                            continue; 
                        }
                        if(paths.find(from.first) == paths.end())
                        {
                            paths[from.first] = unordered_map<string, vector<string>>(); 
                        }
                        for(const auto &to: from.second)
                        {
                            if(getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__INPUT_FU__" || 
                            getPrefix(graphRRG.vertex(to.first).getAttr("device").getStr()) == "__OUTPUT_FU__")
                            {
                                continue; 
                            }
                            assert(paths[from.first].find(to.first) == paths[from.first].end()); 
                            paths[from.first][to.first] = to.second; 
                        }
                    }
                    
                    Utils::writeMap(vertexDFG2RRG, mapped); 
                    Utils::writePaths(paths, routed); 
                    return true; 
                }
            }


            return false;
        }

        std::unordered_map<std::string, std::unordered_set<std::string>> updateCompaTable(const Graph &DFG, const std::unordered_map<std::string, std::unordered_set<std::string>> &compat,
                    const std::unordered_map<std::string, std::unordered_set<std::string>> &FUs)
        {
            unordered_map<string, unordered_set<string>> compatNew;
            unordered_multimap<string, string> compatToDelete;
            for (const auto &vertex : DFG.vertices()) {
                compatNew[vertex.first] = unordered_set<string>();
                auto iter = compat.find(vertex.first);
                if(iter != compat.end()){
                    for(const auto &device : iter->second){
                        if (FUs.find(device) != FUs.end()) {
                            compatNew[vertex.first].insert(device);
                        }
                    }
                    if(compatNew[vertex.first].empty()){
                        clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                } else {
                    string port = getPostfix(vertex.first);
                    string fu = getPrefix(vertex.first);
                    auto jter = compat.find(fu);
                    if(jter == compat.end()){   
                        clog << "FastPlacement: failed: unsupported device: " << fu << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                    for (const auto &device : jter->second) {
                        auto kter = FUs.find(device);
                        if(kter == FUs.end()){
                            continue;
                        }
                        if(kter->second.find(port) == kter->second.end()){
                            clog << "FastPlacement: undefined port found: " << fu << "." << port << "." << endl;
                            clog << "FastPlacement: Erase: " << fu << " -> " << device << endl;
                            compatToDelete.insert({fu, device});
                            continue;
                        }
                        compatNew[vertex.first].insert(device + "." + port);
                    }
                    if(compatNew[vertex.first].empty()){
                        clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                        return unordered_map<string, unordered_set<string>>();
                    }
                }
            }
            for(const auto &toDelete: compatToDelete){
                const string &fu = toDelete.first;
                const string &device = toDelete.second;
                if(compatNew[fu].find(device) != compatNew[fu].end()){
                    compatNew[fu].erase(device);
                }
            }
            for(const auto &vertex: compatNew){
                if(vertex.second.empty()){
                    clog << "FastPlacement: failed: unsupported vertex: " << vertex.first << "." << endl;
                    return unordered_map<string, unordered_set<string>>();
                }
            }
            return compatNew;
        }

        void deleteForbiddened(std::unordered_map<std::string, std::unordered_set<std::string>> &compat, const std::unordered_multimap<std::string, std::string> &forbid)
        {
            for (const auto &tmp : forbid) {
                if (compat.find(tmp.first) != compat.end()) {
                    if (compat[tmp.first].find(tmp.second) != compat[tmp.first].end()) {
                        compat[tmp.first].erase(tmp.second);
                    }
                    if (compat[tmp.first].empty()) {
                        compat.erase(tmp.first);
                    }
                }
            }

        }

        std::vector<std::string> sortDFG(const Graph &dfg,const std::string &sortMode)
        {
            if(sortMode == "BFS"){
                return GraphSort::sortBFS(dfg);
            } else if(sortMode == "DFS"){
                return GraphSort::sortDFS(dfg);
            } else if(sortMode == "TVS"){
                return GraphSort::sortTVS(dfg);
            } else if(sortMode == "STB"){
                return GraphSort::sortSTB(dfg);
            }
            WARN << "FastPlacement::sortDFG: SortDFG Mode Not Found";
            return GraphSort::sortSTB(dfg);
        } 

        std::vector<std::string> sortDFG(const Graph &dfg,const std::string &sortMode, const std::string &seed)
        {
            if(sortMode == "BFS"){
                return GraphSort::sortBFS(dfg, seed);
            } else if(sortMode == "DFS"){
                return GraphSort::sortDFS(dfg, seed);
            } else if(sortMode == "TVS"){
                return GraphSort::sortTVS(dfg, seed);
            } else if(sortMode == "STB"){
                return GraphSort::sortSTB(dfg, seed);
            }
            WARN << "FastPlacement::sortDFG: SortDFG Mode Not Found";
            return GraphSort::sortSTB(dfg, seed);
        } 
        
        bool PlaceTop(const std::string &dfg, const std::string &compat, const std::string &rrg, const std::string &fus, 
                                    const std::vector<std::string> &top,
                                    const std::vector<std::string> &coreplaced, const std::vector<std::string> &corerouted, 
                                    const std::string &mapped, const std::string &routed, const std::string &seed)
        {
            assert(coreplaced.size() == corerouted.size());
            Graph graphDFG(dfg);
            clog << "Placement: DFG loaded. " << "Vertices: " << graphDFG.nVertices() << "; " << "Edges: " << graphDFG.nEdges() << endl; 

            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat);
            unordered_map<string, unordered_set<string>> FUs = NetworkAnalyzer::parse(fus); 

            Graph graphRRG(rrg);
            clog << "Placement: DFG loaded. " << "Vertices: " << graphRRG.nVertices() << "; " << "Edges: " << graphRRG.nEdges() << endl; 
            
            unordered_map<string, unordered_set<string>> compatNew = updateCompaTable(graphDFG, compatFUs, NetworkAnalyzer::parse(fus));

            unordered_map<string, unordered_set<string>> device2vertexDFG;
            unordered_map<string, unordered_set<string>> compatibleVertexRRG;

            NetworkAnalyzerLegacy analyzer(FUs, graphRRG); 
            Graph &RRGAnalyzed = analyzer.RRG();

            for (const auto &vertexDFG: compatNew) {
                for (const auto &deviceDFG: vertexDFG.second) {
                    if (device2vertexDFG.find(deviceDFG) == device2vertexDFG.end()) {
                        device2vertexDFG[deviceDFG] = unordered_set<string>();
                    }
                    device2vertexDFG[deviceDFG].insert(vertexDFG.first);
                }
            }
            for (const auto &vertexRRG: RRGAnalyzed.vertices()) {
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
            
            vector<unordered_map<string, string>> coreDFG2RRG;
            vector<unordered_map<string, unordered_map<string, vector<string>>>> corepaths;
            for(size_t idx = 0; idx < coreplaced.size(); idx++)
            {
                coreDFG2RRG.push_back(Utils::readMap(coreplaced[idx]));
                corepaths.push_back(Utils::readPaths(corerouted[idx]));
            }

            // Coarsen DFG
            Graph coarseDFG;
            unordered_map<string, unordered_set<string>> coarse2fine;
            unordered_map<string, string> fine2coarse;
            unordered_map<string, size_t> packNames;
            for(size_t x = 0; x < coreDFG2RRG.size(); x++){
                string packname = "__core" + to_string(x);
                packNames.insert({packname, x});
                coarse2fine[packname] = unordered_set<string>();
                for(const auto &fine: coreDFG2RRG[x]){
                    if(graphDFG.vertices().find(fine.first) == graphDFG.vertices().end()){ 
                        WARN << "FastPlacement::Placement: Vertex " << fine.first << " in Mapped Core " << coreplaced[x] << " not Found.";
                        return false;
                    }
                    coarse2fine[packname].insert(fine.first);
                    fine2coarse[fine.first] = packname;
                }
            }
            for(const auto &vertex: graphDFG.vertices()){
                if(fine2coarse.find(vertex.first) != fine2coarse.end()){
                    continue;
                } 
                string coarseName = getPrefix(vertex.first);
                if(coarse2fine.find(coarseName) == coarse2fine.end()){
                    coarse2fine[coarseName] = unordered_set<string>();
                }
                coarse2fine[coarseName].insert(vertex.first);
                fine2coarse[vertex.first] = coarseName;
            }
            
            for(const auto &coarse: coarse2fine){
                Vertex vertex(coarse.first);
                if(packNames.find(coarse.first) != packNames.end()){
                    vertex.setAttr("type", Attribute("Pack"));
                } else {
                    vertex.setAttr("type", Attribute("Coin"));
                }
                coarseDFG.addVertex(vertex);
            }
            clog << coarseDFG << endl;
            for(const auto &vertex: graphDFG.vertices()){
                const string &from = vertex.first;
                string fromCoarse = fine2coarse[vertex.first];
                for(const auto &edge: graphDFG.edgesOut(vertex.first)){
                    const string &to = edge.to();
                    string toCoarse = fine2coarse[edge.to()];
                    if(fromCoarse != toCoarse){
                        Edge edge1(fromCoarse, toCoarse);
                        edge1.setAttr("from", Attribute(from));
                        edge1.setAttr("to", Attribute(to));
                        coarseDFG.addEdge(edge1);
                    }
                }
            }
            //Update coarseCompatible
            unordered_map<string, unordered_set<string>> coarseCompatible;
            for(const auto &vertex: coarseDFG.vertices()){
                if(packNames.find(vertex.first) == packNames.end()){ //other nodes
                    coarseCompatible[vertex.first] = compatibleVertexRRG[vertex.first];
                }
            }
            // pack compat
            for(const auto &pack: packNames){
                unordered_set<string> compatible;
                for(auto iter = coarse2fine[pack.first].begin(); iter != coarse2fine[pack.first].end(); iter++){
                    const string &vertex = *iter;
                    unordered_set<string> compatible1;
                    for(const auto &device: compatibleVertexRRG[vertex]){
                        const string &packDevice = getFront(device);
                        compatible1.insert(packDevice);
                    }
                    if(iter == coarse2fine[pack.first].begin()){
                        compatible = compatible1;
                    } else {
                        for(auto jter = compatible.begin(); jter != compatible.end();){
                            if(compatible1.find(*jter) == compatible1.end()){
                                jter = compatible.erase(jter);
                            } else {
                                jter++;
                            }
                        }
                    }
                }
                coarseCompatible[pack.first] = compatible;
            }

            size_t count = 0;
            size_t maxTry = 0;
            FastPlacer placer(graphRRG, analyzer);
            FastRouter router(graphRRG, FUs);
            bool topplaced = false;
            while(count < 32 && maxTry < 1024)
            {
                unordered_map<string, string> dfg2rrg;
                unordered_map<string, unordered_map<string, vector<string>>> pathsall;

                vector<size_t> randorder;
                for(size_t idx = 0; idx < top.size(); idx++)
                {
                    randorder.push_back(idx);
                }
                random_shuffle(randorder.begin(), randorder.end());
                for (size_t idx = 0; idx < coreDFG2RRG.size(); idx++) {
                    for (const auto &vertex : coreDFG2RRG[idx]) {
                        dfg2rrg[vertex.first] = top[randorder[idx]] + "." + vertex.second;
                    }
                    for (const auto &from : corepaths[idx]) {
                        string fromName = top[randorder[idx]] + "." + from.first;
                        if (pathsall.find(fromName) == pathsall.end()) {
                            pathsall[fromName] = unordered_map<string, vector<string>>();
                        }
                        for (const auto &to : from.second) {
                            string toName = top[randorder[idx]] + "." + to.first;
                            if (pathsall[fromName].find(toName) == pathsall[fromName].end()) {
                                pathsall[fromName][toName] = vector<string>();
                            }
                            for (const auto &node : to.second) {
                                pathsall[fromName][toName].push_back(top[randorder[idx]] + "." + node);
                            }
                        }
                    }
                }
                
                // Graph contracted;
                // for(const auto &vertex: graphDFG.vertices()){
                //     if(getPostfix(vertex.first).empty()){
                //         contracted.addVertex(Vertex(vertex.first));
                //     }
                // }
                // for(const auto &vertex: graphDFG.vertices()){
                //     for(const auto &edge: graphDFG.edgesOut(vertex.first)){
                //         string from = getPrefix(edge.from());
                //         string to = getPrefix(edge.to());
                //         if(from != to){
                //             contracted.addEdge(Edge(from, to));
                //         }
                //     }
                // }
                unordered_map<string, unordered_set<string>> compatibleVertexRRG1 = compatibleVertexRRG;
                for(auto iter = compatibleVertexRRG1.begin(); iter != compatibleVertexRRG1.end(); iter++){
                    string vertexDFG = iter->first;
                    if(dfg2rrg.find(vertexDFG) != dfg2rrg.end()){
                        iter->second = {dfg2rrg[vertexDFG]};
                    }
                }
                NOrderValidator validator(graphDFG, RRGAnalyzed, compatibleVertexRRG1);
                bool unplacible = false;
                for (const auto &vertexDFG : graphDFG.vertices()) {
                    if (!getPostfix(vertexDFG.first).empty()) {
                        continue;
                    }
                    if (compatibleVertexRRG1.find(vertexDFG.first) == compatibleVertexRRG1.end()) {
                        WARN << "FastPlacement: Compatible vertices NOT FOUND: " + vertexDFG.first;
                        return false;
                    }
                    unordered_set<string> compatibles;
                    for(const auto &vertexRRG: compatibleVertexRRG1[vertexDFG.first]){
                        cout << "\rFastPlacement: -> Validating " << vertexDFG.first << " : " << vertexRRG << "            ";
                        if (validator.validateSlow(vertexDFG.first, vertexRRG, 1)) { //NorderValidate
                            compatibles.insert(vertexRRG);
                        }
                    }
                    clog << vertexDFG.first << ": " << compatibleVertexRRG1[vertexDFG.first].size() << " -> " << compatibles.size() << endl;
                    if(compatibles.empty()){
                        unplacible = true;
                        // clog << compatibleVertexRRG1[vertexDFG.first] << endl;
                        // for(const auto &edge: contracted.edgesIn(vertexDFG.first)){
                        //     clog << "  <-" << edge.from() << ":" << compatibleVertexRRG1[edge.from()] << endl;
                        // }
                        // for(const auto &edge: contracted.edgesOut(vertexDFG.first)){
                        //     clog << "  ->" << edge.to() << ":" << compatibleVertexRRG1[edge.to()] << endl;
                        // }
                        break;
                    }
                    compatibleVertexRRG1[vertexDFG.first] = compatibles;
                }

                if(unplacible){
                    clog << "FastPlacement: -> Validation Fail, retry initialization." << endl;
                    cout << "FastPlacement: -> Validation Fail, retry initialization." << endl;
                    maxTry++;
                    continue;
                }
                count++;

                cout << "FastPlacement: -> Pass Validation. Try to place, " << count << " time." << endl;
                clog << "FastPlacement: -> Pass Validation. Try to place, " << count << " time." << endl;

                unordered_map<string, unordered_set<string>> coarseCompatible1 = coarseCompatible;
                for(const auto &vertex: compatibleVertexRRG1){
                    if(coarseCompatible1.find(vertex.first) != coarseCompatible1.end()){
                        coarseCompatible1[vertex.first] = vertex.second;
                    }
                }

                

                unordered_multimap<string, pair<string, string>> usedNode;
                unordered_map<string, string>                    usedSignal;
                unordered_map<string, string>                    rrg2dfg;
                for(const auto &vertex: dfg2rrg)
                {
                    rrg2dfg[vertex.second] = vertex.first;
                }
                for(const auto &from: pathsall)
                {
                    for(const auto &to: from.second)
                    {
                        for(const auto &node: to.second)
                        {
                            usedNode.insert({node, {from.first, to.first}});
                            assert(usedSignal.find(node) == usedSignal.end() || usedSignal[node] == rrg2dfg[from.first]);
                            usedSignal[node] = rrg2dfg[from.first];
                        }
                    }
                }
                router.usedNode() = usedNode;
                router.usedSignal() = usedSignal;
                router.paths() = pathsall;

                vector<string> order;
                if(seed.empty()){
                    order = sortDFG(coarseDFG, "STB");
                } else {
                    auto iter = fine2coarse.find(seed);
                    if(iter == fine2coarse.end())
                    {
                        WARN << "FastPlacement::Placement: Seed " << iter->second << " not Found";
                        return false;
                    }
                    order = sortDFG(coarseDFG, "STB", iter->second);
                }
                for(const auto &packname: packNames)
                {
                    auto del = find(order.begin(),order.end(),packname.first);
                    if(del != order.end())
                        order.erase(del);
                }
                vector<string> ordInit = order;
                pair<unordered_map<string, string>, FastRouter> result = placer.place_v2(coarseDFG, graphDFG, coarseCompatible1, router, 
                                                                                        dfg2rrg, coarse2fine, order);
                topplaced = !result.first.empty();      
                if(topplaced)
                {
                    clog << "PlaceTOP: Placed successful." << endl;
                    Utils::writeMap(result.first, mapped); 
                    Utils::writePaths(result.second.paths(), routed); 
                    return true; 
                }
                size_t countFailed = 0;                                                        
                while(countFailed++ < 4)
                {
                    vector<string> failedVertices;
                    const unordered_map<string, size_t> &failures = placer.failedVertices();
                    for(const auto &vertex: failures)
                    {
                        failedVertices.push_back(vertex.first);
                    }
                    sort(failedVertices.begin(), failedVertices.end(), [&failures](const string &a, const string &b){
                        return failures.find(a)->second  > failures.find(b)->second; 
                    });
                    if(failedVertices.empty()){
                        continue;
                    }
                    order = sortDFG(coarseDFG, "STB", failedVertices[0]);
                    pair<unordered_map<string, string>, FastRouter> result = placer.place_v2(coarseDFG, graphDFG, coarseCompatible1, router, 
                                                                                        dfg2rrg, coarse2fine, order);
                    topplaced = !result.first.empty();                                                 
                    if(topplaced)
                    {
                        clog << "PlaceTOP: Placed successful." << endl;
                        Utils::writeMap(result.first, mapped); 
                        Utils::writePaths(result.second.paths(), routed); 
                        return true; 
                    }
                }
            }
            clog << "PlaceTOP: END.";
            return false;
        }
    }
}