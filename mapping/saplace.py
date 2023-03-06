import sys
import random
import math
from queue import Queue
sys.path.append(".")

import numpy as np
import networkx as nx
from networkx.algorithms import isomorphism as iso
from networkx.algorithms import clique

import common.utils as utils
from common.utils import Base
from common.graph import HyperGraph
from arch.protocols import *

class CliqueRouter(Base): 

    # MAXNUM = 1
    # MAXLEN = 8
    MAXNUM = 16
    MAXLEN = 16

    def __init__(self, RRG): 
        self._RRG = RRG
        self._placement = {}
        self._links = {}
        self._paths = {}

    def routeDFG(self, DFG, placement): 
        self._DFG = DFG
        self._placement = placement

        links = []
        for v1, v2 in self._placement.items(): 
            assert v1 in self._DFG.vertices(), v1 + " cannot be found. "
            assert v2 in self._RRG.vertices(), v2 + " cannot be found. "
        for vertex in self._DFG.vertices().keys(): 
            assert vertex in self._placement
            for net in self._DFG.netsOut(vertex): 
                fr = net.fr()
                for to in net.to(): 
                    if not (fr in to or to in fr): 
                        links.append((self._placement[fr], self._placement[to]))
        
        return self.route(links)

    def route(self, links): 
        
        for idx, link in enumerate(links): 
            self._find(link)
            if len(self._links[link]) == 0: 
                # print(" -> Unable to route", link)
                name = link[0] + "->" + link[1] + "--__NONE__"
                self._links[link] = [name, ]
                self._paths[name] = None
                                
        vertices = []
        edges = []
        sources = {}
        sinks = {}
        for idx in range(len(links)): 
            # print(links[idx], ": ", len(paths[idx]))
            assert(len(self._links[links[idx]]) > 0)
            for jdx in range(len(self._links[links[idx]])): 
                name = self._links[links[idx]][jdx]
                vertices.append(name)
                sources[name] = links[idx][0]
                sinks[name]   = links[idx][1]

        addedNets = set()
        for v1 in vertices: 
            for v2 in vertices: 
                if "__NONE__" in v1 or "__NONE__" in v2: 
                    continue
                if sources[v1] == sources[v2]: 
                    if sinks[v1] != sinks[v2]: 
                        if not v1 + "->" + v2 in addedNets and not v2 + "->" + v1 in addedNets: 
                            edges.append([v1, v2])
                            addedNets.add(v1 + "->" + v2)
                else: 
                    conflict = False
                    for vertex1 in self._paths[v1]: 
                        for vertex2 in self._paths[v2]: 
                            if vertex1 == vertex2: 
                                conflict = True
                            if conflict: 
                                break
                        if conflict: 
                            break
                    if not conflict: 
                        if not v1 + "->" + v2 in addedNets and not v2 + "->" + v1 in addedNets: 
                            edges.append([v1, v2])
                            addedNets.add(v1 + "->" + v2)

        graph = nx.Graph()
        graph.add_nodes_from(vertices)
        graph.add_edges_from(edges)

        maxcliq, size = clique.max_weight_clique(graph, weight=None)
        # print(size, "/", len(links))

        self._results = {}
        if size == len(links): 
            for name in maxcliq: 
                source = sources[name]
                sink   = sinks[name]
                self._results[(source, sink)] = self._paths[name]
        
        return len(links) - size
    
    def results(self): 
        return self._results

    def _find(self, link): 
        links = []
        paths = {}

        if link in self._links: 
            tmps = self._links[link]
            for tmp in tmps: 
                paths[tmp] = self._paths[tmp]
            return paths

        que = Queue()
        que.put([link[0], ])
        while que.qsize() > 0 and len(paths) < CliqueRouter.MAXNUM: 
            curr = que.get()
            tail = curr[-1]
            for net in self._RRG.netsOut(tail): 
                for to in net.to(): 
                    if self._RRG.vertex(to).attr("type") == "__MODULE_INPUT_PORT__" or \
                        self._RRG.vertex(to).attr("type") == "__MODULE_OUTPUT_PORT__" or \
                        to == link[1]: 
                        tmp = curr + [to, ]
                        if to == link[1]: 
                            name = link[0] + "->" + link[1] + "--" + str(len(paths))
                            links.append(name)
                            paths[name] = tmp
                        elif len(tmp) < CliqueRouter.MAXLEN: 
                            que.put(tmp)

        self._links[link] = links
        for key, value in paths.items(): 
            self._paths[key] = value
        return paths

        
        

