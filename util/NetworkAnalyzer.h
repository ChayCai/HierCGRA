#ifndef __FASTCGRA_NETWORKANALYZER__
#define __FASTCGRA_NETWORKANALYZER__

#include "common/Common.h"
#include "common/HyperGraph.h"

namespace FastCGRA
{

class NetworkAnalyzer
{
private: 
    std::unordered_map<std::string, std::unordered_set<std::string>> _fus; 
    Graph _RRG; 
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> _linksTable; 
    std::unordered_map<std::string, std::vector<std::string>> _pathsTable; 

public: 
    NetworkAnalyzer()                                                                                             : _fus()                {} 
    NetworkAnalyzer(const std::string &filename)                                                                  : _fus(parse(filename)) {} 
    NetworkAnalyzer(const std::unordered_map<std::string, std::unordered_set<std::string>> &fus)                  : _fus(fus)             {}
    NetworkAnalyzer(std::unordered_map<std::string, std::unordered_set<std::string>> &&fus)                       : _fus(fus)             {}
    NetworkAnalyzer(const Graph &rrg)                                                                             : _fus()                {genFUs(rrg); analyze(rrg); } 
    NetworkAnalyzer(const std::string &filename, const Graph &rrg)                                                : _fus(parse(filename)) {analyze(rrg); } 
    NetworkAnalyzer(const std::unordered_map<std::string, std::unordered_set<std::string>> &fus, const Graph &rrg): _fus(fus)             {analyze(rrg); }
    NetworkAnalyzer(std::unordered_map<std::string, std::unordered_set<std::string>> &&fus, const Graph &rrg)     : _fus(fus)             {analyze(rrg); }
    NetworkAnalyzer(const NetworkAnalyzer &analyzer)                                                              : _fus(analyzer._fus), _RRG(analyzer._RRG), _linksTable(analyzer._linksTable), _pathsTable(analyzer._pathsTable)   {}
    NetworkAnalyzer(NetworkAnalyzer &&analyzer)                                                                   : _fus(std::move(analyzer._fus)), _RRG(std::move(analyzer._RRG)), _linksTable(std::move(analyzer._linksTable)), _pathsTable(std::move(analyzer._pathsTable))   {}
    
    const NetworkAnalyzer &operator = (const NetworkAnalyzer &analyzer) {_fus = analyzer._fus; _RRG = analyzer._RRG; _linksTable = analyzer._linksTable; _pathsTable = analyzer._pathsTable; return *this; } 
    const NetworkAnalyzer &operator = (NetworkAnalyzer &&analyzer)      {_fus = std::move(analyzer._fus); _RRG = std::move(analyzer._RRG); _linksTable = std::move(analyzer._linksTable); _pathsTable = std::move(analyzer._pathsTable); return *this; } 

    std::unordered_map<std::string, std::unordered_set<std::string>> &fus() {return _fus; }
    const std::unordered_map<std::string, std::unordered_set<std::string>> &fus() const {return _fus; }
    Graph &RRG() {return _RRG; }
    const Graph &RRG() const {return _RRG; }
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &linksTable() {return _linksTable; }
    const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> &linksTable() const {return _linksTable; }
    std::unordered_map<std::string, std::vector<std::string>> &pathsTable() {return _pathsTable; }
    const std::unordered_map<std::string, std::vector<std::string>> &pathsTable() const {return _pathsTable; }
    const std::unordered_map<std::string, std::vector<std::string>> &operator () (const std::string &name) const {assert(_linksTable.find(name) != _linksTable.end()); return _linksTable.find(name)->second; }
    const std::vector<std::string> &operator () (const std::string &from, const std::string &to) const {assert(_linksTable.find(from) != _linksTable.end() && _linksTable.find(from)->second.find(to) != _linksTable.find(from)->second.end()); return _linksTable.find(from)->second.find(to)->second; }
    const std::vector<std::string> &operator [] (const std::string &path) const {assert(_pathsTable.find(path) != _pathsTable.end()); return _pathsTable.find(path)->second; }

    bool analyze(const Graph &rrg); 
    bool analyze() {return analyze(_RRG); }
    bool dumpAnalysis(const std::string &graphFilename, const std::string &linksFilename, const std::string &pathsFilename) const; 
    bool loadAnalysis(const std::string &graphFilename, const std::string &linksFilename, const std::string &pathsFilename); 

