#ifndef __GENVERIlOG__
#define __GENVERILOG__


#include "../../../common/Common.h"
#include "../../../common/HyperGraph.h"
#include "../../../util/NOrderValidator.h"
#include "../../../util/NetworkAnalyzer.h"
#include "../../../util/GraphSort.h"
#include "../../../util/Utils.h"
#include "../json/json.h"

namespace FastCGRA
{

namespace GenVerilog
{

static std::vector<std::string> SpecialPorts = {"clk", "reset", "config_sig", "config_clk", "config_reset", "config_in", "config_out"};
static std::vector<std::string> ModuleTypes = {"SWITCH_CELL", "CONFIG_CELL", "FUNC_CELL", "MODULE_CELL"};

class Module
{
private: 
    std::string  _name;
    std::string  _type;
    std::vector<std::string> _inPorts;
    std::vector<std::string> _outPorts;
    std::vector<std::string> _specialPorts;

    std::vector<std::string> _subModuleIns;
    std::vector<std::string> _subModuleNames;
    std::vector<std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>>> _connects;

    std::string _path;
    size_t _configWidth;

public:
    Module() { _configWidth = 0; };
    Module( const std::string &path );
    Module( const Module &module ) : _name(module._name), _type(module._type), _inPorts(module._inPorts), _outPorts(module._outPorts), _specialPorts(module._specialPorts), _subModuleIns(module._subModuleIns), _subModuleNames(module._subModuleNames), _connects(module._connects), _path(module._path), _configWidth(module._configWidth) {};
    
    const std::string &name()   const { return _name; };
    const std::string &type()   const { return _type; };
    const std::vector<std::string> &inPorts() const { return _inPorts; };
    const std::vector<std::string> &outPorts() const { return _outPorts; };
    const std::vector<std::string> &specialPorts() const { return _specialPorts; };
    const std::vector<std::string> &subModuleIns() const { return _subModuleIns; };
    const std::vector<std::string> &subModuleNames() const { return _subModuleNames; };
    const std::vector<std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>>> &connects() const { return _connects; };
    const std::string &path() const { return _path; };
    const size_t &configWidth() const { return _configWidth; };

    void setName( const std::string &name ) { _name = name; };
    void setType( const std::string &type ) { _type = type; };
    void setPath( const std::string &path ) { _path = path; };
    void setconfigWidth( const size_t &width ) { _configWidth = width; };
    void addinPorts( const std::string &port ) { _inPorts.push_back(port); };
    void addoutPorts( const std::string &port ) { _outPorts.push_back(port); };
    void addspecialPorts( const std::string &port ) { _specialPorts.push_back(port); };
    void addsubModules( const std::string &ins, const std::string &name ) { _subModuleIns.push_back(ins); _subModuleNames.push_back(name); };
    void connect(const std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>> &link);
};

class Core
{
private: 
    Module _top;
    std::vector<std::string> _subModuleNames;
    std::vector<Module> _subModules;
    std::vector<std::string> _subModuleType;
    std::unordered_map<std::string, std::string> _module2lib;

    Json::Value _src;
    std::unordered_map<std::string, std::string> _modulelibSrc;
public:
    Core() = delete;
    Core(const std::string &srcFile, const std::string &libFile);
    Module &top() {return _top;};

    bool checkEqua(const Module &mod, const std::string &name);
    void core2RTL(const std::string &path);

};

Module parseRTL(const std::string &path);

void dumpRTL(const Module &module, const std::string &path, const std::unordered_map<std::string, Module> &subMods, const Module &configCell);

Module genSWModule(const size_t &inNum, const size_t &outNum, const std::string &name, const Module &configCell, const std::string &path);

Module genModule(const Json::Value &value, const std::string &name, const std::string &type, const std::unordered_map<std::string, Module> &subMods);

}

}

#endif