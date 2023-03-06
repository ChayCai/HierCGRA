import sys 
sys.path.append(".")

import xmltodict
import json
import networkx as nx
from networkx.algorithms import isomorphism as iso

import common.utils as utils
from common.utils import Base
from common.graph import HyperGraph
from arch.protocols import *
from arch.arch import *

import optuna
import queue


class IsoMatcherBFS(Base): 
    def __init__(self, graph, units, RRG): 
        self._original = graph
        self._units    = units
        self._RRG      = RRG
        self._patterns = {}
        self._matched  = []
        self._map      = {}
        self._graph    = HyperGraph()
        self._compat   = {}

    def match(self, numCandidates=1): 
        for uname, unit in self._units.items(): 
            self._patterns[uname] = {}
            for pname, patt in unit.patterns().items(): 
                self._patterns[uname][pname] = patt.graph()
                # print("Pattern: " + uname + "." + pname)
                # print(self._patterns[uname][pname].info())
                # print("Matching: " + uname + "." + pname)
                g1 = self._original.toNX()
                g2 = self._patterns[uname][pname].toNX()
                matcher = iso.DiGraphMatcher(g1, g2, lambda x, y: x["attrs"]["function"] == y["attrs"]["function"])
                # print(matcher.subgraph_is_isomorphic())
                isomorphisms = matcher.subgraph_isomorphisms_iter()
                for match in isomorphisms: 
                    self._matched.append((uname, pname, match, ))
                    # print(match)
        self._matched.sort(key=lambda x: (len(x[2]), -len(self._units[x[0]].patterns())), reverse=True)
        # print(self._original.info())
        # print(utils.list2str(self._matched))

        def calcValue(pack): 
            used = set()
            for match in pack: 
                for elem in match[2]: 
                    used.add(elem)
            return len(used)
        
        que = queue.PriorityQueue()
        que.put([0, (0, [])])
        results = []
        current = -1
        while not que.empty() and len(results) < numCandidates: 
            print("\rQueue length:", que.qsize(), "; Results:", len(results), end="")
            pack  = que.get()[1]
            value = calcValue(pack[1])
            if pack[0] > current: 
                current = pack[0]
            if value < len(self._original.vertices()) and pack[0] < len(self._matched): 
                que.put([-value, (pack[0] + 1, pack[1])])
                used = set()
                for match in pack[1]: 
                    for elem in match[2]: 
                        used.add(elem)
                corrupt = False
                for elem in self._matched[pack[0]][2]: 
                    if elem in used: 
                        corrupt = True
                if not corrupt: 
                    newpack = pack[1] + [self._matched[pack[0]], ]
                    que.put([-calcValue(newpack), (pack[0] + 1, newpack)])
            elif value == len(self._original.vertices()): 
                results.append(pack[1])

        print("\rQueue length:", que.qsize(), "; Results:", len(results))
        if len(results) == 0: 
            print("IsoMatcher: FAILED. ")
            return False

        countFUs = {}
        for name, vertex in self._RRG.vertices().items(): 
            if not vertex.attr("device") in countFUs: 
                countFUs[vertex.attr("device")] = 0
            countFUs[vertex.attr("device")] += 1
        def maxUtil(pack): 
            util = {}
            for idx in range(len(pack)): 
                if not pack[idx][0] in util: 
                    util[pack[idx][0]] = 0
                util[pack[idx][0]] += 1
            if not self._RRG is None: 
                for key, value in util.items(): 
                    count = countFUs[key]
                    if count == 0: 
                        print("ERROR: FU", name, "not found. ")
                    assert count > 0
                    util[key] = value / count
            return max(util.values()) if len(util) > 0 else 0.0
        results.sort(key = lambda x: (len(x), maxUtil(x)))
        
        result = results[0]
        if calcValue(result) < len(self._original.vertices()): 
            print("IsoMatcher: FAILED. Cannot match all vertices. ", calcValue(result), "/", len(self._original.vertices()))
            used = set()
            for match in result: 
                for elem in match[2]: 
                    used.add(elem)
            for vertex in self._original.vertices(): 
                if not vertex in used: 
                    print(" -> unmatched vertex:", vertex)
            return False

        used = set()
        for match in result: 
            uname = match[0]
            pname = match[1]
            info  = match[2]
            duplicated = False
            for v1, v2 in info.items(): 
                if v1 in used: 
                    duplicated = True
                    break
            assert not duplicated, "IsoMatcher: FAILED. Duplicated match. "
            for v1, v2 in info.items(): 
                used.add(v1)

            vertexName = ""
            for v1, v2 in info.items(): 
                if not "." in v1: 
                    vertexName += v1 + "_"
            vertexName = vertexName[:-1]
            self._graph.addVertex(vertexName, {"unit": uname, "pattern": pname})
            if not vertexName in self._compat: 
                self._compat[vertexName] = set()
            self._compat[vertexName].add(uname)
            for v1, v2 in info.items(): 
                portName = ""
                portType = ""
                for key, value in self._units[uname].pattern(pname).portMap().items(): 
                    if value == v2: 
                        portName = key
                        if portName in self._units[uname].inputs(): 
                            portType = "input"
                        elif portName in self._units[uname].outputs(): 
                            portType = "output"
                        else: 
                            assert portName in self._units[uname].inputs() or portName in self._units[uname].outputs(), "IsoMatcher: Invalid port: " + portName + " of " + uname
                if portName != "": 
                    temp = portName
                    portName = vertexName + "." + portName
                    self._graph.addVertex(portName, {"unit": uname + "." + temp})
                    self._map[v1] = portName
                    if portType == "input": 
                        self._graph.addNet([portName, vertexName], {})
                    elif portType == "output": 
                        self._graph.addNet([vertexName, portName], {})
        # print(utils.dict2str(self._map))

        if len(used) < len(self._original.vertices()): 
            print("IsoMatcher: not enough matched vertices, ", len(used), "/", len(self._original.vertices()), "; FAILED. ")
            for vertex in self._original.vertices(): 
                if not vertex in used: 
                    print(" -> Unmatched vertex:", vertex)
            return False

        for vname, vertex in self._original.vertices().items(): 
            if vname in self._map: 
                for edge in self._original.netsOut()[vname]: 
                    nodes = [self._map[edge.fr()], ]
                    for node in edge.to(): 
                        if node in self._map: 
                            nodes.append(self._map[node])
                    if len(nodes) > 1: 
                        self._graph.addNet(nodes, {})
        
        return True


    def graph(self): 
        return self._graph

    def graphInfo(self): 
        return self._graph.info()

    def compat(self): 
        return self._compat

    def compatInfo(self): 
        info = ""
        for vertex, compats in self._compat.items(): 
            info += vertex
            for compat in compats: 
                info += " " + compat
            info += "\n"
        return info

