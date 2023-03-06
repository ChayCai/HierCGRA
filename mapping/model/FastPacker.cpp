#include "./FastPacker.h"
#include "./util/SimulatedAnnealing.h"

using namespace std; 

namespace FastCGRA
{

FastPacker::FastPacker(const Graph &globalRRG, const std::vector<Graph> &subRRGs, const std::vector<size_t> &numberSubRRGs, const std::unordered_map<std::string, std::unordered_set<std::string>> &FUs, double weightSize): 
    _globalRRG(globalRRG), _subRRGs(subRRGs), _numberSubRRGs(numberSubRRGs), _FUs(FUs), _weightSize(weightSize)
{
    _FUs["__INPUT_FU__"]  = {"in0", "out0"}; 
    _FUs["__OUTPUT_FU__"] = {"in0", "out0"}; 
    NetworkAnalyzerLegacy analyzerGlobal(_globalRRG); 
    _globalContractedRRG = analyzerGlobal.RRG(); 
    // analyzerGlobal.dumpAnalysis("./arch/Top_RRG_Analyzed.txt", "./arch/Top_Links_Analyzed.txt");
    for(const auto &subgraph: _subRRGs)
    {
        NetworkAnalyzerLegacy analyzerSub(subgraph); 
        _subContractedRRGs.push_back(analyzerSub.RRG()); 
        _subRRGIOs.push_back(Utils::insertPortBlock(subgraph));
    }
    for(const auto &subgraph: _subRRGIOs){
        NetworkAnalyzerLegacy analyzerSub(subgraph); 
        _subContractedRRGIOs.push_back(analyzerSub.RRG());
    }
    // Find allowed cut types
    for(const auto &vertex: _globalContractedRRG.vertices())
    {
        for(const auto &edge: _globalContractedRRG.edgesOut(vertex.first))
        {
            if((edge.from().size() > edge.to().size() && edge.from()[edge.to().size()] == '.' && edge.from().substr(0, edge.to().size()) == edge.to()) || 
               (edge.from().size() < edge.to().size() && edge.to()[edge.from().size()] == '.' && edge.to().substr(0, edge.from().size()) == edge.from())) 
               // is FU->FU.port or FU.port->FU ?
            {
                continue; 
            }
            string fromFB   = getFront(edge.from()); 
            string toFB     = getFront(edge.to()); 
            if(fromFB != toFB)
            {
                string typeFrom = _globalContractedRRG.vertex(edge.from()).getAttr("device").getStr(); 
                string typeTo   = _globalContractedRRG.vertex(edge.to()).getAttr("device").getStr(); 
                string type     = typeFrom + "__->__" + typeTo; 
                _linkTypes.insert(type); 
            }
        }
    }
    _packsPlacible = vector<unordered_set<string>>(_subRRGs.size());
    _packsUnplacible = vector<unordered_set<string>>(_subRRGs.size());
}

unordered_map<string, unordered_set<string>> FastPacker::updateCompatTable(const Graph &DFG, const unordered_map<string, unordered_set<string>> &compat)
{
    clog << endl << "BEGIN: FastPacker: Validating the DFG. " << endl; 
    unordered_map<string, unordered_set<string>> compatNew; 
    for(const auto &vertex: DFG.vertices())
    {
        compatNew[vertex.first] = unordered_set<string>(); 
        auto iter = compat.find(vertex.first); 
        if(iter != compat.end())
        {
            for(const auto &device: iter->second)
            {
                // clog << " -> Validating " << device << ". " << endl; 
                if(device == "__TOP__")
                {
                    compatNew[vertex.first].insert(device); 
                    continue; 
                }
                if(_FUs.find(device) == _FUs.end())
                {
                    clog << "FastPacker: failed: unsupported device: " << device << "." << endl; 
                    return unordered_map<string, unordered_set<string>>(); 
                }
                compatNew[vertex.first].insert(device); 
            }
        }
        else
        {
            size_t posDot  = vertex.first.rfind("."); 
            string element = vertex.first.substr(0, posDot); 
            string port    = vertex.first.substr(posDot+1, vertex.first.size()-1 - posDot); 
            auto jter      = compat.find(element); 
            size_t count   = 0; 
            if(jter != compat.end())
            {
                for(const auto &device: jter->second)
                {
                    auto kter = _FUs.find(device); 
                    if(kter == _FUs.end())
                    {
                        clog << "FastPacker: failed: undefined device: " << element << "." << endl; 
                        return unordered_map<string, unordered_set<string>>(); 
                    }
                    if(kter->second.find(port) == kter->second.end())
                    {
                        clog << "FastPacker: undefined port found: " << element << "." << port << "." << endl; 
                        clog << "FastPacker: Erase: " << element << " -> " << device << endl; 
                        compatNew[element].erase(device); 
                        continue; 
                    }
                    count++; 
                    compatNew[vertex.first].insert(device + "." + port); 
                }
                if(count == 0)
                {
                    clog << "FastPacker: failed: undefined port: " << element << "." << port << "." << endl; 
                    return unordered_map<string, unordered_set<string>>(); 
                }
            }
            else
            {
                clog << "FastPacker: failed: unsupported device: " << element << "." << endl; 
                return unordered_map<string, unordered_set<string>>(); 
            }
            
        }
        
    }
    clog << "END: FastPacker: Validating the DFG. " << endl << endl; 
    
    return compatNew; 
}

void FastPacker::prepare(const Graph &DFG, const Graph &globalDFG, const unordered_map<string, unordered_set<string>> &compat)
{
    // Contract the DFG
    _DFG = DFG; 
    _globalDFG = globalDFG;
    _compatDevices = this->updateCompatTable(DFG, compat); 
    _contractedDFG = Graph(); 
    _contractedFUs.clear();
    _coarseDFG = Graph(); 
    _coarseDict.clear(); 
    _fine2coarse.clear(); 
    _usedCoarseDFG.clear();

    for(const auto &vertex: DFG.vertices())
    {
        string fu = getPrefix(vertex.first); 
        if(_contractedFUs.find(fu) == _contractedFUs.end())
        {
            _contractedFUs[fu] = unordered_set<string>(); 
        }
        _contractedFUs[fu].insert(vertex.first); 
    }
    for(const auto &vertex: _contractedFUs)
    {
        _contractedDFG.addVertex(DFG.vertex(vertex.first)); 
    }
    for(const auto &vertex: DFG.vertices())
    {
        for(const auto &edge: DFG.edgesOut(vertex.first))
        {
            string from = getPrefix(edge.from()); 
            string to   = getPrefix(edge.to()); 
            _contractedDFG.addEdge(Edge(from, to, {{"from", Attribute(edge.from())}, {"to", Attribute(edge.to())}})); 
        }
    }

    // Remove the allowed cuts in the contractedDFG
    Graph simplifiedDFG; 
    unordered_set<string> toUse; 
    for(const auto &vertex: _contractedDFG.vertices())
    {
        simplifiedDFG.addVertex(vertex.second); 
        toUse.insert(vertex.first); 
    }
    for(const auto &vertex: _contractedDFG.vertices())
    {
        for(const auto &edge: _contractedDFG.edgesOut(vertex.first))
        {
            if(edge.from() == edge.to())
            {
                continue; 
            }
            unordered_set<string> &setFrom = _compatDevices.find(edge.getAttr("from").getStr())->second; 
            unordered_set<string> &setTo   = _compatDevices.find(edge.getAttr("to").getStr())->second; 
            bool found = false; 
            for(const auto &typeFrom: setFrom)
            {
                for(const auto &typeTo: setTo)
                {
                    string type = typeFrom + "__->__" + typeTo; 
                    if(_linkTypes.find(type) != _linkTypes.end())
                    {
                        found = true; 
                        break; 
                    }
                }
                if(found)
                {
                    break; 
                }
            }
            if(!found)
            {
                simplifiedDFG.addEdge(edge); 
            }
        }
    }
    // Construct the super graph
    size_t countGraphlets = 0; 
    while(!toUse.empty())//put the cannot route node in a coarse
    {
        Graph graphlet; 
        string coarseName = string("Coarse_") + num2str(countGraphlets); 
        clog << "Looking at: " << coarseName << endl; 
        queue<string> que; 
        string info = "("; 
        que.push(*toUse.begin()); 
        info += *toUse.begin() + ", "; 
        _fine2coarse[*toUse.begin()] = coarseName; 
        clog << "-> seed: " << *toUse.begin() << endl; 
        toUse.erase(toUse.begin()); 
        while(!que.empty())
        {
            const string &vertex = que.front(); 
            clog << "-> -> front: " << vertex << endl; 
            graphlet.addVertex(simplifiedDFG.vertex(vertex)); 
            for(const auto &edge: simplifiedDFG.edgesOut(vertex))
            {
                if(toUse.find(edge.to()) != toUse.end())
                {
                    que.push(edge.to()); 
                    toUse.erase(edge.to()); 
                    info += edge.to() + ", "; 
                    _fine2coarse[edge.to()] = coarseName; 
                }
            }
            for(const auto &edge: simplifiedDFG.edgesIn(vertex))
            {
                if(toUse.find(edge.from()) != toUse.end())
                {
                    que.push(edge.from()); 
                    toUse.erase(edge.from()); 
                    info += edge.from() + ", "; 
                    _fine2coarse[edge.from()] = coarseName; 
                }
            }
            que.pop(); 
        }
        info += ")"; 
        for(const auto &vertex: graphlet.vertices())
        {
            for(const auto &edge: simplifiedDFG.edgesOut(vertex.first))
            {
                graphlet.addEdge(edge); 
            }
        }
        _coarseDFG.addVertex(Vertex(coarseName, {{"info", Attribute(info)}})); 
        _coarseDict[coarseName] = graphlet; 
        countGraphlets++; 
    }
    for(const auto &vertex: _coarseDFG.vertices())
    {
        for(const auto &subvertex: _coarseDict[vertex.first].vertices())
        {
            for(const auto &edge: _contractedDFG.edgesOut(subvertex.first))
            {
                if(_fine2coarse[edge.to()] != vertex.first)
                {
                    _coarseDFG.addEdge(Edge(vertex.first, _fine2coarse[edge.to()], {{"from", edge.getAttr("from")}, {"to", edge.getAttr("to")}})); 
                }
            }
        }
    }
    
    // clog << "FastPacker: coarseDFG:" << endl << _coarseDFG << endl; 
    for(const auto &coarse: _coarseDict){
        bool couldPack = true;
        vector<unordered_set<size_t>> vertex2Blocks;
        for(const auto &vertex: coarse.second.vertices()){
            unordered_set<size_t> blocks;
            for(size_t idx = 0;  idx < _subRRGs.size(); idx++){
                const Graph &graph = _subRRGs[idx]; 
                for(const auto &vertex1: graph.vertices())
                {
                    const string &device = vertex1.second.getAttr("device").getStr();
                    if(_compatDevices.find(vertex.first)->second.find(device) != _compatDevices.find(vertex.first)->second.end()){
                        blocks.insert(idx);
                    }
                }
            }
            if(blocks.empty()){
                couldPack = false;
                break;
            }
            vertex2Blocks.push_back(blocks);
        }
        if(couldPack){
            unordered_set<size_t> compatibleBlocks = *vertex2Blocks.begin();
            for(const auto &vertex: vertex2Blocks){
                unordered_set<size_t> toDelete;
                for(const auto &block: compatibleBlocks){
                    if(vertex.find(block) == vertex.end()){
                        toDelete.insert(block);
                    }
                }
                for(const auto &block: toDelete){
                    compatibleBlocks.erase(block);
                }
            }
            _coarse2Blocks.insert({coarse.first, compatibleBlocks});
        }
    }

}

void FastPacker::portConstraint(const std::vector<std::pair<std::size_t, std::size_t>> &portConstraint)
{
    ASSERT(portConstraint.size() == 0 || portConstraint.size() == _subRRGs.size(), "portConstraint format error");
    _portConstraint = portConstraint;
}

std::unordered_map<size_t, std::vector<Graph>> FastPacker::packTabuAnnealing()
{
    static size_t tempratureAnnealing = 50;
    static size_t timesAnnealing =      4096;
    static size_t timesCooling =        64;
    static double coolingScale =        1.1;
    static size_t tabuTennure =         32;
    const Graph &coarseDFG =            _coarseDFG;
    const double &weightSize =          _weightSize;

    unordered_map<size_t, unordered_set<string>> type2pack;
    unordered_map<string, size_t>                pack2type;
    unordered_map<string, unordered_set<string>> pack2coarse;
    unordered_map<string, string>                coarse2pack;
    unordered_map<string, size_t>                pack2portNumRRG; 
    unordered_map<string, unordered_set<string>> coarse2packNames;
    unordered_set<size_t>                        usedBlockType;
    for(const auto &vertex: _coarse2Blocks){
        for(const auto &block: vertex.second){
            usedBlockType.insert(block);
        }
    }
    cout << usedBlockType << endl;
    for(size_t x = 0; x < _subRRGs.size(); x++){
        if(usedBlockType.find(x) == usedBlockType.end()){
            continue;
        }
        size_t portNum = 0;
        for(const auto &vertex: _subContractedRRGIOs[x].vertices()){
            string device = vertex.second.getAttr("device").getStr();
            if(device == "__INPUT_FU__" || device == "__OUTPUT_FU__"){
                portNum++;
            }
        }
        type2pack[x] = unordered_set<string>();
        for(size_t y = 0; y < _numberSubRRGs[x]; y++){
            string packname = "(" + to_string(x) + "-" + to_string(y) + ")";
            type2pack[x].insert(packname);
            pack2type[packname] = x;
            pack2coarse[packname] = unordered_set<string>();
            pack2portNumRRG[packname] = portNum;
        }
    }
    for(const auto &coarse: _coarse2Blocks){
        coarse2packNames[coarse.first] = unordered_set<string>();
        for(const auto &block: coarse.second){
            coarse2packNames[coarse.first].insert(type2pack[block].begin(), type2pack[block].end());
        }
    }

    //Initial
    unordered_map<size_t, vector<Graph>> initialBlocks = Vpack();
    vector<string>                       allowedCoarses;
    for(const auto &blocks: initialBlocks){
        size_t Iter = 0;
        for(const auto &block: blocks.second){
            string packname = "(" + to_string(blocks.first) + "-" + to_string(Iter++) + ")";
            for(const auto &vertex: block.vertices()){
                pack2coarse[packname].insert(vertex.first);
                coarse2pack[vertex.first] = packname;
                allowedCoarses.push_back(vertex.first);
            }
        }
    }
    
    //Prepare SA
    string                                          packMax;
    double                                          usageMax;
    double                                          usageAve;
    unordered_map<string, double>                   pack2portUsage; 
    unordered_map<string, unordered_set<string>>    pack2neighbor;
    unordered_map<string, vector<string>>           neighbor2pack;
    for(const auto &pack: pack2coarse){
        pack2neighbor[pack.first] = unordered_set<string>();
        for(const auto &coarse: pack.second){
            for(const auto &edge: coarseDFG.edgesIn(coarse)){
                if(pack.second.find(edge.from()) == pack.second.end()
                &&coarse2pack.find(edge.from()) != coarse2pack.end()){
                    pack2neighbor[pack.first].insert(edge.from());
                }
            }
            for(const auto &edge: coarseDFG.edgesOut(coarse)){
                if(pack.second.find(edge.to()) == pack.second.end()
                &&coarse2pack.find(edge.to()) != coarse2pack.end()){
                    pack2neighbor[pack.first].insert(edge.to());
                }
            }
        }
    }
    for(const auto &pack: pack2neighbor){
        for(const auto &neighbor: pack.second){
            if(neighbor2pack.find(neighbor) == neighbor2pack.end()){
                neighbor2pack[neighbor] = vector<string>();
            }
            neighbor2pack[neighbor].push_back(pack.first);
        }
    }

    //SA
    double temperature = tempratureAnnealing;
    size_t countIters = 0;
    unordered_set<string>                                  packsToValidate;
    vector<pair<string, pair<string, string>>>             moveCandi;
    unordered_map<string, unordered_map<string, size_t>>   tabuList;
    for(const auto &vertex: allowedCoarses){
        tabuList[vertex] = unordered_map<string, size_t>();
    }

    auto update = [&](){
        unordered_set<string> movedPacks;
        for(const auto &aMove: moveCandi){
            const string &fromPack = aMove.first;
            const string &toPack = aMove.second.first;
            const string &coarse = aMove.second.second;
            pack2coarse[fromPack].erase(coarse);
            pack2coarse[toPack].insert(coarse);
            coarse2pack[coarse] = toPack;
            movedPacks.insert(fromPack);
            movedPacks.insert(toPack);
            tabuList[coarse][fromPack] = countIters + tabuTennure;
            clog << "packTabuAnnealing move:" << fromPack << "->" << toPack << ":" << coarse << endl;
        }
        packsToValidate.insert(movedPacks.begin(), movedPacks.end());
        for(const auto &pack: movedPacks){
            pack2neighbor[pack] = unordered_set<string>();
            for(const auto &coarse: pack2coarse[pack]){
                for(const auto &edge: coarseDFG.edgesIn(coarse)){
                    if(pack2coarse[pack].find(edge.from()) == pack2coarse[pack].end()
                    &&coarse2pack.find(edge.from()) != coarse2pack.end()){
                        pack2neighbor[pack].insert(edge.from());
                    }
                }
                for(const auto &edge: coarseDFG.edgesOut(coarse)){
                    if(pack2coarse[pack].find(edge.to()) == pack2coarse[pack].end()
                    &&coarse2pack.find(edge.to()) != coarse2pack.end()){
                        pack2neighbor[pack].insert(edge.to());
                    }
                }
            }
        }
        neighbor2pack.clear();
        for(const auto &pack: pack2neighbor){
            for(const auto &neighbor: pack.second){
                if(neighbor2pack.find(neighbor) == neighbor2pack.end()){
                    neighbor2pack[neighbor] = vector<string>();
                }
                neighbor2pack[neighbor].push_back(pack.first);
            }
        }
    };

    auto evaluate = [&](){
        usageMax = 0;
        usageAve = 0;
        for(const auto &pack: pack2coarse){
            unordered_set<string> inPorts;
            unordered_set<string> outPorts;
            for(const auto &coarse: pack.second){
                for(const auto &edge: coarseDFG.edgesIn(coarse)){
                    if(pack.second.find(edge.from()) == pack.second.end()){
                        inPorts.insert(edge.getAttr("from").getStr());
                    }
                }
                for(const auto &edge: coarseDFG.edgesOut(coarse)){
                    if(pack.second.find(edge.to()) == pack.second.end()){
                        outPorts.insert(edge.getAttr("from").getStr());
                    }
                }
            }
            pack2portUsage[pack.first] = double(inPorts.size() + outPorts.size()) / pack2portNumRRG[pack.first];
            usageAve += (pack2portUsage[pack.first] / pack2coarse.size());
        }
        for(const auto &pack: pack2portUsage){
            if(pack.second > usageMax){
                packMax = pack.first;
                usageMax = pack.second;
            }
        }
    };

    auto recover = [&](){
        pack2neighbor.clear();
        neighbor2pack.clear();
        for(const auto &pack: pack2coarse){
            pack2neighbor[pack.first] = unordered_set<string>();
            for(const auto &coarse: pack.second){
                for(const auto &edge: coarseDFG.edgesIn(coarse)){
                    if(pack.second.find(edge.from()) == pack.second.end()
                    &&coarse2pack.find(edge.from()) != coarse2pack.end()){
                        pack2neighbor[pack.first].insert(edge.from());
                    }
                }
                for(const auto &edge: coarseDFG.edgesOut(coarse)){
                    if(pack.second.find(edge.to()) == pack.second.end()
                    &&coarse2pack.find(edge.to()) != coarse2pack.end()){
                        pack2neighbor[pack.first].insert(edge.to());
                    }
                }
            }
        }
        for(const auto &pack: pack2neighbor){
            for(const auto &neighbor: pack.second){
                if(neighbor2pack.find(neighbor) == neighbor2pack.end()){
                    neighbor2pack[neighbor] = vector<string>();
                }
                neighbor2pack[neighbor].push_back(pack.first);
            }
        }
        evaluate();
    };

    auto doMoveOnce = [&](const string &fromPack, const string &toPack){
        moveCandi.clear();
        string selectedCoarse;
        vector<string> candidates;
        for(const auto &vertex: pack2neighbor[toPack]){
            if(pack2coarse[fromPack].find(vertex) != pack2coarse[fromPack].end()
            && coarse2packNames[vertex].find(toPack) != coarse2packNames[vertex].end()
            && tabuList[vertex][toPack] <= countIters){
                candidates.push_back(vertex);
            }
        }
        if(!candidates.empty()){
            selectedCoarse = candidates[rand() % candidates.size()];
            moveCandi.push_back({fromPack, {toPack, selectedCoarse}});
            update();
        }
    };

    auto doMove = [&](){
        packsToValidate.clear();
        moveCandi.clear();
        // double randSeed = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        // int moveTimes =   exp(sqrt(temperature / tempratureAnnealing) * randSeed * 2);
        int moveTimes = 1;
        string fromPack;
        string toPack;
        size_t moveType;
        if(usageMax > (1 + weightSize) * usageAve){
            moveType = (rand() % 2 == 0);
        } else {
            moveType = (rand() % 10 != 0);
        }
        if(rand() % 10 == 0){
            moveType = 2;
        }
        if(moveType == 0) {
            fromPack = packMax;
            vector<string> candiTos;
            for(const auto &pack: pack2neighbor){
                if(pack.first != fromPack){
                   for(const auto &vertex: pack.second){
                        if(pack2coarse[fromPack].find(vertex) != pack2coarse[fromPack].end()
                        && coarse2packNames[vertex].find(pack.first) != coarse2packNames[vertex].end()
                        && tabuList[vertex][pack.first] <= countIters){
                            candiTos.push_back(pack.first);
                            break;
                        }
                    }
                }
            }
            if(!candiTos.empty()){
                toPack = candiTos[rand() % candiTos.size()];
                for(int time = 0; time < moveTimes; time++){
                    doMoveOnce(fromPack, toPack);
                }
            } else {
                moveType = 1;
            }
        }
        if(moveType == 1) {
            vector<string> candiFroms;
            for(const auto &pack: pack2coarse){
                if(pack.first != packMax){
                    candiFroms.push_back(pack.first);
                }
            }
            random_shuffle(candiFroms.begin(), candiFroms.end());
            for(const auto &pack1: candiFroms){
                fromPack = pack1;
                for(const auto &pack2: candiFroms){
                    if(pack1 == pack2){
                        continue;
                    }
                    for(const auto &vertex: pack2neighbor[pack2]){
                        if(pack2coarse[pack1].find(vertex) != pack2coarse[pack1].end()
                        && coarse2packNames[vertex].find(pack2) != coarse2packNames[vertex].end()
                        && tabuList[vertex][pack2] <= countIters){
                            toPack = pack2;
                            break;
                        }
                    }
                    if(!toPack.empty()){
                        break;
                    }
                }
                if(!toPack.empty()){
                    break;
                }
            }
            if(!toPack.empty()){
                for(int time = 0; time < moveTimes; time++){
                    doMoveOnce(fromPack, toPack);
                    doMoveOnce(toPack, fromPack);
                }
            }
        } 
        if(moveType == 2) {
            vector<string> candiFroms;
            vector<string> candiTos;
            for(const auto &pack: pack2coarse){
                if(pack.first != packMax){
                    candiFroms.push_back(pack.first);
                }
            }
            fromPack = candiFroms[rand() % candiFroms.size()];
            for(const auto &pack: candiFroms){
                if(pack != fromPack){
                    candiTos.push_back(pack);
                }
            }
            toPack = candiTos[rand() % candiTos.size()];
            int time = 0;
            for(const auto &vertex: pack2coarse[fromPack]){
                if(coarse2packNames[vertex].find(toPack) == coarse2packNames[vertex].end()){
                    continue;
                }
                if(time++ < moveTimes){
                    moveCandi.push_back({fromPack, {toPack, vertex}});
                }
            }
            update();
        }
        clog << "packTabuAnnealing: moveType:" << moveType << ":" << moveTimes << endl;
        evaluate();
        if(fromPack.empty() || toPack.empty()){
            return false;
        }
        return true;
    };

    update();
    evaluate();

    double usageAveBefore = usageAve;
    while(countIters < timesAnnealing)
    {
        countIters++;
        cout << "packTabuAnnealing: Iter " << countIters << ":" << usageAve << endl;
        if(countIters % timesCooling == 0)
        {
            temperature /= coolingScale; 
        }
        double curUsageAve = usageAve;
        unordered_map<string, unordered_set<string>> pack2coarseBak      = pack2coarse;
        unordered_map<string, string> coarse2packBak                     = coarse2pack;
        unordered_map<string, unordered_map<string, size_t>> tabuListBak = tabuList;

        doMove();
        bool accepted = false;
        int delta = (curUsageAve - usageAve) * 5000;

        if(delta > 0)
        {
            accepted = true;
            cout << "packTabuAnnealing: Score: " << usageAve << " vs. " << curUsageAve << endl;
        }
        else
        {
            double probAc = exp(delta / temperature);
            double prob   = static_cast<double>(rand()) / static_cast<double>(RAND_MAX); 
            if(probAc > prob)
            {
                accepted = true; 
                cout << "packTabuAnnealing: prob: " << probAc << ">" << prob << ". Score: " << usageAve << " vs. " << curUsageAve << endl; 
            }
            else
            {
                accepted = false;
                cout << "packTabuAnnealing: Rejected, prob: " << probAc << "<" << prob << ". Score: " << usageAve << " vs. " << curUsageAve << endl; 
            }
        }
        if(accepted){
            for(const auto &pack: packsToValidate){
                if(!validatePack(pack2coarse[pack], pack2type[pack])){
                    cout << "packTabuAnnealing: Rejected, validate fail." << endl; 
                    accepted = false;
                    break;
                }
            }
        }
        if(!accepted)
        {
            pack2coarse = pack2coarseBak;
            coarse2pack = coarse2packBak;
            tabuList = tabuListBak;
            recover();
        } else {
            cout << "packTabuAnnealing: Accepted, validate success." << endl; 
        }

        clog << endl << endl;
    }

    unordered_map<size_t, vector<Graph>> result;
    cout << "packTabuAnnealing: UsageAve after TabuAnnealing: " << usageAve << ", UsageAve before TabuAnnealing:  " << usageAveBefore << endl;
    for(const auto &pack: pack2coarse){
        if(!validatePack(pack.second, pack2type[pack.first])){
            cout << "packTabuAnnealing: Did not pass PlacibleValidation! PackTabuAnnealing Fail!" << endl;
            return result;
        }
    }
    for(const auto &pack: pack2coarse){
        if(pack.second.empty()){
            continue;
        }
        size_t type = pack2type[pack.first];
        Graph subgraph;
        for(const auto &coarse: pack.second){
            unordered_set<string> fines;
            for(const auto &fine: _coarseDict[coarse].vertices()){
                fines.insert(fine.first);
            }
            for(const auto &fine: fines){
                for(const auto &vertex: _contractedFUs[fine]){
                    subgraph.addVertex(Vertex(vertex));
                }
            }
        }
        for(const auto &vertex: subgraph.vertices()){
            for(const auto &edge: _DFG.edgesOut(vertex.first)){
                if(subgraph.vertices().find(edge.to()) != subgraph.vertices().end()){
                    subgraph.addEdge(edge);
                }
            }
        }
        if(result.find(type) == result.end()){
            result[type] = vector<Graph>();
        }
        result[type].push_back(subgraph);
    }

    for(const auto &type: result){
        cout << "packTabuAnnealing: type " << type.first << " with packNum " << type.second.size() << endl; 
    }
    cout << "packTabuAnnealing: Passed PlacibleValidation! PackTabuAnnealing Success!" << endl;

    return result;
}

std::unordered_map<size_t, std::vector<Graph>> FastPacker::Vpack()
{
    unordered_map<size_t, vector<Graph>> result;
    unordered_set<string> allowedCoarse;

    auto updateAllowed = [&](){ 
        allowedCoarse.clear();
        for(const auto &vertex: _coarse2Blocks){
            if(_usedCoarseDFG.find(vertex.first) == _usedCoarseDFG.end()){
                allowedCoarse.insert(vertex.first);
            }
        }
    };

    auto selectToPack = [&]{
        string seed = "";
        vector<string> candidates(allowedCoarse.begin(), allowedCoarse.end());
        ASSERT(!candidates.empty(), "FastPacker: Vpack: selectToPackFail cause allowedCoarse is empty.");

        static size_t toSortLevel = 3;
        unordered_map<string, vector<unordered_set<string>>> vertex2neigbors;
        for(const auto &vertex: allowedCoarse){
            vector<unordered_set<string>> neigborSorted;
            for(size_t level = 0; level < toSortLevel; level ++){
                unordered_set<string> levelSorted;
                if(level == 0){
                    levelSorted = {vertex};
                } else {
                    levelSorted = neigborSorted[level - 1];
                }
                unordered_set<string> candidates;
                for(const auto &vertex1: levelSorted){
                    for(const auto &edge: _coarseDFG.edgesIn(vertex1)){
                        candidates.insert(edge.from());
                    }
                    for(const auto &edge: _coarseDFG.edgesOut(vertex1)){
                        candidates.insert(edge.to());
                    }
                }
                for(auto Iter = candidates.begin();Iter != candidates.end();){
                    bool isSorted = false;
                    for(const auto &levelNeibor: neigborSorted){
                        if(levelNeibor.find(*Iter) != levelNeibor.end()){
                            isSorted = true;
                            break;
                        }
                    }
                    if(isSorted){
                        Iter = candidates.erase(Iter);
                    } else {
                        Iter++;
                    }
                }
                neigborSorted.push_back(candidates);
            }
            vertex2neigbors.insert({vertex, neigborSorted});
            //0:self, 1: its related nodes 2: its related nodes without level_1
        }
        
        unordered_map<string, double> shared; 
        vector<string> seedCandidates;
        for(const auto &vertex: vertex2neigbors){
            shared[vertex.first] = 0;
            for(size_t level = 0; level < vertex.second.size(); level++){
                double score = double(toSortLevel) / (level + 1);
                for(const auto &vertex1: vertex.second[level]){
                    if(_usedCoarseDFG.find(vertex1) != _usedCoarseDFG.end()){
                        shared[vertex.first] += score;
                    }
                }
            }
        }
        // travese all nodes to find nodes which related node has been used
        sort(candidates.begin(), candidates.end(), [&](const string &a, const string &b){return shared[a] < shared[b];});
        //the smaller score the higher order
        for(const auto &vertex: candidates){
            if(shared[vertex] == shared[*candidates.begin()]){
                seedCandidates.push_back(vertex);
            } else {
                break;
            }
        }
        seed = seedCandidates[rand() % seedCandidates.size()];

        vector<size_t> blocks(_coarse2Blocks[seed].begin(), _coarse2Blocks[seed].end());
        unordered_map<size_t, double> connect(blocks.size());
        unordered_map<size_t, double> toPack(blocks.size());
        for(size_t level = 0; level < vertex2neigbors[seed].size(); level++){
            double score = double(toSortLevel) / (level + 1);
            for(const auto &vertex: vertex2neigbors[seed][level]){
                if(_coarse2Blocks.find(vertex) != _coarse2Blocks.end()){
                    for(const auto &block: _coarse2Blocks[vertex]){
                        if(connect.find(block) == connect.end()){
                            connect[block] = 0;
                        }
                        connect[block] += score / _coarse2Blocks[vertex].size();
                    }
                }
            }
        }
        for(const auto &vertex: allowedCoarse){
            for(const auto &block: _coarse2Blocks[vertex]){
                if(toPack.find(block) == toPack.end()){
                    toPack[block] = 1.0;
                }
                toPack[block] += 1.0 / _coarse2Blocks[vertex].size();
            }
        }
        sort(blocks.begin(), blocks.end(), [&](const size_t &a, const size_t &b){
            if(connect[a] == connect[b]){
                return (double(_numberSubRRGs[a]) - double(result[a].size())) / toPack[a] 
                    > (double(_numberSubRRGs[b]) - double(result[b].size())) / toPack[b];
            }
            return connect[a] > connect[b];
        });

        return make_pair(seed, blocks);
    };

    cout << "FastPacker: Vpack: Begin to pack." << endl;
    while(true){
        updateAllowed();
        cout << " FastPacker: Vpack: " << allowedCoarse.size() << " coarse left." << endl;
        clog << " FastPacker: Vpack: " << allowedCoarse.size() << " coarse left." << endl;
        if(allowedCoarse.empty()){
            break;
        }

        pair<string, vector<size_t>> toPack = selectToPack();
        string &seed = toPack.first;
        vector<size_t> &blocks = toPack.second;
        Graph pack;
        size_t packtype;
        for(const auto &type: blocks){
            pack = _packOne(type, seed);
            packtype = type;
            if(!pack.vertices().empty()){
                break;
            }
        }
        clog << "FastPacker: Vpack: pack type " << packtype << " with " << pack.vertices().size() << " coarse" << endl;
        cout << "FastPacker: Vpack: pack type " << packtype << " with " << pack.vertices().size() << " coarse" << endl;
        ASSERT(!pack.vertices().empty(), "FastPacker: Vpack: Failed to pack " + seed)
        for(const auto &vertex: pack.vertices()){
            _usedCoarseDFG.insert(vertex.first); 
        }
        if(result.find(packtype) == result.end()){
            result[packtype] = vector<Graph>();
        }
        result[packtype].push_back(pack);
    }

    return result;
}

Graph FastPacker::_packOne(const size_t &type, const string &seedInit)
{
    assert(type < _subRRGs.size()); 

    unordered_set<string> allowedCoarse;
    for(const auto &coarse: _coarse2Blocks)
    {
        if(_usedCoarseDFG.find(coarse.first) == _usedCoarseDFG.end())
        {
            bool isAllowed = false;
            for(const auto &block: coarse.second)
            {
                if(block == type)
                {
                    isAllowed = true;
                    break;
                }
            }
            if(isAllowed)
            {
                allowedCoarse.insert(coarse.first);
            }
        }
    }

    string seed = seedInit;
    if(seed.empty())
    {
        unordered_map<string, double> shared; 
        vector<string> candidates; 
        for(const auto &vertex: allowedCoarse)
        {
            shared[vertex] = 0.0; 
            candidates.push_back(vertex); 
            unordered_set<string> neigbors;
            for(const auto &edge: _coarseDFG.edgesIn(vertex))
            {
                neigbors.insert(edge.from());
                if(_usedCoarseDFG.find(edge.from()) != _usedCoarseDFG.end())
                {
                    shared[vertex] += 4; 
                }
            }
            for(const auto &edge: _coarseDFG.edgesOut(vertex))
            {
                neigbors.insert(edge.to());
                if(_usedCoarseDFG.find(edge.to()) != _usedCoarseDFG.end())
                {
                    shared[vertex] += 4; 
                }
            }
            for(const auto &vertex1: neigbors)
            {
                for(const auto &edge: _coarseDFG.edgesIn(vertex1))
                {
                    if(_usedCoarseDFG.find(edge.from()) != _usedCoarseDFG.end())
                    {
                        shared[vertex] ++; 
                    }
                }
                for(const auto &edge: _coarseDFG.edgesOut(vertex1))
                {
                    if(_usedCoarseDFG.find(edge.to()) != _usedCoarseDFG.end())
                    {
                        shared[vertex] ++; 
                    }
                }
            }
        }
        sort(candidates.begin(), candidates.end(), [&](const string &a, const string &b){ return shared[a] < shared[b];});
        vector<string> candidates1;
        for(const auto &vertex: candidates){
            if(shared[vertex] == shared[candidates[0]]){
                candidates1.push_back(vertex);
            } else {
                break;
            }
        }
        seed = candidates1[rand() % candidates1.size()];
    }

    Graph pack;
    vector<pair<Graph, size_t>> result;

    auto getExpandQue = [&](){
        unordered_set<string> related; 
        unordered_map<string, double> scores; 
        for(const auto &vertex: pack.vertices())
        {
            for(const auto &edge: _coarseDFG.edgesOut(vertex.first))
            {
                related.insert(edge.to()); 
                if(_usedCoarseDFG.find(edge.to())  == _usedCoarseDFG.end() && 
                   allowedCoarse.find(edge.to())   != allowedCoarse.end() &&
                   pack.vertices().find(edge.to()) == pack.vertices().end()) // May need to filter out uncompatible vertices
                {
                    if(scores.find(edge.to()) == scores.end())
                    {
                        scores[edge.to()] = 0.0; 
                    }
                    scores[edge.to()] += 10.0; 
                }
            }
            for(const auto &edge: _coarseDFG.edgesIn(vertex.first))
            {
                related.insert(edge.from()); 
                if(_usedCoarseDFG.find(edge.from())  == _usedCoarseDFG.end() && 
                   allowedCoarse.find(edge.from())   != allowedCoarse.end() &&
                   pack.vertices().find(edge.from()) == pack.vertices().end())
                {
                    if(scores.find(edge.from()) == scores.end())
                    {
                        scores[edge.from()] = 0.0; 
                    }
                    scores[edge.from()] += 10.0;
                }
            }
        }
        bool considerRelated = true; //PARAM
        if(considerRelated)
        {
            for(const auto &vertex: _coarseDFG.vertices())
            {
                if(_usedCoarseDFG.find(vertex.first)  == _usedCoarseDFG.end()  && 
                   allowedCoarse.find(vertex.first)   != allowedCoarse.end() &&
                   pack.vertices().find(vertex.first) == pack.vertices().end())
                {
                    for(const auto &edge: _coarseDFG.edgesOut(vertex.first))
                    {
                        if(related.find(edge.to()) != related.end())
                        {
                            if(scores.find(vertex.first) == scores.end())
                            {
                                scores[vertex.first] = 0.0; 
                            }
                            scores[vertex.first] += 1.0;
                        }
                    }
                    for(const auto &edge: _coarseDFG.edgesIn(vertex.first))
                    {
                        if(related.find(edge.from()) != related.end())
                        {
                            if(scores.find(vertex.first) == scores.end())
                            {
                                scores[vertex.first] = 0.0; 
                            }
                            scores[vertex.first] += 1.0;
                        }
                    }
                }
            }
        }
        vector<string> candidates;
        queue<string> candiQue;
        for(const auto &score: scores)
        {
            candidates.push_back(score.first); 
        }
        sort(candidates.begin(), candidates.end(), [&](const string &a, const string &b){
            return scores[a] > scores[b]; 
        });
        bool greedy = true; //PARAM
        if(candidates.empty() && greedy){
            for(const auto &vertex: allowedCoarse){
                if(_usedCoarseDFG.find(vertex) == _usedCoarseDFG.end()
                &&pack.vertices().find(vertex) == pack.vertices().end()){
                    candidates.push_back(vertex);
                }
            }
        }
        if(!candidates.empty()){
            random_shuffle(candidates.begin(), candidates.end());
        }
        for(const auto &vertex: candidates){
            candiQue.push(vertex);
        }
        
        return candiQue;
    };

    auto getPortNum = [&](){
        Graph fineFlatten;
        unordered_set<string> inputPorts; 
        unordered_set<string> outputPorts;
        for(const auto &vertex: pack.vertices())
        {
            for(const auto &subvertex: _coarseDict[vertex.first].vertices())
            {
                for(const auto &name: _contractedFUs[subvertex.first])
                {
                    fineFlatten.addVertex(_DFG.vertex(name)); 
                }
            }
        }
        for(const auto &vertex: fineFlatten.vertices())
        {
            for(const auto &edge: _DFG.edgesOut(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.to()) != fineFlatten.vertices().end())
                {
                    fineFlatten.addEdge(edge); 
                }
            }
        } 
        for(const auto &vertex: fineFlatten.vertices())
        {
            for(const auto &edge: _globalDFG.edgesIn(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.from()) == fineFlatten.vertices().end())
                {
                    inputPorts.insert(edge.from()); 
                }
            }
            for(const auto &edge: _globalDFG.edgesOut(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.to()) == fineFlatten.vertices().end())
                {
                    outputPorts.insert(edge.from()); 
                    break; 
                }
            }
        }
        return inputPorts.size() + outputPorts.size();
    };  

    bool failedLast = false;
    stack<Graph> packedStack;
    stack<queue<string>> expandStack;
    pack.addVertex(_coarseDFG.vertex(seed));
    packedStack.push(pack);
    queue<string> candiQue = getExpandQue();
    expandStack.push(candiQue);

    if(!validatePack(pack, type))
    {
        clog << "FastPacker: _packOne: " << seed << " place to type " << type << " failed." << endl;
        return Graph();
    }
    while(result.size() < 3)
    {
        clog << endl 
            << "FastPacker: _packOne: New iteration: " << pack.vertices().size() 
            << "; cur size " << pack.vertices().size() << "; result: " << result.size() << endl << endl;  
        bool failed = false;

        string candi;
        Graph packBackup = pack;
        bool expandSuccess = false;
        while(!candiQue.empty())
        {
            pack = packBackup;
            candi = candiQue.front();
            candiQue.pop();
            pack.addVertex(candi);
            if(validatePack(pack, type)){
                expandSuccess = true;
                break;
            }
        }
        failed = !expandSuccess;
        if(!failed){
            packedStack.push(pack);
            expandStack.push(candiQue);
            candiQue = getExpandQue();
            failedLast = failed;
        }
        else
        {
            pack = packBackup;
            if(packedStack.size() <= 1)
            {
                clog << "FastPacker: _packOne: Break cause nothing to try." << endl;
                break;
            }
            if(!failedLast)
            {
                size_t portSize = getPortNum();
                clog << "FastPacker: _packOne: Get one pack:" << pack.vertices().size() << " with port size " << portSize << endl;
                result.push_back({pack, portSize});
            }
            packedStack.pop();
            pack = packedStack.top();
            packedStack.pop();
            candiQue = expandStack.top();
            failedLast = failed;
        }
    }

    if(result.empty()){
        pack = Graph();
        pack.addVertex(Vertex(seed));
        result.push_back({pack, getPortNum()});
    }

    
    sort(result.begin(), result.end(), [](const pair<Graph, size_t> &a, const pair<Graph, size_t> &b){
        return a.second / static_cast<double>(a.first.vertices().size()) < b.second / static_cast<double>(b.first.vertices().size()); 
        // return static_cast<double>(a.first.vertices().size()) > static_cast<double>(b.first.vertices().size()); 
    }); 

    return result[0].first;
}

