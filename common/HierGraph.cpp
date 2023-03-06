#include "HierGraph.h"

using namespace std; 

namespace FastCGRA
{

std::vector<std::string> HierGraph::subgraphNames() const
{
    vector<string> result; 
    for(const auto &subgraph: _subgraphs)
    {
        result.push_back(subgraph.first); 
    }
    return result; 
}

std::vector<std::string> HierGraph::elementNames() const
{
    vector<string> result; 
    for(const auto &element: _elements)
    {
        result.push_back(element.first); 
    }
    return result; 
}
    
void HierGraph::delSubgraph(const std::string &name)
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
            ASSERT(index != static_cast<size_t>(-1), string("HierGraph::delSubgraph: cannot find corresponding input net of ") + name + "(" + node + ")"); 
            _netsIn[node].erase(_netsIn[node].begin() + index); 
        }
    }
    
    _netsIn.erase(name); 
    _netsOut.erase(name); 
    _subgraphs.erase(name); 
}
    
void HierGraph::delElement(const std::string &name)
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
            ASSERT(index != static_cast<size_t>(-1), string("HierGraph::delElement: cannot find corresponding input net of ") + name + "(" + node + ")"); 
            _netsIn[node].erase(_netsIn[node].begin() + index); 
        }
    }
    
    _netsIn.erase(name); 
    _netsOut.erase(name); 
    _elements.erase(name); 
}

void HierGraph::fromString(const std::string &content)
{
    istringstream fin(content); 

    string line, tmpstr; 
    vector<HierGraph> subgraphs; 
    vector<HyperGraph> elements; 
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
            ASSERT(tmpstr == "subgraph" || tmpstr == "Subgraph" || tmpstr == "SUBGRAPH" || tmpstr == "element" || tmpstr == "Element" || tmpstr == "ELEMENT" || tmpstr == "edge" || tmpstr == "Edge" || tmpstr == "EDGE" || tmpstr == "net" || tmpstr == "Net" || tmpstr == "NET" || tmpstr == "attr" || tmpstr == "Attr" || tmpstr == "ATTR", 
                   string("HyperGraph::loadFrom: Unsupport type: ") + tmpstr +  "; from: " + line); 
            if(tmpstr == "subgraph" || tmpstr == "Subgraph" || tmpstr == "SUBGRAPH")
            {
                // NOTE << "HierGraph::loadFrom: Find a subgraph / element -- " << line;  
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

                HierGraph tmp(name); 
                tmp.loadFrom(attrs["__file__"].asStr()); 
                subgraphs.push_back(tmp); 
            }
            else if(tmpstr == "element" || tmpstr == "Element" || tmpstr == "ELEMENT")
            {
                // NOTE << "HierGraph::loadFrom: Find a subgraph / element -- " << line;  
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

                HyperGraph tmp(name); 
                tmp.loadFrom(attrs["__file__"].asStr()); 
                elements.push_back(tmp); 
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
    for(const auto &subgraph: subgraphs)
    {
        this->addSubgraph(subgraph); 
    }
    for(const auto &element: elements)
    {
        this->addElement(element); 
    }
    for(const auto &net: nets)
    {
        this->addNet(net); 
    }
    if(_name.empty() && _mapAttrs.find("__name__") != _mapAttrs.end() && _mapAttrs.find("__name__")->second.isStr())
    {
        _name = _mapAttrs.find("__name__")->second.asStr(); 
    }
}

std::string HierGraph::toString() const
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
    for(const auto &subgraph: _subgraphs)
    {
        sout << "subgraph " << subgraph.first << endl;
        for(const auto &attr: subgraph.second.attrs())
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
    for(const auto &element: _elements)
    {
        sout << "element " << element.first << endl;
        for(const auto &attr: element.second.attrs())
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
            sout << "edge " << edge.source();
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


bool HierGraph::loadFrom(const std::string &filename)
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

bool HierGraph::dumpTo(const std::string &filename) const
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

}