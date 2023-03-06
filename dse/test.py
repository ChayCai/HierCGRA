import sys 
import os
import time
import subprocess as sp
sys.path.append(".")
from genarch.genarch6.genarch6 import genarch

def line2vars(line):
    vars = []
    values = []
    for item in line.replace(' ','').replace('\n','').replace('\'','').split(','):
        if ':' in item:
            vars.append(item.split(':')[0])
            values.append(item.split(':')[1])

    return vars, values

def parseLog(file):
    careList = {}
    with open(file, 'r') as fi:
        line = fi.readline()
        idx = 0
        while line:
            if '-->' in line:
                idx+=1
                data = line.replace(' ','').replace('\n','').split('-->')[1]
                ii = data.split(',')[1][:-1]
                if 31 < int(ii) < 33:
                    careList[idx] = line2vars(line)
            line = fi.readline()
    
    return careList

def parseReulst(file):
    careList = {}
    with open(file, 'r') as fi:
        line = fi.readline()
        idx = 0
        while line:
            if '->' in line and '{' in line:
                idx+=1
                data = line.replace(' ','').replace('\n','').split('->')[1]
                ii = data.split(',')[1][:-1] 
                if 35 > int(float(ii)) > 27:
                    careList[idx] = line2vars(line)
            line = fi.readline()
    return careList
             
def parseII(file):
    careII = {}
    with open(file, 'r') as fi:
        line = fi.readline()
        idx = 0
        while line:
            if '-->' in line:
                idx+=1
                data = line.replace(' ','').replace('\n','').split('-->')[1]
                ii = data.split(',')[1][:-1]
                careII[idx] = int(ii)
            line = fi.readline()
    return careII

             
def parseAreaPre(file):
    careII = {}
    with open(file, 'r') as fi:
        line = fi.readline()
        idx = 0
        while line:
            if '-->' in line:
                idx+=1
                data = line.replace(' ','').replace('\n','').split('-->')[1]
                area = data.split(',')[0][1:]
                careII[idx] = int(area)
            line = fi.readline()
    return careII

if __name__ == "__main__": 
    # strline = "pe0:ALU2, pe1:ALU2, pe2:ALU2, pe3:ALU2, pe4:ALU2, pe5:ALU2, pe6:ALU2, pe7:ALU2, pe8:ALU2, pe9:ALU2, pe10:ALU2, pe11:ALU2, pe12:ALU2, pe13:ALU2, pe14:ALU2, pe15:ALU2, s0:mesh, s1:mesh, s2:mesh, s3:mesh, s4:mesh, s5:mesh, s6:mesh, s7:mesh, s8:mesh, s9:mesh, s10:mesh, s11:mesh, s12:mesh, s13:mesh, s14:mesh, s15:mesh, io:8,  --> (62700, 44)"
    # vars, values = line2vars(strline)
    # genarch(vars, values)

    careList = parseLog("./testGraph2.log")
    

    for idx in careList:
        path = "./dse/arch/"+str(idx)
        if not os.path.exists(path):
            os.mkdir(path)
        genarch(careList[idx][0], careList[idx][1])
        proGen = sp.Popen(["./build/place", "test", path], stdout=sp.DEVNULL, stderr= sp.DEVNULL)
        finished = False
        while not finished:
            time.sleep(1.0)
            if proGen.poll() == 0:
                finished = True
            if proGen.poll is not None:
                break
        if not finished: 
            print("FAILED")
            sys.exit(1)
        else:
            print("SUCCESS")


    