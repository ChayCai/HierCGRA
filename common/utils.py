

def list2str(lst):
    assert isinstance(lst, list) or isinstance(lst, tuple)
    result = "(List: ["
    for elem in lst:
        if isinstance(elem, list) or isinstance(elem, tuple):
            elem = list2str(elem)
        elif isinstance(elem, dict):
            elem = dict2str(elem)
        result += str(elem) + ", "
        if len(result) > 64:
            result += "\n "
    result += "])"
    return result

def dict2str(dct):
    assert isinstance(dct, dict)
    result = "(Dict: {\n"
    for key in dct:
        elem = str(dct[key])
        if isinstance(elem, list) or isinstance(elem, tuple):
            elem = list2str(elem)
        elif isinstance(elem, dict):
            elem = dict2str(elem)
        result += "\t -- " + str(key) + ": \t" + str(elem) + "\n"
    result += "})"
    return result

def closest(mux_list, num):
    index = 0
    for i in range(0, len(mux_list)):
        if mux_list[i] < num:
            continue
        if mux_list[i] >= num:
            index = i
            break
    
    return index 

def readfile(filename): 
    with open(filename, "r") as fin: 
        return fin.read()

def writefile(filename, content): 
    with open(filename, "w") as fout: 
        fout.write(content)

def dumpCompat(compat, filename): 
    with open(filename, "w") as fout: 
        for key, value in compat.items(): 
            content = key
            for elem in value: 
                content += " " + elem
            content += "\n"
            fout.write(content)

def readCompat(filename): 
    compat = {}
    with open(filename, "r") as fin: 
        lines = fin.readlines()
    for line in lines: 
        splited = line.strip().split()
        if len(splited) < 2: 
            continue
        compat[splited[0]] = splited[1:]
    return compat

def readFUs(filename): 
    result = {}
    with open(filename, "r") as fin: 
        content = fin.readlines()
    for line in content: 
        line = line.strip()
        if len(line) == 0: 
            continue
        elems = line.split()
        name = elems[0]
        typename = elems[1]
        devicename = elems[2]
        inputs = []
        outputs = []
        for idx in range(3, len(elems)): 
            if elems[idx] == "outputs": 
                idx += 1
                break
            inputs.append(elems[idx])
        for jdx in range(idx, len(elems)): 
            outputs.append(elems[idx])
        result[name] = {"type": typename, "device": devicename, "inputs": inputs, "outputs": outputs}
    return result

def writeCompat(filename, compat): 
    with open(filename, "w") as fout: 
        for key, values in compat.items(): 
            fout.write(key)
            for value in values: 
                fout.write(" " + value)
            fout.write("\n")

def getPrefix(content): 
    return content.split(".")[0]

def getPostfix(content): 
    return content.split(".")[-1]

class Base:
    def info(self):
        return "UNKNOWN"