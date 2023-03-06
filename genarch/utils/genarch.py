import sys 
sys.path.append(".")

import xmltodict
import json

from genarch.utils.arch import *
# import dataflow.dataflow as df
# import dataflow.funcmap as fm

class ModuleGen: 

    def __init__(self, inputs, outputs, functions, units, modules): 
        self._inputs = inputs
        self._outputs = outputs
        self._funcs = functions
        self._units = units
        self._modules = modules
        self._switches = {}
        self._groups = {}
        self._connects = []
        self._info = {
            "input": inputs, 
            "output": outputs, 
            "module": {}, 
            "element": {}, 
            "switch": {}, 
            "connection": [], 
        }
        self._toCheck = []

    def _appendConnect(self, fromName, toName):
        self._connects.append([fromName, toName])
        
    def _createConnect(self):
        connection = {}
        for connect in self._connects:
            fromName = connect[0]
            toName = connect[1]
            if toName not in connection:
                connection[toName] = []
            connection[toName].append(fromName)
        for toName in connection:
            numFroms = len(connection[toName])
            # if numFroms > 1:
            #     typeSwitch = self._createSwitch(numFroms,1)
            #     nameSwitch = "SW" + str(numFroms) + "X" + str(1) + "_" + toName.replace(".","_") + "_CONNECT"
            #     assert not nameSwitch in self._info["switch"]
            #     self._info["switch"][nameSwitch] = typeSwitch
            #     connections = self._connectSwitch(connection[toName], [toName], nameSwitch)
            #     self._info["connection"].extend(connections)
            # else:
            #     self.connect(connection[toName][0], toName)
            typeSwitch = self._createSwitch(numFroms,1)
            nameSwitch = "SW" + str(numFroms) + "X" + str(1) + "_" + toName.replace(".","_") + "_CONNECT"
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(connection[toName], [toName], nameSwitch)
            self._info["connection"].extend(connections)

    def _createSwitch(self, numFroms, numTos): 
        required = []
        for idx in range(numFroms): 
            for jdx in range(numTos): 
                required.append("in" + str(idx) + "->" + "out" + str(jdx))
        typeSwitch = "FULLYCONN_" + str(numFroms) + "X" + str(numTos)
        if not typeSwitch in self._switches: 
            self._switches[typeSwitch] = {
                "input": ["in" + str(idx) for idx in range(numFroms)], 
                "output": ["out" + str(idx) for idx in range(numTos)], 
                "required": required, "graph": ""
            }
        return typeSwitch
    
    def _getInPorts(self, name=None): 
        if name is None: 
            return self._inputs
        elif name in self._info["element"]: 
            typename = self._info["element"][name]
            assert typename in self._units
            ports = self._units[typename].inputs()
            return list(map(lambda x: name + "." + x, ports))
        elif name in self._info["module"]: 
            typename = self._info["module"][name]
            assert typename in self._modules
            ports = self._modules[typename].inputs()
            return list(map(lambda x: name + "." + x, ports))
        else: 
            assert name is None or name in self._info["element"] or name in self._info["module"]
        return None
    
    def _getOutPorts(self, name=None): 
        if name is None: 
            return self._outputs
        elif name in self._info["element"]: 
            typename = self._info["element"][name]
            assert typename in self._units
            ports = self._units[typename].outputs()
            return list(map(lambda x: name + "." + x, ports))
        elif name in self._info["module"]: 
            typename = self._info["module"][name]
            assert typename in self._modules
            ports = self._modules[typename].outputs()
            return list(map(lambda x: name + "." + x, ports))
        else: 
            assert name is None or name in self._info["element"] or name in self._info["module"]
        return None
    
    def _getSwitchInPorts(self, name):
        assert name in self._info["switch"]
        typeSwitch = self._info["switch"][name]
        ports = []
        for port in self._switches[typeSwitch]["input"]:
            ports.append(name + "." + port)
        return ports
    
    def _getSwitchOutPorts(self, name):
        assert name in self._info["switch"]
        typeSwitch = self._info["switch"][name]
        ports = []
        for port in self._switches[typeSwitch]["output"]:
            ports.append(name + "." + port)
        return ports

    def _connectSwitch(self, froms, tos, nameSwitch): 
        connections = []
        for idx, fr in enumerate(froms): 
            connections.append(fr + "->" + nameSwitch + ".in" + str(idx))
        for idx, to in enumerate(tos): 
            connections.append(nameSwitch + "." + "out" + str(idx) + "->" + to)
            self._toCheck.append(to)
        return connections

    def addUnits(self, names, types): 
        assert len(names) == len(types)
        for idx in range(len(names)): 
            assert types[idx] in self._units
            self._info["element"][names[idx]] = types[idx]

    def addModules(self, names, types): 
        assert len(names) == len(types)
        for idx in range(len(names)): 
            assert types[idx] in self._modules
            self._info["module"][names[idx]] = types[idx]

    def addGroupStar(self, name, elements, frInputs=True): 
        self._groups[name] = elements
        for elem in elements: 
            assert elem in self._info["element"] or elem in self._info["module"]
        count = 0
        for elem in elements: 
            froms = []
            tos   = []
            if frInputs: 
                froms.extend(self._inputs)
            for elem2 in elements: 
                if elem2 == elem: 
                    continue
                froms.extend(self._getOutPorts(elem2))
            tos.extend(self._getInPorts(elem))
            numFroms = len(froms)
            numTos = len(tos)
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_STAR" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(froms, tos, nameSwitch)
            self._info["connection"].extend(connections)

    def addGroup(self, name, elements, frInputs):
        self._groups[name] = elements
        for elem in elements: 
            assert elem in self._info["element"] or elem in self._info["module"]

    def addGroupSeq(self, name, elements, frInputs=True): 
        self._groups[name] = elements
        for elem in elements: 
            assert elem in self._info["element"] or elem in self._info["module"]
        count = 0
        for idx, elem in enumerate(elements): 
            froms = []
            tos   = []
            if frInputs: 
                froms.extend(self._inputs)
            for jdx, elem2 in enumerate(elements): 
                if jdx <= idx: 
                    continue
                froms.extend(self._getOutPorts(elem2))
            tos.extend(self._getInPorts(elem))
            numFroms = len(froms)
            numTos = len(tos)
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_SEQ" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(froms, tos, nameSwitch)
            self._info["connection"].extend(connections)
                
    def addGroupMesh(self, name, elements, row, col, toroid=True, frInputs=True): 
        assert len(elements) == row * col
        self._groups[name] = elements
        for elem in elements: 
            assert elem in self._info["element"] or elem in self._info["module"]
        coords = []
        for idx in range(row): 
            for jdx in range(col):
                coords.append((idx, jdx))
        count = 0
        for idx, elem in enumerate(elements): 
            x, y = coords[idx]
            froms = []
            tos   = []
            if frInputs: 
                froms.extend(self._inputs)
            for jdx, elem2 in enumerate(elements): 
                x2, y2 = coords[jdx]
                if toroid: 
                    if not ((abs(x - x2) == 1 and abs(y - y2) == 0) or (abs(x - x2) == 0 and abs(y - y2) == 1) or \
                            (abs(x - x2) == row - 1 and abs(y - y2) == 0) or (abs(x - x2) == 0 and abs(y - y2) == col - 1)): 
                        continue
                else: 
                    if not ((abs(x - x2) == 1 and abs(y - y2) == 0) or (abs(x - x2) == 0 and abs(y - y2) == 1)): 
                        continue
                froms.extend(self._getOutPorts(elem2))
            tos.extend(self._getInPorts(elem))
            numFroms = len(froms)
            numTos = len(tos)
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_MESH" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(froms, tos, nameSwitch)
            self._info["connection"].extend(connections)
    
    def addGroupDiag(self, name, elements, row, col, toroid=True, frInputs=True): 
        assert len(elements) == row * col
        self._groups[name] = elements
        for elem in elements: 
            assert elem in self._info["element"] or elem in self._info["module"]
        coords = []
        for idx in range(row): 
            for jdx in range(col):
                coords.append((idx, jdx))
        count = 0
        for idx, elem in enumerate(elements): 
            x, y = coords[idx]
            froms = []
            tos   = []
            if frInputs: 
                froms.extend(self._inputs)
            for jdx, elem2 in enumerate(elements): 
                x2, y2 = coords[jdx]
                if toroid: 
                    if not ((abs(x - x2) == 1) or (abs(y - y2) == 1) or \
                            (abs(x - x2) == row - 1) or (abs(y - y2) == col - 1)): 
                        continue
                else: 
                    if not ((abs(x - x2) == 1) or (abs(y - y2) == 1)): 
                        continue
                froms.extend(self._getOutPorts(elem2))
            tos.extend(self._getInPorts(elem))
            numFroms = len(froms)
            numTos = len(tos)
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_DIAG" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(froms, tos, nameSwitch)
            self._info["connection"].extend(connections)

    def addGroupHop(self, name, elements, hopStep, connectSelf=False): 
        self._groups[name] = elements
        for elem in elements: 
            assert elem in self._info["element"] or elem in self._info["module"]
        for idx, elem in enumerate(elements): 
            froms = []
            froms.extend(self._getOutPorts(elem))
            for jdx in range(-hopStep, hopStep):
                if ~connectSelf and jdx==0:
                    continue
                tos   = []
                elem2 = elements[(idx + jdx) % len(elements)]
                tos.extend(self._getInPorts(elem2))
                numFroms = len(froms)
                numTos = len(tos)
                typeSwitch = self._createSwitch(numFroms, numTos)
                nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_HOP" + str(idx) + "_" + str((idx + jdx) % len(elements))
                assert not nameSwitch in self._info["switch"]
                self._info["switch"][nameSwitch] = typeSwitch
                connections = self._connectSwitch(froms, tos, nameSwitch)
                self._info["connection"].extend(connections)

    def addGroupBar(self, name, elements, row, col, num, toroid=True, frInputs=True, toOutputs=True):
        assert len(elements) == row * col
        self._groups[name] = elements
        for elem in elements: 
            assert elem in self._info["element"] or elem in self._info["module"]
        BarRow = []
        for idx in range(0, row + 1):
            if not frInputs and idx == 0:
                continue
            if not toOutputs and idx == row:
                continue
            BarRow.append(idx)
        Bars = {}
        for idx in BarRow:
            Bars[idx] = {}
            for idy in range(0, col+1):
                froms = []
                tos = []
                if idx == 0 and frInputs:
                    froms.extend(self._inputs)
                else:
                    fromRow = idx-1
                    fromCol = [idy-1,idy]
                    for idz in fromCol:
                        if idz >= 0 and idz < col:
                            froms.extend(self._getOutPorts(elements[fromRow*row+idz]))
                if idx == row and toOutputs:
                    tos.extend(self._outputs)
                else:
                    toRow = idx
                    toCol = [idy-1,idy]
                    for idz in toCol:
                        if idz >= 0 and idz < col:
                            tos.extend(self._getInPorts(elements[toRow*row+idz]))
                numFroms = len(froms)
                numTos = len(tos)
                count = 0
                Bars[idx][idy] = []
                while count < num:
                    typeSwitch = self._createSwitch(numFroms, numTos)
                    nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_Bar" + str(idx) + "_" + str(idy) + "_" + str(count)
                    Bars[idx][idy].append(nameSwitch)
                    count+=1
                    assert not nameSwitch in self._info["switch"]
                    self._info["switch"][nameSwitch] = typeSwitch
                    connections = self._connectSwitch(froms, tos, nameSwitch)
                    self._info["connection"].extend(connections)
        for idx in BarRow:
            for idy in range(0, col+1):
                count = 0
                while count < num:
                    nameSwitchFrom = Bars[idx][idy][count]
                    nameSwitchTos = []
                    if idy > 0:
                        nameSwitchTos.append(Bars[idx][idy-1][count])
                    if idy < col:
                        nameSwitchTos.append(Bars[idx][idy+1][count])
                    if idx < BarRow[-1]:
                        nameSwitchTos.append(Bars[idx+1][idy][count])
                    if toroid and idx==BarRow[-1]:
                        nameSwitchTos.append(Bars[BarRow[0]][idy][count])
                    count+=1
                    froms = []
                    tos   = []
                    froms.extend(self._getSwitchOutPorts(nameSwitchFrom))
                    for nameSwithTo in nameSwitchTos:
                        tos.extend(self._getSwitchInPorts(nameSwithTo))
                    numFroms = len(froms)
                    numTos = len(tos)
                    typeSwitch = self._createSwitch(numFroms, numTos)
                    nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_Bar_from" + str(idx) + "_" + str(idy) + str(count)
                    assert not nameSwitch in self._info["switch"]
                    self._info["switch"][nameSwitch] = typeSwitch
                    connections = self._connectSwitch(froms, tos, nameSwitch)
                    self._info["connection"].extend(connections)

    def connectGroupsStar(self, name, groups, frInputs=False): 
        for group in groups: 
            assert group in self._groups
        count = 0
        for idx, group in enumerate(groups): 
            froms = []
            tos   = []
            if frInputs: 
                froms.extend(self._inputs)
            for jdx, group2 in enumerate(groups): 
                if jdx == idx: 
                    continue
                for elem2 in self._groups[group2]: 
                    froms.extend(self._getOutPorts(elem2))
            for elem in self._groups[group]: 
                tos.extend(self._getInPorts(elem))
            numFroms = len(froms)
            numTos = len(tos)
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_STAR" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(froms, tos, nameSwitch)
            self._info["connection"].extend(connections)
    
    def connect(self, pinFr, pinTo): 
        self._info["connection"].append(pinFr + "->" + pinTo)

    def connectGroupsSeq(self, name, groups, frInputs=False): 
        for group in groups: 
            assert group in self._groups
        count = 0
        for idx, group in enumerate(groups): 
            froms = []
            tos   = []
            if frInputs: 
                froms.extend(self._inputs)
            for jdx, group2 in enumerate(groups): 
                if jdx <= idx: 
                    continue
                for elem2 in self._groups[group2]: 
                    froms.extend(self._getOutPorts(elem2))
            for elem in self._groups[group]: 
                tos.extend(self._getInPorts(elem))
            numFroms = len(froms)
            numTos = len(tos)
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_SEQ" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(froms, tos, nameSwitch)
            self._info["connection"].extend(connections)
    
    def connectGroupsMesh(self, name, groups, row, col, toroid=True, frInputs=False): 
        assert len(groups) == row * col
        for group in groups: 
            assert group in self._groups
        coords = []
        for idx in range(row): 
            for jdx in range(col):
                coords.append((idx, jdx))
        count = 0
        for idx, group in enumerate(groups): 
            x, y = coords[idx]
            froms = []
            tos   = []
            if frInputs: 
                froms.extend(self._inputs)
            for jdx, group2 in enumerate(groups): 
                x2, y2 = coords[jdx]
                if toroid: 
                    if not ((abs(x - x2) == 1 and abs(y - y2) == 0) or (abs(x - x2) == 0 and abs(y - y2) == 1) or \
                            (abs(x - x2) == row - 1 and abs(y - y2) == 0) or (abs(x - x2) == 0 and abs(y - y2) == col - 1)): 
                        continue
                else: 
                    if not ((abs(x - x2) == 1 and abs(y - y2) == 0) or (abs(x - x2) == 0 and abs(y - y2) == 1)): 
                        continue
                for elem2 in self._groups[group2]: 
                    froms.extend(self._getOutPorts(elem2))
            for elem in self._groups[group]: 
                tos.extend(self._getInPorts(elem))
            numFroms = len(froms)
            numTos = len(tos)
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_MESH" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(froms, tos, nameSwitch)
            self._info["connection"].extend(connections)
    
    def connectGroupsDiag(self, name, groups, row, col, toroid=True, frInputs=False): 
        assert len(groups) == row * col
        for group in groups: 
            assert group in self._groups
        coords = []
        for idx in range(row): 
            for jdx in range(col):
                coords.append((idx, jdx))
        count = 0
        for idx, group in enumerate(groups): 
            x, y = coords[idx]
            froms = []
            tos   = []
            if frInputs: 
                froms.extend(self._inputs)
            for jdx, group2 in enumerate(groups): 
                x2, y2 = coords[jdx]
                if toroid: 
                    if not ((abs(x - x2) == 1) or (abs(y - y2) == 1) or \
                            (abs(x - x2) == row - 1) or (abs(y - y2) == col - 1)): 
                        continue
                else: 
                    if not ((abs(x - x2) == 1) or (abs(y - y2) == 1)): 
                        continue
                for elem2 in self._groups[group2]: 
                    froms.extend(self._getOutPorts(elem2))
            for elem in self._groups[group]: 
                tos.extend(self._getInPorts(elem))
            numFroms = len(froms)
            numTos = len(tos)
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_" + name + "_MESH" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            connections = self._connectSwitch(froms, tos, nameSwitch)
            self._info["connection"].extend(connections)

    def dealWithFanin(self):  
        newConns = []
        fanins = {}
        for conn in self._info["connection"]: 
            splited = conn.split("->")
            assert len(splited) > 1, "Invalid connection: " + str(splited)
            fr = splited[0]
            to = splited[1]
            if not to in fanins: 
                fanins[to] = []
            fanins[to].append(fr)
        for conn in self._info["connection"]: 
            splited = conn.split("->")
            fr = splited[0]
            to = splited[1]
            if len(fanins[to]) <= 1: 
                newConns.append(conn)
        count = 0
        for to, froms in fanins.items(): 
            if len(froms) <= 1: 
                continue
            numFroms = len(froms)
            numTos = 1
            typeSwitch = self._createSwitch(numFroms, numTos)
            nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_UNROLL" + str(count)
            count += 1
            assert not nameSwitch in self._info["switch"]
            self._info["switch"][nameSwitch] = typeSwitch
            for idx, fr in enumerate(froms): 
                newConns.append(fr + "->" + nameSwitch + ".in" + str(idx))
            newConns.append(nameSwitch + ".out0->" + to)
        self._info["connection"] = newConns

    # Connect every inputs to the units/modules
    def connectInportsAll(self, elements, inputs=None, postfix=""): 
        if inputs is None: 
            inputs = self._inputs
        froms = inputs
        tos = []
        for elem in elements: 
            tos.extend(self._getInPorts(elem))
        numFroms = len(froms)
        numTos = len(tos)
        typeSwitch = self._createSwitch(numFroms, numTos)
        nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_INPORT" + postfix
        assert not nameSwitch in self._info["switch"]
        self._info["switch"][nameSwitch] = typeSwitch
        connections = self._connectSwitch(froms, tos, nameSwitch)
        self._info["connection"].extend(connections)

    # Connect the units/modules to every outputs
    def connectOutportsAll(self, elements, outputs=None, postfix="", frInputs=False): 
        if outputs is None: 
            outputs = self._outputs
        froms = []
        tos = outputs
        if frInputs: 
            froms.extend(self._inputs)
        for elem in elements: 
            froms.extend(self._getOutPorts(elem))
        numFroms = len(froms)
        numTos = len(tos)
        typeSwitch = self._createSwitch(numFroms, numTos)
        nameSwitch = "SW" + str(numFroms) + "X" + str(numTos) + "_OUTPORT" + postfix
        assert not nameSwitch in self._info["switch"]
        self._info["switch"][nameSwitch] = typeSwitch
        connections = self._connectSwitch(froms, tos, nameSwitch)
        self._info["connection"].extend(connections)


