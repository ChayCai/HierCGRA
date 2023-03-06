import xmltodict
import json

import arch as ac
import dataflow as df
import funcmap as fm

def point_add(prefix = "./benchmarks/point_add_"): 
    df.clearSession()
    numPairs = 8
    inputs0 = [df.INPUT() for _ in range(numPairs)]
    inputs1 = [df.INPUT() for _ in range(numPairs)]
    adds    = [df.ADD(inputs0[idx]["out0"], inputs1[idx]["out0"]) for idx in range(numPairs)]
    outputs = [df.OUTPUT(adds[idx]["out0"]) for idx in range(numPairs)]

    # print(df.dataflow().info())

    mapper = fm.IsoMapper(df.dataflow(), arch.units())
    mapper.match()
    # print(mapper.graphInfo())
    # print(mapper.compatInfo())
    print("Matched: point_add has " + str(len(df.Sess().ops())) + " operators. ")
    # with open(prefix + "OG.txt", "w") as fout: 
    #     fout.write(df.dataflow().info())
    with open(prefix + "DFG.txt", "w") as fout: 
        fout.write(mapper.graphInfo())
    with open(prefix + "compat.txt", "w") as fout: 
        fout.write(mapper.compatInfo())

def point_mul(prefix = "./benchmarks/point_mul_"): 
    df.clearSession()
    numPairs = 8
    inputs0 = [df.INPUT() for _ in range(numPairs)]
    inputs1 = [df.INPUT() for _ in range(numPairs)]
    muls    = [df.MUL(inputs0[idx]["out0"], inputs1[idx]["out0"]) for idx in range(numPairs)]
    outputs = [df.OUTPUT(muls[idx]["out0"]) for idx in range(numPairs)]

    # print(df.dataflow().info())

    mapper = fm.IsoMapper(df.dataflow(), arch.units())
    mapper.match()
    # print(mapper.graphInfo())
    # print(mapper.compatInfo())
    print("Matched: point_mul has " + str(len(df.Sess().ops())) + " operators. ")
    # with open(prefix + "OG.txt", "w") as fout: 
    #     fout.write(df.dataflow().info())
    with open(prefix + "DFG.txt", "w") as fout: 
        fout.write(mapper.graphInfo())
    with open(prefix + "compat.txt", "w") as fout: 
        fout.write(mapper.compatInfo())

