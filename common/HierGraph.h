#ifndef __FASTCGRA_HIERGRAPH__
#define __FASTCGRA_HIERGRAPH__

#include "Common.h"
#include "Logger.h"
#include "HyperGraph.h"

namespace FastCGRA
{
class HierGraph;
class HierGraph
{
private: 
    std::string                       _name; 
    std::map<std::string, HierGraph>  _subgraphs; 
    std::map<std::string, HyperGraph> _elements; 
    std::unordered_map<std::string, std::vector<Net>> _netsOut; 
    std::unordered_map<std::string, std::vector<Net>> _netsIn; 
    AttrHash                                          _mapAttrs; 

public: 
    HierGraph()                        : _name(), _subgraphs(), _elements(), _netsOut(), _netsIn(), _mapAttrs() {}
    HierGraph(const std::string &name) : _name(name), _subgraphs(), _elements(), _netsOut(), _netsIn(), _mapAttrs() {_mapAttrs["__name__"] = _name; }
    HierGraph(const HierGraph &graph)  : _name(graph._name), _subgraphs(graph._subgraphs), _elements(graph._elements), _netsOut(graph._netsOut), _netsIn(graph._netsIn), _mapAttrs(graph._mapAttrs) {}
    HierGraph(HierGraph &&graph)       : _name(graph._name), _subgraphs(graph._subgraphs), _elements(graph._elements), _netsOut(graph._netsOut), _netsIn(graph._netsIn), _mapAttrs(graph._mapAttrs) {}
    
    const HierGraph &operator = (const HierGraph &graph) {_name = graph._name; _subgraphs = graph._subgraphs; _elements = graph._elements; _netsOut = graph._netsOut; _netsIn = graph._netsIn; _mapAttrs = graph._mapAttrs; return *this; }
    const HierGraph &operator = (HierGraph &&graph)      {_name = graph._name; _subgraphs = graph._subgraphs; _elements = graph._elements; _netsOut = graph._netsOut; _netsIn = graph._netsIn; _mapAttrs = graph._mapAttrs; return *this; }

    std::vector<std::string> subgraphNames() const; 
    std::vector<std::string> elementNames() const; 
    
    const std::string &name()                          const {return _name; }
    void              setName(const std::string &name)       {_name = name; _mapAttrs["__name__"] = _name; }

