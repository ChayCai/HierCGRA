#include "./Utils.h"

using namespace std;

namespace FastCGRA
{
    namespace Utils
    {
        unordered_map<string, string> readMap(const string &filename)
        {
            unordered_map<string, string> result; 

            ifstream fin(filename); 
            if(!fin)
            {
                clog << "UtilsBrute: ERROR: Cannot open file: " << filename << endl; 
            }
            assert(fin); 
            while(!fin.eof())
            {
                string line; 
                getline(fin, line); 
                if(line.empty())
                {
                    continue; 
                }
                istringstream sin(line); 
                string from, to; 
                sin >> from >> to; 
                if(from.empty() || to.empty())
                {
                    continue; 
                }
                result[from] = to; 
            }

            return result; 
        }

        unordered_map<string, unordered_map<string, vector<string>>> readPaths(const string &filename)
        {
            unordered_map<string, unordered_map<string, vector<string>>> result; 

            ifstream fin(filename); 
            assert(fin); 
            while(!fin.eof())
            {
                string line; 
                getline(fin, line); 
                if(line.empty())
                {
                    continue; 
                }
                istringstream sin(line); 
                string from, to; 
                sin >> from >> to; 
                if(from.empty() || to.empty())
                {
                    continue; 
                }
                if(result.find(from) == result.end())
                {
                    result[from] = unordered_map<string, vector<string>>(); 
                }
                if(result[from].find(to) == result[from].end())
                {
                    result[from][to] = vector<string>(); 
                }
                while(!sin.eof())
                {
                    string tmp; 
                    sin >> tmp; 
                    if(tmp.empty())
                    {
                        continue; 
                    }
                    result[from][to].push_back(tmp); 
                }
            }

            return result; 
        }
     
        bool writeMap(const unordered_map<string, string> &dict, const string &filename)
        {

            ofstream fout(filename); 
            clog << "WRITE DICT: " << dict.size()<< endl; 

            for(const auto &elem: dict)
            {
                fout << elem.first << " " << elem.second << endl; 
            }

            return 0; 
        }

        bool writePaths(const unordered_map<string, unordered_map<string, vector<string>>> &paths, const string &filename)
        {
            ofstream fout(filename); 
            clog << "WRITE VINILLA ROUTER: " << paths.size()<< endl; 

            for(const auto &from: paths)
            {
                for(const auto &to: from.second)
                {
                    fout << from.first << " " << to.first; 
                    for(const auto &node: to.second)
                    {
                        fout << " " << node; 
                    }
                    fout << endl; 
                }
            }
            fout.close(); 

            return 0; 
        }

        
        std::unordered_map<string, unordered_map<string, vector<string>>> readConifgFile(const std::string &fileName)
        {
            unordered_map<string, unordered_map<string, vector<string>>> result;

            ifstream fin(fileName);
            assert(fin);
            bool undealedLine = false;
            string line;
            while (!fin.eof()) {
                if (!undealedLine) {
                    line.clear();
                    getline(fin, line);
                } else {
                    undealedLine = false;
                }
                if (line.empty()) {
                    continue;
                }
                if (line.find(";") != line.npos) {
                    continue;
                }
                string section;
                if (line.find("[") != line.npos) {
                    section = line.substr(line.find("[") + 1, line.find("]") - line.find("[") - 1);
                    unordered_map<string, vector<string>> tmpKV;
                    while (!fin.eof() && !section.empty()) {
                        line.clear();
                        getline(fin, line);
                        line = removeLineBreak(line);
                        if (line.empty()) {
                            continue;
                        }
                        if (line.find(";") != line.npos) {
                            continue;
                        }
                        if (line.find("[") != line.npos) {
                            undealedLine = true;
                            break;
                        }
                        vector<string> tmpVec = split(line, "=");
                        assert(!tmpVec.empty());
                        string keytmp = removeSpace(tmpVec[0]);
                        vector<string> strtmp = split(tmpVec[1], " ");
                        vector<string> valuetmp;
                        for(const auto &value: strtmp){
                            if(!value.empty()){
                                valuetmp.push_back(removeLineBreak(value));
                            }
                        }
                        tmpKV[keytmp] = valuetmp;
                    }
                    result[section] = tmpKV;
                }
            }
            return result;
        }

        bool validate(const std::string &dfg, const std::string &rrg, const std::string &compat, 
                      const std::string &mapped, const std::string &routed)
        {
            Graph tmprrg(rrg); 
            cout << "RRG loaded. " << "Vertices: " << tmprrg.nVertices() << "; " << "Edges: " << tmprrg.nEdges() << endl; 

            Graph tmpdfg(dfg); 
            cout << "DFG loaded. " << "Vertices: " << tmpdfg.nVertices() << "; " << "Edges: " << tmpdfg.nEdges() << endl; 

            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat); 

