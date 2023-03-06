#include "./GenVerilog.h"

using namespace std;

namespace FastCGRA
{

namespace GenVerilog
{

static bool findIn(const std::string &name, const std::vector<std::string> &vec){
    for(auto iter = vec.begin(); iter != vec.end(); iter++){
        if(*iter == name){
            return true;
        }
    }
    return false;
}

static size_t findId(const std::string &name, const std::vector<std::string> &vec){
    for(size_t x = 0; x < vec.size(); x++){
        if(vec[x] == name){
            return x;
        }
    }
    return vec.size();
}

static size_t getBitWidth(const size_t &number){
    ASSERT(number >= 1, "GenVerilog::getBitWidth: wrong number " + to_string(number));
    size_t result = 0;
    size_t numberCompare = 1;
    while(numberCompare < number){
        numberCompare *= 2;
        result += 1;
    }
    if(result == 0){
        result = 1;
    }
    return result;
};

static void convertRTL(const std::string &from, const std::string &to, const std::string &name){
    if(from == to){
        return;
    }
    ifstream fin(from); 
    ofstream fout(to);
    if(!fin)
    {
        cerr << "Common::copyfile WARN: cannot open the file:" << from; 
        return ; 
    }

    bool isModuleName = false;
    string buf;
    string name1;
    while(getline(fin, buf)){
        if(buf.find("// Module Name:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() <= 1){
                ERROR << "GenVerilog::parseRTL: parseModule Name error" << from; 
            }
            name1 = removeSpace(removeLineBreak(tmp[1]));
            fout << "// Module Name: " << name << endl;
        } else if(!isModuleName && buf.find("module") != string::npos && buf.find(name1) != string::npos){
            fout << buf.replace(buf.find(name1), name1.length(), name) << endl;
        } else {
            fout << buf << endl;
        }
    }
    fin.close();
    fout.close();
};

Module::Module(const std::string &path)
{
    ifstream fin(path); 
    if(!fin)
    {
        ERROR << "GenVerilog::parseRTL: WARN: cannot open the file:" << path; 
        return ; 
    }

    string buf;
    while(getline(fin, buf)){
        if(buf.find("// Module Name:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() <= 1){
                ERROR << "GenVerilog::parseRTL: parseModule Name error" << path; 
            }
            _name = removeSpace(removeLineBreak(tmp[1]));
        }
        if(buf.find("// Module Type:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() <= 1){
                ERROR << "GenVerilog::parseRTL: parseModule Type error" << path; 
            }
            string type = removeSpace(removeLineBreak(tmp[1]));
            if(!findIn(type, ModuleTypes)){
                ERROR << "GenVerilog::parseRTL: parseModule Type error" << path; 
            }
            _type = type;
        }
        if(buf.find("// Include Module:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string mods = removeLineBreak(tmp[1]);
                size_t count = 0;
                for(const auto &mod: split(mods, " ")){
                    if(mod.empty()){
                        continue;
                    }
                    _subModuleIns.push_back("tmp" + to_string(count++));
                    _subModuleNames.push_back(mod);
                }
            }
        }
        if(buf.find("// Input Ports:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string ports = removeLineBreak(tmp[1]);
                for(const auto &port: split(ports, " ")){
                    if(port.empty()){
                        continue;
                    }
                    _inPorts.push_back(port);
                }
            }
        }
        if(buf.find("// Output Ports:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string ports = removeLineBreak(tmp[1]);
                for(const auto &port: split(ports, " ")){
                    if(port.empty()){
                        continue;
                    }
                    _outPorts.push_back(port);
                }
            }
        }
        if(buf.find("// Special Ports:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string ports = removeLineBreak(tmp[1]);
                for(const auto &port: split(ports, " ")){
                    if(port.empty()){
                        continue;
                    }
                    if(!findIn(port, SpecialPorts)){
                        ERROR << "GenVerilog::parseRTL: " << path << " : " << port << " not in SpecialPorts. ";    
                    }
                    _specialPorts.push_back(port);
                }
            }
        }
        if(buf.find("// Config Width:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string numStr = removeSpace(removeLineBreak(tmp[1]));
                if(!numStr.empty()){
                    _configWidth = str2num<size_t>(numStr);            
                }
            }
        }
    }
    _path = path;
}

void Module::connect(const std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>> &link)
{
    _connects.push_back(link);
};

bool Core::checkEqua(const Module &mod, const string &name)
{
    vector<string> inPorts;
    vector<string> outPorts;
    if(mod.type() == "MODULE_CELL"){
        for(const auto &port: _src["Module"][name]["input"]){
            inPorts.push_back(port.asString());
        }
        for(const auto &port: _src["Module"][name]["output"]){
            outPorts.push_back(port.asString());
        }
    }
    for(const auto &port: inPorts){
        if(!findIn(port, mod.inPorts())){
            return false;
        }
    }
    for(const auto &port: outPorts){
        if(!findIn(port, mod.outPorts())){
            return false;
        }
    }
    return true;
};

Core::Core(const std::string &srcFile, const std::string &libFile)
{
    Json::Reader reader;

    ifstream in(srcFile, ios::binary);
    if(!in.is_open()){
        ERROR << "Error opening: " + srcFile;
        return;
    }

    // initial
    reader.parse(in, _src);
    _top.setName("Core");
    _top.setType("MODULE_CELL");

    //ports
    for(const auto &port: _src["Module"]["Core"]["input"]){
        _top.addinPorts(port.asString());
    }
    for(const auto &port: _src["Module"]["Core"]["output"]){
        _top.addoutPorts(port.asString());
    }
    _top.addspecialPorts("clk");
    _top.addspecialPorts("reset");
    _top.addspecialPorts("config_clk");
    _top.addspecialPorts("config_reset");
    _top.addspecialPorts("config_in");
    _top.addspecialPorts("config_out");


    //subModule
    for(const auto &mod: _src["Module"]["Core"]["module"].getMemberNames()){
        _top.addsubModules(mod, _src["Module"]["Core"]["module"][mod].asString());
    }
    for(const auto &mod: _src["Module"]["Core"]["switch"].getMemberNames()){
        _top.addsubModules(mod, _src["Module"]["Core"]["switch"][mod].asString());
    }

    //connections
    for(const auto &link: _src["Module"]["Core"]["connection"]){
        vector<string> item = split( link.asString() , "->");
        vector<string> from = {"", item[0]};
        vector<string> to = {"", item[1]};
        if(item[0].find(".") != string::npos){
            from = split( item[0] , ".");
        }
        if(item[1].find(".") != string::npos){
            to = split( item[1] , ".");
        }
        _top.connect({{from[0], from[1]},{to[0], to[1]}});
    }

    // lib
    unordered_map<string, unordered_map<string, vector<string>>> libMsg = Utils::readConifgFile(libFile);
    for(const auto &item: libMsg){
        for(const auto &mod: item.second){
            _modulelibSrc[mod.first] = mod.second[0];
        }
    }

};

void Core::core2RTL(const std::string &path)
{
    for(const auto &modName: _top.subModuleNames()){
        if(!findIn(modName, _subModuleNames)){
            _subModuleNames.push_back(modName);
            if(_modulelibSrc.find(modName) != _modulelibSrc.end()){
                string path = _modulelibSrc.find(modName)->second;
                _module2lib[modName] = path;
                Module mod(path);
                mod.setName(modName);
                _subModules.push_back(mod);
                ASSERT(checkEqua(mod, modName), "Core::core2RTL: " + modName + " & " + path + " UnEqua")
            } else {
                _module2lib[modName] = "";
                _subModules.push_back(Module());
            }
        }
    }

    vector<string> swithes = _src["Switch"].getMemberNames();
    vector<string> modules = _src["Module"].getMemberNames();
    vector<string> fus = _src["Unit"].getMemberNames();

    Module configcell(_modulelibSrc["ConfigCell"]);
    _subModuleNames.push_back("ConfigCell");
    _module2lib["ConfigCell"] = configcell.path();
    _subModules.push_back(configcell);

    bool isDone = false;
    size_t counter = 0;
    while(!isDone){
        clog << "======= Generating ... =======" << endl;
        clog << _module2lib << endl;

        vector<size_t> toGenList;
        for(size_t idx = 0; idx < _subModuleNames.size(); idx++){
            if(_module2lib[_subModuleNames[idx]].empty()){
                toGenList.push_back(idx);
            }
        }
        isDone = toGenList.empty();
        for(const auto &index: toGenList){
            string moduleName = _subModuleNames[index];
            if(findIn(moduleName, swithes)){
                string libName = path + "/" + moduleName + ".v";
                size_t inNum = _src["Switch"][moduleName]["input"].size();
                size_t outNum = _src["Switch"][moduleName]["output"].size();
                _subModules[index] = genSWModule(inNum, outNum, moduleName, configcell, libName);
                _module2lib[moduleName] = libName;
            } else if (findIn(moduleName, modules)){
                unordered_set<string> submods;
                vector<string> names0 = _src["Module"][moduleName]["element"].getMemberNames();
                vector<string> names1 = _src["Module"][moduleName]["switch"].getMemberNames();
                for(const auto &name: names0){
                    submods.insert(_src["Module"][moduleName]["element"][name].asString());
                }
                for(const auto &name: names1){
                    submods.insert(_src["Module"][moduleName]["switch"][name].asString());
                }
                for(const auto &submodName: submods){
                    if(!findIn(submodName, _subModuleNames)){
                        _subModuleNames.push_back(submodName);
                        if(_modulelibSrc.find(submodName) != _modulelibSrc.end()){
                            string libPath = _modulelibSrc.find(submodName)->second;
                            _module2lib[submodName] = libPath;
                            Module mod = parseRTL(libPath);
                            mod.setName(submodName);
                            _subModules.push_back(mod);
                        } else {
                            _module2lib[submodName] = "";
                            _subModules.push_back(Module());
                        }
                    }
                }
                bool isToGen = true;
                for(const auto &mod: submods){
                    if(_module2lib[mod].empty()){
                        isToGen = false;
                        break;
                    }
                }
                if(isToGen){
                    Json::Value msg = _src["Module"][moduleName];
                    string libPath = path + "/" + moduleName + ".v";
                    unordered_map<string, Module> submodDict;
                    for(size_t idx = 0; idx < _subModuleNames.size(); idx++){
                        string submodName = _subModuleNames[idx];
                        if(submods.find(submodName) != submods.end()){
                            submodDict[submodName] = _subModules[idx];
                        }
                    }
                    Module mod = genModule(msg, moduleName, "MODULE_CELL", submodDict);
                    dumpRTL(mod, libPath, submodDict, configcell);
                    mod.setPath(libPath);
                    _module2lib[moduleName] = libPath;
                    size_t id = findId(moduleName, _subModuleNames); 
                    _subModules[id] = mod;
                }
            } else if (findIn(moduleName, fus)){
                Json::Value msg = _src["Unit"][moduleName];
                string libPath = path + "/" + moduleName + ".v";
                Module mod = genModule(msg, moduleName, "FUNC_CELL", {});
                dumpRTL(mod, libPath, {}, configcell);
                mod.setPath(libPath);
                _module2lib[moduleName] = libPath;
                size_t id = findId(moduleName, _subModuleNames); 
                _subModules[id] = mod;
            }
        }

        if(counter++ == 1024){
            ASSERT(false, "Core::core2RTL GENERATION Failed.")
            isDone = true;
        }
    }

    for(const auto &mod: _subModules){
        string dumpPath = path + "/" + mod.name() + ".v";
        string fromPath = mod.path();
        if(fromPath == dumpPath){
            continue;
        } else {
            convertRTL(fromPath, dumpPath, mod.name());
        }
    }
    string topPath = path + "/" + _top.name() + ".v";

    unordered_map<string, Module> subModulesDict;
    for(size_t idx = 0; idx < _subModuleNames.size(); idx++){
        subModulesDict[_subModuleNames[idx]] = _subModules[idx];
    }
    dumpRTL(_top, topPath, subModulesDict, configcell);

};

Module parseRTL(const std::string &path)
{
    Module module;
    ifstream fin(path); 
    if(!fin)
    {
        ERROR << "GenVerilog::parseRTL: WARN: cannot open the file:" << path; 
        return module; 
    }

    string buf;
    while(getline(fin, buf)){
        if(buf.find("// Module Name:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() <= 1){
                ERROR << "GenVerilog::parseRTL: parseModule Name error" << path; 
            }
            module.setName(removeSpace(removeLineBreak(tmp[1])));
        }
        if(buf.find("// Module Type:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() <= 1){
                ERROR << "GenVerilog::parseRTL: parseModule Type error" << path; 
            }
            string type = removeSpace(removeLineBreak(tmp[1]));
            if(!findIn(type, ModuleTypes)){
                ERROR << "GenVerilog::parseRTL: parseModule Type error" << path; 
            }
            module.setType(type);
        }
        if(buf.find("// Include Module:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string mods = removeLineBreak(tmp[1]);
                size_t count = 0;
                for(const auto &mod: split(mods, " ")){
                    if(mod.empty()){
                        continue;
                    }
                    module.addsubModules("tmp" + to_string(count++), mod);
                }
            }
        }
        if(buf.find("// Input Ports:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string ports = removeLineBreak(tmp[1]);
                for(const auto &port: split(ports, " ")){
                    if(port.empty()){
                        continue;
                    }
                    module.addinPorts(port);
                }
            }
        }
        if(buf.find("// Output Ports:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string ports = removeLineBreak(tmp[1]);
                for(const auto &port: split(ports, " ")){
                    if(port.empty()){
                        continue;
                    }
                    module.addoutPorts(port);
                }
            }
        }
        if(buf.find("// Special Ports:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string ports = removeLineBreak(tmp[1]);
                for(const auto &port: split(ports, " ")){
                    if(port.empty()){
                        continue;
                    }
                    if(!findIn(port, SpecialPorts)){
                        ERROR << "GenVerilog::parseRTL: " << path << " : " << port << " not in SpecialPorts. ";    
                    }
                    module.addspecialPorts(port);
                }
            }
        }
        if(buf.find("// Config Width:") != string::npos){
            vector<string> tmp = split(buf, ":");
            if(tmp.size() > 1){
                string numStr = removeSpace(removeLineBreak(tmp[1]));
                if(!numStr.empty()){
                    module.setconfigWidth(str2num<size_t>(numStr));            
                }
            }
        }
    }
    module.setPath(path);

    // clog << module.name() << endl;
    // clog << module.type() << endl;
    // clog << module.subModuleNames() << endl;
    // clog << module.inPorts() << endl;
    // clog << module.outPorts() << endl;
    // clog << module.specialPorts() << endl;
    // clog << module.path() << endl;
    // clog << module.configWidth() << endl;
    
    return module;
};

void dumpRTL(const Module &module, const std::string &path, const unordered_map<string, Module> &subMods, const Module &configCell)
{
    if(!module.path().empty()){
        copyfile(module.path(), path);
        return;
    }

    ofstream fout(path);
    fout << "// Module Name: " << module.name() << endl;
    fout << "// Module Type: " << module.type() << endl;
    unordered_set<string> includes;
    for(const auto &mod: module.subModuleNames()){
        includes.insert(mod);
    }
    fout << "// Include Module:";
    for(const auto &mod: includes){
        fout << " " << mod;
    }
    fout << endl;
    fout << "// Input Ports:";
    for(const auto &port: module.inPorts()){
        fout << " " << port;
    }
    fout << endl;
    fout << "// Output Ports:";
    for(const auto &port: module.outPorts()){
        fout << " " << port;
    }
    fout << endl;
    fout << "// Special Ports:";
    for(const auto &port: module.specialPorts()){
        fout << " " << port;
    }
    fout << endl;
    fout << "// Config Width: " << endl << endl << endl;

    fout << "module " << module.name() << "(";
    string portStr;
    for(const auto &port: module.specialPorts()){
        portStr += (", " + port);
    }
    for(const auto &port: module.inPorts()){
        portStr += (", " + port);
    }
    for(const auto &port: module.outPorts()){
        portStr += (", " + port);
    }
    fout << portStr.substr(2, portStr.size() - 2);
    fout << ");" << endl;
    fout << "\t" << "parameter size = 32;" << endl << endl;

    portStr.clear();
    for(const auto &port: module.specialPorts()){
        if(port == "config_out"){
            continue;
        }
        portStr += (", " + port);
    }
    if(portStr.size() > 2){
        fout << "\t" << "input " << portStr.substr(2, portStr.size() - 2) << ";" << endl;
    }
    portStr.clear();
    for(const auto &port: module.specialPorts()){
        if(port != "config_out"){
            continue;
        }
        portStr += (", " + port);
    }
    if(portStr.size() > 2){
        fout << "\t" << "output " << portStr.substr(2, portStr.size() - 2) << ";" << endl;
    }
    fout << "\t" << "input [size-1:0] ";
    portStr.clear();
    for(const auto &port: module.inPorts()){
        portStr += (", " + port);
    }
    fout << portStr.substr(2, portStr.size() - 2) << ";" << endl;
    portStr.clear();
    fout << "\t" << "output [size-1:0] ";
    for(const auto &port: module.outPorts()){
        portStr += (", " + port);
    }
    fout << portStr.substr(2, portStr.size() - 2) << ";" << endl << endl;

    vector<size_t> widths;
    unordered_map<string, string> port2linkStr;
    vector<pair<string, string>> toAssign;
    for(size_t idx = 0; idx < module.subModuleIns().size(); idx++){
        string name = module.subModuleNames()[idx];
        const Module &mod = subMods.find(name)->second;
        if(findIn("config_sig", mod.specialPorts())){
            widths.push_back(mod.configWidth());
        }
    }
    for(size_t idx = 0; idx < widths.size(); idx++){
        fout << "\t" << "wire ";
        if(widths[idx] > 1){
            fout << "[" << widths[idx]-1 << ":0] ";
        }
        fout << "config_sig_" << idx << ";" << endl;
    }

    size_t configWireNum = widths.size();
    for(const auto &modName: module.subModuleNames()){
        if(findIn("config_in", subMods.find(modName)->second.specialPorts())){
            configWireNum++;
        }
    }
    if(configWireNum >= 1){
        configWireNum -= 1;
    }
    for(size_t idx = 0; idx < configWireNum; idx++){
        fout << "\t" << "wire " << "config_wire_" << idx << ";" << endl;
    }
    for(const auto &link: module.connects()){
        bool isTopIn = link.first.first.empty();
        bool isTopOut = link.second.first.empty();
        if(isTopIn && isTopOut){
            toAssign.push_back({link.first.second, link.second.second});
        } else if (!isTopIn && !isTopOut){
            string linkStr = link.first.first + "_" + link.first.second;
            port2linkStr[link.first.first + "_" + link.first.second] = linkStr;
            port2linkStr[link.second.first + "_" + link.second.second] = linkStr;
        } else if (isTopIn && !isTopOut){
            port2linkStr[link.second.first + "_" + link.second.second] = link.first.second;
        } else {
            string linkStr = link.first.first + "_" + link.first.second;
            port2linkStr[link.first.first + "_" + link.first.second] = linkStr;
            toAssign.push_back({linkStr, link.second.second});
        }
    }
    unordered_set<string> linkStrs;
    for(const auto &link: port2linkStr){
        linkStrs.insert(link.second);
    }
    for(const auto &link: linkStrs){
        fout << "\t" << "wire [size-1:0] " << link << ";" << endl;
    }
    fout << endl;

    size_t config_wire_count = 0;
    for(size_t idx = 0; idx < widths.size(); idx++){
        fout << "\t" << configCell.name() << " #" << widths[idx] << " config_cell_" << idx << "(" << endl;
        fout << "\t\t" << ".config_clk(config_clk)," << endl;
        fout << "\t\t" << ".config_reset(config_reset)," << endl;
        fout << "\t\t" << ".config_in(";
        if(config_wire_count == 0){
            fout << "config_in";
        } else {
            fout << "config_wire_" << config_wire_count-1;
        }
        fout << ")," << endl;
        fout << "\t\t" << ".config_out(";
        if(configWireNum==0||config_wire_count == configWireNum-1){
            fout << "config_out";
        } else {
            fout << "config_wire_" << config_wire_count;
        }
        fout << ")," << endl;
        fout << "\t\t" << ".config_sig(config_sig_" << idx << ")" << endl;
        fout << "\t" << ");" << endl;
        fout << endl;
        config_wire_count++;
    }

    size_t config_sig_count = 0;
    for(size_t idx = 0; idx < module.subModuleIns().size(); idx++){
        string ins = module.subModuleIns()[idx];
        string name = module.subModuleNames()[idx];
        fout << "\t" << name << " " << ins << "(" << endl;
        const Module &mod = subMods.find(name)->second;
        for(const auto &port: mod.specialPorts()){
            fout << "\t\t" << "." << port << "(";
            if(port == "config_sig"){
                fout << "config_sig_" << config_sig_count++;
            } else if(port == "config_in"){
                if(config_wire_count == 0){
                    fout << "config_in";
                } else {
                    fout << "config_wire_" << config_wire_count-1;
                }
            } else if(port == "config_out"){
                if(config_wire_count == configWireNum){
                    fout << "config_out";
                } else {
                    fout << "config_wire_" << config_wire_count;
                }
            } else {
                fout << port;
            }
            fout << ")," << endl;
        }
        for(const auto &port: mod.inPorts()){
            fout << "\t\t" << "." << port << "(" << port2linkStr[ins+"_"+port] << ")," << endl;
        }
        string endOne = mod.outPorts()[mod.outPorts().size() - 1];
        for(const auto &port: mod.outPorts()){
            fout << "\t\t" << "." << port << "(" << port2linkStr[ins+"_"+port] << ")";
            if(!(port == endOne)){
                fout << ",";
            }
            fout << endl;
        }
        fout << "\t" << ");" << endl;
        fout << endl;
        if(findIn("config_in", mod.specialPorts())){
            config_wire_count++;
        }
    }

    for(const auto &link: toAssign){
        fout << "\t" << "assign " << link.second << " = " << link.first << ";" << endl;
    }

    fout << endl << endl;

    fout << "endmodule" << endl;



    fout.close();
};

Module genSWModule(const size_t &inNum, const size_t &outNum, const std::string &name, const Module &configCell, const std::string &path)
{
    Module result;

    result.setName(name);
    result.setType("SWITCH_CELL");
    result.addspecialPorts("config_clk");
    result.addspecialPorts("config_reset");
    result.addspecialPorts("config_in");
    result.addspecialPorts("config_out");
    for(size_t i = 0; i < inNum; i++){
        result.addinPorts("in" + to_string(i));
    }
    for(size_t i = 0; i < outNum; i++){
        result.addoutPorts("out" + to_string(i));
    }
    result.setPath(path);

    ofstream fout(path);
    fout << "// Module Name: " << result.name() << endl;
    fout << "// Module Type: " << result.type() << endl;
    fout << "// Include Module: " << configCell.name() << endl;
    fout << "// Input Ports:";
    for(const auto &port: result.inPorts()){
        fout << " " << port;
    }
    fout << endl;
    fout << "// Output Ports:";
    for(const auto &port: result.outPorts()){
        fout << " " << port;
    }
    fout << endl;
    fout << "// Special Ports:";
    for(const auto &port: result.specialPorts()){
        fout << " " << port;
    }
    fout << endl;
    fout << "// Config Width: " << endl << endl << endl;

    fout << "module " << result.name() << "(";
    string portStr;
    for(const auto &port: result.specialPorts()){
        portStr += (", " + port);
    }
    for(const auto &port: result.inPorts()){
        portStr += (", " + port);
    }
    for(const auto &port: result.outPorts()){
        portStr += (", " + port);
    }
    fout << portStr.substr(2, portStr.size() - 2);
    fout << ");" << endl;
    fout << "\t" << "parameter size = 32;" << endl << endl;

    fout << "\t" << "input config_clk, config_reset, config_in;" << endl;
    fout << "\t" << "output config_out;" << endl;
    fout << "\t" << "input [size-1:0] ";
    portStr.clear();
    for(const auto &port: result.inPorts()){
        portStr += (", " + port);
    }
    fout << portStr.substr(2, portStr.size() - 2) << ";" << endl;
    portStr.clear();
    fout << "\t" << "output reg [size-1:0] ";
    for(const auto &port: result.outPorts()){
        portStr += (", " + port);
    }
    fout << portStr.substr(2, portStr.size() - 2) << ";" << endl << endl;

    vector<size_t> widths;
    for(size_t idx = 0; idx < result.outPorts().size(); idx++){
        widths.push_back(getBitWidth(result.inPorts().size()));
    }

    for(size_t idx = 0; idx < widths.size(); idx++){
        fout << "\t" << "wire ";
        if(widths[idx] > 1){
            fout << "[" << widths[idx]-1 << ":0] ";
        }
        fout << "mux_sel_" << idx << ";" << endl;
    }
    for(size_t idx = 0; idx < widths.size(); idx++){
        if(idx > 0){
            fout << "\t" << "wire " << "config_wire_" << idx << ";" << endl;
        }
    }
    fout << endl;

    for(size_t idx = 0; idx < widths.size(); idx++){
        fout << "\t" << configCell.name() << " #" << widths[idx] << " config_cell_" << idx << "(" << endl;
        fout << "\t\t" << ".config_clk(config_clk)," << endl;
        fout << "\t\t" << ".config_reset(config_reset)," << endl;
        fout << "\t\t" << ".config_in(";
        if(idx == 0){
            fout << "config_in";
        } else {
            fout << "config_wire_" << idx;
        }
        fout << ")," << endl;
        fout << "\t\t" << ".config_out(";
        if(idx == widths.size()-1){
            fout << "config_out";
        } else {
            fout << "config_wire_" << idx+1;
        }
        fout << ")," << endl;
        fout << "\t\t" << ".config_sig(mux_sel_" << idx << ")" << endl;
        fout << "\t" << ");" << endl;
        fout << endl;
    }

    for(size_t idx = 0; idx < widths.size(); idx++){
        fout << "\t" << "always @(*)" << endl;
        fout << "\t\t" << "case " << "(" << "mux_sel_" << idx << ")" << endl;
        for(size_t idy = 0; idy < inNum; idy++){
            fout << "\t\t\t" << idy << ": " << "out" << idx << " = " << "in" << idy << ";" << endl;
        }
        fout << "\t\t\t" << "default: out" << idx << " = 0;" << endl;
        fout << "\t\t" << "endcase" << endl;
        fout << endl;
    }
    fout << endl;

    fout << "endmodule" << endl;

    fout.close();

    return result;

}

Module genModule(const Json::Value &value, const std::string &name, const std::string &type, const unordered_map<string, Module> &subMods)
{
    if(type != "MODULE_CELL" && type != "FUNC_CELL"){
        ERROR << "GenVerilog::genModule: type " + type + " error."; 
    }

    Module result;
    result.setName(name);
    result.setType(type);
    for(const auto &port: value["input"]){
        result.addinPorts(port.asString());
    }
    for(const auto &port: value["output"]){
        result.addoutPorts(port.asString());
    }
    unordered_set<string> specialPorts;
    for(const auto &mod: subMods){
        for(const auto &port: mod.second.specialPorts()){
            if(port == "config_sig" 
            ||port == "config_in"
            ||port == "config_out"
            ||port == "config_clk"
            ||port == "config_reset" ){
                specialPorts.insert("config_in");
                specialPorts.insert("config_out");
                specialPorts.insert("config_clk");
                specialPorts.insert("config_reset");
                continue;
            } else {
                specialPorts.insert(port);
            }
        }
    }
    for(const auto &port: specialPorts){
        result.addspecialPorts(port);
    }
    if(type == "MODULE_CELL"){
        vector<string> elements = value["element"].getMemberNames();
        vector<string> switches = value["switch"].getMemberNames();
        for(const auto &ele: elements){
            string mod = value["element"][ele].asString();
            ASSERT(subMods.find(mod) != subMods.end(), "GenVerilog::genModule: " + mod + " not in subMods" );
            result.addsubModules(ele, mod);
        }
        for(const auto &ele: switches){
            string mod = value["switch"][ele].asString();
            ASSERT(subMods.find(mod) != subMods.end(), "GenVerilog::genModule: " + mod + " not in subMods" );
            result.addsubModules(ele, mod);
        }
        for(const auto &link: value["connection"]){
            vector<string> item = split( link.asString() , "->");
            vector<string> from = {"", item[0]};
            vector<string> to = {"", item[1]};
            if(item[0].find(".") != string::npos){
                from = split( item[0] , ".");
                size_t idx = findId(from[0], result.subModuleIns());
                if(!findIn(from[1], subMods.find(result.subModuleNames()[idx])->second.outPorts())){
                    ERROR << "GenVerilog::genModule: " << from[1] << " is not a outPort of " << from[0]; 
                }
            } else {
                if(!findIn(from[1], result.inPorts())){
                    ERROR << "GenVerilog::genModule: " << from[1] << " is not a inPort of " << name; 
                }
            }
            if(item[1].find(".") != string::npos){
                to = split( item[1] , ".");
                size_t idx = findId(to[0], result.subModuleIns());
                if(!findIn(to[1], subMods.find(result.subModuleNames()[idx])->second.inPorts())){
                    ERROR << "GenVerilog::genModule: " << to[1] << " is not a inPort of " << to[0]; 
                }
            } else {
                if(!findIn(to[1], result.outPorts())){
                    ERROR << "GenVerilog::genModule: " << to[1] << " is not a outPort of " << name; 
                }
            }
            result.connect({{from[0], from[1]},{to[0], to[1]}});
        }
    } else {
        // future work
    }
    

    return result;
};

}

}