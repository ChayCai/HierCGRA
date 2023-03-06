import sys

import numpy as np
import cvxpy as cp

Alpha = 1.0
Beta  = 1.0
Gamma = 1.0
    
def solveILP(inputPorts, outputPorts, pathsRequired, \
          layersMUX, numMUX, widthMUX, numLeastMUX=None, numOutMUX=None, maxFanout=None, congestAware=False): 
    indexInports  = {}
    indexOutports = {}
    indicesPaths  = [[] for _ in range(len(inputPorts))]
    allPaths      = []
    for idx in range(len(inputPorts)): 
        indexInports[inputPorts[idx]] = idx
    for idx in range(len(outputPorts)): 
        indexOutports[outputPorts[idx]] = idx
    for source in pathsRequired: 
        for sink in pathsRequired[source]: 
            assert source in indexInports
            assert sink   in indexOutports
            indicesPaths[indexInports[source]].append(indexOutports[sink])
            allPaths.append([indexInports[source], indexOutports[sink]])
            
    numLayers = 1 + layersMUX + 1
    
    # Variables
    # Variable 1 x[(a,b,i),(j,k)]: connection [a, b] uses MUX port [i, j, k]
    varX = cp.Variable(shape=(len(allPaths)*numLayers, numMUX*widthMUX), boolean=True, name="x[(a,b,i),(j,k)]")  
    # Variable 2 y[(a, b), (i, m, p, n, k)]: for connection [a, b] layer i MUX m port p is connected to layer i+1 MUX n port k
    varY = cp.Variable(shape=(len(allPaths)*(numLayers-1), numMUX*widthMUX*numMUX*widthMUX), boolean=True, name="y[(a,b,i),(m,p,n,k)]")
    # Variable 3 z[(i, m), (n, k)]: layer i MUX m is connected to layer i+1 MUX n port k
    varZ = cp.Variable(shape=((numLayers-1)*numMUX, numMUX*widthMUX), boolean=True, name="z[(i, m), (n, k)]")
    # Variable 4 m[i, j]: layer i MUX j is used or not
    varM = cp.Variable(shape=(numLayers, numMUX), boolean=True, name="m[i,j]")
    # Variable 5 c[i, j]: congestion metric
    if congestAware: 
        varC = cp.Variable(shape=(numLayers, numMUX), integer=True, name="c[i,j]")
    
    # Constraints
    constraints = []
    # Constraint 1: \forall i and required [a, b]: \sum_{j, k} x_{[a, b], [i, j, k]} \ge 1
    consInports  = []
    consOutports = []
    matrixStart  = []
    matrixEnd    = []
    for idx in range(len(indicesPaths)): 
        pathNumber = len(indicesPaths[idx])
        if pathNumber == 0: 
            continue
        tmpMatrixStart = np.zeros(shape=[pathNumber, numMUX*widthMUX], dtype=np.int)
        tmpMatrixEnd   = np.zeros(shape=[pathNumber, numMUX*widthMUX], dtype=np.int)
        tmpMatrixStart[:, idx*widthMUX] = 1
        for jdx in range(pathNumber): 
            tmpMatrixEnd[jdx, indicesPaths[idx][jdx]*widthMUX] = 1
        matrixStart.append(tmpMatrixStart)
        matrixEnd.append(tmpMatrixEnd)
    matrixStart = np.vstack(matrixStart)
    matrixEnd   = np.vstack(matrixEnd)
    
    reshapedVarX1 = cp.reshape(varX, (len(allPaths), numLayers*numMUX*widthMUX), order="C").T
    reshapedVarX1 = cp.reshape(reshapedVarX1, (numLayers, numMUX*widthMUX*len(allPaths)), order="C")
    reshapedVarX1Input = cp.reshape(reshapedVarX1[0, :], (numMUX*widthMUX, len(allPaths)), order="C").T # shape: (len(allPaths), numMUX*widthMUX)
    reshapedVarX1Output = cp.reshape(reshapedVarX1[-1, :], (numMUX*widthMUX, len(allPaths)), order="C").T # shape: (len(allPaths), numMUX*widthMUX)
    
    accumVarX = cp.sum(varX, axis=1)
    
    consVarX1 = (accumVarX >= 1)
    consVarX2 = (reshapedVarX1Input == matrixStart)
    consVarX3 = (reshapedVarX1Output == matrixEnd)
    constraints.append(consVarX1)
    constraints.append(consVarX2)
    constraints.append(consVarX3)
    
    # Constraint 2: y = x * x
    reshapedVarY1 = cp.reshape(varY, (len(allPaths)*(numLayers-1)*numMUX*widthMUX, numMUX*widthMUX), order="C")
    maxReshapedVarY1 = cp.reshape(cp.max(reshapedVarY1, axis=1), (len(allPaths)*(numLayers-1), numMUX*widthMUX), order="C")
    reshapedVarY2 = cp.reshape(reshapedVarY1.T, (numMUX*widthMUX*len(allPaths)*(numLayers-1), numMUX*widthMUX), order="C")
    maxReshapedVarY2 = cp.reshape(cp.max(reshapedVarY2, axis=1), (numMUX*widthMUX, len(allPaths)*(numLayers-1)), order="C").T # shape: (len(allPaths)*(numLayers-1), numMUX*widthMUX)
    reshapedVarX2 = cp.reshape(varX.T, (numMUX*widthMUX*len(allPaths), numLayers), order="C").T
    reshapedVarX2 = cp.reshape(reshapedVarX2[:(numLayers-1), :], (numLayers-1, numMUX*widthMUX*len(allPaths)), order="C").T
    reshapedVarX2 = cp.reshape(reshapedVarX2, (numMUX*widthMUX, len(allPaths)*(numLayers-1)), order="C").T # shape: (len(allPaths)*(numLayers-1), (numMUX*widthMUX)
    reshapedVarX3 = cp.reshape(varX.T, (numMUX*widthMUX*len(allPaths), numLayers), order="C").T
    reshapedVarX3 = cp.reshape(reshapedVarX3[1:, :], (numLayers-1, numMUX*widthMUX*len(allPaths)), order="C").T
    reshapedVarX3 = cp.reshape(reshapedVarX3, (numMUX*widthMUX, len(allPaths)*(numLayers-1)), order="C").T # shape: (len(allPaths)*(numLayers-1), (numMUX*widthMUX)
    extendedVarX1 = []
    reshapedVarXTmp1 = cp.reshape(varX, (len(allPaths)*numLayers*numMUX*widthMUX, 1), order="C")
    for idx in range(numMUX): 
        for jdx in range(widthMUX): 
            extendedVarX1.append(reshapedVarXTmp1)
    extendedVarX1 = cp.hstack(extendedVarX1)
    extendedVarX1 = cp.reshape(extendedVarX1, (len(allPaths)*numLayers, numMUX*widthMUX*numMUX*widthMUX), order="C")
    extendedVarX1 = cp.reshape(extendedVarX1.T, (numMUX*widthMUX*numMUX*widthMUX*len(allPaths), numLayers), order="C").T
    extendedVarX1 = cp.reshape(extendedVarX1[:(numLayers-1), :], (numLayers-1, numMUX*widthMUX*numMUX*widthMUX*len(allPaths)), order="C").T
    extendedVarX1 = cp.reshape(extendedVarX1, (numMUX*widthMUX*numMUX*widthMUX, len(allPaths)*(numLayers-1)), order="C").T # shape: (len(allPaths)*(numLayers-1), numMUX*widthMUX*numMUX*widthMUX)
    extendedVarX2 = []
    reshapedVarXTmp2 = cp.reshape(varX.T, (1, numMUX*widthMUX*len(allPaths)*numLayers), order="C")
    for idx in range(numMUX): 
        for jdx in range(widthMUX): 
            extendedVarX2.append(reshapedVarXTmp2)
    extendedVarX2 = cp.vstack(extendedVarX2)
    extendedVarX2 = cp.reshape(extendedVarX2, (numMUX*widthMUX*numMUX*widthMUX, len(allPaths)*numLayers), order="C").T # shape: (len(allPaths)*numLayers, numMUX*widthMUX*numMUX*widthMUX)
    extendedVarX2 = cp.reshape(extendedVarX2.T, (numMUX*widthMUX*numMUX*widthMUX*len(allPaths), numLayers), order="C").T
    extendedVarX2 = cp.reshape(extendedVarX2[1:, :], (numLayers-1, numMUX*widthMUX*numMUX*widthMUX*len(allPaths)), order="C").T
    extendedVarX2 = cp.reshape(extendedVarX2, (numMUX*widthMUX*numMUX*widthMUX, len(allPaths)*(numLayers-1)), order="C").T # shape: (len(allPaths)*(numLayers-1), numMUX*widthMUX*numMUX*widthMUX)
    
    consVarY1 = (maxReshapedVarY1 <= reshapedVarX2)
    consVarY2 = (maxReshapedVarY2 <= reshapedVarX3)
    consVarY3 = (varY >= extendedVarX1 + extendedVarX2 - 1)
    constraints.append(consVarY1)
    constraints.append(consVarY2)
    constraints.append(consVarY3)
    
    # Constraint 3: z >= y, z <= 1
    reshapedVarY3    = cp.reshape(varY, (len(allPaths), (numLayers-1)*numMUX*widthMUX*numMUX*widthMUX), order="C")
    maxReshapedVarY3 = cp.reshape(cp.max(reshapedVarY3, axis=0), ((numLayers-1)*numMUX, widthMUX*numMUX*widthMUX), order="C").T 
    maxReshapedVarY3 = cp.reshape(maxReshapedVarY3, (widthMUX, numMUX*widthMUX*(numLayers-1)*numMUX), order="C")
    maxReshapedVarY3 = cp.reshape(cp.max(maxReshapedVarY3, axis=0), (numMUX*widthMUX, (numLayers-1)*numMUX), order="C").T # shape: ((numLayers-1)*numMUX, numMUX*widthMUX)
    reshapedVarZ1    = cp.reshape(varZ.T, (numMUX*widthMUX*(numLayers-1), numMUX), order="C")
    maxReshapedVarZ1 = cp.reshape(cp.sum(reshapedVarZ1, axis=1), (numMUX*widthMUX, numLayers-1), order="C").T # shape: ((numLayers-1), numMUX*widthMUX)
    
    consVarZ1 = (varZ >= maxReshapedVarY3)
    consVarZ2 = (maxReshapedVarZ1 <= 1)
    constraints.append(consVarZ1)
    constraints.append(consVarZ2)
    if not maxFanout is None: 
        # (numLayers-1)*numMUX, numMUX*widthMUX
        consVarZ3 = (cp.sum(varZ, axis=1) <= maxFanout)
        constraints.append(consVarZ3)
    
    # Constraint 4: m >= x; enough MUXes for outputs
    #reshapedVarX4    = cp.reshape(varX, (len(allPaths), numLayers*numMUX*widthMUX), order="C")
    #maxReshapedVarX4 = cp.reshape(cp.max(reshapedVarX4, axis=0), (numLayers*numMUX, widthMUX), order="C")
    #maxReshapedVarX4 = cp.reshape(cp.max(maxReshapedVarX4, axis=1), (numLayers, numMUX), order="C")
    
    #consVarM1 = (varM >= maxReshapedVarX4)
    #constraints.append(consVarM1)
    #if not numLeastMUX is None and varM.shape[0] > 3: 
        #consVarM2 = (cp.sum(varM[1:-2], axis=1) >= numLeastMUX)
        #constraints.append(consVarM2)
    #if not numOutMUX is None: 
        #consVarM3 = (cp.sum(varM[-2, :]) >= numOutMUX)
        #constraints.append(consVarM3)
    
    # varZ = cp.Variable(shape=((numLayers-1)*numMUX, numMUX*widthMUX), boolean=True, name="z[(i, m), (n, k)]")
    reshapedVarZ2    = cp.reshape(varZ.T, (numMUX*widthMUX*(numLayers-1), numMUX), order="C")
    maxReshapedVarZ2 = cp.reshape(cp.max(reshapedVarZ2, axis=1), (numMUX*widthMUX, numLayers-1), order="C")
    maxReshapedVarZ2 = cp.reshape(maxReshapedVarZ2.T, ((numLayers-1)*numMUX, widthMUX), order="C")
    maxReshapedVarZ2 = cp.reshape(cp.max(maxReshapedVarZ2, axis=1), (numLayers-1, numMUX), order="C")
    
    varM = cp.Variable(shape=(numLayers-2, numMUX), boolean=True, name="m[i,j]")
    consVarM1 = (varM >= maxReshapedVarZ2[:-1, :])
    constraints.append(consVarM1)
    if not numLeastMUX is None and varM.shape[0] > 3: 
        consVarM2 = (cp.sum(varM[:-1], axis=1) >= numLeastMUX)
        constraints.append(consVarM2)
    if not numOutMUX is None: 
        consVarM3 = (cp.sum(varM[-1, :]) >= numOutMUX)
        constraints.append(consVarM3)
    
        
    # Constraint 5
    if congestAware: 
        reshapedVarX5 = cp.reshape(varX, (len(allPaths), numLayers*numMUX*widthMUX), order="C")
        sumReshapedVarX5 = cp.reshape(cp.sum(reshapedVarX5, axis=0), (numLayers*numMUX, widthMUX), order="C")
        sumReshapedVarX5 = cp.reshape(cp.sum(sumReshapedVarX5, axis=1), (numLayers, numMUX), order="C")
        
        varC = sumReshapedVarX5
        #consVarC1 = (varC == sumReshapedVarX5)
        #constraints.append(consVarC1)
            
    # Problem
    #cost  = Alpha * cp.sum(varZ) + Beta * cp.sum(varM) + (Gamma * cp.max(varC) if congestAware else 0)
    cost  = Beta * cp.sum(varM) + (Gamma * cp.max(varC) if congestAware else 0)
    
    obj  = cp.Minimize(cost)
    prob = cp.Problem(obj, constraints)
    prob.solve(solver=cp.SCIP, verbose=True)
    # prob.solve(solver=cp.GUROBI, verbose=True)
    # prob.solve(solver=cp.GLPK_MI, verbose=True)
    
    return (prob, varX, varY, varZ, varM, varC) if congestAware else (prob, varX, varY, varZ, varM)