bool FastPacker::validatePack(const Graph &pack, const size_t &type)
{
    Graph &subRRG           = _subRRGs[type]; 
    Graph &subContractedRRG = _subContractedRRGs[type];
    Graph &subRRGWithIO     = _subRRGIOs[type]; 
    Graph &subContractedRRGWithIO = _subContractedRRGIOs[type];
    if(pack.vertices().empty()){
        return true;
    }
    
    string packStr = "(";
    set<string> coarses;
    for(const auto &vertex: pack.vertices())
    {
        coarses.insert(vertex.first);
    }
    for(const auto &coarse: coarses)
    {
        packStr += (coarse + ",");
    }
    packStr += ")";
    if(_packsPlacible[type].find(packStr) != _packsPlacible[type].end()){
        clog << "FastPacker: validatePack: Placible cause found in _packsPlacible." << endl;
        return true;
    }
    if(_packsUnplacible[type].find(packStr) != _packsUnplacible[type].end()){
        clog << "FastPacker: validatePack: Unplacible cause found in _packsUnPlacible." << endl;
        return false;
    }
    Graph fine; 
    Graph fineFlatten; 
    Graph fineFlattenIO; 
    unordered_set<string> inputPorts; 
    unordered_set<string> outputPorts; 
    unordered_map<string, unordered_set<string>> compatDevicesWithIO = _compatDevices; 
    size_t countInputPorts = 0; 
    size_t countOutputPorts = 0; 
    auto getFine = [&](){
        // clog << "VanillaPacker: Get fine. " << endl; 
        fine = Graph(); 
        for(const auto &vertex: pack.vertices())
        {
            for(const auto &subvertex: _coarseDict[vertex.first].vertices())
            {
                fine.addVertex(subvertex.second); 
            }
        }
        for(const auto &vertex: fine.vertices())
        {
            for(const auto &edge: _contractedDFG.edgesOut(vertex.first))
            {
                if(fine.vertices().find(edge.to()) != fine.vertices().end())
                {
                    fine.addEdge(edge); 
                }
            }
        }
    }; 
    auto getFineFlatten = [&](){
        // clog << "VanillaPacker: Get fineFlatten. " << endl; 
        fineFlatten = Graph(); 
        for(const auto &vertex: fine.vertices())
        {
            for(const auto &name: _contractedFUs[vertex.first])
            {
                fineFlatten.addVertex(_DFG.vertex(name)); 
            }
        }
        for(const auto &vertex: fineFlatten.vertices())
        {
            for(const auto &edge: _DFG.edgesOut(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.to()) != fineFlatten.vertices().end())
                {
                    fineFlatten.addEdge(edge); 
                }
            }
        } 
    }; 
    auto getFineFlattenIO = [&](){
        // clog << "VanillaPacker: Get fineFlattenIO. " << endl; 
        inputPorts.clear(); 
        outputPorts.clear(); 
        compatDevicesWithIO = _compatDevices;
        fineFlattenIO = fineFlatten;   
        for(const auto &vertex: fineFlatten.vertices())
        {
            for(const auto &edge: _globalDFG.edgesIn(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.from()) == fineFlatten.vertices().end())
                {
                    inputPorts.insert(edge.from()); 
                }
            }
            for(const auto &edge: _globalDFG.edgesOut(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.to()) == fineFlatten.vertices().end())
                {
                    outputPorts.insert(edge.from()); 
                    break; 
                }
            }
        }
        for(const auto &inport: inputPorts)
        {
            string basename = string("__inserted_inport_") + num2str(countInputPorts++); 
            Vertex port   (basename); 
            Vertex portIn (basename + ".in0"); 
            Vertex portOut(basename + ".out0"); 
            fineFlattenIO.addVertex(port); 
            fineFlattenIO.addVertex(portIn); 
            fineFlattenIO.addVertex(portOut); 
            fineFlattenIO.addEdge(Edge(basename + ".in0", basename)); 
            fineFlattenIO.addEdge(Edge(basename,          basename + ".out0")); 
            for(const auto &vertex: fineFlatten.vertices()) 
            {
                for(const auto &edge: _globalDFG.edgesIn(vertex.first))
                {
                    if(edge.from() == inport)
                    {
                        fineFlattenIO.addEdge(Edge(basename + ".out0", vertex.first)); 
                    }
                }
            }
            compatDevicesWithIO[basename]           = {"__INPUT_FU__"}; 
            compatDevicesWithIO[basename + ".in0"]  = {"__INPUT_FU__.in0"}; 
            compatDevicesWithIO[basename + ".out0"] = {"__INPUT_FU__.out0"}; 
        }
        for(const auto &outport: outputPorts)
        {
            string basename = string("__inserted_outport_") + num2str(countOutputPorts++); 
            Vertex port   (basename); 
            Vertex portIn (basename + ".in0"); 
            Vertex portOut(basename + ".out0"); 
            fineFlattenIO.addVertex(port); 
            fineFlattenIO.addVertex(portIn); 
            fineFlattenIO.addVertex(portOut); 
            fineFlattenIO.addEdge(Edge(basename + ".in0", basename)); 
            fineFlattenIO.addEdge(Edge(basename,          basename + ".out0")); 
            fineFlattenIO.addEdge(Edge(outport, basename + ".in0")); 
            compatDevicesWithIO[basename]           = {"__OUTPUT_FU__"}; 
            compatDevicesWithIO[basename + ".in0"]  = {"__OUTPUT_FU__.in0"}; 
            compatDevicesWithIO[basename + ".out0"] = {"__OUTPUT_FU__.out0"}; 
        }
    }; 
    auto aggregate = [&](){
        getFine(); 
        getFineFlatten(); 
        getFineFlattenIO(); 
    }; 

    aggregate();

    if(!_portConstraint.empty()){
        if(countInputPorts > _portConstraint[type].first
        ||countOutputPorts > _portConstraint[type].second){
            return false;
        }
    }

    //Validate
    unordered_map<string, unordered_set<string>> device2vertexDFG; 
    unordered_map<string, unordered_set<string>> compatibleVertexRRG; 
    for(const auto &vertexDFG: fine.vertices())
    {
        for(const auto &deviceDFG: _compatDevices[vertexDFG.first])
        {
            if(device2vertexDFG.find(deviceDFG) == device2vertexDFG.end())
            {
                device2vertexDFG[deviceDFG] = unordered_set<string>(); 
            }
            device2vertexDFG[deviceDFG].insert(vertexDFG.first); 
        }
    }
    for(const auto &vertexRRG: subContractedRRG.vertices())
    {
        string device = vertexRRG.second.getAttr("device").getStr(); 
        if(device2vertexDFG.find(device) != device2vertexDFG.end())
        {
            for(const auto &vertexDFG: device2vertexDFG[device])
            {
                if(compatibleVertexRRG.find(vertexDFG) == compatibleVertexRRG.end())
                {
                    compatibleVertexRRG[vertexDFG] = unordered_set<string>(); 
                }
                compatibleVertexRRG[vertexDFG].insert(vertexRRG.first); 
            }
        }
    }
    VanillaMatcher matcher(compatibleVertexRRG); 
    size_t matched = matcher.match(); 
    if(matched < compatibleVertexRRG.size())
    {
        clog << "FastPacker: validatePack: Unplacible cause did not pass VanillaMatcher." << endl;
        _packsUnplacible[type].insert(packStr);
        return false;
    }
    FastPlacer placer0(subRRG, _FUs, subContractedRRG); 
    pair<unordered_map<string, string>, FastRouter> result0 = placer0.placeGivenCompat(fineFlatten, _compatDevices); 
    if(result0.first.empty())
    {
        clog << "FastPacker: validatePack: Unplacible cause did not pass placeWithoutIO." << endl;
        _packsUnplacible[type].insert(packStr);
        return false;
    }
    FastPlacer placer1(subRRGWithIO, _FUs, subContractedRRGWithIO); 
    pair<unordered_map<string, string>, FastRouter> result1 = placer1.placeGivenCompat(fineFlattenIO, compatDevicesWithIO);
    if(result1.first.empty())
    {
        clog << "FastPacker: validatePack: Unplacible cause did not pass placeWithIO." << endl;
        _packsUnplacible[type].insert(packStr);
        return false;
    }

    _packsPlacible[type].insert(packStr);
    clog << "FastPacker: validatePack: Placible." << endl;
    return true;
}

