import sys 
sys.path.append(".")

import common.utils as utils
from common.utils import Base
import networkx as nx
import graphviz as gv

class Vertex(Base): 
    def __init__(self, name, attrs = {}): 
        self._name  = name
        self._attrs = attrs.copy()

    def name(self): 
        return self._name

    def attrs(self): 
        return self._attrs

    def addAttr(self, name, attr): 
        self._attrs[name] = attr

    def attr(self, name): 
        return self._attrs[name] if name in self._attrs else None

    def copy(self): 
        return Vertex(self._name, self._attrs)

    def info(self): 
        return "Vertex: " + self._name + "; Attributes: " + utils.dict2str(self._attrs) + "."

class Net(Base): 
    def __init__(self, nodes, attrs = {}): 
        assert len(nodes) > 1, "Net: At least two nodes"
        self._nodes = nodes
        self._fr    = nodes[0]
        self._to    = nodes[1:]
        self._attrs = attrs.copy()

    def nodes(self): 
        return self._nodes

    def fr(self): 
        return self._fr

    def to(self): 
        return self._to

    def attrs(self): 
        return self._attrs

    def attr(self, name): 
        return self._attrs[name] if name in self._attrs else None

    def copy(self): 
        return Net(self._nodes, self._attrs)

    def info(self): 
        return "Net: " + self._fr + " -> " + self._to + "; Attributes: " + utils.dict2str(self._attrs) + "."

class HyperGraph(Base): 
    MAX = 1024*1024
    def __init__(self, filename=None): 
        self._vertices = {}
        self._netsIn  = {}
        self._netsOut = {}
        self._attrs   = {}
        if not filename is None: 
            self.parse(utils.readfile(filename))
    
    def vertex(self, name): 
        return self._vertices[name] if name in self._vertices else None
    
    def vertices(self): 
        return self._vertices
    
    def netsIn(self, name = ""): 
        if len(name) == 0: 
            return self._netsIn
        return self._netsIn[name] if name in self._netsIn else None
    
    def netsOut(self, name = ""): 
        if len(name) == 0: 
            return self._netsOut
        return self._netsOut[name] if name in self._netsOut else None

    def nets(self): 
        results = []
        for vertex in self._vertices: 
            results.extend(self._netsOut[vertex])
        return results

    def attrs(self): 
        return self._attrs

    def attr(self, name): 
        return self._attrs[name] if name in self._attrs else None

    def addVertex(self, name, attrs = {}): 
        self._vertices[name] = Vertex(name, attrs)
        if not name in self._netsIn: 
            self._netsIn[name] = []
        if not name in self._netsOut: 
            self._netsOut[name] = []

    def addNet(self, nodes, attrs = {}): 
        for node in nodes: 
            assert node in self._vertices, "HyperGraph: Invalid net node: " + node
        self._netsOut[nodes[0]].append(Net(nodes, attrs))
        for node in nodes[1:]: 
            self._netsIn[node].append(Net(nodes, attrs))

    def parse(self, info): 
        def evalAttr(typeAttr, content): 
            if typeAttr == "int": 
                return int(content)
            elif typeAttr == "str": 
                return str(content)
            elif typeAttr == "float": 
                return float(content)
            elif typeAttr == "arr": 
                result = content[1:-1]
                result = result.split(", ")
                if len(result[-1]) == 0: 
                    result = result[:-1]
                return result
            elif typeAttr == "vec": 
                result = content[1:-1]
                result = result.split(", ")
                if len(result[-1]) == 0: 
                    result = result[:-1]
                for idx in range(len(result)): 
                    result[idx] = float(result[idx])
                return result
            return str(content)
        lines = info.split("\n")
        index = 0
        while index < len(lines): 
            line = lines[index]
            splited = line.split()
            if len(splited) > 1 and splited[0].lower() == "attr": 
                temp = index
                tempSplited = splited
                while len(tempSplited) == 0 or lines[temp][0] == "#" or tempSplited[0].lower() == "attr": 
                    if temp >= len(lines): 
                        break
                    if len(tempSplited) == 0 or lines[temp][0] == "#": 
                        temp += 1
                        tempSplited = lines[temp].split() if temp < len(lines) else []
                        continue
                    if tempSplited[0].lower() == "attr": 
                        attrName  = tempSplited[1]
                        attrType  = tempSplited[2]
                        attrValue = " ".join(tempSplited[3:])
                        attrValue = evalAttr(attrType, attrValue)
                        self._attrs[attrName] = attrValue
                    temp += 1
                    tempSplited = lines[temp].split() if temp < len(lines) else []
                index = temp
            elif len(splited) == 0 or lines[index][0] == "#": 
                index += 1
                continue
            else: 
                break

        while index < len(lines): 
            line = lines[index]
            splited = line.split()
            if len(splited) > 1 and splited[0].lower() == "vertex": 
                name  = splited[1]
                attrs = {}
                temp = index + 1
                tempSplited = lines[temp].split() if temp < len(lines) else []
                while len(tempSplited) == 0 or lines[temp][0] == "#" or tempSplited[0].lower() == "attr": 
                    if temp >= len(lines): 
                        break
                    if len(tempSplited) == 0 or lines[temp][0] == "#": 
                        temp += 1
                        tempSplited = lines[temp].split() if temp < len(lines) else []
                        continue
                    if tempSplited[0].lower() == "attr": 
                        attrName  = tempSplited[1]
                        attrType  = tempSplited[2]
                        attrValue = " ".join(tempSplited[3:])
                        attrValue = evalAttr(attrType, attrValue)
                        attrs[attrName] = attrValue
                    temp += 1
                    tempSplited = lines[temp].split() if temp < len(lines) else []
                self.addVertex(name, attrs)
                index = temp
            if (len(line) > 3 and line[0:3].lower() == "net") or (len(line) > 4 and line[0:4].lower() == "edge"): 
                nodes = splited[1:]
                attrs = {}
                temp = index + 1
                tempSplited = lines[temp].split() if temp < len(lines) else []
                while len(tempSplited) == 0 or lines[temp][0] == "#" or tempSplited[0].lower() == "attr": 
                    if temp >= len(lines): 
                        break
                    if len(tempSplited) == 0 or lines[temp][0] == "#": 
                        temp += 1
                        tempSplited = lines[temp].split() if temp < len(lines) else []
                        continue
                    if tempSplited[0].lower() == "attr": 
                        attrName  = tempSplited[1]
                        attrType  = tempSplited[2]
                        attrValue = " ".join(tempSplited[3:])
                        attrValue = evalAttr(attrType, attrValue)
                        attrs[attrName] = attrValue
                    temp += 1
                    tempSplited = lines[temp].split() if temp < len(lines) else []
                self.addNet(nodes, attrs)
                index = temp

    def getMatrix(self): 
        names  = []
        name2index = {}
        result = []
        for vertex in self._vertices: 
            name2index[vertex] = len(names)
            names.append(vertex)
            result.append([0 for _ in self._vertices])
        for vertex in self._vertices: 
            for net in self._netsOut[vertex]: 
                for sink in net.to(): 
                    result[name2index[vertex]][name2index[sink]] = 1
        return names, result

    def copy(self): 
        newgraph = HyperGraph()
        newgraph._vertices = {}
        newgraph._netsIn  = {}
        newgraph._netsOut = {}
        for name in self._vertices: 
            newgraph._vertices[name] = self._vertices[name].copy()
            newgraph._netsIn[name] = []
            newgraph._netsOut[name] = []
            for net in self._netsIn[name]: 
                newgraph._netsIn[name].append(net.copy())
            for net in self._netsOut[name]: 
                newgraph._netsOut[name].append(net.copy())
        return newgraph