            NetworkAnalyzerLegacy analyzer(tmprrg); 

            unordered_map<string, string> vertexDFG2RRG = Utils::readMap(mapped); 
            unordered_map<string, unordered_map<string, vector<string>>> paths = Utils::readPaths(routed); 

            VanillaValidator validator(tmprrg, tmpdfg, compatFUs); 
            return validator.validate(vertexDFG2RRG, paths); 
        }

        bool validateWithoutIO(const std::string &dfg, const std::string &rrg, const std::string &compat, 
                      const std::string &mapped, const std::string &routed)
        {
            Graph tmprrg(rrg); 
            clog << "RRG loaded. " << "Vertices: " << tmprrg.nVertices() << "; " << "Edges: " << tmprrg.nEdges() << endl; 

            unordered_map<string, unordered_set<string>> compatFUs = readSets(compat); 

            Graph initialDFG(dfg); 
            Graph tmpdfg; 
            for(const auto &vertex: initialDFG.vertices())
            {
                if(*compatFUs[getPrefix(vertex.first)].begin() == "DI" || 
                   *compatFUs[getPrefix(vertex.first)].begin() == "DO" || 
                   *compatFUs[getPrefix(vertex.first)].begin() == "BUF" || 
                   *compatFUs[getPrefix(vertex.first)].begin() == "BUFE")
                {
                    continue; 
                }
                tmpdfg.addVertex(vertex.second); 
            }
            for(const auto &vertex: tmpdfg.vertices())
            {
                for(const auto &edge: initialDFG.edgesOut(vertex.first))
                {
                    if(tmpdfg.vertices().find(edge.to()) != tmpdfg.vertices().end())
                    {
                        tmpdfg.addEdge(edge); 
                    }
                }
            }
            clog << "DFG loaded. " << "Vertices: " << tmpdfg.nVertices() << "; " << "Edges: " << tmpdfg.nEdges() << endl; 

            NetworkAnalyzerLegacy analyzer(tmprrg); 

            unordered_map<string, string> vertexDFG2RRG = Utils::readMap(mapped); 
            unordered_map<string, unordered_map<string, vector<string>>> paths = Utils::readPaths(routed); 

            VanillaValidator validator(tmprrg, tmpdfg, compatFUs); 
            return validator.validate(vertexDFG2RRG, paths); 
        }
        
        bool dumpBlock(const std::string &path, const std::vector<std::string> &blockfiles,  const std::vector<std::string> &blocktypes)
        {
            ASSERT(blockfiles.size() == blocktypes.size(), "Utils: dumpBlock: initial Fail");
            ofstream fout(path);
            if(!fout)
            {
                return false;
            }
            for(size_t x = 0; x < blockfiles.size(); x++)
            {
                clog << blockfiles[x] << " " << blocktypes[x] << endl;
                fout << blockfiles[x] << " " << blocktypes[x] << endl;
            }
            fout.close();
            return true;
        }

        std::unordered_map<std::string,std::string> parseBlock(const std::string &path)
        {       
            unordered_map<string,string> blockfile;
            ifstream fin(path); 
            if(!fin)
            {
                return blockfile;
            }
            string line, tmp1,tmp2; 
            while(!fin.eof())
            {  
                getline(fin, line); 
                if(line.empty())
                {
                    continue; 
                }
                istringstream sin(line); 
                tmp1.clear(); 
                tmp2.clear();
                sin >> tmp1 >> tmp2; 
                if(tmp1.empty())
                {
                    continue; 
                }
                if(tmp2.empty())
                {
                    continue; 
                }
                blockfile[tmp1] = tmp2;
            }
            fin.close(); 
            return blockfile;
        }
    
