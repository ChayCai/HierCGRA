import sys 
sys.path.append(".")

import xmltodict
import json

import common.utils as utils
from common.utils import Base
from common.graph import HyperGraph
from genarch.protocols import *

Sessions = {}
SessCurrent = ""
def Sess(): 
    return Sessions[SessCurrent]
def clearSess(): 
    Sessions[SessCurrent].clear()
def clearSession(): 
    Sessions[SessCurrent].clear()

OpTypes = {}

class Session(Base): 
    def __init__(self, name): 
        self._name = name
        self._ops = {}
        self._conns = {}
        self._count = {}
        self._graph = HyperGraph()
        Sessions[name] = self

    def clear(self): 
        self._ops = {}
        self._conns = {}
        self._count = {}
        self._graph = HyperGraph()
    
    def genName(self, typename): 
        if typename[0:2] == "__": 
            typename = typename[2:]
        if typename[-2:] == "__": 
            typename = typename[:-2]
        if not typename in self._count: 
            self._count[typename] = 0
        else: 
            self._count[typename] += 1
        return typename + str(self._count[typename])

    def addOp(self, name, op, modify=False): 
        if not modify: 
            assert not name in self._ops, "Session: Op " + name + " has been defined"
        self._ops[name] = op
        for port in self._ops[name].inputs(): 
            fr = self._ops[name].inputs()[port]
            if fr is None: 
                continue
            splited = fr.split(".")
            assert len(splited) == 2, "Session: Invalid inward connection name: " + fr
            frop, port = splited
            assert frop in self._ops, "Session: Invalid inward op: " + frop
            typeFrop = self._ops[frop].type()
            if typeFrop[0:2] == "__": 
                typeFrop = typeFrop[2:]
            if typeFrop[-2:] == "__": 
                typeFrop = typeFrop[:-2]
            assert typeFrop in OpTypes, "OpType: Invalid inward op type: " + typeFrop
            assert port in OpTypes[typeFrop].outports(), "OpType: Invalid port: " + fr
            if not frop in self._conns: 
                self._conns[frop] = []
            self._conns[frop].append([fr, name + "." + port])
        self._graph.addVertex(name, {"function": op.type()})
        for port in self._ops[name].inputs(): 
            self._graph.addVertex(name + "." + port, {"function": op.type() + "." + port})
            self._graph.addNet([name + "." + port, name], {})
        for port in self._ops[name].outputs(): 
            self._graph.addVertex(name + "." + port, {"function": op.type() + "." + port})
            self._graph.addNet([name, name + "." + port], {})
        for port in self._ops[name].inputs(): 
            fr = self._ops[name].inputs()[port]
            if fr is None: 
                continue
            self._graph.addNet([fr, name + "." + port], {})

    def modOp(self, name, op): 
        self.addOp(name, op, True)

    def ops(self): 
        return self._ops

    def op(self, name): 
        return self._ops[name] if name in self._ops else None

    def graph(self): 
        return self._graph

Sessions["__DEFAULT__"] = Session("__DEFAULT__")
SessCurrent = "__DEFAULT__"