# dump graph to result, can print it to txt
    def info(self, prefix = ""): 
        def infoAttr(attr): 
            typeAttr = "str "
            valAttr = str(attr)
            if isinstance(attr, int): 
                typeAttr = "int "
                valAttr = str(attr)
            elif isinstance(attr, tuple) or isinstance(attr, list): 
                isvec = True
                for value in attr: 
                    if isinstance(value, str): 
                        isvec = False
                        break
                if isvec: 
                    typeAttr = "vec "
                    valAttr = "("
                    for value in attr: 
                        valAttr += str(value) + ", "
                    valAttr += ")"
                else: 
                    typeAttr = "arr "
                    valAttr = "("
                    for value in attr: 
                        valAttr += str(value) + ", "
                    valAttr += ")"
            return typeAttr + valAttr
            
        result = ""
        for nameAttr in self.attrs(): 
            result += "attr " + nameAttr + " " + infoAttr(self.attr(nameAttr)) + "\n"
        for name in self._vertices: 
            vertex = self._vertices[name]
            result += "vertex " + vertex.name() + "\n"
            for nameAttr in vertex.attrs(): 
                result += "    attr " + nameAttr + " " + infoAttr(vertex.attr(nameAttr)) + "\n"
        for vertex in self._vertices: 
            assert vertex in self._netsOut, "HyperGraph: Invalid net source: " + vertex
            for net in self._netsOut[vertex]: 
                result += "net " + " ".join(net.nodes()) + "\n"
                for nameAttr in net.attrs(): 
                    result += "    attr " + nameAttr + " " + infoAttr(net.attr(nameAttr)) + "\n"
        return result

    def dump(self, filename): 
        with open(filename, "w") as fout: 
            fout.write(self.info())

