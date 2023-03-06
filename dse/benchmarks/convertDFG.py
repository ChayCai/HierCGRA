def parseDFG(file):
    content = ""
    compats = ""

    vertices = set()
    edges = []
    compat = {}
    with open(filename, "r") as fin: 
        lines = fin.readlines()
    for line in lines:
        if 'opcode' in line:
            vertex = line.split('[')[0]
            opcode = line.split('=')[1][:-3]
            vertices.add(vertex)
            compat[vertex] = opcode
        if 'operand' in line:
            vertex = line.split('->')[1].split('[')[0]
            port = line.split('=')[1].split(']')[0]
            vertexTo = vertex + ".in" + port
            vertexFrom = line.split('->')[0] + ".out0"
            vertices.add(vertexTo)
            vertices.add(vertexFrom)
            edges.append("edge "+vertexFrom+" "+vertexTo+"\n")
    
    for vertex in vertices:
        content += "vertex " + vertex + "\n"
    for edge in edges: 
        content += edge

    for vertex in vertices: 
        if not "." in vertex:  
            if compat[vertex] in set(["add"]): 
                compats += vertex + " ALU0 ALU1 ALU2" + "\n" 
            elif compat[vertex] in set(["shra"]): 
                compats += vertex + " ALU1 ALU2" + "\n"
            elif compat[vertex] in set(["mul"]): 
                compats += vertex + " ALU2" + "\n"
            elif compat[vertex] in set(["const"]): 
                compats += vertex + " IO" + "\n"
            elif compat[vertex] in set(["load"]): 
                compats += vertex + " MEM" + "\n"
            elif compat[vertex] in set(["store"]): 
                compats += vertex + " MEM" + "\n"
            elif compat[vertex] in set(["input"]): 
                compats += vertex + " IO" + "\n"
            elif compat[vertex] in set(["output"]): 
                compats += vertex + " IO" + "\n"
            else: 
                print("ERROR: Unsupported operation: " + compat[vertex])
                exit(1)
    print(compats)

    return content, compats


if __name__ == "__main__": 
    import sys
    filename = sys.argv[1]
    content, compats = parseDFG(filename)
    with open(filename[:-4]+"_DFG.txt", "w") as fout: 
        fout.write(content)
    with open(filename[:-4]+"_compat.txt", "w") as fout: 
        fout.write(compats)