class ArchGen: 
    def __init___(self, functions, units): 
        self._funcs = functions
        self._units = units

# ====================== demo =======================

# if __name__ == "__main__": 
#     import units
#     infoFuncs = units.functions()
#     infoUnits = units.units()
#     infoSwitches = {}
#     infoModules = {}
    
#     funcStructs = {}
#     unitStructs = {}
#     moduleStructs = {}
#     for name, info in infoFuncs.items(): 
#         funcStructs[name] = Function(name, info)
#     for name, info in infoUnits.items(): 
#         unitStructs[name] = Unit(name, info)

#     blockArith = ModuleGen(["in" + str(idx) for idx in range(32)], 
#                            ["out" + str(idx) for idx in range(16)], 
#                            funcStructs, unitStructs, [])
#     mults = ["MUL" + str(idx) for idx in range(16)]
#     adds  = ["ADD" + str(idx) for idx in range(4)]
#     blockArith.addUnits(mults, ["MAD" for _ in range(len(mults))])
#     blockArith.addUnits(adds,  ["ADDTREE4" for _ in range(len(adds))])
#     blockArith.addGroupMesh("MULs", mults, row=4, col=4, frInputs=True)
#     blockArith.addGroupMesh("ADDs", adds,  row=2, col=2, frInputs=True)
#     blockArith.connectGroupsMesh("MULADD", ["MULs", "ADDs"], row=2, col=1, frInputs=False)
#     blockArith.connectOutportsAll(mults + adds)
#     blockArith.dealWithFanin()