def vecmul(prefix = "./benchmarks/vecmul_"): 
    df.clearSession()
    numLevel = 3
    numPairs = 2 ** numLevel
    inputs0 = [df.INPUT() for _ in range(numPairs)]
    inputs1 = [df.INPUT() for _ in range(numPairs)]
    muls    = [df.MUL(inputs0[idx]["out0"], inputs1[idx]["out0"]) for idx in range(numPairs)]
    levels  = [muls, ]
    for idx in range(numLevel): 
        level = levels[idx]
        tmp = []
        for jdx in range(len(level) // 2): 
            tmp.append(df.ADD(level[2*jdx]["out0"], level[2*jdx+1]["out0"]))
        levels.append(tmp)
    output = df.OUTPUT(levels[-1][0]["out0"])

    # print(df.dataflow().info())

    mapper = fm.IsoMapper(df.dataflow(), arch.units())
    mapper.match()
    # print(mapper.graphInfo())
    # print(mapper.compatInfo())
    print("Matched: vecmul has " + str(len(df.Sess().ops())) + " operators. ")
    # with open(prefix + "OG.txt", "w") as fout: 
    #     fout.write(df.dataflow().info())
    with open(prefix + "DFG.txt", "w") as fout: 
        fout.write(mapper.graphInfo())
    with open(prefix + "compat.txt", "w") as fout: 
        fout.write(mapper.compatInfo())

def linear(prefix = "./benchmarks/linear_"): 
    df.clearSession()
    numLevel = 3
    numPairs = 2 ** numLevel
    inputs0 = [df.INPUT() for _ in range(numPairs)]
    inputs1 = [df.INPUT() for _ in range(numPairs)]
    muls    = [df.MUL(inputs0[idx]["out0"], inputs1[idx]["out0"]) for idx in range(numPairs)]
    levels  = [muls, ]
    for idx in range(numLevel): 
        level = levels[idx]
        tmp = []
        for jdx in range(len(level) // 2): 
            tmp.append(df.ADD(level[2*jdx]["out0"], level[2*jdx+1]["out0"]))
        levels.append(tmp)

    thres = df.CONST()
    result = df.GREATER(levels[-1][0]["out0"], thres["out0"])
    
    output = df.OUTPUT(result["out0"])

    # print(df.dataflow().info())

    mapper = fm.IsoMapper(df.dataflow(), arch.units())
    mapper.match()
    # print(mapper.graphInfo())
    # print(mapper.compatInfo())
    print("Matched: linear has " + str(len(df.Sess().ops())) + " operators. ")
    # with open(prefix + "OG.txt", "w") as fout: 
    #     fout.write(df.dataflow().info())
    with open(prefix + "DFG.txt", "w") as fout: 
        fout.write(mapper.graphInfo())
    with open(prefix + "compat.txt", "w") as fout: 
        fout.write(mapper.compatInfo())

def conv2(prefix = "./benchmarks/conv2_"): 
    df.clearSession()
    sizeTotal = 7
    sizeKernel = 2
    vector = [df.INPUT() for _ in range(sizeTotal)]
    kernel = [df.INPUT() for _ in range(sizeKernel)]
    iters = sizeTotal - sizeKernel + 1
    results = []
    for idx in range(iters): 
        tmp0 = df.MUL(kernel[0]["out0"], vector[idx]["out0"]) 
        tmp1 = df.MUL(kernel[1]["out0"], vector[idx+1]["out0"]) 
        tmp3 = df.ADD(tmp0["out0"], tmp1["out0"]) 
        results.append(df.OUTPUT(tmp3["out0"]))

    # print(df.dataflow().info())

    mapper = fm.IsoMapper(df.dataflow(), arch.units())
    mapper.match()
    # print(mapper.graphInfo())
    # print(mapper.compatInfo())
    print("Matched: conv2 has " + str(len(df.Sess().ops())) + " operators. ")
    # with open(prefix + "OG.txt", "w") as fout: 
    #     fout.write(df.dataflow().info())
    with open(prefix + "DFG.txt", "w") as fout: 
        fout.write(mapper.graphInfo())
    with open(prefix + "compat.txt", "w") as fout: 
        fout.write(mapper.compatInfo())

def conv3(prefix = "./benchmarks/conv3_"): 
    df.clearSession()
    sizeTotal = 7
    sizeKernel = 3
    vector = [df.INPUT() for _ in range(sizeTotal)]
    kernel = [df.INPUT() for _ in range(sizeKernel)]
    iters = sizeTotal - sizeKernel + 1
    results = []
    for idx in range(iters): 
        tmp0 = df.MUL(kernel[0]["out0"], vector[idx]["out0"]) 
        tmp1 = df.MUL(kernel[1]["out0"], vector[idx+1]["out0"]) 
        tmp2 = df.MUL(kernel[2]["out0"], vector[idx+2]["out0"]) 
        tmp3 = df.ADD(tmp0["out0"], tmp1["out0"]) 
        tmp4 = df.ADD(tmp3["out0"], tmp2["out0"]) 
        results.append(df.OUTPUT(tmp4["out0"]))

    # print(df.dataflow().info())

    mapper = fm.IsoMapper(df.dataflow(), arch.units())
    mapper.match()
    # print(mapper.graphInfo())
    # print(mapper.compatInfo())
    print("Matched: conv3 has " + str(len(df.Sess().ops())) + " operators. ")
    # with open(prefix + "OG.txt", "w") as fout: 
    #     fout.write(df.dataflow().info())
    with open(prefix + "DFG.txt", "w") as fout: 
        fout.write(mapper.graphInfo())
    with open(prefix + "compat.txt", "w") as fout: 
        fout.write(mapper.compatInfo())


def greater(prefix = "./benchmarks/greater_"): 
    df.clearSession()
    size = 16
    inputs = [df.INPUT() for _ in range(size)]
    consts = [df.CONST() for _ in range(size)]
    greaters = [df.GREATER(inputs[idx]["out0"], consts[idx]["out0"]) for idx in range(size)]
    outputs = [df.OUTPUT(greaters[idx]["out0"]) for idx in range(size)]

    # print(df.dataflow().info())

    mapper = fm.IsoMapper(df.dataflow(), arch.units())
    mapper.match()
    # print(mapper.graphInfo())
    # print(mapper.compatInfo())
    print("Matched: greater has " + str(len(df.Sess().ops())) + " operators. ")
    # with open(prefix + "OG.txt", "w") as fout: 
    #     fout.write(df.dataflow().info())
    with open(prefix + "DFG.txt", "w") as fout: 
        fout.write(mapper.graphInfo())
    with open(prefix + "compat.txt", "w") as fout: 
        fout.write(mapper.compatInfo())

def decision_tree(prefix = "./benchmarks/decision_tree_"): 
    df.clearSession()
    size     = 4
    inputs   = [df.INPUT() for _ in range(size)]
    consts   = [df.CONST() for _ in range(size)]
    greaters = [df.GREATER(inputs[idx]["out0"], consts[idx]["out0"]) for idx in range(size)]
    lesses   = [df.LESS(inputs[idx]["out0"], consts[idx]["out0"]) for idx in range(size)]
    
    case0 = df.LOGICAND3(greaters[0]["out0"], greaters[1]["out0"], lesses[2]["out0"])
    case1 = df.LOGICAND2(greaters[0]["out0"], lesses[1]["out0"])
    case2 = df.LOGICAND4(lesses[0]["out0"], lesses[1]["out0"], lesses[2]["out0"], lesses[3]["out0"])
    # outputs = [df.OUTPUT(case0["out0"]), df.OUTPUT(case1["out0"]), df.OUTPUT(case2["out0"]), df.OUTPUT(case3["out0"])]
    
    result = df.LOGICOR3(case0["out0"], case1["out0"], case2["out0"])
    output = df.OUTPUT(result["out0"])

    # print(df.dataflow().info())

    mapper = fm.IsoMapper(df.dataflow(), arch.units())
    mapper.match()
    # print(mapper.graphInfo())
    # print(mapper.compatInfo())
    print("Matched: decision_tree has " + str(len(df.Sess().ops())) + " operators. ")
    # with open(prefix + "OG.txt", "w") as fout: 
    #     fout.write(df.dataflow().info())
    with open(prefix + "DFG.txt", "w") as fout: 
        fout.write(mapper.graphInfo())
    with open(prefix + "compat.txt", "w") as fout: 
        fout.write(mapper.compatInfo())

if __name__ == "__main__": 
    arch = ac.AbsArch("./optimized1.json")
    df.LoadFunctions(arch.functions())

    point_add()
    point_mul()
    vecmul()
    linear()
    conv2()
    conv3()
    greater()
    decision_tree()