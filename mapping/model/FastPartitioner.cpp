#include "./FastPartitioner.h"

using namespace std;

namespace FastCGRA
{

void FastPartitioner::prepare(std::unordered_map<std::string, std::string> compat, const std::unordered_map<std::string, double> &weights,
        const std::unordered_map<std::string, size_t> &blockConstraint, const std::unordered_map<std::string, size_t> &fuConstraint, const std::pair<size_t, size_t> &portConstraint)
{
    _compat = compat;
    _weights = weights;
    _blockConstraint = blockConstraint;
    _fuConstraint = fuConstraint;
    _portConstraint = portConstraint;
    _numConstraint = blockConstraint;
    for(const auto &vertex: fuConstraint){
        _numConstraint[vertex.first] = vertex.second;
    }
}

size_t FastPartitioner::distance(std::vector<std::unordered_set<std::string>> P0, std::vector<std::unordered_set<std::string>> P1)
{
    ASSERT(P0.size() == P1.size(), "Should be of the same size.");

    int sum = 0;
    int optiamlResult = 0;
    vector<vector<int>> matrix(P0.size(), vector<int>(P0.size(), 0));
    for(size_t idx = 0; idx < P0.size(); idx++){
        for(size_t idy = 0; idy < P1.size(); idy++){
            for(const auto &vertex: P0[idx]){
                if(P1[idy].find(vertex) != P1[idy].end()){
                    matrix[idx][idy]--;
                }
            }
            sum -= matrix[idx][idy];
        }
    }

    vector<int> optimal = assignment(matrix);

    for(size_t idx = 0; idx < optimal.size(); idx++){
        optiamlResult += matrix[idx][optimal[idx]];
    }
    return sum + optiamlResult;
}

bool FastPartitioner::better(std::unordered_map<std::string, int> score1, std::unordered_map<std::string, int> score2)
{
    for(const auto &item: score1){
        if(item.second > score2.find(item.first)->second){
            return false;
        }
    }
    return true;
}

std::unordered_map<std::string, int> FastPartitioner::score(const std::vector<std::unordered_set<std::string>> &P, const Graph &graphGlobal)
{
    int maxInportNum = 0;
    int maxOutportNum = 0;
    int maxDegree = 0;
    double maxUsage = 0;

    unordered_map<string, int> result;
    for(const auto &part: P){
        unordered_set<string> inputPorts;
        unordered_map<string, unordered_set<string>> inputPorts2Vertex;
        unordered_set<string> outputPorts;
        unordered_map<string, unordered_set<string>> outputPorts2Vertex;
        unordered_map<string, int> type2num;
        for(const auto &vertex: part){
            for(const auto &edge: graphGlobal.edgesIn(vertex)){
                if(part.find(edge.from()) == part.end()){
                    string from = edge.getAttr("from").getStr();
                    string to = edge.getAttr("to").getStr();
                    inputPorts.insert(from);
                    if(inputPorts2Vertex.find(from) == inputPorts2Vertex.end()){
                        inputPorts2Vertex[from] = unordered_set<string>();
                    }
                    inputPorts2Vertex[from].insert(to);
                }
            }
            for(const auto &edge: graphGlobal.edgesOut(vertex)){
                if(part.find(edge.to()) == part.end()){
                    string from = edge.getAttr("from").getStr();
                    string to = edge.getAttr("to").getStr();
                    outputPorts.insert(from);
                    if(outputPorts2Vertex.find(from) == outputPorts2Vertex.end()){
                        outputPorts2Vertex[from] = unordered_set<string>();
                    }
                    outputPorts2Vertex[from].insert(to);
                }
            }
            string type = _compat.find(vertex)->second;
            if(type2num.find(type) == type2num.end()){
                type2num[type] = 0;
            }
            type2num[type]++;
        }
        maxInportNum = (int(inputPorts.size()) > maxInportNum) ? inputPorts.size() : maxInportNum;
        maxOutportNum = (int(outputPorts.size()) > maxOutportNum) ? outputPorts.size() : maxOutportNum;
        for(const auto &input: inputPorts2Vertex){
            maxDegree = (int(input.second.size()) > maxDegree) ? input.second.size() : maxDegree;
        }
        for(const auto &output: outputPorts2Vertex){
            maxDegree = (int(output.second.size()) > maxDegree) ? output.second.size() : maxDegree;
        }
        for(const auto &type: type2num){
            double usage = double(type.second) / _numConstraint.find(type.first)->second; 
            maxUsage = (usage > maxUsage) ? usage : maxUsage;
        }
    }

    int addition = 0;
    if(maxInportNum > int(_portConstraint.first)){
        addition += _portConstraint.first * (maxInportNum - _portConstraint.first);
    }
    if(maxOutportNum > int(_portConstraint.second)){
        addition += _portConstraint.second * (maxOutportNum - _portConstraint.second);
    }
    if(maxUsage > 1){
        addition += (_portConstraint.first + _portConstraint.second) * (maxUsage - 1) * 100;
    }

    result["maxInportNum"] = maxInportNum + addition;
    result["maxOutportNum"] = maxOutportNum + addition;
    result["maxDegree"] = maxDegree + addition;
    result["maxUsage"] = maxUsage * 100 + addition;

    return result; 
}

std::vector<std::vector<std::unordered_set<std::string>>> FastPartitioner::initialPoP(const Graph &graphInit, const size_t &partNum, const size_t &popNum, const size_t &randNum)
{   
    ASSERT(partNum > 1, "FastPartitioner::initalPop: partNum showed be more than 1.")
    vector<vector<unordered_set<string>>> result;
    unordered_set<string> blocks;
    unordered_set<string> fus;
    Graph blockGraph;
    vector<size_t> randIters;
    for(size_t idx = 0; idx < partNum; idx++){
        randIters.push_back(idx);
    }
    for(const auto &vertex: graphInit.vertices()){
        string device = _compat.find(vertex.first)->second;
        if(_blockConstraint.find(device) != _blockConstraint.end()){
            blocks.insert(vertex.first);
        } else {
            fus.insert(vertex.first);
        }
    }
    for(const auto &vertex: blocks){
        blockGraph.addVertex(Vertex(vertex));
    }
    for(const auto &vertex: blockGraph.vertices()){
        for(const auto &edge: graphInit.edgesOut(vertex.first)){
            if(blockGraph.vertices().find(edge.to()) != blockGraph.vertices().end()){
                blockGraph.addEdge(edge);
            }
        }
    }
    for(size_t idx = 0; idx < popNum; idx++){
        vector<unordered_set<string>> part(partNum, unordered_set<string>());
        if(idx < randNum){
            size_t vertexIter = 0;
            for(const auto &vertex: graphInit.vertices()){
                size_t idy = vertexIter++ % randIters.size();
                if(idy == 0){
                    random_shuffle(randIters.begin(), randIters.end());
                }
                part[randIters[idy]].insert(vertex.first);
            }
        } else {
            part = spectralClustering(blockGraph, partNum);
            size_t vertexIter = 0;
            for(const auto &vertex: graphInit.vertices()){
                if(blockGraph.vertices().find(vertex.first) == blockGraph.vertices().end()){
                    size_t idy = vertexIter++ % randIters.size();
                    if(idy == 0){
                        random_shuffle(randIters.begin(), randIters.end());
                    }
                    part[randIters[idy]].insert(vertex.first);
                }
            }
        }
        result.push_back(part);
    }

    return result;
}

std::vector<std::vector<std::unordered_set<std::string>>> FastPartitioner::geneOffspring(std::vector<std::vector<std::unordered_set<std::string>>> parents, size_t OffNum)
{
    ASSERT(parents.size() == 2, "FastPartitioner::geneOffspring: input number error");
    ASSERT((parents[0].size() == parents[1].size() && parents[0].size() > 1), "FastPartitioner::geneOffspring: parants should be of correct size");
    size_t partNum = parents[0].size();

    const vector<unordered_set<string>> &P0 = parents[0];
    const vector<unordered_set<string>> &P1 = parents[1];
    unordered_map<string, size_t> vertex2P0; 
    unordered_map<string, size_t> vertex2P1;
    for(size_t idx = 0; idx < P0.size(); idx++){
        for(const auto &vertex: P0[idx]){
            vertex2P0[vertex] = idx;
        }
        for(const auto &vertex: P1[idx]){
            vertex2P1[vertex] = idx;
        }
    } 

    unordered_set<string> allVertices;
    unordered_set<string> backboneVertices;
    vector<vector<int>> matrix(P0.size(), vector<int>(P0.size(), 0));
    for(size_t idx = 0; idx < P0.size(); idx++){
        for(const auto &vertex: P0[idx]){
            allVertices.insert(vertex);
        }
    }
    for(size_t idx = 0; idx < P0.size(); idx++){
        for(size_t idy = 0; idy < P1.size(); idy++){
            for(const auto &vertex: P0[idx]){
                if(P1[idy].find(vertex) != P1[idy].end()){
                    matrix[idx][idy]--;
                }
            }
        }
    }
    vector<int> optimal = assignment(matrix);
    for(size_t idx = 0; idx < P0.size(); idx++){
        size_t idy = optimal[idx];
        for(const auto &vertex: P0[idx]){
            if(P1[idy].find(vertex) != P1[idy].end()){
                backboneVertices.insert(vertex);
            }
        }
    }
    
    vector<vector<unordered_set<string>>> Offsprings(OffNum);
    if(backboneVertices.size() == allVertices.size()){
        for(size_t idx = 0; idx < OffNum; idx++){
            string vertexSelect;
            size_t randIter = rand() % allVertices.size();
            size_t vertexIter = 0;
            for(const auto &vertex: allVertices){
                if(vertexIter++ == randIter){
                    vertexSelect = vertex;
                    break; 
                }
            }
            size_t fromPart = vertex2P0[vertexSelect];
            size_t toPart = (fromPart + rand() % (partNum - 1) + 1) % partNum;
            Offsprings[idx] = vector<unordered_set<string>>(P0.size());
            for(size_t idy = 0; idy < P0.size(); idy++){
                for(const auto &vertex: P0[idy]){
                    if(vertex == vertexSelect
                    && idy == fromPart){
                        continue;
                    }
                    Offsprings[idx][idy].insert(vertex);
                }
                Offsprings[idx][toPart].insert(vertexSelect);
            }
        }
    } else {
        vector<unordered_set<string>> P10(P1.size());
        unordered_map<string, size_t> vertex2P10;
        for(size_t idx = 0; idx < P1.size(); idx++){
            P10[idx] = P1[optimal[idx]];
            for(const auto &vertex: P10[idx]){
                vertex2P10[vertex] = idx;
            }
        }
        for(size_t idx = 0; idx < OffNum; idx++){
            string vertexSelect;
            vector<string> candidates;
            for(const auto &vertex: allVertices){
                if(backboneVertices.find(vertex) == backboneVertices.end()){
                    candidates.push_back(vertex);
                }
            }
            vertexSelect = candidates[rand() % candidates.size()];
            bool isMutation = (rand() % 100 < 10);
            if(candidates.size() > 2 && rand() % 100 < 20){//move
                size_t fromPart;
                size_t toPart;
                if(idx % 2 == 1){
                    Offsprings[idx] = P0;
                    fromPart = vertex2P0[vertexSelect];
                    toPart = vertex2P10[vertexSelect];
                } else {
                    Offsprings[idx] = P10;
                    fromPart = vertex2P10[vertexSelect];
                    toPart = vertex2P0[vertexSelect];
                }
                Offsprings[idx][fromPart].erase(vertexSelect);
                Offsprings[idx][toPart].insert(vertexSelect);
            } else {//exchange
                size_t fromPart;
                size_t toPart;
                string vertexSelect1;
                string typeSelect;
                unordered_map<string, size_t> vertex2Offspring;
                if(idx % 2 == 1){
                    Offsprings[idx] = P0;
                    vertex2Offspring = vertex2P0;
                } else {
                    Offsprings[idx] = P10;
                    vertex2Offspring = vertex2P10;
                }
                fromPart = vertex2Offspring[vertexSelect];
                typeSelect = _compat[vertexSelect];
                vector<string> candidates1;
                for(const auto &vertex: candidates){
                    if(vertex != vertexSelect
                    &&vertex2Offspring[vertex] != fromPart
                    &&_compat[vertex] == typeSelect){
                        candidates1.push_back(vertex);
                    }
                }
                if(candidates1.empty()){
                    for(const auto &vertex: candidates){
                        if(vertex != vertexSelect
                        &&vertex2Offspring[vertex] != fromPart){
                            candidates1.push_back(vertex);
                        }
                    }
                }
                if(candidates1.empty()){
                    isMutation = true;
                } else {
                    vertexSelect1 = candidates1[rand() % candidates1.size()];
                    toPart = vertex2Offspring[vertexSelect1];
                    Offsprings[idx][fromPart].erase(vertexSelect);
                    Offsprings[idx][toPart].insert(vertexSelect);
                    Offsprings[idx][fromPart].insert(vertexSelect1);
                    Offsprings[idx][toPart].erase(vertexSelect1);
                }
            }
            if(isMutation){//mutation
                size_t randIter = rand() % allVertices.size();
                size_t vertexIter = 0;
                for(const auto &vertex: allVertices){
                    if(vertexIter++ == randIter){
                        vertexSelect = vertex;
                        break; 
                    }
                }
                if(rand() % 100 < 20){
                    size_t fromPart;
                    size_t toPart;
                    if(idx % 2 == 1){
                        Offsprings[idx] = P0;
                        fromPart = vertex2P0[vertexSelect];
                    } else {
                        Offsprings[idx] = P10;
                        fromPart = vertex2P10[vertexSelect];
                    }
                    toPart = (fromPart + rand() % (partNum - 1) + 1) % partNum;
                    Offsprings[idx][fromPart].erase(vertexSelect);
                    Offsprings[idx][toPart].insert(vertexSelect);
                } else {
                    size_t fromPart;
                    size_t toPart;
                    string vertexSelect1;
                    string typeSelect;
                    unordered_map<string, size_t> vertex2Offspring;
                    if(idx % 2 == 1){
                        Offsprings[idx] = P0;
                        vertex2Offspring = vertex2P0;
                    } else {
                        Offsprings[idx] = P10;
                        vertex2Offspring = vertex2P10;
                    }
                    fromPart = vertex2Offspring[vertexSelect];
                    typeSelect = _compat[vertexSelect];
                    vector<string> candidates1;
                    for(const auto &vertex: allVertices){
                        if(vertex != vertexSelect
                        &&vertex2Offspring[vertex] != fromPart
                        &&_compat[vertex] == typeSelect){
                            candidates1.push_back(vertex);
                        }
                    }
                    if(candidates1.empty()){
                        for(const auto &vertex: allVertices){
                            if(vertex != vertexSelect
                            &&vertex2Offspring[vertex] != fromPart){
                                candidates1.push_back(vertex);
                            }
                        }
                    }
                    if(candidates1.empty()){
                        continue;
                    } else {
                        vertexSelect1 = candidates1[rand() % candidates1.size()];
                        toPart = vertex2Offspring[vertexSelect1];
                        Offsprings[idx][fromPart].erase(vertexSelect);
                        Offsprings[idx][toPart].insert(vertexSelect);
                        Offsprings[idx][fromPart].insert(vertexSelect1);
                        Offsprings[idx][toPart].erase(vertexSelect1);
                    }
                }
            }
        }
    }

    return Offsprings;
}

std::vector<std::unordered_set<std::string>> FastPartitioner::spectralClustering(const Graph &graphInit, const size_t &partNum)
{
    if(graphInit.vertices().empty()){
        return std::vector<std::unordered_set<std::string>>(partNum, unordered_set<string>());
    }
    vector<unordered_set<string>> result;

    vector<string> id2vertex;
    unordered_map<string, size_t> vertex2id;
    size_t id = 0;
    for(const auto &vertex: graphInit.vertices()){
        id2vertex.push_back(vertex.first);
        vertex2id[vertex.first] = id++;
    }

    vector<vector<double>> matrix(id2vertex.size(), vector<double>(id2vertex.size(), 1.0));
    for(const auto &vertex: graphInit.vertices()){
        for(const auto &edge: graphInit.edgesOut(vertex.first)){
            size_t idx = vertex2id[edge.from()];
            size_t idy = vertex2id[edge.to()];
            matrix[idx][idy] += 2.0;
            matrix[idy][idx] += 2.0;
        }
    }

    ofstream fout("./util/matrix.tmp");
    for (const auto &row : matrix) {
        for (const auto &element : row) {
            fout << element << " ";
        }
        fout << endl;
    }
    fout.close();

    system((string("python3 ./util/partition.py ./util/matrix.tmp ./util/partition.tmp ") + num2str<size_t>(partNum)).c_str());

    ifstream fin("./util/partition.tmp");
    ASSERT(fin, "FastPartitioner: run partition.py fail.");
    while (!fin.eof()) {
        string line;
        getline(fin, line);
        if (line.empty()) {
            continue;
        }
        result.push_back(unordered_set<string>());
        istringstream sin(line);
        while (!sin.eof()) {
            string tmp;
            sin >> tmp;
            if (tmp.empty()) {
                continue;
            }
            result[result.size() - 1].insert(id2vertex[str2num<size_t>(tmp)]);
        }
    }
    fin.close();

    return result;
}

std::vector<std::unordered_set<std::string>> FastPartitioner::memeticPartition(const Graph &graphInit, const Graph &graphGlobal, const size_t &partNum)
{
    vector<unordered_set<string>> result;
    const size_t popNum = 16;
    const size_t initRandNum = 8;
    const size_t offSpringNum = 4;
    const size_t iterationTimes = 4096;

    vector<vector<unordered_set<string>>> population;
    vector<unordered_map<string, int>> pop2score;

    ASSERT( partNum > 0, "INPUT partNum error.");
    if(partNum == 1){
        result.push_back(unordered_set<string>());
        for(const auto &vertex: graphInit.vertices()){
            result[0].insert(vertex.first);
        }
        clog << this->score(result, graphGlobal) << endl;
        return result;
    }
   //initial
    population = this->initialPoP(graphInit, partNum, popNum, initRandNum);
    for(const auto &Individual: population){
        unordered_map<string, int> score = this->score(Individual, graphGlobal);
        pop2score.push_back(score);
    }

    size_t iterTimes = 0;
    while(iterTimes++ < iterationTimes){
        clog << "memticPartition: Iter " << iterTimes << endl;
        //gene Offspring
        vector<vector<unordered_set<string>>> parents;
        vector<vector<unordered_set<string>>> offSrpings;
        parents.push_back(population[rand() % population.size()]);
        parents.push_back(population[rand() % population.size()]);
        offSrpings = this->geneOffspring(parents, offSpringNum);

        //update population
        for(const auto &offSrping: offSrpings){
            unordered_map<string, int> newScore = this->score(offSrping, graphGlobal);
            vector<size_t> candiIters;
            unordered_map<size_t, size_t> candiIter2distance;
            for(size_t idx = 0; idx < population.size(); idx++){
                if(this->better(newScore, pop2score[idx])){
                    candiIters.push_back(idx);
                    candiIter2distance[idx] = distance(offSrping, population[idx]);
                }
            }
            if(!candiIters.empty()){
                clog << " memticPartition: population update" << endl; 
                random_shuffle(candiIters.begin(), candiIters.end());
                size_t toDeleteIter = candiIters[0];
                for(const auto &iter: candiIters){
                    toDeleteIter = (candiIter2distance[toDeleteIter] < candiIter2distance[iter]) ? toDeleteIter : iter;
                }
                population[toDeleteIter] = offSrping;
                pop2score[toDeleteIter] = newScore;
            }
        }
    }

    double bestScore = 4096;
    size_t bestIter = 0;
    for(size_t idx = 0; idx < pop2score.size(); idx++){
        double score = 0;
        for(const auto &item: pop2score[idx]){
            score += _weights.find(item.first)->second * item.second;
        }
        clog << "memticPartition: population after optimazation: " << idx << " score: " << score << endl;
        clog << pop2score[idx] << endl;
        if(bestScore > score){
            bestScore = score;
            bestIter = idx;
        }
    }
    result = population[bestIter];

    return result;
}

}