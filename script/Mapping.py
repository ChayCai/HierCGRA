import sys
import time
import datetime as dt
import subprocess as sp
import configparser

def pack(fileDFG, fileDFGGlobal, fileCompat, arch):
    print("Pack")

    filename = fileDFG[ : len(fileDFG) - 3] + "runpack.log"
    fo =open(filename,"wb")
    proPack = sp.Popen(["./build/pack", fileDFG, fileDFGGlobal, fileCompat, arch], stdout=fo, stderr= sp.STDOUT)

    finished = False
    while not finished:
        time.sleep(1.0)
        if proPack.poll() == 0:
            finished = True
    fo.flush()
    fo.close()

    if not finished: 
        print("Pack FAILED")
        sys.exit(1)
    else:
        print("Pack SUCCESS")

def partition(fileDFG, fileCompat, arch):
    print("Partition")

    filename = fileDFG[ : len(fileDFG) - 3] + "runpartition.log"
    fo =open(filename,"wb")
    proPartition = sp.Popen(["./build/partition", fileDFG, fileCompat, arch], stdout=fo, stderr= sp.STDOUT)

    finished = False
    while not finished:
        time.sleep(1.0)
        if proPartition.poll() == 0:
            finished = True
    fo.flush()
    fo.close()

    if not finished: 
        print("Partition FAILED")
        sys.exit(1)
    else:
        print("Partition SUCCESS")

def placeCore(fileDFGGlobal, fileCompat, arch, threadNum, partNum): 
    print("PlaceCore")
    partList = []
    for idx in range(int(partNum)):
        partList.append(fileDFGGlobal[:-4]+"_part"+str(idx)+".txt")
    threadSource = 0
    proPlaces = []
    proIndex = []
    for idx in range(len(partList)):
        for _ in range(int(threadNum)):
           proPlaces.append(sp.Popen(["./build/place", "placeCore", partList[idx], fileDFGGlobal, fileCompat, arch], stdout=sp.DEVNULL, stderr=sp.DEVNULL))
           proIndex.append(idx)
           threadSource += 1

    finished = False
    finishSet = set()
    while not finished:
        time.sleep(1.0)
        finishOne = False
        threadRunning = 0
        for idx in range(len(proPlaces)):
            if proPlaces[idx].poll() == 0 and proIndex[idx] not in finishSet:
                finishOne = True
                print("\nplaceCore: "+partList[proIndex[idx]] + " success. ", end="")
            if proPlaces[idx].poll() is None:
                threadRunning+=1
        if finishOne:
            for idx in range(len(proPlaces)):
                if proPlaces[idx].poll() == 0:
                    finishSet.add(proIndex[idx])
            for idx in range(len(proPlaces)):
                if proPlaces[idx].poll() is None and proIndex[idx] in finishSet:  
                    proPlaces[idx].kill()
            if len(finishSet) == len(partList):
                finished = True
            time.sleep(1.0)
            threadRunning = 0
            for idx in range(len(proPlaces)):
                if proPlaces[idx].poll() is None:
                    threadRunning+=1
            count = 0
            while count < threadSource-threadRunning:
                if finished:
                    break
                for idx in range(len(partList)):
                    if idx not in finishSet:
                        proPlaces.append(sp.Popen(["./build/place", "placeCore", partList[idx], fileDFGGlobal, fileCompat, arch], stdout=sp.DEVNULL, stderr=sp.DEVNULL))
                        proIndex.append(idx)
                        count += 1
                        if count >= threadSource-threadRunning:
                            break
        if threadRunning == 0:
            break
        print("\r" + str(threadRunning) + " threads is running for placeCore", end ="")
        
    if not finished: 
        print("\nPlaceCore FAILED")
        sys.exit(1)
    else:
        print("\nPlaceCore SUCCESS")

def placeTop(fileDFG, fileCompat, arch):
    print("PlaceTop")

    proPlaces = []
    for _ in range(int(threadNum)):
        proPlaces.append(sp.Popen(["./build/place", "placeTop", fileDFG, fileCompat, arch], stdout=sp.DEVNULL, stderr=sp.DEVNULL))
    
    finished = False
    while not finished:
        threadRunning = 0
        time.sleep(1.0)
        for idx in range(len(proPlaces)):
            if proPlaces[idx].poll() == 0:
                print("\nplaceTop: success. ", end = "")
                finished = True
                break
            if proPlaces[idx].poll() is None:
                threadRunning+=1
        if threadRunning == 0:
            break
        print("\r" + str(threadRunning) + " threads is running for placeTop", end ="")


    if not finished: 
        print("\nPlaceTop FAILED")
        sys.exit(1)
    else:
        print("\nPlaceTop SUCCESS")

if __name__ == "__main__": 
    assert len(sys.argv) >= 3
    fileDFG    = sys.argv[1]
    fileCompat = sys.argv[2]
    arch       = sys.argv[3]
    threadNum  = sys.argv[4]

    config = configparser.ConfigParser()
    config.read(arch)
    partNum = config["Partition"]["PartNum"]
    threadCore = str(int(int(threadNum)/int(partNum)))
    timeStart = dt.datetime.now()


# pack
    pack(fileDFG, fileDFG, fileCompat, arch)

# partition
    partition(fileDFG, fileCompat, arch)

# placeCore
    placeCore(fileDFG, fileCompat, arch, threadCore, partNum)

# placeTop
    placeTop(fileDFG, fileCompat, arch)

    
    timePlace = dt.datetime.now() - timeStart
    print("[", fileDFG, "]", "Mapping Time:", str((timePlace.seconds + timePlace.microseconds / 1000000)), 's')

