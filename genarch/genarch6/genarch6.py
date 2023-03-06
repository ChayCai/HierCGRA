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

def genarch(vars, values):
    var2val = {}
    for idx in range(len(vars)):
        var2val[vars[idx]] = values[idx]

    import genarch.genarch6.units as units
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

    blockPEs = []
    for idx in range(16):
        blockPEs.append(ModuleGen(["in0", "in1"],
                        ["out0"],
                        funcStructs, unitStructs,[]))
        blockPEs[idx].addUnits(["ALU0"], [values[idx]])
        blockPEs[idx].addUnits(["MEM0"], ["MEM"])
        typeSwitch = blockPEs[idx]._createSwitch(4, 4)
        blockPEs[idx]._info["switch"]["SW_INPORT_FU"] = typeSwitch
        blockPEs[idx]._info["connection"].extend(blockPEs[idx]._connectSwitch(blockPEs[idx]._getInPorts() + ["ALU0.out0", "MEM0.out0"], ["ALU0.in0", "ALU0.in1", "MEM0.in0", "MEM0.in1"], "SW_INPORT_FU"))
        typeSwitch = blockPEs[idx]._createSwitch(2, 1)
        blockPEs[idx]._info["switch"]["SW_FU_OUTPORT"] = typeSwitch
        blockPEs[idx]._info["connection"].extend(blockPEs[idx]._connectSwitch(["ALU0.out0", "MEM0.out0"], blockPEs[idx]._getOutPorts(), "SW_FU_OUTPORT"))

    blockIO = ModuleGen(["in0", "in1"],
                        ["out0"],
                        funcStructs, unitStructs,[])
    blockIO.addUnits(["IO0"], ["IO"])
    blockIO.connectInportsAll(["IO0"])
    blockIO.connectOutportsAll(["IO0"])
    
    blockMEM = ModuleGen(["in0", "in1"],
                        ["out0"],
                        funcStructs, unitStructs,[])
    blockMEM.addUnits(["MEM0"], ["MEM"])
    blockMEM.connect("in0", "MEM0.in0")
    blockMEM.connect("in1", "MEM0.in1")
    blockMEM.connect("MEM0.out0", "out0")

    for idx in range(len(blockPEs)):
        blockPE = blockPEs[idx]
        for key, value in blockPE._switches.items(): 
            infoSwitches[key] = value
        infoModules["BlockPE"+str(idx)] = blockPE._info
    for key, value in blockIO._switches.items(): 
        infoSwitches[key] = value
    for key, value in blockMEM._switches.items(): 
        infoSwitches[key] = value
    infoModules["BlockIO"] = blockIO._info
    infoModules["BlockMEM"] = blockMEM._info
    for name, info in infoModules.items(): 
        moduleStructs[name] = Module(name, info)

    ionum = int(var2val["io"])
    core = ModuleGen(["in" + str(idx) for idx in range(8)],
                     ["out" + str(idx) for idx in range(8)], 
                     funcStructs, unitStructs, moduleStructs)
    for idx in range(len(blockPEs)):
        core.addModules(["BlockPE"+str(idx)], ["BlockPE"+str(idx)])
    core.addModules(["BlockMEM" + str(idx) for idx in range(8)], ["BlockMEM"] * 8)
    core.addModules(["BlockIO" + str(idx) for idx in range(ionum)], ["BlockIO"] * ionum)
    for idx in range(0, 4):
        for idy in range(0, 4):
            if var2val["s"+str(4 * idx + idy)] == "mesh" or var2val["s"+str(4 * idx + idy)] == "diag":
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * ((idx - 1) % 4) + idy) + ".in0")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * ((idx - 1) % 4) + idy) + ".in1")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx + 1) + idy) % 16) + ".in0")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx + 1) + idy) % 16) + ".in1")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * idx + ((idy - 1) % 4)) + ".in0")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * idx + ((idy - 1) % 4)) + ".in1")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * idx + ((idy + 1) % 4)) + ".in0")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * idx + ((idy + 1) % 4)) + ".in1")
            if var2val["s"+str(4 * idx + idy)] == "diag":
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * ((idx - 1) % 4) + idy - 1) % 16) + ".in0")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * ((idx - 1) % 4) + idy - 1) % 16) + ".in1")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx + 1) + idy - 1) % 16) + ".in0")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx + 1) + idy - 1) % 16) + ".in1")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx - 1) + ((idy + 1) % 4)) % 16) + ".in0")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx - 1) + ((idy + 1) % 4)) % 16) + ".in1")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx + 1) + ((idy + 1) % 4)) % 16) + ".in0")
                core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx + 1) + ((idy + 1) % 4)) % 16) + ".in1")
    for idx in range(ionum):
        core._appendConnect("BlockIO" + str(idx) + ".out0", "BlockPE" + str(idx) + ".in0")
        core._appendConnect("BlockIO" + str(idx) + ".out0", "BlockPE" + str(idx) + ".in1")
        core._appendConnect("BlockPE" + str(idx) + ".out0", "BlockIO" + str(idx) + ".in0")
        core._appendConnect("BlockPE" + str(idx) + ".out0", "BlockIO" + str(idx) + ".in1")
    for idx in range(0, 4):
        core._appendConnect("BlockMEM" + str(idx) + ".out0", "BlockPE" + str(idx * 4) + ".in0")
        core._appendConnect("BlockMEM" + str(idx) + ".out0", "BlockPE" + str(idx * 4) + ".in1")
        core._appendConnect("BlockPE" + str(idx * 4) + ".out0", "BlockMEM" + str(idx) + ".in0")
        core._appendConnect("BlockPE" + str(idx * 4) + ".out0", "BlockMEM" + str(idx) + ".in1")
        core._appendConnect("BlockMEM" + str(4 + idx) + ".out0", "BlockPE" + str(idx * 4 + 3) + ".in0")
        core._appendConnect("BlockMEM" + str(4 + idx) + ".out0", "BlockPE" + str(idx * 4 + 3) + ".in1")
        core._appendConnect("BlockPE" + str(idx * 4 + 3) + ".out0", "BlockMEM" + str(4 + idx) + ".in0")
        core._appendConnect("BlockPE" + str(idx * 4 + 3) + ".out0", "BlockMEM" + str(4 + idx) + ".in1")
    core._createConnect()

    for key, value in core._switches.items(): 
        infoSwitches[key] = value
    infoModules["Core"] = core._info
    for name, info in infoModules.items(): 
        moduleStructs[name] = Module(name, info)
    
    name = "Core"
    top = {
        "Function": infoFuncs, 
        "Unit": infoUnits, 
        "Switch": infoSwitches, 
        "Module": infoModules
    }
    top["Module"]["TOP"] = infoModules[name]
    with open(f"./arch/arch6/{name}.json", "w") as fout: 
        fout.write(json.dumps(top, indent=4))
    arch = Arch(f"./arch/arch6/{name}.json")
    with open(f"./arch/arch6/{name}_RRG.txt", "w") as fout: 
        fout.write(arch.rrg())
    with open(f"./arch/arch6/{name}_FUs.txt", "w") as fout: 
        fout.write(arch.fus())


if __name__ == "__main__": 
    vars = ["pe0", "pe1", "pe2", "pe3", "pe4", "pe5", "pe6", "pe7",
            "pe8", "pe9", "pe10", "pe11", "pe12", "pe13", "pe14", "pe15", 
            "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
            "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", 
            "io"]
    values = ["ALU0", "ALU2", "ALU1","ALU1", "ALU2", "ALU1", "ALU2","ALU2", "ALU1", "ALU2", "ALU0","ALU0", "ALU2", "ALU2", "ALU2","ALU2", 
                "mesh", "diag", "mesh", "mesh", "diag", "mesh", "mesh", "diag", "mesh", "mesh", "mesh", "mesh", "mesh", "mesh", "mesh", "mesh", 
                "16"]
    # values = ["ALU2", "ALU2", "ALU2","ALU2", "ALU2", "ALU2", "ALU2","ALU2", "ALU2", "ALU2", "ALU2","ALU2", "ALU2", "ALU2", "ALU2","ALU2", 
    #             "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", "diag", 
    #             "16"]        
    genarch(vars, values)