    void genFUs(const Graph &rrg); 
    void genFUs() {genFUs(_RRG); }

    static std::unordered_map<std::string, std::unordered_set<std::string>> parse(const std::string &filename); 
    static bool compatible(const std::vector<std::string> &path1, const std::vector<std::string> &path2); 
}; 

class NetworkAnalyzerLegacy
{
private: 
    std::unordered_map<std::string, std::unordered_set<std::string>> _fus; 
    Graph _RRG; 
    std::unordered_map<std::string, std::unordered_set<std::string>> _linksTable; 

public: 
    NetworkAnalyzerLegacy()                                                                                             : _fus()                {} 
    NetworkAnalyzerLegacy(const std::string &filename)                                                                  : _fus(parse(filename)) {} 
    NetworkAnalyzerLegacy(const std::unordered_map<std::string, std::unordered_set<std::string>> &fus)                  : _fus(fus)             {}
    NetworkAnalyzerLegacy(std::unordered_map<std::string, std::unordered_set<std::string>> &&fus)                       : _fus(fus)             {}
    NetworkAnalyzerLegacy(const Graph &rrg)                                                                             : _fus()                {genFUs(rrg); analyze(rrg); } 
    NetworkAnalyzerLegacy(const std::string &filename, const Graph &rrg)                                                : _fus(parse(filename)) {analyze(rrg); } 
    NetworkAnalyzerLegacy(const std::unordered_map<std::string, std::unordered_set<std::string>> &fus, const Graph &rrg): _fus(fus)             {analyze(rrg); }
    NetworkAnalyzerLegacy(std::unordered_map<std::string, std::unordered_set<std::string>> &&fus, const Graph &rrg)     : _fus(fus)             {analyze(rrg); }
    NetworkAnalyzerLegacy(const NetworkAnalyzerLegacy &analyzer)                                                        : _fus(analyzer._fus), _RRG(analyzer._RRG), _linksTable(analyzer._linksTable)   {}
    NetworkAnalyzerLegacy(NetworkAnalyzerLegacy &&analyzer)                                                             : _fus(std::move(analyzer._fus)), _RRG(std::move(analyzer._RRG)), _linksTable(std::move(analyzer._linksTable))   {}
    
    const NetworkAnalyzerLegacy &operator = (const NetworkAnalyzerLegacy &analyzer) {_fus = analyzer._fus; _RRG = analyzer._RRG; _linksTable = analyzer._linksTable; return *this; } 
    const NetworkAnalyzerLegacy &operator = (NetworkAnalyzerLegacy &&analyzer)      {_fus = std::move(analyzer._fus); _RRG = std::move(analyzer._RRG); _linksTable = std::move(analyzer._linksTable); return *this; } 

    std::unordered_map<std::string, std::unordered_set<std::string>> &fus() {return _fus; }
    const std::unordered_map<std::string, std::unordered_set<std::string>> &fus() const {return _fus; }
    Graph &RRG() {return _RRG; }
    const Graph &RRG() const {return _RRG; }
    std::unordered_map<std::string, std::unordered_set<std::string>> &linksTable() {return _linksTable; }
    const std::unordered_map<std::string, std::unordered_set<std::string>> &linksTable() const {return _linksTable; }
    const std::unordered_set<std::string> &operator () (const std::string &name) const {assert(_linksTable.find(name) != _linksTable.end()); return _linksTable.find(name)->second; }
    bool operator () (const std::string &from, const std::string &to) const {if(_linksTable.find(from) == _linksTable.end() || _linksTable.find(from)->second.find(to) == _linksTable.find(from)->second.end()) {return false; } return true; }
    const std::unordered_set<std::string> &operator [] (const std::string &name) const {assert(_linksTable.find(name) != _linksTable.end()); return _linksTable.find(name)->second; }

    bool analyze(const Graph &rrg); 
    bool analyze() {return analyze(_RRG); }
    bool dumpAnalysis(const std::string &graphFilename, const std::string &linksFilename) const; 
    bool loadAnalysis(const std::string &graphFilename, const std::string &linksFilename); 
    
    void genFUs(const Graph &rrg); 
    void genFUs() {genFUs(_RRG); }