# generate a new graph with the prefixed you want
    def prefixed(self, prefix = ""): 
        def addPrefix(name): 
            if len(prefix) == 0: 
                return name
            if prefix in name:
                return name
            return prefix + "." + name
        def addPrefixes(names, prefix = ""): 
            results = []
            for name in names: 
                results.append(addPrefix(name))
            return results
        newgraph = HyperGraph()
        for name in self._vertices: 
            namePrefixed = addPrefix(name)
            newgraph._vertices[namePrefixed] = self._vertices[name].copy()
            newgraph._vertices[namePrefixed]._name = namePrefixed
            newgraph._netsIn[namePrefixed] = []
            newgraph._netsOut[namePrefixed] = []
        for name in self._vertices: 
            assert name in self._netsOut, "HyperGraph: Invalid net source: " + name 
            namePrefixed = addPrefix(name)
            for idx in range(len(self._netsIn[name])): 
                newgraph._netsIn[namePrefixed].append(Net(addPrefixes(self._netsIn[name][idx].nodes()), self._netsIn[name][idx].attrs()))
            for idx in range(len(self._netsOut[name])): 
                newgraph._netsOut[namePrefixed].append(Net(addPrefixes(self._netsOut[name][idx].nodes()), self._netsOut[name][idx].attrs()))
        return newgraph

# extend a graph
    def extend(self, graph): 
        for key in graph.vertices(): 
            if not key in self._vertices: 
                self._vertices[key] = graph.vertices()[key].copy()
                self._netsIn[key]  = []
                self._netsOut[key] = []
            else: 
                print("HyperGraph: Warning: duplicated vertex " + key + ": ignored")
        for key in graph.vertices(): 
            for net in graph.netsIn(key): 
                self._netsIn[key].append(net.copy())
            for net in graph.netsOut(key): 
                self._netsOut[key].append(net.copy())

# Topological sort
    def topologic(self): 
        used = set()
        index = {}
        que = []
        for vertex in self._vertices: 
            if (len(self._netsIn[vertex]) == 0) or (len(self._netsIn[vertex]) == 1 and self._netsIn[vertex][0].fr() == vertex): 
                que.append(vertex)
                used.add(vertex)
                index[vertex] = 0
        while not len(que) == 0: 
            tmp = que[0]
            que = que[1:]
            for net in self._netsOut[tmp]: 
                for sink in net.to(): 
                    if sink == tmp: 
                        continue
                    if not sink in used: 
                        que.append(sink)
                        used.add(sink)
                        index[sink] = index[tmp] + 1
                    else: 
                        index[sink] = max(index[sink], index[tmp] + 1)
        seq = sorted(list(self._vertices.keys()), key = lambda x: index[x])
        assert(len(seq) == len(list(self._vertices.keys())))

        results = {}
        for name in seq: 
            results[name] = index[name]
        return results

# Adjacent matrix
    def adjMat(self): 
        vertices = list(self._vertices.keys())
        name2index = {}
        for idx in range(len(vertices)): 
            name2index[vertices[idx]] = idx
        mat = [[HyperGraph.MAX for _ in range(len(vertices))] for _ in range(len(vertices))]
        for vertex in vertices: 
            index = name2index[vertex]
            mat[index][index] = 0
            for net in self._netsOut[vertex]: 
                for to in net.to(): 
                    jndex = name2index[to]
                    mat[index][jndex] = 1
        return mat

# Distance matrix directional
    def distMatDi(self): 
        vertices = list(self._vertices.keys())
        name2index = {}
        for idx in range(len(vertices)): 
            name2index[vertices[idx]] = idx
        adjMat = self.adjMat()
        distMat = adjMat.copy()
        for kdx in range(len(distMat)): 
            for idx in range(len(distMat)): 
                for jdx in range(len(distMat)): 
                    if distMat[idx][jdx] > distMat[idx][kdx] + distMat[kdx][jdx]: 
                        distMat[idx][jdx] = distMat[idx][kdx] + distMat[kdx][jdx]
        return distMat

# Distance matrix
    def distMat(self): 
        vertices = list(self._vertices.keys())
        name2index = {}
        for idx in range(len(vertices)): 
            name2index[vertices[idx]] = idx
        adjMat = self.adjMat()
        distMat = adjMat.copy()
        for idx in range(len(distMat)): #Optional?
            for jdx in range(len(distMat)): 
                distMat[idx][jdx] = min(distMat[idx][jdx], distMat[jdx][idx])
        for kdx in range(len(distMat)): 
            for idx in range(len(distMat)): 
                for jdx in range(len(distMat)): 
                    if distMat[idx][jdx] > distMat[idx][kdx] + distMat[kdx][jdx]: 
                        distMat[idx][jdx] = distMat[idx][kdx] + distMat[kdx][jdx]
        for idx in range(len(distMat)): 
            for jdx in range(len(distMat)): 
                distMat[idx][jdx] = min(distMat[idx][jdx], distMat[jdx][idx])
        return distMat

    def toNX(self): 
        g = nx.DiGraph()
        for vname, vertex in self._vertices.items(): 
            g.add_node(vertex.name(), attrs=vertex.attrs())
        for vname, nets in self._netsOut.items(): 
            for net in nets: 
                for node in net.to(): 
                    g.add_edge(net.fr(), node)
        return g

    def toGV(self): 
        g = gv.Digraph('cluster_graph', engine= 'dot')
        for vname, vertex in self._vertices.items(): 
            g.node(vertex.name())
        for vname, nets in self._netsOut.items(): 
            for net in nets: 
                for node in net.to(): 
                    g.edge(net.fr(), node)
        return g