#     blockDiv = ModuleGen(["in" + str(idx) for idx in range(32)], 
#                          ["out" + str(idx) for idx in range(16)], 
#                          funcStructs, unitStructs, [])
#     muls = ["MUL" + str(idx) for idx in range(4)]
#     divs = ["DIV" + str(idx) for idx in range(4)]
#     blockDiv.addUnits(muls, ["MAD" for _ in range(len(muls))])
#     blockDiv.addUnits(divs, ["DIV" for _ in range(len(divs))])
#     blockDiv.addGroupMesh("MULs", muls, row=2, col=2, frInputs=True)
#     blockDiv.addGroupMesh("DIVs", divs, row=2, col=2, frInputs=True)
#     blockDiv.connectGroupsStar("MULDIV", ["MULs", "DIVs", ], frInputs=False)
#     blockDiv.connectOutportsAll(muls + divs)
#     blockDiv.dealWithFanin()

#     blockLogic = ModuleGen(["in" + str(idx) for idx in range(32)], 
#                            ["out" + str(idx) for idx in range(16)], 
#                            funcStructs, unitStructs, [])
#     logics = ["LGC" + str(idx) for idx in range(16)]
#     blockLogic.addUnits(logics, ["LOGIC" for _ in range(len(logics))])
#     blockLogic.addGroupMesh("LOGICs", logics, row=4, col=4, frInputs=True)
#     blockLogic.connectOutportsAll(logics)
#     blockLogic.dealWithFanin()

