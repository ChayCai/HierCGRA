import sys 
sys.path.append(".")

from audioop import mul
import json
from sqlite3 import connect
from typing import Tuple
from genarch.utils.genarch import ModuleGen
from genarch.utils.arch import Arch
from genarch.utils.protocols import Function
from genarch.utils.protocols import Unit
from genarch.utils.protocols import Module


if __name__ == "__main__": 
    import genarch.genarch1.units as units
    infoFuncs = units.functions()
    infoUnits = units.units()
    infoSwitches = {}
    infoModules = {}

    funcStructs = {}
    unitStructs = {}
    moduleStructs = {}
    for name, info in infoFuncs.items(): 
        funcStructs[name] = Function(name, info)
    for name, info in infoUnits.items(): 
        unitStructs[name] = Unit(name, info)

    blockALB = ModuleGen(["in" + str(idx) for idx in range(16)], 
                           ["out" + str(idx) for idx in range(16)], 
                           funcStructs, unitStructs, [])
    alus = ["ALU" + str(idx) for idx in range(16)]
    blockALB.addUnits(alus, ["ALU" for _ in range(len(alus))])
    blockALB.addGroupMesh("ALUs", alus, row=4, col=4, frInputs=True)
    blockALB.connectInportsAll(alus)
    blockALB.connectOutportsAll(alus)
    blockALB.dealWithFanin()

    blockARI = ModuleGen(["in" + str(idx) for idx in range(16)], 
                           ["out" + str(idx) for idx in range(16)], 
                           funcStructs, unitStructs, [])
    muls = ["MUL" + str(idx) for idx in range(8)]
    adds = ["ADD" + str(idx) for idx in range(4)]
    subs = ["SUB" + str(idx) for idx in range(2)]
    divs = ["DIV" + str(idx) for idx in range(2)]
    blockARI.addUnits(muls, ["MUL" for _ in range(len(muls))])
    blockARI.addUnits(adds, ["ADD" for _ in range(len(adds))])
    blockARI.addUnits(subs, ["SUB" for _ in range(len(subs))])
    blockARI.addUnits(divs, ["DIV" for _ in range(len(divs))])
    blockARI.addGroupMesh("ARITHs", muls[0:4] + adds + muls[4:8] + subs + divs, row=4, col=4, toroid = True, frInputs=False)
    blockARI.addGroupBar("ARITHs", muls[0:4] + adds + muls[4:8] + subs + divs, row=4, col=4, num=2, toroid=True, frInputs=True, toOutputs=True)
    blockARI.dealWithFanin()

    blockMEB = ModuleGen(["in" + str(idx) for idx in range(16)], 
                           ["out" + str(idx) for idx in range(16)], 
                           funcStructs, unitStructs, [])
    mems = ["MEM" + str(idx) for idx in range(16)]
    blockMEB.addUnits(mems, ["MEM" for _ in range(len(mems))])
    blockMEB.addGroupHop("MEMs", mems, hopStep=4, connectSelf=True)
    for idx in range(16):
        for port in blockMEB._getInPorts(mems[idx]):
            blockMEB.connect("in" + str(idx), port)
        for port in blockMEB._getOutPorts(mems[idx]):
            blockMEB.connect(port, "out" + str(idx))
        blockMEB.connect("in" + str(idx), "out" + str(idx))
    blockMEB.dealWithFanin()

    for key, value in blockALB._switches.items(): 
        infoSwitches[key] = value
    for key, value in blockMEB._switches.items(): 
        infoSwitches[key] = value
    for key, value in blockARI._switches.items(): 
        infoSwitches[key] = value
    infoModules["BlockALB"] = blockALB._info
    infoModules["BlockMEB"] = blockMEB._info
    infoModules["BlockARI"] = blockARI._info
    for name, info in infoModules.items(): 
        moduleStructs[name] = Module(name, info)

    core = ModuleGen(["in" + str(idx) for idx in range(32)],
                     ["out" + str(idx) for idx in range(32)], 
                     funcStructs, unitStructs, moduleStructs)
    blocksALB = ["ALB0", "ALB1", "ALB2", "ALB3"]
    blocksARI = ["ARI0", "ARI1", "ARI2", "ARI3"]
    blocksMEB = ["MEB0", "MEB1"]
    blocks = ["ALB0", "ALB1", "ALB2", "ALB3", "ARI0", "ARI1", "ARI2", "ARI3", "MEB0", "MEB1"]
    types = ["BlockALB", "BlockALB", "BlockALB", "BlockALB", "BlockARI", "BlockARI", "BlockARI", "BlockARI", "BlockMEB", "BlockMEB"]
    core.addModules(blocks, types)
    core.dealWithFanin()
    core.addGroupStar("ALBs", blocksALB, frInputs=False)
    core.addGroupStar("ARIs", blocksARI, frInputs=False)

    # blockALB_MEM = ["ALBs",]
    core.addGroup("MEMs", blocksMEB, frInputs=False)

    # core.connectGroupsStar("ALB_MEM", {"ALBs","MEMs"}, frInputs=False)
    # core.connectGroupsStar("ARI_MEM", {"ARIs", "MEMs"}, frInputs=False)

    for idx in range (0, 16):#jn
        for mod1 in blocksMEB:
            for mod2 in blocksMEB:
                if mod1 == mod2:
                    continue
                core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
                core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
    for idx in range(0, 16):
        for mod1 in blocksARI:
            for mod2 in blocksMEB:
                core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
                core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
        for mod1 in blocksALB:
            for mod2 in blocksMEB:
                core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
                core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
        for mod1 in blocksALB:
            for mod2 in blocksARI:
                core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
                core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
    for idx in range (0, 16):
        for mod in blocksARI:
            core.connect("in" + str(idx), mod + ".in" + str(idx))
            core.connect("in" + str(idx + 16), mod + ".in" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx + 16))
        for mod in blocksALB:
            core.connect("in" + str(idx), mod + ".in" + str(idx))
            core.connect("in" + str(idx + 16), mod + ".in" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx + 16))
        for mod in blocksMEB:
            core.connect("in" + str(idx), mod + ".in" + str(idx))
            core.connect("in" + str(idx+16), mod + ".in" + str(idx))

            core.connect("in" + str(idx+1), mod + ".in" + str(idx))
            core.connect("in" + str(idx+17), mod + ".in" + str(idx)) if idx != 15 else core.connect("in" + str(0), mod + ".in" + str(15))
            core.connect(mod + ".out" + str(idx), "out" + str(idx+1))
            core.connect(mod + ".out" + str(idx), "out" + str(idx+17)) if idx != 15 else core.connect(mod + ".out" + str(15), "out" + str(0))

            core.connect(mod + ".out" + str(idx), "out" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx+16))
    core.dealWithFanin()

    for key, value in core._switches.items(): 
        infoSwitches[key] = value
    infoModules["Core"] = core._info
    for name, info in infoModules.items(): 
        moduleStructs[name] = Module(name, info)

    top = ModuleGen([], [], 
                     funcStructs, unitStructs, moduleStructs)
    inputs = ["IN" + str(idx) for idx in range(64)]
    top.addUnits(inputs, ["INPUT" for _ in range(len(inputs))])
    outputs = ["OUT" + str(idx) for idx in range(32)]
    top.addUnits(outputs, ["OUTPUT" for _ in range(len(outputs))])
    cores = ["Core0", "Core1", "Core2", "Core3"]
    types = ["Core", "Core", "Core", "Core"]
    top.addModules(cores, types)
    top.addGroupStar("Cores", cores, frInputs=False)
    for idx in range(16):
        for core in cores:
            top.connect("IN" + str(idx) + ".out0",core + ".in" + str(idx))
            top.connect("IN" + str(idx+16) + ".out0",core + ".in" + str(idx))
            top.connect("IN" + str(idx+32) + ".out0",core + ".in" + str(idx))
            top.connect("IN" + str(idx+48) + ".out0",core + ".in" + str(idx))
    for idx in range(16):
        for core in cores:
            top.connect(core + ".out" + str(idx), "OUT" + str(idx*2) + ".in0")
            top.connect(core + ".out" + str(idx), "OUT" + str(idx*2+1) + ".in0")
    top.dealWithFanin()

    for key, value in top._switches.items(): 
        infoSwitches[key] = value
    infoModules["Top"] = top._info


    names = list(infoModules.keys())
    for name in names: 
        top = {
            "Function": infoFuncs, 
            "Unit": infoUnits, 
            "Switch": infoSwitches, 
            "Module": infoModules
        }
        top["Module"]["TOP"] = infoModules[name]
        with open(f"./arch/arch1/{name}.json", "w") as fout: 
            fout.write(json.dumps(top, indent=4))
        arch = Arch(f"./arch/arch1/{name}.json")
        with open(f"./arch/arch1/{name}_RRG.txt", "w") as fout: 
            fout.write(arch.rrg())
        with open(f"./arch/arch1/{name}_FUs.txt", "w") as fout: 
            fout.write(arch.fus())