bool FastPacker::validatePack(const std::unordered_set<std::string> &pack, const size_t &type)
{
    Graph &subRRG           = _subRRGs[type]; 
    Graph &subContractedRRG = _subContractedRRGs[type];
    Graph &subRRGWithIO     = _subRRGIOs[type]; 
    Graph &subContractedRRGWithIO = _subContractedRRGIOs[type];
    if(pack.empty()){
        return true;
    }
    
    string packStr = "(";
    set<string> coarses;
    for(const auto &vertex: pack)
    {
        coarses.insert(vertex);
    }
    for(const auto &coarse: coarses)
    {
        packStr += (coarse + ",");
    }
    packStr += ")";
    if(_packsPlacible[type].find(packStr) != _packsPlacible[type].end()){
        clog << "FastPacker: validatePack: Placible cause found in _packsPlacible." << endl;
        return true;
    }
    if(_packsUnplacible[type].find(packStr) != _packsUnplacible[type].end()){
        clog << "FastPacker: validatePack: Unplacible cause found in _packsUnPlacible." << endl;
        return false;
    }

    Graph fine; 
    Graph fineFlatten; 
    Graph fineFlattenIO; 
    unordered_set<string> inputPorts; 
    unordered_set<string> outputPorts; 
    unordered_map<string, unordered_set<string>> compatDevicesWithIO = _compatDevices; 
    size_t countInputPorts = 0; 
    size_t countOutputPorts = 0; 
    auto getFine = [&](){
        // clog << "VanillaPacker: Get fine. " << endl; 
        fine = Graph(); 
        for(const auto &vertex: pack)
        {
            for(const auto &subvertex: _coarseDict[vertex].vertices())
            {
                fine.addVertex(subvertex.second); 
            }
        }
        for(const auto &vertex: fine.vertices())
        {
            for(const auto &edge: _contractedDFG.edgesOut(vertex.first))
            {
                if(fine.vertices().find(edge.to()) != fine.vertices().end())
                {
                    fine.addEdge(edge); 
                }
            }
        }
    }; 
    auto getFineFlatten = [&](){
        // clog << "VanillaPacker: Get fineFlatten. " << endl; 
        fineFlatten = Graph(); 
        for(const auto &vertex: fine.vertices())
        {
            for(const auto &name: _contractedFUs[vertex.first])
            {
                fineFlatten.addVertex(_DFG.vertex(name)); 
            }
        }
        for(const auto &vertex: fineFlatten.vertices())
        {
            for(const auto &edge: _DFG.edgesOut(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.to()) != fineFlatten.vertices().end())
                {
                    fineFlatten.addEdge(edge); 
                }
            }
        } 
    }; 
    auto getFineFlattenIO = [&](){
        // clog << "VanillaPacker: Get fineFlattenIO. " << endl; 
        inputPorts.clear(); 
        outputPorts.clear(); 
        compatDevicesWithIO = _compatDevices;
        fineFlattenIO = fineFlatten;   
        for(const auto &vertex: fineFlatten.vertices())
        {
            for(const auto &edge: _globalDFG.edgesIn(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.from()) == fineFlatten.vertices().end())
                {
                    inputPorts.insert(edge.from()); 
                }
            }
            for(const auto &edge: _globalDFG.edgesOut(vertex.first))
            {
                if(fineFlatten.vertices().find(edge.to()) == fineFlatten.vertices().end())
                {
                    outputPorts.insert(edge.from()); 
                    break; 
                }
            }
        }
        for(const auto &inport: inputPorts)
        {
            string basename = string("__inserted_inport_") + num2str(countInputPorts++); 
            Vertex port   (basename); 
            Vertex portIn (basename + ".in0"); 
            Vertex portOut(basename + ".out0"); 
            fineFlattenIO.addVertex(port); 
            fineFlattenIO.addVertex(portIn); 
            fineFlattenIO.addVertex(portOut); 
            fineFlattenIO.addEdge(Edge(basename + ".in0", basename)); 
            fineFlattenIO.addEdge(Edge(basename,          basename + ".out0")); 
            for(const auto &vertex: fineFlatten.vertices()) 
            {
                for(const auto &edge: _globalDFG.edgesIn(vertex.first))
                {
                    if(edge.from() == inport)
                    {
                        fineFlattenIO.addEdge(Edge(basename + ".out0", vertex.first)); 
                    }
                }
            }
            compatDevicesWithIO[basename]           = {"__INPUT_FU__"}; 
            compatDevicesWithIO[basename + ".in0"]  = {"__INPUT_FU__.in0"}; 
            compatDevicesWithIO[basename + ".out0"] = {"__INPUT_FU__.out0"}; 
        }
        for(const auto &outport: outputPorts)
        {
            string basename = string("__inserted_outport_") + num2str(countOutputPorts++); 
            Vertex port   (basename); 
            Vertex portIn (basename + ".in0"); 
            Vertex portOut(basename + ".out0"); 
            fineFlattenIO.addVertex(port); 
            fineFlattenIO.addVertex(portIn); 
            fineFlattenIO.addVertex(portOut); 
            fineFlattenIO.addEdge(Edge(basename + ".in0", basename)); 
            fineFlattenIO.addEdge(Edge(basename,          basename + ".out0")); 
            fineFlattenIO.addEdge(Edge(outport, basename + ".in0")); 
            compatDevicesWithIO[basename]           = {"__OUTPUT_FU__"}; 
            compatDevicesWithIO[basename + ".in0"]  = {"__OUTPUT_FU__.in0"}; 
            compatDevicesWithIO[basename + ".out0"] = {"__OUTPUT_FU__.out0"}; 
        }
    }; 
    auto aggregate = [&](){
        getFine(); 
        getFineFlatten(); 
        getFineFlattenIO(); 
    }; 

    aggregate();
    if(!_portConstraint.empty()){
        if(countInputPorts > _portConstraint[type].first
        ||countOutputPorts > _portConstraint[type].second){
            return false;
        }
    }

    //Validate
    unordered_map<string, unordered_set<string>> device2vertexDFG; 
    unordered_map<string, unordered_set<string>> compatibleVertexRRG; 
    for(const auto &vertexDFG: fine.vertices())
    {
        for(const auto &deviceDFG: _compatDevices[vertexDFG.first])
        {
            if(device2vertexDFG.find(deviceDFG) == device2vertexDFG.end())
            {
                device2vertexDFG[deviceDFG] = unordered_set<string>(); 
            }
            device2vertexDFG[deviceDFG].insert(vertexDFG.first); 
        }
    }
    for(const auto &vertexRRG: subContractedRRG.vertices())
    {
        string device = vertexRRG.second.getAttr("device").getStr(); 
        if(device2vertexDFG.find(device) != device2vertexDFG.end())
        {
            for(const auto &vertexDFG: device2vertexDFG[device])
            {
                if(compatibleVertexRRG.find(vertexDFG) == compatibleVertexRRG.end())
                {
                    compatibleVertexRRG[vertexDFG] = unordered_set<string>(); 
                }
                compatibleVertexRRG[vertexDFG].insert(vertexRRG.first); 
            }
        }
    }
    VanillaMatcher matcher(compatibleVertexRRG); 
    size_t matched = matcher.match(); 
    if(matched < compatibleVertexRRG.size())
    {
        clog << "FastPacker: validatePack: Unplacible cause did not pass VanillaMatcher." << endl;
        _packsUnplacible[type].insert(packStr);
        return false;
    }
    FastPlacer placer0(subRRG, _FUs, subContractedRRG); 
    pair<unordered_map<string, string>, FastRouter> result0 = placer0.placeGivenCompat(fineFlatten, _compatDevices); 
    if(result0.first.empty())
    {
        clog << "FastPacker: validatePack: Unplacible cause did not pass placeWithoutIO." << endl;
        _packsUnplacible[type].insert(packStr);
        return false;
    }
    FastPlacer placer1(subRRGWithIO, _FUs, subContractedRRGWithIO); 
    pair<unordered_map<string, string>, FastRouter> result1 = placer1.placeGivenCompat(fineFlattenIO, compatDevicesWithIO);
    if(result1.first.empty())
    {
        clog << "FastPacker: validatePack: Unplacible cause did not pass placeWithIO." << endl;
        _packsUnplacible[type].insert(packStr);
        return false;
    }

    _packsPlacible[type].insert(packStr);
    clog << "FastPacker: validatePack: Placible." << endl;
    return true;
}


}