#     for key, value in blockArith._switches.items(): 
#         infoSwitches[key] = value
#     for key, value in blockDiv._switches.items(): 
#         infoSwitches[key] = value
#     for key, value in blockLogic._switches.items(): 
#         infoSwitches[key] = value

#     infoModules["BlockArith"] = blockArith._info
#     infoModules["BlockDiv"]   = blockDiv._info
#     infoModules["BlockLogic"] = blockLogic._info
#     for name, info in infoModules.items(): 
#         moduleStructs[name] = Module(name, info)

#     core = ModuleGen([], [], 
#                      funcStructs, unitStructs, moduleStructs)
#     inputs = ["IN" + str(idx) for idx in range(32)]
#     core.addUnits(inputs, ["INPUT" for _ in range(len(inputs))])
#     outputs = ["OUT" + str(idx) for idx in range(16)]
#     core.addUnits(outputs, ["OUTPUT" for _ in range(len(outputs))])
#     mems = ["MEM" + str(idx) for idx in range(32)]
#     core.addUnits(mems, ["MEMORY" for _ in range(len(mems))])
#     blocks = ["Arith0", "Arith1", "Arith2", "Arith3", "Div0", "Div1", "Logic0", "Logic1"]
#     types = ["BlockArith", "BlockArith", "BlockArith", "BlockArith", "BlockDiv", "BlockDiv", "BlockLogic", "BlockLogic"]
#     core.addModules(blocks, types)
#     core.addGroupStar("ArithLogic", blocks, frInputs=False)
#     for idx in range(len(inputs)): 
#         for jdx in range(len(blocks)): 
#             core.connect(inputs[idx] + ".out0", blocks[jdx] + ".in" + str(idx))
#             core.connect(mems[idx] + ".out0", blocks[jdx] + ".in" + str(idx))
#     for idx in range(len(outputs)): 
#         for jdx in range(len(blocks)): 
#             core.connect(blocks[jdx] + ".out" + str(idx), outputs[idx] + ".in0")
#     for idx in range(len(blocks)):
#         portNum = len(infoModules[types[idx]]["output"])
#         memNum = len(mems)
#         interval = int(memNum/portNum)
#         for jdx in range(0, portNum):
#             indexFrom = jdx
#             for kdx in range(0, 8):
#                 indexTo = (jdx * interval + kdx) % memNum
#                 core.connect(blocks[idx] + ".out" + str(indexFrom), mems[indexTo] + ".in0")
#                 core.connect(blocks[idx] + ".out" + str(indexFrom), mems[indexTo] + ".in1")
#                 core.connect(blocks[idx] + ".out" + str(indexFrom), mems[indexTo] + ".in2")
#     for idx in range(len(blocks)):
#         for jdx in range(len(blocks)):
#             if(idx == jdx):
#                 continue
#             iOutportNum = len(infoModules[types[idx]]["output"])
#             jInportNum = len(infoModules[types[jdx]]["input"])
#             if(types[idx] == types[jdx]):
#                 for mdx in range(0, int(iOutportNum/2)):
#                     for ndx in range(0, int(jInportNum/2)):
#                         core.connect(blocks[idx] + ".out" + str(mdx), blocks[jdx] + ".in" + str(ndx))
#             else:
#                 for mdx in range(int(iOutportNum/2), iOutportNum):
#                     for ndx in range(int(jInportNum/2), jInportNum):
#                         core.connect(blocks[idx] + ".out" + str(mdx), blocks[jdx] + ".in" + str(ndx))
#     core.dealWithFanin()

    

                