class IsoMatcherNSGAII(Base): 
    def __init__(self, graph, units, RRG=None): 
        self._original = graph
        self._units    = units
        self._RRG      = RRG
        self._patterns = {}
        self._matched  = []
        self._map      = {}
        self._graph    = HyperGraph()
        self._compat   = {}

    def match(self): 
        countFUs = {}
        for name, vertex in self._RRG.vertices().items(): 
            if not vertex.attr("device") in countFUs: 
                countFUs[vertex.attr("device")] = 0
            countFUs[vertex.attr("device")] += 1
        for uname, unit in self._units.items(): 
            self._patterns[uname] = {}
            for pname, patt in unit.patterns().items(): 
                self._patterns[uname][pname] = patt.graph()
                # print("Pattern: " + uname + "." + pname)
                # print(self._patterns[uname][pname].info())
                # print("Matching: " + uname + "." + pname)
                g1 = self._original.toNX()
                g2 = self._patterns[uname][pname].toNX()
                matcher = iso.DiGraphMatcher(g1, g2, lambda x, y: x["attrs"]["function"] == y["attrs"]["function"])
                # print(matcher.subgraph_is_isomorphic())
                isomorphisms = matcher.subgraph_isomorphisms_iter()
                for match in isomorphisms: 
                    self._matched.append((uname, pname, match, ))
                    # print(match)
        self._matched.sort(key=lambda x: (len(x[2]), -len(self._units[x[0]].patterns())), reverse=True)
        # print(self._original.info())
        # print(utils.list2str(self._matched))

        def calcValue(pack): 
            used = set()
            for match in pack: 
                for elem in match[2]: 
                    used.add(elem)
            return len(used)
        def maxUtil(result): 
            util = {}
            for idx in range(len(result)): 
                if result[idx] == 1: 
                    if not self._matched[idx][0] in util: 
                        util[self._matched[idx][0]] = 0
                    util[self._matched[idx][0]] += 1
            if not self._RRG is None: 
                for key, value in util.items(): 
                    count = countFUs[key]
                    if count == 0: 
                        print("ERROR: FU", name, "not found. ")
                    assert count > 0
                    util[key] = value / count
            return max(util.values()) if len(util) > 0 else 0.0

        def suggestInt(trial:optuna.Trial, numVars:int = 1, fromVal:int = 0, toVal:int = 1, prefix:str = "i"): 
            variables = []
            for idx in range(numVars): 
                variables.append(trial.suggest_int(prefix + "{}".format(idx), fromVal, toVal))
            return variables
        numVars = len(self._matched)
        def cost(var): 
            pack = []
            for idx in range(numVars): 
                if var[idx] == 1: 
                    pack.append(self._matched[idx])
            duplicated = 0
            used = set()
            for match in pack: 
                for v1, v2 in match[2].items(): 
                    if v1 in used: 
                        duplicated += 1
                    else: 
                        used.add(v1)
            cover = len(self._original.vertices()) - len(used)
            size  = len(pack)
            valid = duplicated
            util  = maxUtil(var)
            return [valid + cover, size]
        def objective(trial:optuna.Trial): 
            return cost(suggestInt(trial, numVars = numVars, fromVal = 0, toVal = 1, prefix = "x"))
            
        print("Begin Matching:", len(self._matched), "vertices. ")

        sampler = optuna.samplers.NSGAIISampler()
        optimizer = optuna.create_study(sampler = sampler, directions = ["minimize", "minimize", ])
        optimizer.optimize(objective, n_trials=2 ** 12, show_progress_bar=True)

        finals = []
        results = optimizer.best_trials
        print("Pareto-optimal results:", len(results))
        for result in results: 
            valid, size = cost(list(result.params.values()))
            if valid == 0: 
                finals.append(list(result.params.values()))
        print("Valid results:", len(finals))

        if len(finals) == 0: 
            return False
        
        finals.sort(key = lambda x: maxUtil(x))
        print(cost(finals[0]))
        result = []
        for idx in range(len(finals[0])): 
            if finals[0][idx] == 1: 
                result.append(self._matched[idx])

        if calcValue(result) < len(self._original.vertices()): 
            print("IsoMatcher: FAILED. Cannot match all vertices. ", calcValue(result), "/", len(self._original.vertices()))
            used = set()
            for match in result: 
                for elem in match[2]: 
                    used.add(elem)
            for vertex in self._original.vertices(): 
                if not vertex in used: 
                    print(" -> unmatched vertex:", vertex)
            return False

        used = set()
        for match in result: 
            uname = match[0]
            pname = match[1]
            info  = match[2]
            duplicated = False
            for v1, v2 in info.items(): 
                if v1 in used: 
                    duplicated = True
                    break
            assert not duplicated, "IsoMatcher: FAILED. Duplicated match. "
            for v1, v2 in info.items(): 
                used.add(v1)

            vertexName = ""
            for v1, v2 in info.items(): 
                if not "." in v1: 
                    vertexName += v1 + "_"
            vertexName = vertexName[:-1]
            self._graph.addVertex(vertexName, {"unit": uname, "pattern": pname})
            if not vertexName in self._compat: 
                self._compat[vertexName] = set()
            self._compat[vertexName].add(uname)
            for v1, v2 in info.items(): 
                portName = ""
                portType = ""
                for key, value in self._units[uname].pattern(pname).portMap().items(): 
                    if value == v2: 
                        portName = key
                        if portName in self._units[uname].inputs(): 
                            portType = "input"
                        elif portName in self._units[uname].outputs(): 
                            portType = "output"
                        else: 
                            assert portName in self._units[uname].inputs() or portName in self._units[uname].outputs(), "IsoMatcher: Invalid port: " + portName + " of " + uname
                if portName != "": 
                    temp = portName
                    portName = vertexName + "." + portName
                    self._graph.addVertex(portName, {"unit": uname + "." + temp})
                    self._map[v1] = portName
                    if portType == "input": 
                        self._graph.addNet([portName, vertexName], {})
                    elif portType == "output": 
                        self._graph.addNet([vertexName, portName], {})
        # print(utils.dict2str(self._map))

        if len(used) < len(self._original.vertices()): 
            print("IsoMatcher: not enough matched vertices, ", len(used), "/", len(self._original.vertices()), "; FAILED. ")
            for vertex in self._original.vertices(): 
                if not vertex in used: 
                    print(" -> Unmatched vertex:", vertex)
            return False

        for vname, vertex in self._original.vertices().items(): 
            if vname in self._map: 
                for edge in self._original.netsOut()[vname]: 
                    nodes = [self._map[edge.fr()], ]
                    for node in edge.to(): 
                        if node in self._map: 
                            nodes.append(self._map[node])
                    if len(nodes) > 1: 
                        self._graph.addNet(nodes, {})
        
        return True


    def graph(self): 
        return self._graph

    def graphInfo(self): 
        return self._graph.info()

    def compat(self): 
        return self._compat

    def compatInfo(self): 
        info = ""
        for vertex, compats in self._compat.items(): 
            info += vertex
            for compat in compats: 
                info += " " + compat
            info += "\n"
        return info


