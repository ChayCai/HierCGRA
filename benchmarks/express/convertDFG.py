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
        content += "vertex " + vertex + "\n"
        content += "    attr optype str " + compat[vertex] + "\n"
    for edge in edges: 
        content += "edge " + edge[0] + " " + edge[1] + "\n"

    compats = ""
    for vertex in vertices: 
        if not "." in vertex:  
            assert vertex in compat
            # if compat[vertex] in set(["mul", "div", "add", "sub", "shl", "shrl", "shra", "and", "or", "xor", "neg", "asr", "lsl", "lsr", "bge", "bne"]): 
            #     compats += vertex + " ALU" + "\n" 
            if compat[vertex] in set(["mul"]): 
                compats += vertex + " MUL ALU" + "\n" 
            elif compat[vertex] in set(["div"]): 
                compats += vertex + " DIV ALU" + "\n"
            elif compat[vertex] in set(["add"]): 
                compats += vertex + " ADD ALU" + "\n"
            elif compat[vertex] in set(["sub"]): 
                compats += vertex + " SUB ALU" + "\n"
            elif compat[vertex] in set(["shl", "shrl", "shra", "and", "or", "xor", "neg", "asr", "lsl", "lsr", "bge", "bne"]): 
                compats += vertex + " ALU" + "\n"
            elif compat[vertex] in set(["const", ]): 
                compats += vertex + " CONST" + "\n" 
            elif compat[vertex] in set(["load", "lod", "store", "str", "memr", "memw", ]): 
                compats += vertex + " MEM" + "\n" 
            elif compat[vertex] in set(["input", "imp", ]): 
                compats += vertex + " INPUT" + "\n" 
            elif compat[vertex] in set(["output", "exp", ]): 
                compats += vertex + " OUTPUT" + "\n" 
            else: 
                print("ERROR: Unsupported operation: " + compat[vertex])
                exit(1)
    
    # list(map(lambda x: print(x), vertices))
    # list(map(lambda x: print(x), edges))
    
    return content, compats, vertices, edges

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
    
