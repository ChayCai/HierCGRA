#ifndef __FASTCGRA_HYPERGRAPH__
#define __FASTCGRA_HYPERGRAPH__

#include "Common.h"
#include "Logger.h"

namespace FastCGRA
{
//=================================================================================
// Attribute
//=================================================================================
    
class Attribute
{
private:
    int         _attrInt; 
    double      _attrFloat; 
    std::string _attrStr; 
    std::vector<double>      _attrVec; 
    std::vector<std::string> _attrArr; 
    std::string _type; 

public: 
    Attribute()                        : _attrInt(0),     _attrFloat(0.0),   _attrStr(""),    _attrVec(),      _attrArr(),      _type("int") {}
    Attribute(int value)               : _attrInt(value), _attrFloat(0.0),   _attrStr(""),    _attrVec(),      _attrArr(),      _type("int") {}
    Attribute(double value)            : _attrInt(0),     _attrFloat(value), _attrStr(""),    _attrVec(),      _attrArr(),      _type("float") {}
    Attribute(const std::string &value): _attrInt(0),     _attrFloat(0.0),   _attrStr(value), _attrVec(),      _attrArr(),      _type("str") {}
    Attribute(std::string &&value)     : _attrInt(0),     _attrFloat(0.0),   _attrStr(value), _attrVec(),      _attrArr(),      _type("str") {}
    Attribute(const std::vector<double> &value): _attrInt(0),    _attrFloat(0.0), _attrStr(""), _attrVec(value), _attrArr(), _type("vec") {}
    Attribute(std::vector<double> &&value)     : _attrInt(0),    _attrFloat(0.0), _attrStr(""), _attrVec(value), _attrArr(), _type("vec") {}
    Attribute(const std::vector<std::string> &value): _attrInt(0),    _attrFloat(0.0), _attrStr(""), _attrVec(), _attrArr(value), _type("arr") {}
    Attribute(std::vector<std::string> &&value)     : _attrInt(0),    _attrFloat(0.0), _attrStr(""), _attrVec(), _attrArr(value), _type("arr") {}
    Attribute(const Attribute &attr)   : _attrInt(attr._attrInt), _attrFloat(attr._attrFloat), _attrStr(attr._attrStr), _attrVec(attr._attrVec), _attrArr(attr._attrArr), _type(attr._type) {}
    Attribute(Attribute &&attr)        : _attrInt(attr._attrInt), _attrFloat(attr._attrFloat), _attrStr(attr._attrStr), _attrVec(attr._attrVec), _attrArr(attr._attrArr), _type(attr._type) {}

    const Attribute &operator = (const Attribute &attr)    {_attrInt = attr._attrInt; _attrFloat = attr._attrFloat; _attrStr = attr._attrStr; _attrVec = attr._attrVec; _attrArr = attr._attrArr; _type = attr._type; return *this; }
    const Attribute &operator = (Attribute &&attr)         {_attrInt = attr._attrInt; _attrFloat = attr._attrFloat; _attrStr = attr._attrStr; _attrVec = attr._attrVec; _attrArr = attr._attrArr; _type = attr._type; return *this; }
    const Attribute &operator = (int value)                {_type = "int";   _attrInt   = value; return *this; }
    const Attribute &operator = (double value)             {_type = "float"; _attrFloat = value; return *this; }
    const Attribute &operator = (const std::string &value) {_type = "str";   _attrStr   = value; return *this; }
    const Attribute &operator = (std::string &&value)      {_type = "str";   _attrStr   = value; return *this; }
    const Attribute &operator = (const std::vector<std::string> &value) {_type = "arr"; _attrArr   = value; return *this; }
    const Attribute &operator = (std::vector<std::string> &&value)      {_type = "arr"; _attrArr   = value; return *this; }
    const Attribute &operator = (const std::vector<double> &value)      {_type = "vec"; _attrVec   = value; return *this; }
    const Attribute &operator = (std::vector<double> &&value)           {_type = "vec"; _attrVec   = value; return *this; }

    const std::string &type() const {return _type; }
    bool isInt()   const            {return _type == "int"; }
    bool isFloat() const            {return _type == "float"; }
    bool isStr()   const            {return _type == "str"; }
    bool isArr()   const            {return _type == "arr"; }
    bool isVec()   const            {return _type == "vec"; }