    std::map<std::string, HierGraph>                        &subgraphs()        {return _subgraphs; }
    const std::map<std::string, HierGraph>                  &subgraphs()  const {return _subgraphs; }
    std::map<std::string, HyperGraph>                       &elements()         {return _elements; }
    const std::map<std::string, HyperGraph>                 &elements()   const {return _elements; }
    const std::unordered_map<std::string, std::vector<Net>> &netsOut()    const {return _netsOut; }
    const std::unordered_map<std::string, std::vector<Net>> &netsIn()     const {return _netsIn; }
    const std::unordered_map<std::string, std::vector<Net>> &edgesOut()   const {return _netsOut; }
    const std::unordered_map<std::string, std::vector<Net>> &edgesIn()    const {return _netsIn; }
    AttrHash                                                &attributes()       {return _mapAttrs; }
    const AttrHash                                          &attributes() const {return _mapAttrs; }
    AttrHash                                                &attrs()            {return _mapAttrs; }
    const AttrHash                                          &attrs()      const {return _mapAttrs; }
    Attribute &attribute(const std::string &name)             {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("HierGraph::attribute: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    const Attribute &attribute(const std::string &name) const {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("HierGraph::attribute: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    Attribute &attr(const std::string &name)                  {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("HierGraph::attr: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    const Attribute &attr(const std::string &name)      const {ASSERT(_mapAttrs.find(name) != _mapAttrs.end(), std::string("HierGraph::attr: Attribute ") + name + " not found. "); return _mapAttrs.find(name)->second; }
    
    HierGraph       &subgraph(const std::string name)         {ASSERT(_subgraphs.find(name) != _subgraphs.end(), std::string("HierGraph::subgraph: HierGraph ") + name + " not found. "); return _subgraphs.find(name)->second; }
    const HierGraph &subgraph(const std::string name)   const {ASSERT(_subgraphs.find(name) != _subgraphs.end(), std::string("HierGraph::subgraph: HierGraph ") + name + " not found. "); return _subgraphs.find(name)->second; }
    HyperGraph       &element(const std::string name)         {ASSERT(_elements.find(name)  != _elements.end(),  std::string("HierGraph::element: HyperGraph ") + name + " not found. "); return _elements.find(name)->second; }
    const HyperGraph &element(const std::string name)   const {ASSERT(_elements.find(name)  != _elements.end(),  std::string("HierGraph::element: HyperGraph ") + name + " not found. "); return _elements.find(name)->second; }
    
    const std::vector<Net> &netsOut(const std::string &name)  const {ASSERT(_netsOut.find(name) != _netsOut.end(), std::string("HierGraph::netsOut: Nets of vertex ") + name + " not found. ");  return _netsOut.find(name)->second; }
    const std::vector<Net> &netsIn(const std::string &name)   const {ASSERT(_netsIn.find(name)  != _netsIn.end(),  std::string("HierGraph::netsIn: Nets of vertex ") + name + " not found. ");   return _netsIn.find(name)->second; }
    const std::vector<Net> &edgesOut(const std::string &name) const {ASSERT(_netsOut.find(name) != _netsOut.end(), std::string("HierGraph::edgesOut: Nets of vertex ") + name + " not found. "); return _netsOut.find(name)->second; }
    const std::vector<Net> &edgesIn(const std::string &name)  const {ASSERT(_netsIn.find(name)  != _netsIn.end(),  std::string("HierGraph::edgesIn: Nets of vertex ") + name + " not found. ");  return _netsIn.find(name)->second; }
    
    size_t nSubgraphs() const {return _subgraphs.size(); }
    size_t nNets()      const {size_t count = 0; for(const auto &subgraph: _subgraphs) {count += _netsOut.find(subgraph.first)->second.size(); } for(const auto &element: _elements) {count += _netsOut.find(element.first)->second.size(); } return count; }
    size_t nEdges()     const {return nNets(); }

    bool empty() const {return _subgraphs.empty() && _elements.empty(); }
    void clear() {_subgraphs = std::map<std::string, HierGraph>(); _elements = std::map<std::string, HyperGraph>(); _netsOut = std::unordered_map<std::string, std::vector<Net>>(); _netsIn = std::unordered_map<std::string, std::vector<Net>>(); _mapAttrs = AttrHash(); }

    void addSubgraph(const HierGraph &subgraph) {ASSERT(_elements.find(subgraph.name()) == _elements.end(), "HierGraph::addSubgraph: There is an element with the same name. "); if(_subgraphs.find(subgraph.name()) != _subgraphs.end()){WARN << "HierGraph::addSubgraph: WARN: duplicated subgraph: " << subgraph.name(); return; } _subgraphs[subgraph.name()] = subgraph; _netsOut[subgraph.name()] = std::vector<Net>(); _netsIn[subgraph.name()] = std::vector<Net>(); }
    void addElement(const HyperGraph &element)  {ASSERT(_subgraphs.find(element.name()) == _subgraphs.end(), "HierGraph::addElement: There is an subgraph with the same name. "); if(_elements.find(element.name()) != _elements.end()){WARN << "HierGraph::addElement: WARN: duplicated element: " << element.name(); return; } _elements[element.name()] = element; _netsOut[element.name()] = std::vector<Net>(); _netsIn[element.name()] = std::vector<Net>(); }
    void addNet(const Net &edge)                {ASSERT(_subgraphs.find(edge.source()) != _subgraphs.end() || _elements.find(edge.source()) != _elements.end(), std::string("HierGraph::addNet: Subgraph / Element ") + edge.source() + " not found. "); _netsOut[edge.source()].push_back(edge); for(size_t idx = 0; idx < edge.sizeSinks(); idx++){ASSERT(_subgraphs.find(edge.source()) != _subgraphs.end() || _elements.find(edge.source()) != _elements.end(), std::string("HierGraph::addNet: Subgraph / Element ") + edge.sink(idx) + " not found. ");  _netsIn[edge.sink(idx)].push_back(edge); }}
    void addEdge(const Net &edge)               {addNet(edge); }
    void addAttr(const std::string &name, const Attribute &attr)      {_mapAttrs[name] = attr; }
    void addAttribute(const std::string &name, const Attribute &attr) {_mapAttrs[name] = attr; }
    void setAttr(const std::string &name, const Attribute &attr)      {_mapAttrs[name] = attr; }
    void setAttribute(const std::string &name, const Attribute &attr) {_mapAttrs[name] = attr; }
    HierGraph &operator << (const HierGraph &subgraph) {addSubgraph(subgraph); return *this; }
    HierGraph &operator << (const HyperGraph &element) {addElement(element);   return *this; }
    HierGraph &operator << (const Net &edge)           {addNet(edge);          return *this; }
    
    void delSubgraph(const std::string &name); 
    void delElement(const std::string &name); 

    void fromString(const std::string &content); 
    std::string toString() const; 
    std::string str()      const {return toString(); }

    bool loadFrom(const std::string &filename); 
    bool dumpTo(const std::string &filename) const; 
    bool load(const std::string &filename)       {return loadFrom(filename); }
    bool dump(const std::string &filename) const {return dumpTo(filename); }

}; 
}

#endif 