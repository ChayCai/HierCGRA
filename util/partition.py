import random
import sys
import numpy as np 
from sklearn.cluster import SpectralClustering

def readMatrix(filename): 
    with open(filename, "r") as fin: 
        lines = fin.readlines()
        mat = []
        for line in lines: 
            words = line.split()
            if len(line) > 0: 
                mat.append([])
                for word in words: 
                    if len(word) > 0: 
                        mat[-1].append(float(word))
    return np.array(mat)

def convert(clustered): 
    groups = []
    maxGroupID = 0
    for groupID in clustered: 
        if groupID > maxGroupID: 
            maxGroupID = groupID
    for idx in range(maxGroupID+1): 
        groups.append([])
    for idx in range(len(clustered)): 
        index = clustered[idx]
        groups[index].append(idx)
    return groups

def dump(filename, clusters):
    with open(filename, 'w') as fout: 
        for cluster in clusters: 
            for index in cluster: 
                fout.write(str(index) + ' ')
            fout.write("\n")

if __name__ == "__main__": 
    import sys
    fileFrom = sys.argv[1]
    fileTo   = sys.argv[2]
    num      = int(sys.argv[3])
    mat      = readMatrix(fileFrom)
    random_state_partition  = random.randint(0, 1024)
    solver   = SpectralClustering(n_clusters=num, n_init=64, affinity='precomputed', random_state=int(random_state_partition))
    results  = convert(solver.fit_predict(mat))
    dump(fileTo, results)