class MazeRouter(Base): 

    # MAXNUM = 1
    # MAXLEN = 8
    MAXNUM = 16
    MAXLEN = 16

    def __init__(self, RRG, patience=256): 
        self._RRG = RRG
        self._patience = patience
        self._links = {}
        self._paths = {}
        self._results = {}

    def routeDFG(self, DFG, placement): 
        toRoute = []
        for fr in DFG.vertices().keys(): 
            for net in DFG.netsOut(fr): 
                for to in net.to(): 
                    frSplited = fr.split(".")
                    toSplited = to.split(".")
                    frFU = placement[frSplited[0]]
                    toFU = placement[toSplited[0]]
                    frRRG = frFU + "." + frSplited[1] if len(frSplited) > 1 else frFU
                    toRRG = toFU + "." + toSplited[1] if len(toSplited) > 1 else toFU
                    if not (frSplited[0] == to or toSplited[0] == fr): 
                        toRoute.append((frRRG, toRRG))
                    else: 
                        assert len(frSplited) == 1 or len(toSplited) == 1
        return self.route(toRoute)

    def route(self, links): 
        candidates = {}
        for link in links: 
            # print(" -> Finding", link)
            paths = self._find(link)
            candidates[link] = sorted(list(paths.keys()), key=lambda x: len(x))

        # Figure out the compatibility of the links
        conflicts = set()
        allLinks = []
        sources = {}
        sinks = {}
        for link in links: 
            if len(self._links[link]) == 0: 
                # print(" -> Unable to route", link)
                name = link[0] + "->" + link[1] + "--__NONE__"
                self._links[link] = [name, ]
                self._paths[name] = None
                allLinks.append(name)
                sources[name] = link[0]
                sinks[name] = link[1]
            for name in self._links[link]: 
                allLinks.append(name)
                sources[name] = link[0]
                sinks[name] = link[1]
        for v1 in allLinks: 
            for v2 in allLinks: 
                if "__NONE__" in v1 or "__NONE__" in v2: 
                    conflicts.add((v1, v2))
                    conflicts.add((v2, v1))
                elif sources[v1] == sources[v2]: 
                    if sinks[v1] == sinks[v2]: 
                        conflicts.add((v1, v2))
                        conflicts.add((v2, v1))
                else: 
                    conflict = False
                    for vertex1 in self._paths[v1]: 
                        for vertex2 in self._paths[v2]: 
                            if vertex1 == vertex2: 
                                conflict = True
                            if conflict: 
                                break
                        if conflict: 
                            break
                    if conflict: 
                        conflicts.add((v1, v2))
                        conflicts.add((v2, v1))

        # Initialization, all links use the first candidates
        status = {}
        for link in links: 
            status[link] = 0

        # Iterations
        self._counts = {}
        def countConflicts(): 
            results = 0
            self._counts = {}
            for key1, value1 in status.items(): 
                assert key1 in self._links, key1 + " not found"
                name1 = self._links[key1][value1]
                self._counts[key1] = 0
                for key2, value2 in status.items(): 
                    assert key2 in self._links, key2 + " not found"
                    name2 = self._links[key2][value2]
                    if name1 != name2 and (name1, name2) in conflicts: 
                        results += 1
                        self._counts[key1] += 1
            return results / 2
        
        failures = 0
        numConflicts = []
        numConflicts.append(countConflicts())
        while numConflicts[-1] > 0 and failures < self._patience: 
            # print(" -> Iteration", failures, "Conflicts", countConflicts())
            failures += 1
            
            seq = sorted(list(status.keys()), key=lambda x: self._counts[x], reverse=True)
            tried = False
            okay1 = False
            for idx, link in enumerate(seq): 
                if status[link] + 1 < len(self._links[link]): 
                    for jdx in range(status[link] + 1, len(self._links[link])): 
                        tmp = 0
                        for link2, kdx in status.items(): 
                            name1 = self._links[link][jdx]
                            name2 = self._links[link2][kdx]
                            if not (name1, name2) in conflicts: 
                                okay1 = True
                                tried = True
                                status[link] = jdx
                                break
                        if okay1: 
                            break
                    if okay1: 
                        break
            if not tried: 
                for idx, link in enumerate(seq): 
                    if status[link] + 1 < len(self._links[link]): 
                        if status[link] + 1 < len(self._links[link]): 
                            tried = True
                            status[link] += 1
                            break
            if not tried: 
                break
            else: 
                numConflicts.append(countConflicts())

        self._results = {}
        if numConflicts[-1] == 0: 
            for link, index in status.items(): 
                self._results[link] = self._paths[self._links[link][index]]
            return 0
        else: 
            return min(numConflicts)
    
    def results(self): 
        return self._results

    def _find(self, link): 
        links = []
        paths = {}

        if link in self._links: 
            tmps = self._links[link]
            for tmp in tmps: 
                paths[tmp] = self._paths[tmp]
            return paths

        que = Queue()
        que.put([link[0], ])
        while que.qsize() > 0 and len(paths) < CliqueRouter.MAXNUM: 
            curr = que.get()
            tail = curr[-1]
            for net in self._RRG.netsOut(tail): 
                for to in net.to(): 
                    if self._RRG.vertex(to).attr("type") == "__MODULE_INPUT_PORT__" or \
                        self._RRG.vertex(to).attr("type") == "__MODULE_OUTPUT_PORT__" or \
                        to == link[1]: 
                        tmp = curr + [to, ]
                        if to == link[1]: 
                            name = link[0] + "->" + link[1] + "--" + str(len(paths))
                            links.append(name)
                            paths[name] = tmp
                        elif len(tmp) < CliqueRouter.MAXLEN: 
                            que.put(tmp)

        self._links[link] = links
        for key, value in paths.items(): 
            self._paths[key] = value
        return paths