        Graph insertPortBlock(const Graph &origin)
        {
            Graph inserted; 

            for(const auto &vertex: origin.vertices())
            {
                string fu = getPrefix(vertex.second.getAttr("device").getStr());
                string type = vertex.second.getAttr("type").getStr();
                if(fu == "TOP" && type == "__MODULE_INPUT_PORT__")
                {
                    Vertex vertexIn (vertex.first + ".in0",  {{"type",   Attribute("__TOP_INPUT_PORT__")}, 
                                                            {"device", Attribute("__INPUT_FU__.in0")}}); 
                    Vertex vertexFU (vertex.first,           {{"type",   Attribute("__INPUT_FU__")}, 
                                                            {"device", Attribute("__INPUT_FU__")}}); 
                    Vertex vertexOut(vertex.first + ".out0", {{"type",   Attribute("__TOP_INPUT_PORT__")}, 
                                                            {"device", Attribute("__INPUT_FU__.out0")}}); 
                    inserted.addVertex(vertexIn); 
                    inserted.addVertex(vertexFU); 
                    inserted.addVertex(vertexOut); 
                }
                else if(fu == "TOP" && type == "__MODULE_OUTPUT_PORT__")
                {
                    Vertex vertexIn (vertex.first + ".in0",  {{"type",   Attribute("__TOP_OUTPUT_PORT__")}, 
                                                            {"device", Attribute("__OUTPUT_FU__.in0")}}); 
                    Vertex vertexFU (vertex.first,           {{"type",   Attribute("__OUTPUT_FU__")}, 
                                                            {"device", Attribute("__OUTPUT_FU__")}}); 
                    Vertex vertexOut(vertex.first + ".out0", {{"type",   Attribute("__TOP_OUTPUT_PORT__")}, 
                                                            {"device", Attribute("__OUTPUT_FU__.out0")}}); 
                    inserted.addVertex(vertexIn); 
                    inserted.addVertex(vertexFU); 
                    inserted.addVertex(vertexOut); 
                }
                else
                {
                    inserted.addVertex(vertex.second); 
                }
            }

            for(const auto &vertex: origin.vertices())
            {
                string fu = getPrefix(vertex.second.getAttr("device").getStr());
                string type = vertex.second.getAttr("type").getStr();
                if(fu == "TOP" && type == "__MODULE_INPUT_PORT__")
                {
                    inserted.addEdge(Edge(vertex.first + ".in0", vertex.first)); 
                    inserted.addEdge(Edge(vertex.first,          vertex.first + ".out0")); 
                    for(const auto &edge: origin.edgesOut(vertex.first))
                    {
                        inserted.addEdge(Edge(vertex.first + ".out0", edge.to())); 
                    }
                }
                else if(fu == "TOP" && type == "__MODULE_OUTPUT_PORT__")
                {
                    inserted.addEdge(Edge(vertex.first + ".in0", vertex.first)); 
                    inserted.addEdge(Edge(vertex.first,          vertex.first + ".out0")); 
                    for(const auto &edge: origin.edgesIn(vertex.first))
                    {
                        inserted.addEdge(Edge(edge.from(), vertex.first + ".in0")); 
                    }
                }
                else
                {
                    for(const auto &edge: origin.edgesOut(vertex.first))
                    {
                        string fuTo = getPrefix(origin.vertex(edge.to()).getAttr("device").getStr());
                        string typeTo = origin.vertex(edge.to()).getAttr("type").getStr();
                        if((fuTo == "TOP" && typeTo == "__MODULE_INPUT_PORT__") || 
                        (fuTo == "TOP" && typeTo == "__MODULE_OUTPUT_PORT__" ))
                        {
                            continue;
                        }
                        inserted.addEdge(edge); 
                    }
                }
            }

            return inserted; 
        }
        

        Graph insertPortFU(const Graph &origin)
        {
            Graph inserted; 

            for(const auto &vertex: origin.vertices())
            {
                string fu = getPrefix(vertex.second.getAttr("device").getStr());
                string type = vertex.second.getAttr("type").getStr();
                if(fu == "TOP" && type == "__MODULE_INPUT_PORT__")
                {
                    Vertex vertexIn (vertex.first + ".in0",  {{"type",   Attribute("__TOP_INPUT_PORT__")}, 
                                                            {"device", Attribute("__INPUT_FU__.in0")}}); 
                    Vertex vertexFU (vertex.first,           {{"type",   Attribute("__INPUT_FU__")}, 
                                                            {"device", Attribute("__INPUT_FU__")}}); 
                    Vertex vertexOut(vertex.first + ".out0", {{"type",   Attribute("__TOP_INPUT_PORT__")}, 
                                                            {"device", Attribute("__INPUT_FU__.out0")}}); 
                    inserted.addVertex(vertexIn); 
                    inserted.addVertex(vertexFU); 
                    inserted.addVertex(vertexOut); 
                }
                else if(fu == "TOP" && type == "__MODULE_OUTPUT_PORT__")
                {
                    Vertex vertexIn (vertex.first + ".in0",  {{"type",   Attribute("__TOP_OUTPUT_PORT__")}, 
                                                            {"device", Attribute("__OUTPUT_FU__.in0")}}); 
                    Vertex vertexFU (vertex.first,           {{"type",   Attribute("__OUTPUT_FU__")}, 
                                                            {"device", Attribute("__OUTPUT_FU__")}}); 
                    Vertex vertexOut(vertex.first + ".out0", {{"type",   Attribute("__TOP_OUTPUT_PORT__")}, 
                                                            {"device", Attribute("__OUTPUT_FU__.out0")}}); 
                    inserted.addVertex(vertexIn); 
                    inserted.addVertex(vertexFU); 
                    inserted.addVertex(vertexOut); 
                }
                else
                {
                    inserted.addVertex(vertex.second); 
                }
            }

            for(const auto &vertex: origin.vertices())
            {
                string fu = getPrefix(vertex.second.getAttr("device").getStr());
                string type = vertex.second.getAttr("type").getStr();
                if(fu == "TOP" && type == "__MODULE_INPUT_PORT__")
                {
                    inserted.addEdge(Edge(vertex.first + ".in0", vertex.first)); 
                    inserted.addEdge(Edge(vertex.first,          vertex.first + ".out0")); 
                    for(const auto &edge: origin.edgesOut(vertex.first))
                    {
                        inserted.addEdge(Edge(vertex.first + ".out0", edge.to())); 
                    }
                }
                else if(fu == "TOP" && type == "__MODULE_OUTPUT_PORT__")
                {
                    inserted.addEdge(Edge(vertex.first + ".in0", vertex.first)); 
                    inserted.addEdge(Edge(vertex.first,          vertex.first + ".out0")); 
                    for(const auto &edge: origin.edgesIn(vertex.first))
                    {
                        inserted.addEdge(Edge(edge.from(), vertex.first + ".in0")); 
                    }
                }
                else
                {
                    for(const auto &edge: origin.edgesOut(vertex.first))
                    {
                        string fuTo = getPrefix(origin.vertex(edge.to()).getAttr("device").getStr());
                        string typeTo = origin.vertex(edge.to()).getAttr("type").getStr();
                        if((fuTo == "TOP" && typeTo == "__MODULE_INPUT_PORT__") || 
                        (fuTo == "TOP" && typeTo == "__MODULE_OUTPUT_PORT__" ))
                        {
                            continue;
                        }
                        inserted.addEdge(edge); 
                    }
                }
            }

            return inserted; 
        }