def genRRGILP(inputPorts, outputPorts, pathsRequired, layersMUX, numMUX, widthMUX, varZ, varM): 
    indexInports  = {}
    indexOutports = {}
    indicesPaths  = [[] for _ in range(len(inputPorts))]
    allPaths      = []
    for idx in range(len(inputPorts)): 
        indexInports[inputPorts[idx]] = idx
    for idx in range(len(outputPorts)): 
        indexOutports[outputPorts[idx]] = idx
    for source in pathsRequired: 
        for sink in pathsRequired[source]: 
            assert source in indexInports
            assert sink   in indexOutports
            indicesPaths[indexInports[source]].append(indexOutports[sink])
            allPaths.append([indexInports[source], indexOutports[sink]])
    
    # varZ = cp.Variable(shape=((numLayers-1)*numMUX, numMUX*widthMUX), boolean=True, name="z[(i, m), (n, k)]")
    
    vertices = set()
    edges = []
    
    for idx in range(len(inputPorts)): 
        vertices.add("I" + str(idx))
    for idx in range(len(outputPorts)): 
        vertices.add("O" + str(idx))
        
    for idx in range(layersMUX): 
        for jdx in range(numMUX): 
            if varM[idx, jdx] < 1:
                continue
            nameMUX = "L" + str(idx) + "_M" + str(jdx)
            nameOut = nameMUX + ".out0"
            vertices.add(nameMUX)
            vertices.add(nameOut)
            edges.append([nameMUX, nameOut])
            for kdx in range(widthMUX): 
                nameIn = nameMUX + ".in" + str(kdx)
                vertices.add(nameIn)
                edges.append([nameIn, nameMUX])
                
    varZ = varZ.reshape([layersMUX+1, numMUX, numMUX, widthMUX])
    for idx in range(layersMUX+1): 
        for jdx in range(numMUX): 
            nameFrom = ""
            if idx == 0: 
                nameFrom = "I" + str(jdx)
            else: 
                nameFrom = "L" + str(idx-1) + "_M" + str(jdx) + ".out0"
            for kdx in range(numMUX): 
                for ldx in range(widthMUX): 
                    if varZ[idx, jdx, kdx, ldx] < 1: 
                        continue
                    nameTo = ""
                    if idx == layersMUX: 
                        nameTo = "O" + str(kdx)
                    else: 
                        nameTo = "L" + str(idx) + "_M" + str(kdx) + ".in" + str(ldx)
                    edges.append([nameFrom, nameTo])
                    
    result = ""
    for vertex in vertices: 
        result += "vertex " + vertex + "\n"
    for edge in edges: 
        result += "edge " + edge[0] + " " + edge[1] + "\n"
    
    return result
    