    operator int()                     const     {return _attrInt; }
    operator float()                   const     {return _attrFloat; }
    operator double()                  const     {return _attrFloat; }
    int         &asInt()                         {return _attrInt; }
    int         &getInt()                        {return _attrInt; }
    int         asInt()                const     {return _attrInt; }
    int         getInt()               const     {return _attrInt; }
    void        setInt(int value)                {_attrInt = value; }
    double      &asFloat()                       {return _attrFloat; }
    double      &getFloat()                      {return _attrFloat; }
    double      asFloat()              const     {return _attrFloat; }
    double      getFloat()             const     {return _attrFloat; }
    void        setFloat(double value)           {_attrFloat = value; }
    std::string &asStr()                         {return _attrStr; } 
    std::string &getStr()                        {return _attrStr; } 
    const std::string &asStr()         const     {return _attrStr; } 
    const std::string &getStr()        const     {return _attrStr; } 
    const std::string &str()           const     {return _attrStr; } 
    void        setStr(const std::string &value) {_attrStr = value; }
    void        setStr(std::string &&value)      {_attrStr = value; }
    std::vector<double> &asVec()              {return _attrVec; } 
    std::vector<double> &getVec()             {return _attrVec; } 
    const std::vector<double> &asVec()  const {return _attrVec; } 
    const std::vector<double> &getVec() const {return _attrVec; } 
    const std::vector<double> &vec()    const {return _attrVec; } 
    void        setVec(const std::vector<double> &value) {_attrVec = value; }
    void        setVec(std::vector<double> &&value)      {_attrVec = value; }
    std::vector<std::string>       &asArr()        {return _attrArr; } 
    std::vector<std::string>       &getArr()       {return _attrArr; } 
    const std::vector<std::string> &asArr()  const {return _attrArr; } 
    const std::vector<std::string> &getArr() const {return _attrArr; } 
    const std::vector<std::string> &arr()    const {return _attrArr; } 
    void        setArr(const std::vector<std::string> &value) {_attrArr = value; }
    void        setArr(std::vector<std::string> &&value)      {_attrArr = value; }
    template<typename Type> const Type &as() const;   

}; 

std::ostream &operator << (std::ostream &out, const Attribute &attr); 

template<> const int                      &Attribute::as<int>()                      const; 
template<> const double                   &Attribute::as<double>()                   const; 
template<> const std::string              &Attribute::as<std::string>()              const; 
template<> const std::vector<double>      &Attribute::as<std::vector<double>>()      const; 
template<> const std::vector<std::string> &Attribute::as<std::vector<std::string>>() const; 

typedef std::unordered_map<std::string, Attribute> AttrHash; 


//=================================================================================
// Vertex
//=================================================================================

class Vertex
{
private: 
    std::string _name; 
    AttrHash    _mapAttrs; 

public:
    Vertex() = default; 
    Vertex(const std::string &name)                           : _name(name),         _mapAttrs()                 {}
    Vertex(const std::string &name, const AttrHash &mapAttrs) : _name(name),         _mapAttrs(mapAttrs)         {}
    Vertex(const Vertex &vertex)                              : _name(vertex._name), _mapAttrs(vertex._mapAttrs) {}
    Vertex(Vertex &&vertex)                                   : _name(vertex._name), _mapAttrs(vertex._mapAttrs) {}

    const Vertex &operator = (const Vertex &vertex) {_name = vertex._name; _mapAttrs = vertex._mapAttrs; return *this; }
    const Vertex &operator = (Vertex &&vertex)      {_name = vertex._name; _mapAttrs = vertex._mapAttrs; return *this; }

    const std::string &name() const {return _name; }

    bool             hasAttr(const std::string &name) const                  {if(_mapAttrs.find(name) != _mapAttrs.end()){return true; } return false; }
    Attribute &      getAttr(const std::string &name)                        {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("Attribute ") + name + " not found. "); return _mapAttrs[name]; }
    const Attribute &getAttr(const std::string &name) const                  {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    void             setAttr(const std::string &name, const Attribute &attr) {_mapAttrs[name] = attr; }
    Attribute &      operator[](const std::string &name)                     {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("Attribute ") + name + " not found. "); return _mapAttrs[name]; }
    const Attribute &operator[](const std::string &name) const               {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }

    AttrHash &      attrs()       {return _mapAttrs; }
    const AttrHash &attrs() const {return _mapAttrs; }
    
    bool operator == (const Vertex &vertex) {return _name == vertex._name; }
}; 


//=================================================================================
// Net
//=================================================================================

class Net
{
private: 
    std::vector<std::string> _nodes; 
    AttrHash                 _mapAttrs; 

public:
    Net() {}
    Net(const std::string &from, const std::string &to): _nodes({from, to}) {} 
    Net(const std::string &from, const std::string &to, const AttrHash &mapAttrs): _nodes({from, to}), _mapAttrs(mapAttrs) {} 
    Net(const std::string &from, const std::vector<std::string> &tos): _nodes(from + tos) {} 
    Net(const std::string &from, const std::vector<std::string> &tos, const AttrHash &mapAttrs): _nodes(from + tos), _mapAttrs(mapAttrs) {} 
    Net(const std::vector<std::string> &nodes): _nodes(nodes) {} 
    Net(const std::vector<std::string> &nodes, const AttrHash &mapAttrs): _nodes(nodes), _mapAttrs(mapAttrs) {}
    Net(const Net &net): _nodes(net._nodes), _mapAttrs(net._mapAttrs) {}
    Net(Net &&net): _nodes(std::move(net._nodes)), _mapAttrs(std::move(net._mapAttrs)) {}

    const Net &operator = (const Net &net) {_nodes = net._nodes; _mapAttrs = net._mapAttrs; return *this; }
    const Net &operator = (Net &&net)      {_nodes = net._nodes; _mapAttrs = net._mapAttrs; return *this; }
    
    std::string &source() {ASSERT(_nodes.size() > 0, "Net::source: Empty net. "); return _nodes[0]; }
    std::string &sink(size_t index = 0) {ASSERT(index+1 < _nodes.size(), "Net::sink: Index out of range. "); return _nodes[index+1]; }
    const std::string &source() const {ASSERT(_nodes.size() > 0, "Net::source: Empty net. "); return _nodes[0]; }
    const std::string &sink(size_t index = 0) const {ASSERT(index+1 < _nodes.size(), "Net::sink: Index out of range. "); return _nodes[index+1]; }
    std::string &from() {ASSERT(_nodes.size() > 0, "Net::from: Empty net. "); return _nodes[0]; }
    std::string &to() {ASSERT(_nodes.size() > 1, "Net::to: Invalid net, no sink. "); return _nodes[1]; }
    const std::string &from() const {ASSERT(_nodes.size() > 0, "Net::from: Empty net. "); return _nodes[0]; }
    const std::string &to() const {ASSERT(_nodes.size() > 1, "Net::to: Invalid net, no sink. "); return _nodes[1]; }
    
    void setSource(const std::string &name) {ASSERT(_nodes.size() > 0, "Net::setSource: Empty net. "); _nodes[0] = name; }
    void appendSink(const std::string &name) {ASSERT(_nodes.size() > 0, "Net::appendSink: Empty net. "); _nodes.push_back(name); }
    void setSink(size_t index, const std::string &name) {ASSERT(index+1 < _nodes.size(), "Net::setSink: Index out of range. "); _nodes[index+1] = name; }
    void source(const std::string &name) {setSource(name); }
    void sink(const std::string &name) {appendSink(name); }
    void sink(size_t index, const std::string &name) {setSink(index, name); }
    void delSink(const std::string &name); 
    std::vector<std::string>       &nodes()       {return _nodes; }
    const std::vector<std::string> &nodes() const {return _nodes; }