class Operator(Base): 
    def __init__(self, name, typename, inputs, outputs): 
        self._name = name
        self._type = typename
        self._inputs = inputs
        self._outputs = outputs
    
    def type(self): 
        return self._type
    
    def inputs(self): 
        return self._inputs
    
    def outputs(self): 
        return self._outputs
    
    def input(self, index): 
        keys = list(self._inputs.keys())
        return keys[index] if index < len(keys) else None
    
    def output(self, index): 
        keys = list(self._outputs.keys())
        return keys[index] if index < len(keys) else None

    def __call__(self, *args, **kwargs): 
        # assert self._type in OpTypes, "Operator: Invalid op type: " + self._type
        # return OpTypes[self._type](args)
        assert len(args) <= len(self._inputs)
        if len(args) > 0 and isinstance(args[0], tuple): 
            args = tuple(args[0])
        args = list(args)
        for idx in range(len(args)): 
            if isinstance(args[idx], Operator): 
                args[idx] = args[idx][args[idx].output(0)]
            assert isinstance(args[idx], str), "Operator: Invalid input instance"
        typename = self._type
        if typename[0:2] == "__": 
            typename = typename[2:]
        if typename[-2:] == "__": 
            typename = typename[:-2]
        assert typename in OpTypes, "Operator: Invalid op type: " + typename
        inports = OpTypes[typename].inports()
        if len(args) < len(inports): 
            diff = len(inports) - len(args)
            args = args + [None for _ in range(diff)]
        for key in kwargs: 
            found = False
            for idx in range(len(inports)): 
                if inports[idx] == key: 
                    args[idx] = kwargs[key]
                    found = True
                    break
            assert found, "OpType: Invalid output port: " + key
        temp = {}
        for idx, fr in enumerate(args): 
            if fr is None: 
                temp[inports[idx]] = None
                continue
            splited = fr.split(".")
            assert len(splited) == 2, "Operator: Invalid inward connection name: " + fr
            op, port = splited
            assert op in Sess().ops(), "Operator: Invalid op: " + op
            typeOp = Sess().op(op).type()
            if typeOp[0:2] == "__": 
                typeOp = typeOp[2:]
            if typeOp[-2:] == "__": 
                typeOp = typeOp[:-2]
            assert typeOp in OpTypes, "Operator: Invalid op type: " + typeOp
            assert port in OpTypes[typeOp].outports(), "Operator: Invalid port: " + fr
            temp[inports[idx]] = fr
        self._inputs = temp
        Sess().modOp(self._name, self)

    def __getitem__(self, port): 
        assert port in self._inputs or port in self._outputs, "Operator: Invalid port " + self._name + "." + port
        return self._name + "." + port

    def info(self): 
        return "Operator: " + self._name + " of type " + self._type + "\n" + \
               " -> Inputs: " + utils.dict2str(self._inputs) + "\n" + \
               " -> Outputs: " + utils.list2str(list(self._outputs.keys()))

class OpType(Base): 
    def __init__(self, typename, inports, outports): 
        self._type = typename
        self._inports = inports
        self._outports = outports
    
    def type(self): 
        return self._type
    
    def typename(self): 
        return self._type
    
    def inports(self): 
        return self._inports
    
    def outports(self): 
        return self._outports

    def __call__(self, *args, **kwargs): 
        assert len(args) <= len(self._inports)
        if len(args) > 0 and isinstance(args[0], tuple): 
            args = tuple(args[0])
        args = list(args)
        for idx in range(len(args)): 
            if isinstance(args[idx], Operator): 
                args[idx] = args[idx][args[idx].output(0)]
            assert isinstance(args[idx], str), "OpType: Invalid input instance"
        if len(args) < len(self._inports): 
            diff = len(self._inports) - len(args)
            args = args + [None for _ in range(diff)]
        for key in kwargs: 
            found = False
            for idx in range(len(self._inports)): 
                if self._inports[idx] == key: 
                    args[idx] = kwargs[key]
                    found = True
                    break
            assert found, "OpType: Invalid output port: " + key
        inports = {}
        for idx, fr in enumerate(args): 
            if fr is None: 
                inports[self._inports[idx]] = None
                continue
            splited = fr.split(".")
            assert len(splited) == 2, "OpType: Invalid inward connection name: " + fr
            op, port = splited
            assert op in Sess().ops(), "OpType: Invalid op: " + op
            typeOp = Sess().op(op).type()
            if typeOp[0:2] == "__": 
                typeOp = typeOp[2:]
            if typeOp[-2:] == "__": 
                typeOp = typeOp[:-2]
            assert typeOp in OpTypes, "OpType: Invalid op type: " + typeOp
            assert port in OpTypes[typeOp].outports(), "OpType: Invalid port: " + fr
            inports[self._inports[idx]] = fr
        outports = {}
        for to in self._outports: 
            outports[to] = None
        name = Sess().genName(self._type)
        operator = Operator(name, self._type, inports, outports)
        Sess().addOp(name, operator)
        
        return operator

def LoadFunctions(functions): 
    for key in functions: 
        func = functions[key]
        assert not key in OpTypes, "OpType: Duplicated unit type. "
        typename = key
        if typename[0:2] == "__": 
            typename = typename[2:]
        if typename[-2:] == "__": 
            typename = typename[:-2]
        OpTypes[typename] = OpType(key, func.inputs(), func.outputs())
        globals()[typename] = OpTypes[typename]

def dataflow(): 
    return Sess().graph()
    