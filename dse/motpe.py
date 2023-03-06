import os
import sys 
import time
sys.path.append(".")

from dse.runarch import eval
import optuna

class MOTPE: 

    def __init__(self, objs, vars, types, ranges, eval, nInit=2**4):
        self._objs      = objs
        self._vars   = vars
        self._types   = types
        self._ranges  = ranges
        self._eval   = eval
        self._name2index = {}
        for idx, name in enumerate(self._vars): 
            self._name2index[name] = idx
        self._model = optuna.create_study(directions=["minimize" for _ in range(self._objs)], \
                                          sampler=optuna.samplers.TPESampler(n_startup_trials=nInit, multivariate=False, group=False))
        self._visited = {}

    def optimize(self, steps=2**6):
        def name2range(idx):
            typename = self._types[idx]
            if typename == "int": 
                return self._ranges[idx][0], self._ranges[idx][1]
            elif typename == "enum":
                return 0, len(self._ranges[idx])-1
            else:
                return 0,0

        def name2value(idx, varaible):
            value = None
            typename = self._types[idx]
            if typename == "int": 
                value = int(varaible)
            elif typename == "enum": 
                value = self._ranges[idx][int(varaible)]
            else: 
                assert typename in ["int", "enum"]
            assert not value is None
            return value

        def objective(trial): 
            variables = []
            values = []
            for idx, name in enumerate(self._vars): 
                low, high = name2range(idx)
                variables.append(trial.suggest_int(name, int(low), int(high)))
                value = name2value(idx, variables[idx])
                values.append(value)
            name = str(variables)
            if name in self._visited: 
                return self._visited[name]
            score = self._eval(self._vars, values) #eval
            self._visited[name] = score
            for idx in range(0, len(self._vars)):
                print(self._vars[idx] + ':' + str(values[idx]), end = ', ')
            print(' --> ' + str(score))
            return score

        self._model.optimize(objective, n_trials=steps, timeout=None)
        trials = self._model.best_trials
        results = []
        for trial in trials: 
            params = {}
            for name in trial.params:
                params[name] = str(name2value(self._name2index[name], trial.params[name]))
            values = trial.values
            results.append((params, values))
        return results

def parseConfig(configFile):
    vars = []
    types = []
    ranges = []
    with open(configFile, 'r') as fi:
        line = fi.readline()
        while line:
            arange = []
            vecline = line.split()
            vars.append(vecline[0])
            types.append(vecline[1])
            for item in vecline[2:]:
                arange.append(item.replace("\n",""))
            ranges.append(arange)
            line = fi.readline()
    
    return vars, types, ranges

if __name__ == "__main__": 
    configFile    = sys.argv[1]
    objs = 2
    nInit = 2**5
    steps = 2**10

    vars = []
    types = []
    ranges = []
    vars, types, ranges = parseConfig(configFile)

    model = MOTPE(objs, vars, types, ranges, eval, nInit)
    results = model.optimize(steps=steps)

    print("all results: -> ")
    for name in model._visited:
        print(model._visited[name])

    print("bestTrials: -> ")
    for result in results:
        print(result[0], end=" -> ")
        print(result[1])