        Graph genMRRG(Graph rrg, std::unordered_map<std::string, std::unordered_set<std::string>> FUs, size_t ii)
        {
            Graph MRRG;
            unordered_set<string> functionNodes;
            for(const auto &vertex: rrg.vertices()){
                string device = vertex.second.getAttr("device").getStr();
                if(FUs.find(device) != FUs.end()){
                    functionNodes.insert(vertex.first);
                }
            }
            for(const auto &vertex: rrg.vertices()){
                string device = getPrefix(vertex.first);
                if(functionNodes.find(device) != functionNodes.end()){
                    functionNodes.insert(vertex.first);
                }
            }
            for(size_t idx = 0; idx < ii; idx++){
                string prefix = "T" + num2str(idx) + ".";
                for(const auto &vertex: rrg.vertices()){
                    if(functionNodes.find(vertex.first)==functionNodes.end()){
                        continue;
                    }
                    Vertex vertex1(prefix + vertex.first, vertex.second.attrs());
                    MRRG.addVertex(vertex1);
                }
                for(const auto &vertex: rrg.vertices()){
                    if(functionNodes.find(vertex.first)==functionNodes.end()){
                        continue;
                    }
                    for(const auto &edge: rrg.edgesOut(vertex.first)){
                        if(functionNodes.find(edge.to())!=functionNodes.end()){
                            string from = prefix + edge.from();
                            string to = prefix + edge.to();
                            MRRG.addEdge(Edge(from, to));
                        }
                    }
                }
            }
            for(size_t idx = 0; idx < ii; idx++){
                size_t idy = (idx+1) % ii;
                string prefix = "T" + num2str(idx) + "_" + "T" + num2str(idy) + ".";
                clog << prefix << endl;
                for(const auto &vertex: rrg.vertices()){
                    if(functionNodes.find(vertex.first) != functionNodes.end()){
                        continue;
                    }
                    Vertex vertex1(prefix + vertex.first, vertex.second.attrs());
                    MRRG.addVertex(vertex1);
                }
                string prefixFrom = "T" + num2str(idx) + ".";
                string prefixTo = "T" + num2str(idy) + ".";
                for(const auto &vertex: rrg.vertices()){
                    bool isFromPR = functionNodes.find(vertex.first) != functionNodes.end();
                    for(const auto &edge: rrg.edgesOut(vertex.first)){
                        bool isToPR = functionNodes.find(edge.to()) != functionNodes.end();
                        if(isFromPR && !isToPR){
                            MRRG.addEdge(Edge(prefixFrom + edge.from(), prefix + edge.to()));
                        }
                        if(!isFromPR && isToPR){
                            MRRG.addEdge(Edge(prefix + edge.from(), prefixTo + edge.to()));
                        }
                        if(!isFromPR && !isToPR){
                            MRRG.addEdge(Edge(prefix + edge.from(), prefix + edge.to()));
                        }
                    }
                }
            }

            return MRRG;
        }

    }
}