class AnnealingPlacer: 

    def __init__(self, RRG: HyperGraph, fixed: dict = {}, patience: int = 1812, temper: float = 0.5, early: int = 512, maxDist: int = 8, silent: bool = False): 
        self._silent = silent
        self._temper = temper
        self._early  = early
        self._patience = patience
        self._maxDist = maxDist
        self._fixed = fixed
        self._RRG = RRG
        self._router = MazeRouter(self._RRG)
        # self._router = CliqueRouter(self._RRG)
        self._placement = {}
        self._routing = {}

        self._types = {}
        self._dmys = {}
        self._mems = {}
        self._ios = {}
        self._categories = {}
        self._grid = {}
        self._gridFUs = {}
        self._fuTypes = {}
        self._fu2coords = {}
    
    def place(self, DFG: HyperGraph, compat: dict, init: dict = {}, fixed: dict = {}): 
        self._DFG = DFG
        self._contracted = HyperGraph()
        self._compat = compat
        self._fixed = fixed
        self._placement = {}
        self._routing = {}
        self._ops = {}
        self._opList = []
        self._numOps = 0
        self._name2index = {}
        for name, vertex in self._DFG.vertices().items(): 
            if len(name.split(".")) == 1: 
                if not name in self._fixed: 
                    self._ops[name] = vertex.attr("optype")
                    self._opList.append(name)
                    self._name2index[name] = len(self._name2index)
                    self._numOps += 1 
                self._contracted.addVertex(name)
        for vname, vertex in self._DFG.vertices().items(): 
            for net in self._DFG.netsOut(vname): 
                source = net.fr().split(".")[0]
                sink   = net.to()[0].split(".")[0]
                if net.fr().split(".")[0] == net.fr() or net.to()[0].split(".")[0] == net.to()[0]: 
                    continue
                self._contracted.addNet([source, sink])
        self._distMat = self._contracted.distMat()

        return self.solve(init)

    def solve(self, init: dict = {}): 
        candidates = {}
        for op in self._opList: 
            if not op in candidates: 
                candidates[op] = []
            for name, vertex in self._RRG.vertices().items(): 
                if vertex.attr("device") in self._compat[op]: 
                    candidates[op].append(name)

        ops = self._opList.copy()
        random.shuffle(ops)
        
        # Initialization
        places = self._fixed.copy()
        used = set()
        count = 0
        for op in ops: 
            pos = random.randint(0, len(candidates[op]) - 1)
            while candidates[op][pos] in used: 
                pos = random.randint(0, len(candidates[op]) - 1)
                count += 1
                if count > 1024: 
                    print("FAILED: Not enough candidates:", op, candidates[op], self._fixed, places)
                    return {}, {}
            places[op] = candidates[op][pos]
            used.add(candidates[op][pos])
        if len(init) > 0: 
            for v1, v2 in init.items(): 
                if len(v1.split(".")) > 1: 
                    continue
                assert v1 in self._fixed or v2 in candidates[v1]
                places[v1] = v2

        def getToRoute(): 
            toRoute = []
            for fr in self._DFG.vertices().keys(): 
                for net in self._DFG.netsOut(fr): 
                    for to in net.to(): 
                        frSplited = fr.split(".")
                        toSplited = to.split(".")
                        frFU = places[frSplited[0]]
                        toFU = places[toSplited[0]]
                        frRRG = frFU + "." + frSplited[1] if len(frSplited) > 1 else frFU
                        toRRG = toFU + "." + toSplited[1] if len(toSplited) > 1 else toFU
                        if not (frSplited[0] == to or toSplited[0] == fr): 
                            toRoute.append((frRRG, toRRG))
                        else: 
                            assert len(frSplited) == 1 or len(toSplited) == 1
            return toRoute
        
        temper = self._temper
        iter = 0
        lastImp = 0
        last = self._router.route(getToRoute())
        if not self._silent: 
            print("Routing conflicts:", last)
        while last > 0: 
            if iter > self._patience: 
                return {}, {}

            backup = places.copy()

            index = random.randint(0, len(self._opList) - 1)
            op = self._opList[index]
            count = 0
            while len(candidates[op]) <= 1: 
                index = random.randint(0, len(self._opList) - 1)
                op = self._opList[index]
                count += 1
                if count > 1024: 
                    print("FAILED: Not enough candidates. ")
                    return {}, {}
            
            op = self._opList[index]
            jndex = -1
            for idx, candidate in enumerate(candidates[op]): 
                if places[op] == candidate: 
                    jndex = idx
                    break
            assert jndex > -1

            kndex = random.randint(0, len(candidates[op]) - 1)
            fu = candidates[op][kndex]
            count = 0
            while jndex == kndex: 
                kndex = random.randint(0, len(candidates[op]) - 1)
                fu = candidates[op][kndex]
                count += 1
                if count > 1024: 
                    print("FAILED: Not enough candidates. ")
                    return {}, {}

            op2 = ""
            fu2 = places[op]
            for name in list(places.keys()): 
                if name != op and places[name] == fu: 
                    op2 = name

            if op2 == "": 
                places[op] = fu
            else: 
                places[op2] = fu2
                places[op] = fu

            conflicts = self._router.route(getToRoute())
            prob = math.exp((last - conflicts) / temper)
            if not self._silent: 
                print("\rIteration:", iter, "; conflicts:", conflicts, "vs.", last, "; prob:", str(prob)[0:min(4, len(str(prob)))], "; T =", str(temper)[0:min(4, len(str(temper)))], end="")
            # else: 
            #     print("\rIteration:", iter, "; conflicts:", conflicts, "vs.", last, "; prob:", str(prob)[0:min(4, len(str(prob)))], "; T =", str(temper)[0:min(4, len(str(temper)))], end="")

            if conflicts < last: 
                lastImp = iter
            elif iter - lastImp > self._early: 
                if not self._silent: 
                    print("FAILED: Early stopping. ")
                return {}, {}
            iter += 1
            temper *= 0.9995

            if conflicts == 0: 
                if not self._silent: 
                    print(" -> OK", end="")
                break
            elif conflicts < last: 
                if not self._silent: 
                    print(" -> Accepted", end="")
                last = conflicts
                continue
            elif conflicts == last: 
                if random.randint(0, 1) == 0: 
                    if not self._silent: 
                        print(" -> Accepted", end="")
                    last = conflicts
                    continue
                else: 
                    if not self._silent: 
                        print(" -> Rejected", end="")
                    places = backup
                    continue
            else: 
                delta = conflicts - last
                if random.random() < math.exp((-delta) / temper): 
                    if not self._silent: 
                        print(" -> Accepted", end="")
                    last = conflicts
                    continue
                else: 
                    if not self._silent: 
                        print(" -> Rejected", end="")
                    places = backup
                    continue

        self._placement = {}
        self._routing = {}
        
        for vertex in self._DFG.vertices(): 
            splited = vertex.split(".")
            fu = places[splited[0]]
            if len(splited) > 1: 
                fu += "." + splited[1]
            self._placement[vertex] = fu

        for link, path in self._router.results().items(): 
            self._routing[link] = path

        # print(utils.dict2str(self._placement))
        # print(utils.dict2str(self._routing))

        # Validate 
        used = set()
        for vertex in self._DFG.vertices(): 
            if not vertex in self._placement: 
                print("ERROR: Unmapped vertex:", vertex)
            else: 
                if self._placement[vertex] in used: 
                    print("ERROR: Overused RRG FU:", self._placement[vertex])
                else: 
                    used.add(self._placement[vertex])
                    
        for vertex in self._DFG.vertices(): 
            for net in self._DFG.netsOut(vertex): 
                if self._placement[net.fr()] in self._placement[net.to()[0]] or self._placement[net.to()[0]] in self._placement[net.fr()]: 
                    continue
                if not (self._placement[net.fr()], self._placement[net.to()[0]]) in self._routing: 
                    print("ERROR: Unrouted edge:", (self._placement[net.fr()], self._placement[net.to()[0]]))

        for link1, path1 in self._routing.items(): 
            for link2, path2 in self._routing.items(): 
                if link1[0] != link2[0]: 
                    used = set(path1)
                    for vertex in path2: 
                        if vertex in used: 
                            print("ERROR: Overused RRG vertex:", vertex, "in", link1, "and", link2)

        if not self._silent: 
            print("")
        return self._placement, self._routing


if __name__ == "__main__": 
    rrgfile = sys.argv[1]
    dfgfile = sys.argv[2]
    compatfile = sys.argv[3]

    rrg    = HyperGraph(rrgfile)
    dfg    = HyperGraph(dfgfile)
    compat = utils.readCompat(compatfile)
    print("Data loaded. ")
    placer = AnnealingPlacer(rrg, patience=1024*16, early=1024*4, temper=0.5, silent=False)
    print("Begin placing. ")
    placement, routing = placer.place(dfg, compat)