    static std::string _graphFilename; 
    static std::string _linksFilename; 
    static void setDefault(const std::string &graphFilename, const std::string &linksFilename) {_graphFilename = graphFilename; _linksFilename = linksFilename; }
    static std::unordered_map<std::string, std::unordered_set<std::string>> parse(const std::string &filename); 
}; 

class NetworkAnalyzerLegacySilent
{
private: 
    std::unordered_map<std::string, std::unordered_set<std::string>> _fus; 
    Graph _RRG; 
    std::unordered_map<std::string, std::unordered_set<std::string>> _linksTable; 

public: 
    NetworkAnalyzerLegacySilent()                                                                                             : _fus()                {} 
    NetworkAnalyzerLegacySilent(const std::string &filename)                                                                  : _fus(parse(filename)) {} 
    NetworkAnalyzerLegacySilent(const std::unordered_map<std::string, std::unordered_set<std::string>> &fus)                  : _fus(fus)             {}
    NetworkAnalyzerLegacySilent(std::unordered_map<std::string, std::unordered_set<std::string>> &&fus)                       : _fus(fus)             {}
    NetworkAnalyzerLegacySilent(const Graph &rrg)                                                                             : _fus()                {genFUs(rrg); analyze(rrg); } 
    NetworkAnalyzerLegacySilent(const std::string &filename, const Graph &rrg)                                                : _fus(parse(filename)) {analyze(rrg); } 
    NetworkAnalyzerLegacySilent(const std::unordered_map<std::string, std::unordered_set<std::string>> &fus, const Graph &rrg): _fus(fus)             {analyze(rrg); }
    NetworkAnalyzerLegacySilent(std::unordered_map<std::string, std::unordered_set<std::string>> &&fus, const Graph &rrg)     : _fus(fus)             {analyze(rrg); }
    NetworkAnalyzerLegacySilent(const NetworkAnalyzerLegacySilent &analyzer)                                                        : _fus(analyzer._fus), _RRG(analyzer._RRG), _linksTable(analyzer._linksTable)   {}
    NetworkAnalyzerLegacySilent(NetworkAnalyzerLegacySilent &&analyzer)                                                             : _fus(std::move(analyzer._fus)), _RRG(std::move(analyzer._RRG)), _linksTable(std::move(analyzer._linksTable))   {}
    
    const NetworkAnalyzerLegacySilent &operator = (const NetworkAnalyzerLegacySilent &analyzer) {_fus = analyzer._fus; _RRG = analyzer._RRG; _linksTable = analyzer._linksTable; return *this; } 
    const NetworkAnalyzerLegacySilent &operator = (NetworkAnalyzerLegacySilent &&analyzer)      {_fus = std::move(analyzer._fus); _RRG = std::move(analyzer._RRG); _linksTable = std::move(analyzer._linksTable); return *this; } 

    std::unordered_map<std::string, std::unordered_set<std::string>> &fus() {return _fus; }
    const std::unordered_map<std::string, std::unordered_set<std::string>> &fus() const {return _fus; }
    Graph &RRG() {return _RRG; }
    const Graph &RRG() const {return _RRG; }
    std::unordered_map<std::string, std::unordered_set<std::string>> &linksTable() {return _linksTable; }
    const std::unordered_map<std::string, std::unordered_set<std::string>> &linksTable() const {return _linksTable; }
    const std::unordered_set<std::string> &operator () (const std::string &name) const {assert(_linksTable.find(name) != _linksTable.end()); return _linksTable.find(name)->second; }
    bool operator () (const std::string &from, const std::string &to) const {if(_linksTable.find(from) == _linksTable.end() || _linksTable.find(from)->second.find(to) == _linksTable.find(from)->second.end()) {return false; } return true; }
    const std::unordered_set<std::string> &operator [] (const std::string &name) const {assert(_linksTable.find(name) != _linksTable.end()); return _linksTable.find(name)->second; }

    bool analyze(const Graph &rrg); 
    bool analyze() {return analyze(_RRG); }
    bool dumpAnalysis(const std::string &graphFilename, const std::string &linksFilename) const; 
    bool loadAnalysis(const std::string &graphFilename, const std::string &linksFilename); 
    
    void genFUs(const Graph &rrg); 
    void genFUs() {genFUs(_RRG); }

    static std::string _graphFilename; 
    static std::string _linksFilename; 
    static void setDefault(const std::string &graphFilename, const std::string &linksFilename) {_graphFilename = graphFilename; _linksFilename = linksFilename; }
    static std::unordered_map<std::string, std::unordered_set<std::string>> parse(const std::string &filename); 
}; 

}

#endif
