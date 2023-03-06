from ast import Mod
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
    import genarch.genarch3.units as units
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

    blockALB = ModuleGen(["in" + str(idx) for idx in range(8)], 
                           ["out" + str(idx) for idx in range(8)], 
                           funcStructs, unitStructs, [])
    alus = ["ALU" + str(idx) for idx in range(8)]
    blockALB.addUnits(alus, ["ALU" for _ in range(len(alus))])
    blockALB.addGroupMesh("ALUs", alus, row=2, col=4, frInputs=True)
    blockALB.connectInportsAll(["ALU0", "ALU1", "ALU2", "ALU3"])
    blockALB.connectOutportsAll(["ALU0", "ALU1", "ALU2", "ALU3"])
    for idx in range(0, 4):
        blockALB.connect("in"+str(idx), alus[idx+4] + ".in0")
        blockALB.connect("in"+str(idx), alus[idx+4] + ".in1")
        blockALB.connect("in"+str(idx+4), alus[idx+4] + ".in0")
        blockALB.connect("in"+str(idx+4), alus[idx+4] + ".in1")
        blockALB.connect(alus[idx+4] + ".out0", "out"+str(idx))

    blockALB.dealWithFanin()

    blockARI = ModuleGen(["in" + str(idx) for idx in range(8)], 
                           ["out" + str(idx) for idx in range(8)], 
                           funcStructs, unitStructs, [])
    muls = ["MUL" + str(idx) for idx in range(4)]
    adds = ["ADD" + str(idx) for idx in range(2)]
    subs = ["SUB" + str(idx) for idx in range(1)]
    divs = ["DIV" + str(idx) for idx in range(1)]
    blockARI.addUnits(muls, ["MUL" for _ in range(len(muls))])
    blockARI.addUnits(adds, ["ADD" for _ in range(len(adds))])
    blockARI.addUnits(subs, ["SUB" for _ in range(len(subs))])
    blockARI.addUnits(divs, ["DIV" for _ in range(len(divs))])
    blockARI.addGroupBar("ARITHs", muls[0:2] + adds + muls[2:4] + subs + divs, row=2, col=4, num=2, toroid=True, frInputs=False, toOutputs=False)
    blockARI.connectInportsAll(muls[0:2] + adds + subs + divs)
    blockARI.connectOutportsAll(muls[0:2] + adds + subs + divs)
    blockARI.dealWithFanin()

    blockMEB = ModuleGen(["in" + str(idx) for idx in range(16)], 
                           ["out" + str(idx) for idx in range(16)], 
                           funcStructs, unitStructs, [])
    mems = ["MEM" + str(idx) for idx in range(8)]
    blockMEB.addUnits(mems, ["MEM" for _ in range(len(mems))])
    blockMEB.addGroupHop("MEMs", mems, hopStep=2, connectSelf=False)
    for idx in range(8):
        if idx % 2 == 0:
            for portTo in blockMEB._getInPorts(mems[idx]):
                for portFrom in blockMEB._getOutPorts(mems[idx]):
                    blockMEB.connect(portFrom, portTo)
        for port in blockMEB._getInPorts(mems[idx]):
            for idy in range(idx*2-2,idx*2+2):
                if 0<=idy<=16:
                    blockMEB.connect("in" + str(idy), port)
        for port in blockMEB._getOutPorts(mems[idx]):
            for idy in range(idx*2-2,idx*2+2):
                if 0<=idy<=16:
                    blockMEB.connect(port, "out" + str(idy))
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
    blocksALB = ["ALB" + str(idx) for idx in range(8)]
    blocksARI = ["ARI" + str(idx) for idx in range(4)]
    blocksMEB = ["MEB" + str(idx) for idx in range(4)]
    blocks = blocksALB + blocksARI + blocksMEB
    types = ["BlockALB"] * 8 + ["BlockARI"] * 4 + ["BlockMEB"] * 4
    core.addModules(blocks, types)

    for idx in range(0, 4):
        idy = (idx + 1) % 4
        mod1 = blocksALB[idx]
        mod2 = blocksALB[idy]
        for idz in range(0, 8):
            core.connect(mod1 + ".out" + str(idz), mod2 + ".in" + str(idz))
            core.connect(mod2 + ".out" + str(idz), mod1 + ".in" + str(idz))
    for idx in range(4, 8):
        idy = (idx + 1) % 4 + 4
        mod1 = blocksALB[idx]
        mod2 = blocksALB[idy]
        for idz in range(0, 8):
            core.connect(mod1 + ".out" + str(idz), mod2 + ".in" + str(idz))
            core.connect(mod2 + ".out" + str(idz), mod1 + ".in" + str(idz))
    for mod1 in ["ALB0", "ALB1", "ALB4", "ALB5"]:
        for mod2 in ["ALB0", "ALB1", "ALB4", "ALB5"]:
            if mod1 == mod2:
                continue
            for idz in range(4, 8):
                core.connect(mod1 + ".out" + str(idz), mod2 + ".in" + str(idz))
                core.connect(mod2 + ".out" + str(idz), mod1 + ".in" + str(idz))
    for mod1 in ["ALB2", "ALB3", "ALB6", "ALB7"]:
        for mod2 in ["ALB2", "ALB3", "ALB6", "ALB7"]:
            if mod1 == mod2:
                continue
            for idz in range(4, 8):
                core.connect(mod1 + ".out" + str(idz), mod2 + ".in" + str(idz))
                core.connect(mod2 + ".out" + str(idz), mod1 + ".in" + str(idz))
    for mod1 in ["ALB0", "ALB1"]:
        for mod2 in ["ALB6", "ALB7"]:
            if mod1 == mod2:
                continue
            for idz in range(0, 4):
                core.connect(mod1 + ".out" + str(idz), mod2 + ".in" + str(idz))
                core.connect(mod2 + ".out" + str(idz), mod1 + ".in" + str(idz))
    for mod1 in ["ALB2", "ALB3"]:
        for mod2 in ["ALB4", "ALB5"]:
            if mod1 == mod2:
                continue
            for idz in range(0, 4):
                core.connect(mod1 + ".out" + str(idz), mod2 + ".in" + str(idz))
                core.connect(mod2 + ".out" + str(idz), mod1 + ".in" + str(idz))
    
            
    for idx in range(0, 4):
        mod1 = blocksARI[idx]
        for idy in range(idx-1,idx+1):
            if 0<=idy<=3:
                mod2 = blocksALB[idy]
                for idz in range(0, 8):
                    core.connect(mod1 + ".out" + str(idz), mod2 + ".in" + str(idz))
                    core.connect(mod2 + ".out" + str(idz), mod1 + ".in" + str(idz))
        for idy in range(idx-1,idx+1):
            if 0<=idy<=3:
                mod2 = blocksALB[idy+4]
                for idz in range(0, 8):
                    core.connect(mod1 + ".out" + str(idz), mod2 + ".in" + str(idz))
                    core.connect(mod2 + ".out" + str(idz), mod1 + ".in" + str(idz))

    for idx in range (0, 8):
        for idy in range(len(blocksARI)):
            idz = (idy+1)%len(blocksARI)
            mod1 = blocksARI[idy]
            mod2 = blocksARI[idz]
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
    
    for idx in range(0, 8):
        for idy in range(0, 4):
            mod1 = blocksALB[idy]
            mod2 = blocksMEB[1]
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx+8))
            core.connect(mod2 + ".out" + str(idx+8), mod1 + ".in" + str(idx))
        for idy in range(4, 8):
            mod1 = blocksALB[idy]
            mod2 = blocksMEB[3]
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx+8))
            core.connect(mod2 + ".out" + str(idx+8), mod1 + ".in" + str(idx))
        for mod1 in blocksALB:
            mod2 = blocksMEB[0]
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx+8))
            core.connect(mod2 + ".out" + str(idx+8), mod1 + ".in" + str(idx))
        for mod1 in ["ALB2", "ALB3", "ALB6", "ALB7"]:
            mod2 = blocksMEB[2]
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx+8))
            core.connect(mod2 + ".out" + str(idx+8), mod1 + ".in" + str(idx))
        
        mod1 = blocksARI[0]
        for mod2 in ["MEB0", "MEB1"]:
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx+8))
            core.connect(mod2 + ".out" + str(idx+8), mod1 + ".in" + str(idx))
        mod1 = blocksARI[1]
        for mod2 in ["MEB0", "MEB1", "MEB3"]:
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx+8))
            core.connect(mod2 + ".out" + str(idx+8), mod1 + ".in" + str(idx))
        mod1 = blocksARI[2]
        for mod2 in ["MEB2", "MEB1", "MEB3"]:
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx+8))
            core.connect(mod2 + ".out" + str(idx+8), mod1 + ".in" + str(idx))
        mod1 = blocksARI[3]
        for mod2 in ["MEB2", "MEB3"]:
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx))
            core.connect(mod2 + ".out" + str(idx), mod1 + ".in" + str(idx))
            core.connect(mod1 + ".out" + str(idx), mod2 + ".in" + str(idx+8))
            core.connect(mod2 + ".out" + str(idx+8), mod1 + ".in" + str(idx))

    for idx in range (0, 8):
        mod = "ARI0"
        core.connect("in" + str(idx), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx))
        core.connect("in" + str(idx + 8), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx + 8))
        core.connect("in" + str(idx + 16), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx + 16))
        core.connect("in" + str(idx + 24), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx + 24))
        mod = "ARI1"
        core.connect("in" + str(idx + 8), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx + 8))
        core.connect("in" + str(idx + 24), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx + 24))
        mod = "ARI2"
        core.connect("in" + str(idx + 8), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx + 8))
        core.connect("in" + str(idx + 24), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx + 24))
        mod = "ARI3"
        core.connect("in" + str(idx + 16), mod + ".in" + str(idx))
        core.connect(mod + ".out" + str(idx), "out" + str(idx + 16))
        for mod in blocksALB:
            core.connect("in" + str(idx), mod + ".in" + str(idx))
            core.connect("in" + str(idx + 16), mod + ".in" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx + 16))
        for mod in blocksALB[0:4]:
            core.connect("in" + str(idx + 8), mod + ".in" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx + 8))
        for mod in blocksALB[4:8]:
            core.connect("in" + str(idx + 24), mod + ".in" + str(idx))
            core.connect(mod + ".out" + str(idx), "out" + str(idx + 24))
        for mod in blocksMEB:
            core.connect("in" + str(idx), mod + ".in" + str(idx))
            core.connect("in" + str(idx + 8), mod + ".in" + str(idx))
            core.connect("in" + str(idx + 16), mod + ".in" + str(idx))
            core.connect("in" + str(idx + 24), mod + ".in" + str(idx))
            core.connect("in" + str(idx), mod + ".in" + str(idx+8))
            core.connect("in" + str(idx + 8), mod + ".in" + str(idx+8))
            core.connect("in" + str(idx + 16), mod + ".in" + str(idx+8))
            core.connect("in" + str(idx + 24), mod + ".in" + str(idx+8))
            core.connect(mod + ".out" + str(idx+8), "out" + str(idx))
            core.connect(mod + ".out" + str(idx+8), "out" + str(idx + 8))
            core.connect(mod + ".out" + str(idx+8), "out" + str(idx + 16))
            core.connect(mod + ".out" + str(idx+8), "out" + str(idx + 24))
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
            for idy in range(8):
                top.connect("IN" + str(idy*4+idx) + ".out0",core + ".in" + str(idx))
    for idx in range(16):
        for core in cores:
            for idy in range(4):
                top.connect(core + ".out" + str(idx), "OUT" + str(idy*4+idx) + ".in0")
    top.dealWithFanin()
    

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
        with open(f"./arch/arch3/{name}.json", "w") as fout: 
            fout.write(json.dumps(top, indent=4))
        arch = Arch(f"./arch/arch3/{name}.json")
        with open(f"./arch/arch3/{name}_RRG.txt", "w") as fout: 
            fout.write(arch.rrg())
        with open(f"./arch/arch3/{name}_FUs.txt", "w") as fout: 
            fout.write(arch.fus())
