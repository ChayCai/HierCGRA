import sys 
import time
import subprocess as sp
sys.path.append(".")
from genarch.genarch6.genarch6 import genarch

def eval(vars, values):

    genarch(vars, values)

    score0 = predictArea(vars, values)
    # score1 = predictArea(vars, values)
    score1 = evalMapping(4, 180)

    
    print(values,end=": -> ")
    print(score0,end=",")
    print(score1)

    return score0, score1

def predictArea(vars, values):
    result = 0
    for idx,item in enumerate(values):
        if item == "ALU0":
            result+=1000
        elif item == "ALU1":
            result+=2500
        elif item == "ALU2":
            result+=7500
        elif item == "mesh":
            result+=200
        elif item == "diag":
            result+=400
        elif vars[idx] == "io":
            result+=500*int(item)
    return result


def evalMapping(threadNum, timeLimit):
    benches = ["accumulate", "cap", "conv2", "conv3", "mac", "mac2", "matrixmultiply", "mults1", "mults2", "simple", "simple2", "sum", "nomem1"]
    baseline = [2, 3, 1, 2, 1, 3, 3, 4, 5, 1, 1, 1, 1]
    bestline = [2, 3, 1, 2, 1, 2, 2, 2, 4, 1, 1, 1, 1]

    result=[]
    for idy,bench in enumerate(benches):
        fileDFG = "./dse/benchmarks/cgrame/"+bench+"/"+bench+"_DFG.txt"
        fileCompat = "./dse/benchmarks/cgrame/"+bench+"/"+bench+"_compat.txt"
        ii = bestline[idy]

        while ii <= baseline[idy]+1:
            # print("\ntry to place "+fileDFG+" with ii: "+str(ii), end = "")
            proPlaces = []
            for _ in range(int(threadNum)):
                proPlaces.append(sp.Popen(["./build/place", "placeCoreII", fileDFG, fileCompat, str(ii)], stdout=sp.DEVNULL, stderr=sp.DEVNULL))
            finished = False
            count=0
            while not finished and count < timeLimit:
                threadRunning = 0
                time.sleep(1.0)
                count+=1
                for idx in range(len(proPlaces)):
                    if proPlaces[idx].poll() == 0:
                        # print("\nplace "+fileDFG+" success. with ii: "+str(ii), end = "")
                        finished = True
                        break
                    if proPlaces[idx].poll() is None:
                        threadRunning+=1
                if threadRunning == 0:
                    break
            for idx in range(len(proPlaces)):
                    if proPlaces[idx].poll() is None:
                        proPlaces[idx].kill()
            if finished:
                break
            ii+=1
        if ii <= baseline[idy]+1:
            result.append(ii)   
        else:
            result.append(8) 
    return sum(result)

if __name__ == "__main__": 
    result = evalMapping(4, 180)
    print(result)
