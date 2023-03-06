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
    import genarch.genarch2.units as units
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
    
    blockPE = ModuleGen(["in0", "in1"],
                        ["out0"],
                        funcStructs, unitStructs,[])
    blockPE.addUnits(["ALU0"], ["ALU"])
    blockPE.addUnits(["MEM0"], ["MEM"])

    typeSwitch = blockPE._createSwitch(4, 4)
    blockPE._info["switch"]["SW_INPORT_FU"] = typeSwitch
    blockPE._info["connection"].extend(blockPE._connectSwitch(blockPE._getInPorts() + ["ALU0.out0", "MEM0.out0"], ["ALU0.in0", "ALU0.in1", "MEM0.in0", "MEM0.in1"], "SW_INPORT_FU"))
    typeSwitch = blockPE._createSwitch(2, 1)
    blockPE._info["switch"]["SW_FU_OUTPORT"] = typeSwitch
    blockPE._info["connection"].extend(blockPE._connectSwitch(["ALU0.out0", "MEM0.out0"], blockPE._getOutPorts(), "SW_FU_OUTPORT"))

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

    for key, value in blockPE._switches.items(): 
        infoSwitches[key] = value
    for key, value in blockIO._switches.items(): 
        infoSwitches[key] = value
    for key, value in blockMEM._switches.items(): 
        infoSwitches[key] = value
    infoModules["BlockPE"] = blockPE._info
    infoModules["BlockIO"] = blockIO._info
    infoModules["BlockMEM"] = blockMEM._info
    for name, info in infoModules.items(): 
        moduleStructs[name] = Module(name, info)

    core = ModuleGen(["in" + str(idx) for idx in range(8)],
                     ["out" + str(idx) for idx in range(8)], 
                     funcStructs, unitStructs, moduleStructs)
    core.addModules(["BlockPE" + str(idx) for idx in range(16)], ["BlockPE"] * 16)
    core.addModules(["BlockMEM" + str(idx) for idx in range(8)], ["BlockMEM"] * 8)
    core.addModules(["BlockIO" + str(idx) for idx in range(8)], ["BlockIO"] * 8)
    for idx in range(0, 4):
        for idy in range(0, 4):
            core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * ((idx - 1) % 4) + idy) + ".in0")
            core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * ((idx - 1) % 4) + idy) + ".in1")
            core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx + 1) + idy) % 16) + ".in0")
            core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str((4 * (idx + 1) + idy) % 16) + ".in1")
            core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * idx + ((idy - 1) % 4)) + ".in0")
            core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * idx + ((idy - 1) % 4)) + ".in1")
            core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * idx + ((idy + 1) % 4)) + ".in0")
            core._appendConnect("BlockPE" + str(4 * idx + idy) + ".out0", "BlockPE" + str(4 * idx + ((idy + 1) % 4)) + ".in1")
    for idx in range(0, 4):
        core._appendConnect("BlockIO" + str(idx) + ".out0", "BlockPE" + str(idx) + ".in0")
        core._appendConnect("BlockIO" + str(idx) + ".out0", "BlockPE" + str(idx) + ".in1")
        core._appendConnect("BlockPE" + str(idx) + ".out0", "BlockIO" + str(idx) + ".in0")
        core._appendConnect("BlockPE" + str(idx) + ".out0", "BlockIO" + str(idx) + ".in1")
        core._appendConnect("BlockIO" + str(4 + idx) + ".out0", "BlockPE" + str(12 + idx) + ".in0")
        core._appendConnect("BlockIO" + str(4 + idx) + ".out0", "BlockPE" + str(12 + idx) + ".in1")
        core._appendConnect("BlockPE" + str(12 + idx) + ".out0", "BlockIO" + str(4 + idx) + ".in0")
        core._appendConnect("BlockPE" + str(12 + idx) + ".out0", "BlockIO" + str(4 + idx) + ".in1")
    for idx in range(0, 8):
        core._appendConnect("BlockIO" + str(idx) + ".out0", "out" + str(idx))
        core._appendConnect("in" + str(idx), "BlockIO" + str(idx) + ".in0")
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
    
    names = list(infoModules.keys())
    for name in names: 
        top = {
            "Function": infoFuncs, 
            "Unit": infoUnits, 
            "Switch": infoSwitches, 
            "Module": infoModules
        }
        top["Module"]["TOP"] = infoModules[name]
        with open(f"./arch/arch2/{name}.json", "w") as fout: 
            fout.write(json.dumps(top, indent=4))
        arch = Arch(f"./arch/arch2/{name}.json")
        with open(f"./arch/arch2/{name}_RRG.txt", "w") as fout: 
            fout.write(arch.rrg())
        with open(f"./arch/arch2/{name}_FUs.txt", "w") as fout: 
            fout.write(arch.fus())
