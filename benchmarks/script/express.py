import sys 
sys.path.append(".")

import common.utils as utils
from common.utils import Base
from common.graph import HyperGraph

def readArchFile(filename): 
    with open(filename, "r") as fin: 
        lines = fin.readlines()
    lines = list(filter(lambda x: len(x.strip()) > 0, lines))
    lines = list(map(lambda x: x.strip().split(";")[0], lines))
    while not "digraph" in lines[0]: 
        lines = lines[1:]
    lines = lines[1:-1]
    #list(map(lambda x: print(x), lines))
    return lines

def parseDFG(lines): 

    lut = {
        "mul": "__MUL__", 
        "div": "__DIV__", 
        "add": "__ADD__", 
        "sub": "__SUB__", 
        "shl": "__LSHIFT__", 
        "shrl": "__RSHIFT__", 
        "shra": "__RSHIFT__", 
        "or": "__OR__", 
        "xor": "__XOR__", 
        "neg": "__SUB__", 
        "asr": "__RSHIFT__", 
        "lsl": "__LSHIFT__", 
        "const": "__CONST__", 
        "input": "__INPUT__", 
        "imp": "__INPUT__", 
        "output": "__OUTPUT__", 
        "exp": "__OUTPUT__", 
        "load": "__MEMORY__", 
        "lod": "__MEMORY__", 
        "store": "__MEMORY__", 
        "str": "__MEMORY__", 
        "memr": "__MEMORY__", 
        "memw": "__MEMORY__"
    }

    vertexLines = []
    edgeLines = []
    for line in lines: 
        if len(line.split("->")) > 1: 
            edgeLines.append(line)
        elif "label" in line: 
            vertexLines.append(line)
    
    vertices = set()
    edges = []
    compat = {}
    edgeNames = set()
    ports = {}
    for line in vertexLines: 
        splited = line.split("[")
        vertexName = splited[0].strip()
        opcode = splited[1].split("=")[1][:-1].strip().lower()
        vertices.add(vertexName)
        compat[vertexName] = opcode
        vertices.add(vertexName + ".out0")
        compat[vertexName + ".out0"] = opcode + ".out0"
    for line in edgeLines: 
        splited = line.split("->")
        fromVertex = splited[0].strip() + ".out0"
        prefix = splited[1].split("[")[0].strip()
        if not prefix in ports: 
            ports[prefix] = []
        index = str(len(ports[prefix]))
        ports[prefix].append(fromVertex)
        toVertex = prefix + ".in" + index
        edgeName = fromVertex + "->" + toVertex
        if not toVertex in vertices: 
            vertices.add(toVertex)
            compat[toVertex] = compat[prefix] + ".in" + index
        if not edgeName in edgeNames: 
            edges.append((fromVertex, toVertex))
            edgeNames.add(edgeName)

    op2inports = {}
    for vertex in vertices: 
        if len(vertex.split(".")) == 1: 
            op2inports[vertex] = []
        else: 
            prefix = vertex.split(".")[0]
            port   = vertex.split(".")[1]
            if not prefix in op2inports: 
                op2inports[prefix] = []
            if port[0:2] == "in": 
                op2inports[prefix].append(port)
    
    for op in op2inports: 
        prefixCompat = compat[op]
        assert prefixCompat in lut, "Illegal compat: " + str(op) + "->" + prefixCompat
        func = lut[prefixCompat]
        if (not func in ["__INPUT__", "__OUTPUT__", "__CONST__", "__MEMORY__"]) and len(op2inports[op]) < 2: 
            while len(op2inports[op]) < 2: 
                portname = "in" + str(len(op2inports[op]))
                vertices.add(op + "." + portname)
                edges.append((op + "." + portname, op))
                compat[op + "." + portname] = prefixCompat + "." + portname
                op2inports[op].append(portname)
        elif (func in ["__INPUT__", "__OUTPUT__", "__CONST__"]) and len(op2inports[op]) < 1: 
            while len(op2inports[op]) < 1: 
                portname = "in" + str(len(op2inports[op]))
                vertices.add(op + "." + portname)
                edges.append((op + "." + portname, op))
                compat[op + "." + portname] = prefixCompat + "." + portname
                op2inports[op].append(portname)
        elif (func in ["__MEMORY__", ]) and len(op2inports[op]) < 3: 
            while len(op2inports[op]) < 3: 
                portname = "in" + str(len(op2inports[op]))
                vertices.add(op + "." + portname)
                edges.append((op + "." + portname, op))
                compat[op + "." + portname] = prefixCompat + "." + portname
                op2inports[op].append(portname)

    for vertex in vertices: 
        splited = vertex.split(".")
        if len(splited) >= 2 and "in" in splited[1]: 
            edgeName = vertex + "->" + splited[0]
            if not edgeName in edgeNames: 
                edges.append((vertex, splited[0]))
                edgeNames.add(edgeName)
        elif len(splited) >= 2 and "out" in splited[1]: 
            edgeName = splited[0] + "->" + vertex
            if not edgeName in edgeNames: 
                edges.append((splited[0], vertex))
                edgeNames.add(edgeName)
                
    content = ""
    for vertex in vertices: 
        prefixCompat = compat[vertex].split(".")[0]
        assert prefixCompat in lut, "Illegal compat: " + str(vertex) + "->" + prefixCompat
        func = lut[prefixCompat]
        if prefixCompat != compat[vertex]: 
            func += "." + compat[vertex].split(".")[1]
        content += "vertex " + vertex + "\n"
        content += "    attr function str " + func + "\n"
    for edge in edges: 
        content += "edge " + edge[0] + " " + edge[1] + "\n"
        
    compats = ""
    for vertex in vertices: 
        if not "." in vertex:  
            assert vertex in compat
            if compat[vertex] in lut: 
                compats += vertex + " " + lut[compat[vertex]] + "\n" 
            else: 
                print("ERROR: Unsupported operation: " + compat[vertex])
                exit(1)
    
    # list(map(lambda x: print(x), vertices))
    # list(map(lambda x: print(x), edges))
    
    return content, compats, vertices, edges

def legalize(filename): 
    before = HyperGraph()
    with open(filename) as fin: 
        before.parse(fin.read())

    
if __name__ == "__main__": 
    import sys
    filename = sys.argv[1]
    lines = readArchFile(filename)
    content, compats, vertices, edges = parseDFG(lines)
    print(content)
    with open(filename[:-4]+"_DFG.txt", "w") as fout: 
        fout.write(content)
    print(compats)
    with open(filename[:-4]+"_compat.txt", "w") as fout: 
        fout.write(compats)