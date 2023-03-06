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

    typeSwitch = blockPE._createSwitch(2, 4)
    blockPE._info["switch"]["SW_INPORT_FU"] = typeSwitch
    blockPE._info["connection"].extend(blockPE._connectSwitch(blockPE._getInPorts(), ["ALU0.in0", "ALU0.in1", "MEM0.in0", "MEM0.in1"], "SW_INPORT_FU"))
    typeSwitch = blockPE._createSwitch(1, 2)
    blockPE._info["switch"]["SW_ALU_MEM"] = typeSwitch
    blockPE._info["connection"].extend(blockPE._connectSwitch(["ALU0.out0"], ["MEM0.in0", "MEM0.in1"], "SW_ALU_MEM"))
    typeSwitch = blockPE._createSwitch(2, 2)
    blockPE._info["switch"]["SW_FU_OUTPORT"] = typeSwitch
    blockPE._info["connection"].extend(blockPE._connectSwitch(["ALU0.out0", "MEM0.out0"], blockPE._getOutPorts(), "SW_FU_OUTPORT"))

    blockIO = ModuleGen(["in0", "in1"],
                        ["out0"],
                        funcStructs, unitStructs,[])
    blockIO.addUnits(["IO0"], ["IO"])
    blockIO.connectInportsAll(["IO0"])
    blockIO.connectOutportsAll(["IO0"])

    for key, value in blockPE._switches.items(): 
        infoSwitches[key] = value
    for key, value in blockIO._switches.items(): 
        infoSwitches[key] = value
    infoModules["BlockPE"] = blockPE._info
    infoModules["BlockIO"] = blockIO._info
    for name, info in infoModules.items(): 
        moduleStructs[name] = Module(name, info)


    core = ModuleGen([],
                     [], 
                     funcStructs, unitStructs, moduleStructs)
    core.addModules(["Block" + str(idx) for idx in range(24)],
        ["BlockPE"] + ["BlockIO"] * 2 + ["BlockPE"]
    +   ["BlockIO"] + ["BlockPE"] * 2 + ["BlockIO"]
    +   ["BlockPE"] * 4
    +   ["BlockPE"] * 4
    +   ["BlockIO"] + ["BlockPE"] * 2 + ["BlockIO"]
    +   ["BlockPE"] + ["BlockIO"] * 2 + ["BlockPE"]
    )
    core.addGroupMesh("Core", ["Block" + str(idx) for idx in range(24)], row=6, col=4, toroid=True, frInputs=False)

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
        with open(f"./arch/arch0/{name}.json", "w") as fout: 
            fout.write(json.dumps(top, indent=4))
        arch = Arch(f"./arch/arch0/{name}.json")
        with open(f"./arch/arch0/{name}_RRG.txt", "w") as fout: 
            fout.write(arch.rrg())
        with open(f"./arch/arch0/{name}_FUs.txt", "w") as fout: 
            fout.write(arch.fus())
