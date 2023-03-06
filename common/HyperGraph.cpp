#include "HyperGraph.h" 

using namespace std; 

namespace FastCGRA
{

ostream &operator << (ostream &out, const Attribute &attr)
{
    if(attr.isStr())
    {
        out << attr.getStr();  
    }
    else if(attr.isFloat())
    {
        out << attr.getFloat();  
    }
    else if(attr.isInt())
    {
        out << attr.getInt();  
    }
    else if(attr.isArr())
    {
        out << attr.getArr();  
    }
    else if(attr.isVec())
    {
        out << attr.getVec();  
    }
    
    return out; 
}

template<> const int                      &Attribute::as<int>()                      const {return _attrInt; } 
template<> const double                   &Attribute::as<double>()                   const {return _attrFloat; } 
template<> const std::string              &Attribute::as<std::string>()              const {return _attrStr; } 
template<> const std::vector<double>      &Attribute::as<std::vector<double>>()      const {return _attrVec; } 
template<> const std::vector<std::string> &Attribute::as<std::vector<std::string>>() const {return _attrArr; } 

void Net::delSink(const std::string &name)
{
    ASSERT(_nodes.size() > 0, "Net::delSink: Cannot delete the sink of an empty net. "); 
    for(size_t idx = 1; idx < _nodes.size(); idx++)
    {
        if(_nodes[idx] == name)
        {
            _nodes.erase(_nodes.begin() + idx); 
            break; 
        }
    }
}

bool Net::operator == (const Net &net)
{
    if(_nodes.size() == 0 && net._nodes.size() == 0)
    {
        return true; 
    }
    
    bool same = (_nodes[0] == net._nodes[0] && _nodes.size() == net._nodes.size()); 
    for(size_t idx = 1; same && idx < _nodes.size(); idx++)
    {
        if(_nodes[idx] != net._nodes[idx])
        {
            same = false; 
        }
    }
    
    return same; 
}

bool HyperGraph::loadFrom(const std::string &filename)
{
    ifstream fin(filename); 
    if(!fin)
    {
        WARN << "HyperGraph::loadFrom: WARN: cannot open the file: " << filename; 
        return false; 
    }
    
    stringstream sin; 
    sin << fin.rdbuf(); 
    this->fromString(sin.str()); 
    fin.close(); 

    _mapAttrs["__file__"] = filename; 

    return true; 
}

bool HyperGraph::dumpTo(const std::string &filename) const
{
    ofstream fout(filename); 
    if(!fout)
    {
        WARN << "HyperGraph::dumpTo: WARN: cannot open the file: " << filename; 
        return false; 
    }

    string tmp = this->toString(); 
    fout.write(tmp.c_str(), tmp.size()); 

    return true; 
}

std::ostream &operator << (std::ostream &out, const HyperGraph &graph)
{
    out << graph.toString() << endl;
    return out;
}


std::vector<std::string> HyperGraph::vertexNames() const
{
    vector<string> result; 
    for(const auto &vertex: _vertices)
    {
        result.push_back(vertex.first); 
    }
    return result; 
}

void HyperGraph::fromString(const std::string &content)
{
    istringstream fin(content); 
    ASSERT(fin, "HyperGraph: ERROR: parse " + content + " fail")

    string line, tmpstr; 
    vector<Vertex> vertices; 
    vector<Net> nets; 
    bool undealedLine = false; 
    while(!fin.eof() || undealedLine)
    {
        if(undealedLine)
        {
            undealedLine = false; 
        }
        else
        {
            line.clear(); 
            getline(fin, line);
        }
        
        if(line.empty())
        {
            continue; 
        }
        istringstream sin(line); 
        tmpstr.clear(); 
        sin >> tmpstr; 
        if(tmpstr.empty())
        {
            continue; 
        }
        else
        {
            ASSERT(tmpstr == "vertex" || tmpstr == "Vertex" || tmpstr == "VERTEX" || tmpstr == "node" || tmpstr == "Node" || tmpstr == "NODE" || tmpstr == "edge" || tmpstr == "Edge" || tmpstr == "EDGE" || tmpstr == "net" || tmpstr == "Net" || tmpstr == "NET" || tmpstr == "attr" || tmpstr == "Attr" || tmpstr == "ATTR", 
                   string("HyperGraph::loadFrom: Unsupport type: ") + tmpstr +  "; from: " + line); 
            if(tmpstr == "vertex" || tmpstr == "Vertex" || tmpstr == "VERTEX" || tmpstr == "node" || tmpstr == "Node" || tmpstr == "NODE")
            {
                // NOTE << "HyperGraph::loadFrom: Find a vertex -- " << line;  
                string name; 
                sin >> name; 
                AttrHash attrs; 
                while(!fin.eof())
                {
                    line.clear(); 
                    getline(fin, line); 
                    if(line.empty())
                    {
                        continue; 
                    }
                    istringstream sin(line); 
                    tmpstr.clear(); 
                    sin >> tmpstr; 
                    if(tmpstr.empty())
                    {
                        continue; 
                    }
                    else if(tmpstr == "attr" || tmpstr == "Attr" || tmpstr == "ATTR")
                    {
                        string name; 
                        string type; 
                        string value; 
                        sin >> name >> type >> value; 
                        while(!sin.eof())
                        {
                            string tmp; 
                            sin >> tmp; 
                            if(tmp.size() > 0)
                            {
                                value += string(" ") + tmp; 
                            }
                        }
                        ASSERT(type == "int" || type == "float" || type == "str" || type == "arr" || type == "vec", 
                               string("HyperGraph::loadFrom: Unsupported attribute type: ") + type + "; from: " + line); 
                        if(type == "int")
                        {
                            attrs[name] = Attribute(atoi(value.c_str())); 
                        }
                        else if(type == "float")
                        {
                            attrs[name] = Attribute(as<double>(value.c_str())); 
                        }
                        else if(type == "str")
                        {
                            attrs[name] = Attribute(value.c_str()); 
                        }
                        else if(type == "arr")
                        {
                            attrs[name] = Attribute(parseArr(value)); 
                        }
                        else if(type == "vec")
                        {
                            attrs[name] = Attribute(parseVec(value)); 
                        }
                        else
                        {
                            attrs[name] = Attribute(value); 
                        }
                    }
                    else if(tmpstr == "end" || tmpstr == "End" || tmpstr == "END")
                    {
                        break; 
                    }
                    else
                    {
                        undealedLine = true; 
                        break; 
                    }
                }
                vertices.push_back(Vertex(name, attrs)); 
            }
            else if(tmpstr == "edge" || tmpstr == "Edge" || tmpstr == "EDGE" || tmpstr == "net" || tmpstr == "Net" || tmpstr == "NET")
            {
                vector<string> nodes; 
                while(!sin.eof())
                {
                    string tmp; 
                    sin >> tmp; 
                    if(tmp.size() > 0)
                    {
                        nodes.push_back(tmp); 
                    }
                }
                AttrHash attrs; 
                while(!fin.eof())
                {
                    line.clear(); 
                    getline(fin, line); 
                    if(line.empty())
                    {
                        continue; 
                    }
                    istringstream sin(line); 
                    tmpstr.clear(); 
                    sin >> tmpstr; 
                    if(tmpstr.empty())
                    {
                        continue; 
                    }
                    else if(tmpstr == "attr" || tmpstr == "Attr" || tmpstr == "ATTR")
                    {
                        string name; 
                        string type; 
                        string value; 
                        sin >> name >> type >> value; 
                        while(!sin.eof())
                        {
                            string tmp; 
                            sin >> tmp; 
                            if(tmp.size() > 0)
                            {
                                value += string(" ") + tmp; 
                            }
                        }
                        ASSERT(type == "int" || type == "float" || type == "str" || type == "arr" || type == "vec", 
                               string("HyperGraph::loadFrom: Unsupported attribute type: ") + type + "; from: " + line); 
                        if(type == "int")
                        {
                            attrs[name] = Attribute(atoi(value.c_str())); 
                        }
                        else if(type == "float")
                        {
                            attrs[name] = Attribute(as<double>(value.c_str())); 
                        }
                        else if(type == "str")
                        {
                            attrs[name] = Attribute(value.c_str()); 
                        }
                        else if(type == "arr")
                        {
                            attrs[name] = Attribute(parseArr(value)); 
                        }
                        else if(type == "vec")
                        {
                            attrs[name] = Attribute(parseVec(value)); 
                        }
                        else
                        {
                            attrs[name] = Attribute(value); 
                        }
                    }
                    else if(tmpstr == "end" || tmpstr == "End" || tmpstr == "END")
                    {
                        break; 
                    }
                    else
                    {
                        undealedLine = true; 
                        break; 
                    }
                }
                nets.push_back(Net(nodes, attrs)); 
            }
            else if(tmpstr == "attr" || tmpstr == "Attr" || tmpstr == "ATTR")
            {
                AttrHash attrs; 
                if(tmpstr == "attr" || tmpstr == "Attr" || tmpstr == "ATTR")
                {
                    string name; 
                    string type; 
                    string value; 
                    sin >> name >> type >> value; 
                    while(!sin.eof())
                    {
                        string tmp; 
                        sin >> tmp; 
                        if(tmp.size() > 0)
                        {
                            value += string(" ") + tmp; 
                        }
                    }
                    ASSERT(type == "int" || type == "float" || type == "str" || type == "arr" || type == "vec", 
                            string("HyperGraph::loadFrom: Unsupported attribute type: ") + type + "; from: " + line); 
                    if(type == "int")
                    {
                        attrs[name] = Attribute(atoi(value.c_str())); 
                    }
                    else if(type == "float")
                    {
                        attrs[name] = Attribute(as<double>(value.c_str())); 
                    }
                    else if(type == "str")
                    {
                        attrs[name] = Attribute(value.c_str()); 
                    }
                    else if(type == "arr")
                    {
                        attrs[name] = Attribute(parseArr(value)); 
                    }
                    else if(type == "vec")
                    {
                        attrs[name] = Attribute(parseVec(value)); 
                    }
                    else
                    {
                        attrs[name] = Attribute(value); 
                    }
                }
                while(!fin.eof())
                {
                    line.clear(); 
                    getline(fin, line); 
                    if(line.empty())
                    {
                        continue; 
                    }
                    istringstream sin(line); 
                    tmpstr.clear(); 
                    sin >> tmpstr; 
                    if(tmpstr.empty())
                    {
                        continue; 
                    }
                    else if(tmpstr == "attr" || tmpstr == "Attr" || tmpstr == "ATTR")
                    {
                        string name; 
                        string type; 
                        string value; 
                        sin >> name >> type >> value; 
                        while(!sin.eof())
                        {
                            string tmp; 
                            sin >> tmp; 
                            if(tmp.size() > 0)
                            {
                                value += string(" ") + tmp; 
                            }
                        }
                        ASSERT(type == "int" || type == "float" || type == "str" || type == "arr" || type == "vec", 
                               string("HyperGraph::loadFrom: Unsupported attribute type: ") + type + "; from: " + line); 
                        if(type == "int")
                        {
                            attrs[name] = Attribute(atoi(value.c_str())); 
                        }
                        else if(type == "float")
                        {
                            attrs[name] = Attribute(as<double>(value.c_str())); 
                        }
                        else if(type == "str")
                        {
                            attrs[name] = Attribute(value.c_str()); 
                        }
                        else if(type == "arr")
                        {
                            attrs[name] = Attribute(parseArr(value)); 
                        }
                        else if(type == "vec")
                        {
                            attrs[name] = Attribute(parseVec(value)); 
                        }
                        else
                        {
                            attrs[name] = Attribute(value); 
                        }
                    }
                    else if(tmpstr == "end" || tmpstr == "End" || tmpstr == "END")
                    {
                        break; 
                    }
                    else
                    {
                        undealedLine = true; 
                        break; 
                    }
                }
                _mapAttrs = attrs; 
            }
        }
    }
    for(const auto &vertex: vertices)
    {
        this->addVertex(vertex); 
    }
    for(const auto &net: nets)
    {
        this->addNet(net); 
    }
    if(_name.empty() && _mapAttrs.find("__name__") != _mapAttrs.end() && _mapAttrs.find("__name__")->second.isStr())
    {
        _name = _mapAttrs.find("__name__")->second.asStr(); 
    }
    else if(!_name.empty())
    {
        _mapAttrs["__name__"] = _name; 
    }
}

std::string HyperGraph::toString() const
{
    ostringstream sout; 

    for(const auto &attr: _mapAttrs)
    {
        sout << "attr " << attr.first; 
        if(attr.second.isStr())
        {
            sout << " str " << attr.second.getStr();  
        }
        else if(attr.second.isFloat())
        {
            sout << " float " << attr.second.getFloat();  
        }
        else if(attr.second.isInt())
        {
            sout << " int " << attr.second.getInt();  
        }
        else if(attr.second.isArr())
        {
            sout << " arr " << attr.second.getArr();  
        }
        else if(attr.second.isVec())
        {
            sout << " vec " << attr.second.getVec();  
        }
        sout << endl; 
    } 
    for(const auto &vertex: _vertices)
    {
        sout << "vertex " << vertex.first << endl;
        for(const auto &attr: vertex.second.attrs())
        {
            sout << "    attr " << attr.first; 
            if(attr.second.isStr())
            {
                sout << " str " << attr.second.getStr();  
            }
            else if(attr.second.isFloat())
            {
                sout << " float " << attr.second.getFloat();  
            }
            else if(attr.second.isInt())
            {
                sout << " int " << attr.second.getInt();  
            }
            else if(attr.second.isArr())
            {
                sout << " arr " << attr.second.getArr();  
            }
            else if(attr.second.isVec())
            {
                sout << " vec " << attr.second.getVec();  
            }
            sout << endl; 
        } 
    }
    for(const auto &edges: _netsOut)
    {
        for(const auto &edge: edges.second)
        {
            sout << "net " << edge.source();
            for(size_t idx = 0; idx < edge.sizeSinks(); idx++)
            {
                sout << " " << edge.sink(idx); 
            }
            sout << endl; 
            for(const auto &attr: edge.attrs())
            {
                sout << "    attr " << attr.first; 
                if(attr.second.isStr())
                {
                    sout << " str " << attr.second.getStr();  
                }
                else if(attr.second.isFloat())
                {
                    sout << " float " << attr.second.getFloat();  
                }
                else if(attr.second.isInt())
                {
                    sout << " int " << attr.second.getInt();  
                }
                else if(attr.second.isArr())
                {
                    sout << " arr " << attr.second.getArr();  
                }
                else if(attr.second.isVec())
                {
                    sout << " vec " << attr.second.getVec();  
                }
                sout << endl; 
            } 
        }
    }
    
    return sout.str(); 
}
    
void HyperGraph::delVertex(const std::string &name)
{
    // Delete nets
    for(size_t idx = 0; idx < _netsIn[name].size(); idx++)
    {
        const string &source = _netsIn[name][idx].source(); 
        for(size_t jdx = 0; jdx < _netsOut[source].size(); jdx++)
        {
            if(_netsOut[source][jdx] == _netsIn[name][idx])
            {
                _netsOut[source][jdx].delSink(name); 
                if(_netsOut[source][jdx].sizeSinks() == 0)
                {
                    _netsOut[source].erase(_netsOut[source].begin() + jdx); 
                }
                for(size_t kdx = 0; kdx < _netsOut[source][jdx].sizeSinks(); kdx++)
                {
                    const string &sink = _netsOut[source][jdx].sink(kdx); 
                    for(auto &edge: _netsIn[sink])
                    {
                        if(edge == _netsIn[name][idx])
                        {
                            edge.delSink(name); 
                        }
                    }
                }
            }
        }
    }
    for(size_t idx = 0; idx < _netsOut[name].size(); idx++)
    {
        for(size_t jdx = 0; jdx < _netsOut[name][idx].sizeSinks(); jdx++)
        {
            const string &node = _netsOut[name][idx].sink(jdx); 
            size_t index = static_cast<size_t>(-1); 
            for(size_t kdx = 0; kdx < _netsIn[node].size(); kdx++)
            {
                if(_netsIn[node][kdx] == _netsOut[name][idx])
                {
                    index = kdx; 
                    break; 
                }
            }
            ASSERT(index != static_cast<size_t>(-1), string("HyperGraph::delVertex: cannot find corresponding input net of ") + name + "(" + node + ")"); 
            _netsIn[node].erase(_netsIn[node].begin() + index); 
        }
    }
    
    _netsIn.erase(name); 
    _netsOut.erase(name); 
    _vertices.erase(name); 
}

unordered_map<string, vector<vector<string>>> HyperGraph::reachableVertices(const string &from, const function<bool(const Vertex &)> &funcPath, const function<bool(const Vertex &)> &funcSink) const
{
    unordered_map<string, vector<vector<string>>> result; 
    
    queue<string> queNow; 
    queue<vector<string>> quePath; 

    queNow.push(from); 
    quePath.push(vector<string>()); 
    while(!queNow.empty() && !quePath.empty())
    {
        string now = queNow.front();  
        for(const auto &edge: _netsOut.find(now)->second)
        {
            vector<string> path = quePath.front(); 
            for(size_t idx = 1; idx < edge.nodes().size(); idx++)
            {
                if(funcSink(_vertices.find(edge.nodes()[idx])->second))
                {
                    if(result.find(edge.nodes()[idx]) == result.end())
                    {
                        result[edge.nodes()[idx]] = vector<vector<string>>(); 
                    }
                    result[edge.nodes()[idx]].push_back(path);
                    continue; 
                }
                if(funcPath(_vertices.find(edge.nodes()[idx])->second))
                {
                    path.push_back(edge.nodes()[idx]); 
                    queNow.push(edge.nodes()[idx]); 
                    quePath.push(path); 
                }
            }
        }
        queNow.pop(); 
        quePath.pop(); 
    }

    return result; 
}

unordered_set<string> HyperGraph::reachableFrom(const string &from, const function<bool(const Vertex &)> &funcPath, const function<bool(const Vertex &)> &funcSink) const
{
    unordered_set<string> result; 
    
    queue<string> queNow; 
    unordered_map<string, bool> used; 
    for(const auto &vertex: _vertices)
    {
        used[vertex.first] = false; 
    }

    queNow.push(from); 
    used[from] = true; 
    while(!queNow.empty())
    {
        string now = queNow.front(); queNow.pop(); 
        for(const auto &edge: _netsOut.find(now)->second)
        {
            for(size_t idx = 1; idx < edge.nodes().size(); idx++)
            {
                if(funcSink(_vertices.find(edge.nodes()[idx])->second))
                {
                    result.insert(edge.nodes()[idx]);
                    continue; 
                }
                if(!used[edge.nodes()[idx]] && funcPath(_vertices.find(edge.nodes()[idx])->second))
                {
                    used[edge.nodes()[idx]] = true;
                    queNow.push(edge.nodes()[idx]); 
                }
            }
        }
    }

    return result; 
}

pair<vector<string>, bool> HyperGraph::findPath(const string &from, const string &to) const
{
    vector<string> result; 
    
    queue<string> queNow; 
    queue<vector<string>> quePath; 
    unordered_map<string, bool> used; 
    for(const auto &vertex: _vertices)
    {
        used[vertex.first] = false; 
    }

    queNow.push(from); 
    quePath.push(vector<string>()); 
    used[from] = true; 
    bool found = false; 
    while(!queNow.empty() && !quePath.empty() && !found)
    {
        string now = queNow.front();  
        for(const auto &edge: _netsOut.find(now)->second)
        {
            vector<string> path = quePath.front(); 
            if(edge.to() == to)
            {
                result = path;
                found = true; 
                break; 
            }
            if(!used[edge.to()])
            {
                used[edge.to()] = true; 
                path.push_back(edge.to()); 
                queNow.push(edge.to()); 
                quePath.push(path); 
            }
        }
        queNow.pop(); 
        quePath.pop(); 
    }

    return {result, found}; 
}
   
pair<vector<string>, bool> HyperGraph::findPath(const string &from, const string &to, const std::function<bool(const Vertex &)> &funcPath) const
{
    vector<string> result; 
    
    queue<string> queNow; 
    queue<vector<string>> quePath; 
    unordered_map<string, bool> used; 
    for(const auto &vertex: _vertices)
    {
        used[vertex.first] = false; 
    }

    queNow.push(from); 
    quePath.push(vector<string>()); 
    used[from] = true; 
    bool found = false;
    while(!queNow.empty() && !quePath.empty() && !found)
    {
        string now = queNow.front();  
        for(const auto &edge: _netsOut.find(now)->second)
        {
            vector<string> path = quePath.front(); 
            if(edge.to() == to)
            {
                result = path;
                found = true; 
                break; 
            }
            if(!used[edge.to()] && funcPath(_vertices.find(edge.to())->second))
            {
                used[edge.to()] = true; 
                path.push_back(edge.to()); 
                queNow.push(edge.to()); 
                quePath.push(path); 
            }
        }
        queNow.pop(); 
        quePath.pop(); 
    }

    return {result, found}; 
}

    
std::vector<std::vector<std::string>> HyperGraph::findPaths(const std::string &from, const std::string &to) const
{
    vector<vector<string>> result; 
    
    queue<string> queNow; 
    queue<vector<string>> quePath; 

    queNow.push(from); 
    quePath.push(vector<string>());
    while(!queNow.empty() && !quePath.empty())
    {
        string now = queNow.front();  
        for(const auto &edge: _netsOut.find(now)->second)
        {
            vector<string> path = quePath.front(); 
            if(edge.to() == to)
            {
                result.push_back(path);
                continue; 
            }
            path.push_back(edge.to()); 
            queNow.push(edge.to()); 
            quePath.push(path); 
        }
        queNow.pop(); 
        quePath.pop(); 
    }

    return result;

}

std::vector<std::vector<std::string>> HyperGraph::findPaths(const std::string &from, const std::string &to, const std::function<bool(const Vertex &)> &funcPath) const
{
    const size_t maxTime = 1024 * 128;
    vector<vector<string>> result; 
    
    queue<string> queNow; 
    queue<vector<string>> quePath; 

    queNow.push(from); 
    quePath.push(vector<string>());
    size_t Iter = 0;
    while(!queNow.empty() && !quePath.empty())
    {
        if(Iter++ > maxTime){
            break;
        }
        string now = queNow.front();  
        for(const auto &edge: _netsOut.find(now)->second)
        {
            vector<string> path = quePath.front(); 
            if(edge.to() == to)
            {
                result.push_back(path);
                continue; 
            }
            if(funcPath(_vertices.find(edge.to())->second))
            {
                path.push_back(edge.to()); 
                queNow.push(edge.to()); 
                quePath.push(path); 
            }
        }
        queNow.pop(); 
        quePath.pop();
    }

    return result;

}


}


 