    bool             hasAttr(const std::string &name) const                  {if(_mapAttrs.find(name) != _mapAttrs.end()){return true; } return false; }
    Attribute &      getAttr(const std::string &name)                        {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("Net::getAttr: Attribute ") + name + " not found. "); return _mapAttrs[name]; }
    const Attribute &getAttr(const std::string &name) const                  {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("Net::getAttr: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    void             setAttr(const std::string &name, const Attribute &attr) {_mapAttrs[name] = attr; }
    Attribute &      operator[](const std::string &name)                     {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("Net::operator[]: Attribute ") + name + " not found. "); return _mapAttrs[name]; }
    const Attribute &operator[](const std::string &name) const               {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("Net::operator[]: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }

    AttrHash &      attrs()       {return _mapAttrs; }
    const AttrHash &attrs() const {return _mapAttrs; }
    
    size_t size() const {return _nodes.size(); }
    size_t sizeSinks() const {ASSERT(_nodes.size() > 0, std::string("Net::sizeSinks: Empty net. ")); return _nodes.size() - 1; }
    
    bool operator == (const Net &net); 

}; 

typedef Net Edge; 
typedef Net HyperEdge; 


//=================================================================================
// HyperGraph
//=================================================================================

class HyperGraph
{
private: 
    std::string                                       _name; 
    std::unordered_map<std::string, Vertex>           _vertices; 
    std::unordered_map<std::string, std::vector<Net>> _netsOut; 
    std::unordered_map<std::string, std::vector<Net>> _netsIn; 
    AttrHash                                          _mapAttrs; 

public: 
    HyperGraph()                        : _name(),     _vertices(), _netsOut(), _netsIn(), _mapAttrs() {}
    HyperGraph(const std::string &name) : _name(name), _vertices(), _netsOut(), _netsIn(), _mapAttrs() {_mapAttrs["__name__"] = _name; this->loadFrom(name); }
    HyperGraph(const HyperGraph &graph) : _name(graph._name), _vertices(graph._vertices), _netsOut(graph._netsOut), _netsIn(graph._netsIn), _mapAttrs(graph._mapAttrs) {}
    HyperGraph(HyperGraph &&graph)      : _name(graph._name), _vertices(graph._vertices), _netsOut(graph._netsOut), _netsIn(graph._netsIn), _mapAttrs(graph._mapAttrs) {}

    const HyperGraph &operator = (const HyperGraph &graph) {_name = graph._name; _vertices = graph._vertices; _netsOut = graph._netsOut; _netsIn = graph._netsIn; _mapAttrs = graph._mapAttrs; return *this; }
    const HyperGraph &operator = (HyperGraph &&graph)      {_name = graph._name; _vertices = graph._vertices; _netsOut = graph._netsOut; _netsIn = graph._netsIn; _mapAttrs = graph._mapAttrs; return *this; }

    std::vector<std::string> vertexNames() const; 
    
    const std::string &name()                          const {return _name; }
    void              setName(const std::string &name)       {_name = name; _mapAttrs["__name__"] = _name; }

    std::unordered_map<std::string, Vertex>                 &vertices()         {return _vertices; }
    const std::unordered_map<std::string, Vertex>           &vertices()   const {return _vertices; }
    const std::unordered_map<std::string, std::vector<Net>> &netsOut()    const {return _netsOut; }
    const std::unordered_map<std::string, std::vector<Net>> &netsIn()     const {return _netsIn; }
    const std::unordered_map<std::string, std::vector<Net>> &edgesOut()   const {return _netsOut; }
    const std::unordered_map<std::string, std::vector<Net>> &edgesIn()    const {return _netsIn; }
    AttrHash                                                &attributes()       {return _mapAttrs; }
    const AttrHash                                          &attributes() const {return _mapAttrs; }
    AttrHash                                                &attrs()            {return _mapAttrs; }
    const AttrHash                                          &attrs()      const {return _mapAttrs; }
    Attribute &attribute(const std::string &name)             {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("HyperGraph::attribute: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    const Attribute &attribute(const std::string &name) const {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("HyperGraph::attribute: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    Attribute &attr(const std::string &name)                  {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("HyperGraph::attr: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    const Attribute &attr(const std::string &name)      const {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("HyperGraph::attr: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    
    Vertex       &vertex(const std::string &name)            {ASSERT(_vertices.find(name) != _vertices.end(), std::string("HyperGraph::vertex: Vertex ") + name + " not found. "); return _vertices.find(name)->second; }
    const Vertex &vertex(const std::string &name)      const {ASSERT(_vertices.find(name) != _vertices.end(), std::string("HyperGraph::vertex: Vertex ") + name + " not found. "); return _vertices.find(name)->second; }
    const Vertex &operator[] (const std::string &name) const {return vertex(name); }
    
    const std::vector<Net> &netsOut(const std::string &name)  const {ASSERT(_netsOut.find(name) != _netsOut.end(), std::string("HyperGraph::netsOut: Nets of vertex ") + name + " not found. "); return _netsOut.find(name)->second; }
    const std::vector<Net> &netsIn(const std::string &name)   const {ASSERT(_netsIn.find(name) != _netsIn.end(), std::string("HyperGraph::netsIn: Nets of vertex ") + name + " not found. "); return _netsIn.find(name)->second; }
    const std::vector<Net> &edgesOut(const std::string &name) const {ASSERT(_netsOut.find(name) != _netsOut.end(), std::string("HyperGraph::edgesOut: Nets of vertex ") + name + " not found. "); return _netsOut.find(name)->second; }
    const std::vector<Net> &edgesIn(const std::string &name)  const {ASSERT(_netsIn.find(name) != _netsIn.end(), std::string("HyperGraph::edgesIn: Nets of vertex ") + name + " not found. "); return _netsIn.find(name)->second; }
    
    size_t nVertices() const {return _vertices.size(); }
    size_t nNets()     const {size_t count = 0; for(const auto &vertex: _vertices) {count += _netsOut.find(vertex.first)->second.size(); } return count; }
    size_t nEdges()    const {return nNets(); }

    bool empty() const {return _vertices.empty(); }
    void clearVertices() {_vertices = std::unordered_map<std::string, Vertex>(); _netsOut = std::unordered_map<std::string, std::vector<Net>>(); _netsIn = std::unordered_map<std::string, std::vector<Net>>(); }

    void addVertex(const Vertex &vertex) {if(_vertices.find(vertex.name()) != _vertices.end()){WARN << "HyperGraph::addVertex: WARN: duplicated vertex: " << vertex.name(); return; } _vertices[vertex.name()] = vertex; _netsOut[vertex.name()] = std::vector<Net>(); _netsIn[vertex.name()] = std::vector<Net>(); }
    void addNet(const Net &edge)         {ASSERT(_vertices.find(edge.source()) != _vertices.end(), std::string("HyperGraph::addNet: Vertex ") + edge.source() + " not found. "); _netsOut[edge.source()].push_back(edge); for(size_t idx = 0; idx < edge.sizeSinks(); idx++){ASSERT(_vertices.find(edge.sink(idx)) != _vertices.end(), std::string("HyperGraph::addNet: Vertex ") + edge.sink(idx) + " not found. ");  _netsIn[edge.sink(idx)].push_back(edge); }}
    void addEdge(const Net &edge)        {addNet(edge); }
    void addAttr(const std::string &name, const Attribute &attr)      {_mapAttrs[name] = attr; }
    void addAttribute(const std::string &name, const Attribute &attr) {_mapAttrs[name] = attr; }
    void setAttr(const std::string &name, const Attribute &attr)      {_mapAttrs[name] = attr; }
    void setAttribute(const std::string &name, const Attribute &attr) {_mapAttrs[name] = attr; }
    HyperGraph &operator << (const Vertex &vertex)   {addVertex(vertex); return *this; }
    HyperGraph &operator << (const Net &edge)        {addNet(edge);      return *this; }
    
    void delVertex(const std::string &name); 
    
    void fromString(const std::string &content); 
    std::string toString() const; 
    std::string str()      const {return toString(); }

    bool loadFrom(const std::string &filename); 
    bool dumpTo(const std::string &filename) const; 
    bool load(const std::string &filename)       {return loadFrom(filename); }
    bool dump(const std::string &filename) const {return dumpTo(filename); }

    std::unordered_map<std::string, std::vector<std::vector<std::string>>> reachableVertices(const std::string &from, const std::function<bool(const Vertex &)> &funcPath, const std::function<bool(const Vertex &)> &funcSink) const; 
    std::unordered_set<std::string> reachableFrom(const std::string &from, const std::function<bool(const Vertex &)> &funcPath, const std::function<bool(const Vertex &)> &funcSink) const; 

    std::pair<std::vector<std::string>, bool> findPath(const std::string &from, const std::string &to) const;
    std::pair<std::vector<std::string>, bool> findPath(const std::string &from, const std::string &to, const std::function<bool(const Vertex &)> &funcPath) const;
    std::vector<std::vector<std::string>> findPaths(const std::string &from, const std::string &to) const; 
    std::vector<std::vector<std::string>> findPaths(const std::string &from, const std::string &to, const std::function<bool(const Vertex &)> &funcPath) const; 

}; 

std::ostream &operator << (std::ostream &out, const HyperGraph &graph); 

typedef HyperGraph Graph; 

}



#endif
