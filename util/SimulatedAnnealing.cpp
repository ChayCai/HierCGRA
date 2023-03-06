#include "./SimulatedAnnealing.h"

using namespace std; 

namespace FastCGRA
{

namespace SimulatedAnnealing
{

unordered_map<string, string> run(const function<double(const unordered_map<string, string> &vars)> &eval, const unordered_map<string, unordered_set<string>> &values, bool unique, const unordered_map<string, string> &init, size_t iters, double temperature, double scale)
{
    unordered_map<string, string> vars(init);
    if(vars.size() == 0)
    {
        if(!unique)
        {
            for(const auto &elem: values)
            {
                auto iter = elem.second.begin(); 
                size_t index = rand() % elem.second.size(); 
                for(size_t idx = 0; idx < index; idx++)
                {
                    iter++; 
                }
                vars[elem.first] = *iter; 
            }
        }
        else
        {
            VanillaMatcher matcher(values); 
            size_t matched = matcher.match(); 
            ASSERT(matched == values.size(), "SimulatedAnnealing: Initialization failed. ");  

            for(const auto &elem: matcher.matchTable())
            {
                vars[elem.first] = elem.second; 
            }
        }
    }

    vector<string> keys; 
    unordered_set<string> used; 
    for(const auto &elem: vars)
    {
        keys.push_back(elem.first); 
        used.insert(elem.second); 
    }

    double lastScore = eval(vars); 
    
    size_t iter = 0; 
    while(iter < iters)
    {
        size_t idx = rand() % keys.size(); 

        string from; 
        string to; 
        string fromOriginal; 
        string toOriginal; 
        size_t action = rand() % 2;  
        if(action == 0) // swap
        {
            size_t jdx = rand() % keys.size(); 
            const unordered_set<string> &range1 = values.find(keys[idx])->second; 
            const unordered_set<string> &range2 = values.find(keys[jdx])->second; 
            bool ok = range1.find(vars[keys[jdx]]) != range1.end() && range2.find(vars[keys[idx]]) != range2.end(); 
            if(!ok)
            {
                jdx = idx; 
            }
            size_t tried = 0; 
            while(idx == jdx && tried < 1024)
            {
                tried++; 
                jdx = rand() % keys.size(); 
                const unordered_set<string> &range1 = values.find(keys[idx])->second; 
                const unordered_set<string> &range2 = values.find(keys[jdx])->second; 
                bool ok = range1.find(vars[keys[jdx]]) != range1.end() && range2.find(vars[keys[idx]]) != range2.end(); 
                if(!ok)
                {
                    jdx = idx; 
                }
            }
            if(tried >= 1024)
            {
                NOTE << string("SimulatedAnnealing: Cannot swap ") + keys[idx]; 
                continue; 
            }

            from = keys[idx]; 
            to = keys[jdx]; 
            fromOriginal = vars[from]; 
            toOriginal = vars[to]; 
            swap(vars[from], vars[to]); 
        }
        else if(action == 1) // move
        {
            from = keys[idx]; 
            to = ""; 
            fromOriginal = vars[from]; 
            toOriginal = ""; 
            vector<string> candsFrom; 
            ASSERT(values.find(from) != values.end(), ""); 
            for(const auto &value: values.find(from)->second)
            {
                if(!unique || (fromOriginal != value && used.find(value) == used.end()))
                {
                    candsFrom.push_back(value); 
                }
            }
            if(candsFrom.size() > 0)
            {
                size_t index = rand() % candsFrom.size(); 
                if(unique)
                {
                    used.erase(fromOriginal); 
                    used.insert(candsFrom[index]); 
                }
                vars[keys[idx]] = candsFrom[index]; 
            }
            else
            {
                continue; 
            }
        }
        else
        {
            ASSERT(action == 0 || action == 1, "SimulatedAnnealing: Invalid action. "); 
        }
        
        double score = eval(vars); 
        double delta = score - lastScore; 
        string log = string("[SA]: Iter ") + num2str<size_t>(iter) + " " + num2str<double>(score) + "/" + num2str<double>(lastScore); 
        if(delta > 0)
        {
            double prob = exp(-delta / temperature); 
            double rnd = static_cast<double>(rand()%(1<<20)) / static_cast<double>(1<<20); 
            if(rnd >= prob) // rollback
            {
                if(to.size() > 0)
                {
                    vars[to] = toOriginal; 
                }
                else
                {
                    used.erase(vars[from]); 
                    used.insert(fromOriginal); 
                }
                log += " -> Rejected with prob " + num2str<double>(1.0 - prob); 
                vars[from] = fromOriginal; 
            }
            else
            {
                log += " -> Accepted with prob " + num2str<double>(prob); 
                lastScore = score; 
            }
        }
        else
        {
            log += " -> Accepted. "; 
            lastScore = score; 
        }
        cerr << log << "          \r"; 

        iter++; 
        temperature *= scale; 
    }
    cerr << endl; 

    if(unique)
    {
        unordered_set<string> used; 
        for(const auto &elem: vars)
        {
            ASSERT(used.find(elem.second) == used.end(), string("Simulated Annealing: Used item: ") + string(elem.second)); 
            used.insert(elem.second); 
        }
    }

    return vars; 
}

}

}