class IsoMatcherDP(Base): 
    def __init__(self, graph, units): 
        self._original = graph
        self._units    = units
        self._patterns = {}
        self._matched  = []
        self._map      = {}
        self._graph    = HyperGraph()
        self._compat   = {}

    def match(self): 
        for uname, unit in self._units.items(): 
            self._patterns[uname] = {}
            for pname, patt in unit.patterns().items(): 
                self._patterns[uname][pname] = patt.graph()
                # print("Pattern: " + uname + "." + pname)
                # print(self._patterns[uname][pname].info())
                # print("Matching: " + uname + "." + pname)
                g1 = self._original.toNX()
                g2 = self._patterns[uname][pname].toNX()
                matcher = iso.DiGraphMatcher(g1, g2, lambda x, y: x["attrs"]["function"] == y["attrs"]["function"])
                # print(matcher.subgraph_is_isomorphic())
                isomorphisms = matcher.subgraph_isomorphisms_iter()
                for match in isomorphisms: 
                    self._matched.append((uname, pname, match, ))
                    # print(match)
        self._matched.sort(key=lambda x: (len(x[2]), -len(self._units[x[0]].patterns())), reverse=True)
        # print(self._original.info())
        # print(utils.list2str(self._matched))

        def calcValue(pack): 
            used = set()
            for match in pack: 
                for elem in match[2]: 
                    used.add(elem)
            return len(used)

        # print("Matched: ", len(self._matched))
        # print("Destination: ", len(self._original.vertices()))
        states = [[] for _ in range(len(self._matched))]
        for idx in range(len(self._matched)): 
            for jdx in range(len(states) - 1, 0, -1): 
                valueCurrent  = calcValue(states[jdx])
                stateNew      = states[jdx - 1] + [self._matched[idx], ]
                valueNew      = calcValue(stateNew)
                if valueCurrent < valueNew and len(stateNew) == jdx: 
                    # print("Substitute:", valueCurrent, "vs.", valueNew)
                    states[jdx] = stateNew
            # print(utils.list2str(states))
            # print("====================================================")
        states = sorted(states, key = lambda x: (calcValue(x), -len(x)), reverse = True)
        # values = list(map(lambda x: calcValue(x), states))
        # print(utils.list2str(values))
        # print(utils.list2str(states))
        result = states[0]
        if calcValue(result) < len(self._original.vertices()): 
            print("IsoMatcher: FAILED. Cannot match all vertices. ", calcValue(result), "/", len(self._original.vertices()))
            used = set()
            for match in result: 
                for elem in match[2]: 
                    used.add(elem)
            for vertex in self._original.vertices(): 
                if not vertex in used: 
                    print(" -> unmatched vertex:", vertex)
            return False

        used = set()
        for match in result: 
            uname = match[0]
            pname = match[1]
            info  = match[2]
            duplicated = False
            for v1, v2 in info.items(): 
                if v1 in used: 
                    duplicated = True
                    break
            assert not duplicated, "IsoMatcher: FAILED. Duplicated match. "
            for v1, v2 in info.items(): 
                used.add(v1)

            vertexName = ""
            for v1, v2 in info.items(): 
                if not "." in v1: 
                    vertexName += v1 + "_"
            vertexName = vertexName[:-1]
            self._graph.addVertex(vertexName, {"unit": uname, "pattern": pname})
            if not vertexName in self._compat: 
                self._compat[vertexName] = set()
            self._compat[vertexName].add(uname)
            for v1, v2 in info.items(): 
                portName = ""
                portType = ""
                for key, value in self._units[uname].pattern(pname).portMap().items(): 
                    if value == v2: 
                        portName = key
                        if portName in self._units[uname].inputs(): 
                            portType = "input"
                        elif portName in self._units[uname].outputs(): 
                            portType = "output"
                        else: 
                            assert portName in self._units[uname].inputs() or portName in self._units[uname].outputs(), "IsoMatcher: Invalid port: " + portName + " of " + uname
                if portName != "": 
                    temp = portName
                    portName = vertexName + "." + portName
                    self._graph.addVertex(portName, {"unit": uname + "." + temp})
                    self._map[v1] = portName
                    if portType == "input": 
                        self._graph.addNet([portName, vertexName], {})
                    elif portType == "output": 
                        self._graph.addNet([vertexName, portName], {})
        # print(utils.dict2str(self._map))

        if len(used) < len(self._original.vertices()): 
            print("IsoMatcher: not enough matched vertices, ", len(used), "/", len(self._original.vertices()), "; FAILED. ")
            for vertex in self._original.vertices(): 
                if not vertex in used: 
                    print(" -> Unmatched vertex:", vertex)
            return False

        for vname, vertex in self._original.vertices().items(): 
            if vname in self._map: 
                for edge in self._original.netsOut()[vname]: 
                    nodes = [self._map[edge.fr()], ]
                    for node in edge.to(): 
                        if node in self._map: 
                            nodes.append(self._map[node])
                    if len(nodes) > 1: 
                        self._graph.addNet(nodes, {})
        
        return True


    def graph(self): 
        return self._graph

    def graphInfo(self): 
        return self._graph.info()

    def compat(self): 
        return self._compat

    def compatInfo(self): 
        info = ""
        for vertex, compats in self._compat.items(): 
            info += vertex
            for compat in compats: 
                info += " " + compat
            info += "\n"
        return info

