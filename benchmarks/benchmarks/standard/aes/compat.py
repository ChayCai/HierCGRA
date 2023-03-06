import sys 
sys.path.append("../../../")

import common.utils as utils
from common.utils import Base
from common.graph import HyperGraph
from arch.protocols import *

op2fu = {
    "select": "ALU", 
    "icmp": "ALU", 
    "and": "ALU", 
    "or": "ALU", 
    "xor": "ALU", 
    "shl": "ALU", 
    "lshr": "ALU", 
    "ashr": "ALU", 
    "add": "ADD ALU", 
    "sub": "SUB ALU", 
    "mul": "MUL ALU", 
    "fadd": "ALU", 
    "fsub": "ALU", 
    "fmul": "ALU", 
    "const": "CONST", 
    "input": "INPUT", 
    "output": "OUTPUT", 
    "ret": "OUTPUT", 
    "load": "MEM", 
    "store": "MEM", 
}

if __name__ == "__main__": 
    dfgfile = sys.argv[1]
    compatfile = sys.argv[2]

    dfg = HyperGraph(dfgfile)
    compat = {}
    for name, vertex in dfg.vertices().items(): 
        splited = name.split(".")
        if len(splited) == 1: 
            op = vertex.attr("optype")
            fu = ""
            if not op in op2fu: 
                fu = "ALU"
                print("WARNING: Unknown operation " + op + ", mapped to ALU. ")
            else: 
                fu = op2fu[op]
            compat[name] = [fu, ]

    with open(compatfile, "w") as fout: 
        for key, value in compat.items(): 
            fout.write(key + " " + value[0] + "\n")
