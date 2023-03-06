import sys 
sys.path.append(".")

import xmltodict
import json

from common import utils as utils
from common.utils import Base
from genarch.utils.protocols import *

class Arch(Base): 
    def __init__(self, filename = None): 
        self._filename     = filename

        self._archRaw      = None
        self._functionsRaw = None
        self._unitsRaw     = None
        self._switchesRaw  = None
        self._modulesRaw   = None

        self._arch      = None
        self._functions = None
        self._units     = None
        self._switches  = None
        self._modules   = None
        self._top       = None
        self._rrg       = None
        self._fus       = None
        self._fusInfo   = None

        self._readArch()
        self._parseFunctions()
        self._parseUnits()
        self._parseSwitchs()
        self._parseModules()
        self._parseTop()
        self._getRRG()
        self._getFUs()

    def arch(self): 
        return self._arch

    def functions(self): 
        return self._functions

    def units(self): 
        return self._units

    def switches(self): 
        return self._switches

    def modules(self): 
        return self._modules

    def top(self): 
        return self._top

    def rrg(self): 
        return self._rrg

    def fus(self): 
        return self._fusInfo

    def _readArch(self): 
        self._archRaw = None
        with open(self._filename, "r") as fin: 
            content = fin.read()
            if self._filename[-5:-1] == '.xml': 
                self._archRaw = dict(xmltodict.parse(content))
            else: 
                self._archRaw = json.loads(content)
        self._functionsRaw = self._archRaw["Function"]
        self._unitsRaw     = self._archRaw["Unit"]
        self._switchesRaw  = self._archRaw["Switch"]
        self._modulesRaw   = self._archRaw["Module"]
    
    def _parseFunctions(self): 
        info = self._functionsRaw
        self._functions = {}
        for name in info.keys(): 
            self._functions[name] = Function(name, info[name])

    def _parseUnits(self): 
        info = self._unitsRaw
        self._units = {}
        for name in info.keys(): 
            self._units[name] = Unit(name, info[name])
            self._units[name].construct(self._functions)
    
    def _parseSwitchs(self): 
        info = self._switchesRaw
        self._switches = {}
        for name in info.keys(): 
            self._switches[name] = Switch(name, info[name])

    def _parseModules(self): 
        info = self._modulesRaw
        self._modules = {}
        for name in info.keys(): 
            self._modules[name] = Module(name, info[name])
        for name in info.keys(): 
            if len(self._modules[name].graph().vertices()) == 0: 
                self._modules[name].construct(self._units, self._switches, self._modules)

    def _parseTop(self): 
        if "TOP" in self._modules: 
            self._top = self._modules["TOP"]
        elif "SINGLE_ARCH_TOP" in self._modules: 
            self._top = self._modules["SINGLE_ARCH_TOP"]

    def _getRRG(self): 
        self._rrg = self._top.graph().info()

    def _getFUs(self): 
        self._fus = self._top.fus()
        self._fusInfo = ""
        for fname, fu in self._fus.items(): 
            self._fusInfo += fname
            self._fusInfo += " " + fu["type"]
            self._fusInfo += " " + fu["device"]
            self._fusInfo += " inputs"
            for port in fu["inputs"]: 
                self._fusInfo += " " + port
            self._fusInfo += " outputs"
            for port in fu["outputs"]: 
                self._fusInfo += " " + port
            self._fusInfo += "\n"

    def info(self): 
        result = ""
        for name in self._functions.keys(): 
            result += self._functions[name].info() + "\n"
        result += "\n"
        for name in self._units.keys(): 
            result += self._units[name].info() + "\n"
        result += "\n"
        for name in self._switches.keys(): 
            result += self._switches[name].info() + "\n"
        result += "\n"
        for name in self._modules.keys(): 
            result += self._modules[name].info() + "\n"
        result += "\n"
        return result
    