class TrivialIsoMatcher(Base): 
    def __init__(self, graph, units): 
        self._original = graph
        self._units    = units
        self._patterns = {}
        self._matched  = []
        self._map      = {}
        self._graph    = HyperGraph()
        self._compat   = {}

    def match(self): 
        for uname, unit in self._units.items(): 
            self._patterns[uname] = {}
            for pname, patt in unit.patterns().items(): 
                self._patterns[uname][pname] = patt.graph()
                # print("Pattern: " + uname + "." + pname)
                # print(self._patterns[uname][pname].info())
                # print("Matching: " + uname + "." + pname)
                g1 = self._original.toNX()
                g2 = self._patterns[uname][pname].toNX()
                matcher = iso.DiGraphMatcher(g1, g2, lambda x, y: x["attrs"]["function"] == y["attrs"]["function"])
                # print(matcher.subgraph_is_isomorphic())
                isomorphisms = matcher.subgraph_isomorphisms_iter()
                for match in isomorphisms: 
                    self._matched.append((uname, pname, match, ))
                    # print(match)
        self._matched.sort(key=lambda x: (len(x[2]), -len(self._units[x[0]].patterns())), reverse=True)
        # print(self._original.info())
        # print(utils.list2str(self._matched))
        
        used = set()
        for match in self._matched: 
            uname = match[0]
            pname = match[1]
            info  = match[2]
            duplicated = False
            for v1, v2 in info.items(): 
                if v1 in used: 
                    duplicated = True
                    break
            if duplicated: 
                continue
            for v1, v2 in info.items(): 
                used.add(v1)

            vertexName = ""
            for v1, v2 in info.items(): 
                if not "." in v1: 
                    vertexName += v1 + "_"
            vertexName = vertexName[:-1]
            self._graph.addVertex(vertexName, {"unit": uname, "pattern": pname})
            if not vertexName in self._compat: 
                self._compat[vertexName] = set()
            self._compat[vertexName].add(uname)
            for v1, v2 in info.items(): 
                portName = ""
                portType = ""
                for key, value in self._units[uname].pattern(pname).portMap().items(): 
                    if value == v2: 
                        portName = key
                        if portName in self._units[uname].inputs(): 
                            portType = "input"
                        elif portName in self._units[uname].outputs(): 
                            portType = "output"
                        else: 
                            assert portName in self._units[uname].inputs() or portName in self._units[uname].outputs(), "IsoMatcher: Invalid port: " + portName + " of " + uname
                if portName != "": 
                    temp = portName
                    portName = vertexName + "." + portName
                    self._graph.addVertex(portName, {"unit": uname + "." + temp})
                    self._map[v1] = portName
                    if portType == "input": 
                        self._graph.addNet([portName, vertexName], {})
                    elif portType == "output": 
                        self._graph.addNet([vertexName, portName], {})
        # print(utils.dict2str(self._map))

        if len(used) < len(self._original.vertices()): 
            print("IsoMatcher: FAILED. ")
            exit(1) 

        for vname, vertex in self._original.vertices().items(): 
            if vname in self._map: 
                for edge in self._original.netsOut()[vname]: 
                    nodes = [self._map[edge.fr()], ]
                    for node in edge.to(): 
                        if node in self._map: 
                            nodes.append(self._map[node])
                    if len(nodes) > 1: 
                        self._graph.addNet(nodes, {})

    def graph(self): 
        return self._graph

    def graphInfo(self): 
        return self._graph.info()

    def compat(self): 
        return self._compat

    def compatInfo(self): 
        info = ""
        for vertex, compats in self._compat.items(): 
            info += vertex
            for compat in compats: 
                info += " " + compat
            info += "\n"
        return info

if __name__ == "__main__": 
    archfile = sys.argv[1]
    ogfile = sys.argv[2]
    mapper = None
    if len(sys.argv) > 3: 
        mapper = sys.argv[3]

    og = HyperGraph()
    with open(ogfile, "r") as fin: 
        og.parse(fin.read())
    
    arch = Arch(archfile)
    rrg = HyperGraph()
    rrg.parse(arch.rrg())

    print(f"OpGraph read: {len(og.vertices())} vertices, {len(og.nets())} edges.")

    if mapper == "NSGAII": 
        mapper = IsoMatcherNSGAII(og, arch.units(), rrg)
    elif mapper == "DP": 
        mapper = IsoMatcherDP(og, arch.units())
    else: 
        mapper = IsoMatcherBFS(og, arch.units(), rrg)
    if mapper.match(): 
        with open(ogfile[:-6] + "DFG.txt", "w") as fout: 
            fout.write(mapper.graphInfo())
        with open(ogfile[:-6] + "compat.txt", "w") as fout: 
            fout.write(mapper.compatInfo())