#     for key, value in core._switches.items(): 
#         infoSwitches[key] = value

#     infoModules["Core"] = core._info

#     names = list(infoModules.keys())
#     for name in names: 
#         top = {
#             "Function": infoFuncs, 
#             "Unit": infoUnits, 
#             "Switch": infoSwitches, 
#             "Module": infoModules
#         }
#         top["Module"]["TOP"] = infoModules[name]
#         with open(f"./tmp/{name}.json", "w") as fout: 
#             fout.write(json.dumps(top, indent=4))
#         arch = Arch(f"./tmp/{name}.json")
#         with open(f"./tmp/{name}RRG.txt", "w") as fout: 
#             fout.write(arch.rrg())
#         with open(f"./tmp/{name}FUs.txt", "w") as fout: 
#             fout.write(arch.fus())

    # og = HyperGraph()
    # with open("./tmp/exampleOG.txt", "r") as fin: 
    #     og.parse(fin.read())
    # rrg = HyperGraph()
    # rrg.parse(arch.rrg())

    # print(f"OpGraph read: {len(og.vertices())} vertices, {len(og.nets())} edges.")

    # # mapper = fm.IsoMatcherNSGAII(og, arch.units(), rrg)
    # mapper = fm.IsoMatcherBFS(og, arch.units(), rrg)
    # if mapper.match(): 
    #     with open("./tmp/exampleDFG.txt", "w") as fout: 
    #         fout.write(mapper.graphInfo())
    #     with open("./tmp/exampleCompat.txt", "w") as fout: 
    #         fout.write(mapper.compatInfo())

    