if __name__ == "__main__": 
    inputPorts  = ["in0","in1","in2","in3", ]
    outputPorts = ["out0","out1", "out2","out3", ]
    layersMUX   = 2
    numMUX      = 4
    widthMUX    = 2
    pathsRequired = {"in0": ["out0", "out1", ],
                        "in1": ["out0", "out1", ],
                        "in2": ["out2", "out3", ],
                        "in3": ["out2", "out3", ],}
    if len(sys.argv) > 1 and sys.argv[1] == '4x4_0.5_0': 
        inputPorts  = ["in0","in1","in2","in3", ]
        outputPorts = ["out0","out1", "out2","out3", ]
        layersMUX   = 2
        numMUX      = 4
        widthMUX    = 2
        pathsRequired = {"in0": ["out0", "out1", ],
                         "in1": ["out2", "out3", ],
                         "in2": ["out0", "out1", ],
                         "in3": ["out2", "out3", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '4x4_0.5_1': 
        inputPorts  = ["in0","in1","in2","in3", ]
        outputPorts = ["out0","out1", "out2","out3", ]
        layersMUX   = 2
        numMUX      = 4
        widthMUX    = 2
        pathsRequired = {"in0": ["out0", "out3", ],
                         "in1": ["out0", "out2", ],
                         "in2": ["out1", "out3", ],
                         "in3": ["out1", "out2", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '4x4_0.5_2': 
        inputPorts  = ["in0","in1","in2","in3", ]
        outputPorts = ["out0","out1", "out2","out3", ]
        layersMUX   = 2
        numMUX      = 4
        widthMUX    = 2
        pathsRequired = {"in0": ["out0", "out1", ],
                         "in1": ["out0", "out1", ],
                         "in2": ["out2", "out3", ],
                         "in3": ["out2", "out3", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '4x4_0.75_0': 
        inputPorts  = ["in0","in1","in2","in3", ]
        outputPorts = ["out0","out1", "out2","out3", ]
        layersMUX   = 2
        numMUX      = 4
        widthMUX    = 2
        pathsRequired = {"in0": ["out0", "out1", "out2", ],
                         "in1": ["out0", "out1", "out3", ],
                         "in2": ["out0", "out2", "out3", ],
                         "in3": ["out1", "out2", "out3", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '4x4_0.75_1': 
        inputPorts  = ["in0","in1","in2","in3", ]
        outputPorts = ["out0","out1", "out2","out3", ]
        layersMUX   = 2
        numMUX      = 4
        widthMUX    = 2
        pathsRequired = {"in0": ["out0", "out1", "out2", ],
                         "in1": ["out0", "out1", "out3", ],
                         "in2": ["out0", "out2", "out3", ],
                         "in3": ["out1", "out2", "out3", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '4x4_0.75_2': 
        inputPorts  = ["in0","in1","in2","in3", ]
        outputPorts = ["out0","out1", "out2","out3", ]
        layersMUX   = 2
        numMUX      = 4
        widthMUX    = 2
        pathsRequired = {"in0": ["out0", "out1", "out2", ],
                         "in1": ["out0", "out1", "out2", ],
                         "in2": ["out1", "out2", "out3", ],
                         "in3": ["out1", "out2", "out3", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '6x6_0.5_0': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", ]
        layersMUX   = 2
        numMUX      = 6
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", ],
                         "in1": ["out0", "out1", "out4", ],
                         "in2": ["out0", "out1", "out2", ],
                         "in3": ["out1", "out3", "out5", ],
                         "in4": ["out0", "out3", "out4", ],
                         "in5": ["out1", "out4", "out5", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '6x6_0.5_1': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", ]
        layersMUX   = 2
        numMUX      = 6
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", ],
                         "in1": ["out0", "out2", "out4", ],
                         "in2": ["out0", "out3", "out5", ],
                         "in3": ["out1", "out3", "out5", ],
                         "in4": ["out1", "out3", "out4", ],
                         "in5": ["out1", "out2", "out4", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '6x6_0.5_2': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", ]
        layersMUX   = 2
        numMUX      = 6
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", ],
                         "in1": ["out0", "out1", "out2", ],
                         "in2": ["out0", "out1", "out2", ],
                         "in3": ["out3", "out4", "out5", ],
                         "in4": ["out3", "out4", "out5", ],
                         "in5": ["out3", "out4", "out5", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '6x6_0.875_0': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", ]
        layersMUX   = 2
        numMUX      = 6
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", "out3", ],
                         "in1": ["out0", "out1", "out3", "out4", ],
                         "in2": ["out0", "out1", "out2", "out3", ],
                         "in3": ["out1", "out3", "out4", "out5", ],
                         "in4": ["out0", "out3", "out4", "out5", ],
                         "in5": ["out1", "out3", "out4", "out5", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '6x6_0.875_1': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", ]
        layersMUX   = 2
        numMUX      = 6
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", "out3", ],
                         "in1": ["out0", "out2", "out3", "out4", ],
                         "in2": ["out0", "out3", "out4", "out5", ],
                         "in3": ["out1", "out2", "out3", "out5", ],
                         "in4": ["out0", "out1", "out3", "out4", ],
                         "in5": ["out1", "out2", "out4", "out5", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '6x6_0.875_2': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", ]
        layersMUX   = 2
        numMUX      = 6
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", "out3", ],
                         "in1": ["out0", "out1", "out2", "out3", ],
                         "in2": ["out0", "out1", "out2", "out3", ],
                         "in3": ["out2", "out3", "out4", "out5", ],
                         "in4": ["out2", "out3", "out4", "out5", ],
                         "in5": ["out2", "out3", "out4", "out5", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '8x8_0.5_0': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5","in6","in7", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", "out6","out7", ]
        layersMUX   = 2
        numMUX      = 8
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out3", "out5", "out7", ],
                         "in1": ["out0", "out1", "out2", "out5", ],
                         "in2": ["out1", "out3", "out6", "out7", ],
                         "in3": ["out0", "out1", "out2", "out3", ],
                         "in4": ["out0", "out5", "out6", "out7", ],
                         "in5": ["out1", "out3", "out6", "out7", ],
                         "in6": ["out0", "out5", "out6", "out7", ],
                         "in7": ["out2", "out3", "out4", "out7", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '8x8_0.5_1': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5","in6","in7", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", "out6","out7", ]
        layersMUX   = 2
        numMUX      = 8
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", "out7", ],
                         "in1": ["out0", "out3", "out4", "out5", ],
                         "in2": ["out1", "out3", "out5", "out7", ],
                         "in3": ["out1", "out2", "out5", "out6", ],
                         "in4": ["out0", "out3", "out6", "out7", ],
                         "in5": ["out1", "out2", "out6", "out7", ],
                         "in6": ["out1", "out3", "out6", "out7", ],
                         "in7": ["out0", "out3", "out4", "out7", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '8x8_0.5_2': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5","in6","in7", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", "out6","out7", ]
        layersMUX   = 2
        numMUX      = 8
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", "out3", ],
                         "in1": ["out0", "out1", "out2", "out3", ],
                         "in2": ["out0", "out1", "out2", "out3", ],
                         "in3": ["out0", "out1", "out2", "out3", ],
                         "in4": ["out4", "out5", "out6", "out7", ],
                         "in5": ["out4", "out5", "out6", "out7", ],
                         "in6": ["out4", "out5", "out6", "out7", ],
                         "in7": ["out4", "out5", "out6", "out7", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '8x8_0.75_0': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5","in6","in7", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", "out6","out7", ]
        layersMUX   = 2
        numMUX      = 8
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", "out3", "out5", "out7", ],
                         "in1": ["out0", "out1", "out2", "out3", "out4", "out5", ],
                         "in2": ["out1", "out3", "out4", "out5", "out6", "out7", ],
                         "in3": ["out0", "out1", "out2", "out3", "out4", "out5", ],
                         "in4": ["out0", "out2", "out3", "out5", "out6", "out7", ],
                         "in5": ["out1", "out3", "out4", "out5", "out6", "out7", ],
                         "in6": ["out0", "out1", "out3", "out5", "out6", "out7", ],
                         "in7": ["out1", "out2", "out3", "out4", "out6", "out7", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '8x8_0.75_1': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5","in6","in7", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", "out6","out7", ]
        layersMUX   = 2
        numMUX      = 8
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out3", "out4", "out5", "out7", ],
                         "in1": ["out0", "out1", "out2", "out4", "out5", "out6", ],
                         "in2": ["out1", "out2", "out3", "out5", "out6", "out7", ],
                         "in3": ["out0", "out1", "out2", "out3", "out6", "out7", ],
                         "in4": ["out0", "out2", "out3", "out4", "out5", "out6", ],
                         "in5": ["out1", "out3", "out4", "out5", "out6", "out7", ],
                         "in6": ["out1", "out2", "out3", "out4", "out5", "out6", ],
                         "in7": ["out1", "out2", "out3", "out5", "out6", "out7", ],}
    elif len(sys.argv) > 1 and sys.argv[1] == '8x8_0.75_2': 
        inputPorts  = ["in0","in1","in2","in3", "in4","in5","in6","in7", ]
        outputPorts = ["out0","out1", "out2","out3", "out4","out5", "out6","out7", ]
        layersMUX   = 2
        numMUX      = 8
        widthMUX    = 3
        pathsRequired = {"in0": ["out0", "out1", "out2", "out3", "out4", "out5", ],
                         "in1": ["out0", "out1", "out2", "out3", "out4", "out5", ],
                         "in2": ["out0", "out1", "out2", "out3", "out4", "out5", ],
                         "in3": ["out0", "out1", "out2", "out3", "out4", "out5", ],
                         "in4": ["out2", "out3", "out4", "out5", "out6", "out7", ],
                         "in5": ["out2", "out3", "out4", "out5", "out6", "out7", ],
                         "in6": ["out2", "out3", "out4", "out5", "out6", "out7", ],
                         "in7": ["out2", "out3", "out4", "out5", "out6", "out7", ],}
    prob, varX, varY, varZ, varM = solve(inputPorts, outputPorts, pathsRequired, layersMUX, numMUX, widthMUX, numLeastMUX=2, numOutMUX=2)
    # prob, varX, varY, varZ, varM = solve(inputPorts, outputPorts, pathsRequired, layersMUX, numMUX, widthMUX, numLeastMUX=None, numOutMUX=None, maxFanout=2)
    #prob, varX, varY, varZ, varM = solve(inputPorts, outputPorts, pathsRequired, layersMUX, numMUX, widthMUX, numLeastMUX=4, numOutMUX=4)
    # prob, varX, varY, varZ, varM, varC = solve(inputPorts, outputPorts, pathsRequired, layersMUX, numMUX, widthMUX, numLeastMUX=None, numOutMUX=None, congestAware=True)
        
    print("Status:", prob.status)
    print("Value:", prob.value)
    print("Variable X: ")
    print(varX.value.astype(np.int32))
    print("Variable Y: ")
    print(varY.value.astype(np.int32))
    print("Variable Z: ")
    print(varZ.value.astype(np.int32))
    print("Variable M: ")
    print(varM.value.astype(np.int32))
    #print("Variable C: ")
    #print(varC.value)
    
    graph = genRRG(inputPorts, outputPorts, pathsRequired, layersMUX, numMUX, widthMUX, varZ.value, varM.value)
    with open("G.txt", "w") as fout: 
        fout.write(graph)
        
        
# Running Times: 
'''
4x4_0.5_0: 0.76s
4x4_0.5_1: 0.76s
4x4_0.5_2: 0.78s
4x4_0.75_0: 0.82s
4x4_0.75_1: 0.89s
4x4_0.75_2: 0.87s
6x6_0.5_0: 2.25s
6x6_0.5_1: 2.78s
6x6_0.5_2: 4.11s
6x6_0.875_0: 3.88s
6x6_0.875_1: 3.18s
6x6_0.875_2: 3.45s
8x8_0.5_0: 45.78s
8x8_0.5_1: 1min3s = 63s
8x8_0.5_2: 35.18s
8x8_0.75_0: 6min31s = 391s
8x8_0.75_1: 16s.75s
8x8_0.75_2: 55.20s
'''