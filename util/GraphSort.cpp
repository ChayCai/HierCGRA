#include "./GraphSort.h"

using namespace std; 

namespace FastCGRA
{
    namespace GraphSort
    {
        vector<string> sortBFS(const Graph &graph){
            if(graph.vertices().empty()){
                return {};
            }

            vector<string> candiSeeds;
            for(const auto &vertex: graph.vertices()){
                if(graph.edgesIn(vertex.first).empty()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            if(candiSeeds.empty()){
                for(const auto &vertex: graph.vertices()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            const string &seed = candiSeeds[rand() % candiSeeds.size()];
            return sortBFS(graph, seed);
        }

        vector<string> sortBFS(const Graph &graph, const string &seed){
            assert(graph.vertices().find(seed) != graph.vertices().end());
            if(graph.vertices().empty()){
                return {};
            }

            vector<string> result;
            unordered_set<string> verticesSorted;
            queue<string> verticesToSortNext;

            string toSort = seed;
            while(verticesSorted.size() < graph.vertices().size()){
                if(verticesSorted.find(toSort) == verticesSorted.end()){
                    result.push_back(toSort);
                    verticesSorted.insert(toSort);
                }
                if(verticesSorted.size() == graph.vertices().size()){
                    break;
                }
                for(const auto &edge: graph.edgesOut(toSort)){
                    if(verticesSorted.find(edge.to()) == verticesSorted.end()){
                        verticesToSortNext.push(edge.to());
                    }
                }
                for(const auto &edge: graph.edgesIn(toSort)){
                    if(verticesSorted.find(edge.from()) == verticesSorted.end()){
                        verticesToSortNext.push(edge.from());
                    }
                }
                if(verticesToSortNext.empty()){
                    for(const auto &vertex: graph.vertices()){
                        if(graph.edgesIn(vertex.first).empty() 
                        &&verticesSorted.find(vertex.first) == verticesSorted.end()){
                            toSort = vertex.first;
                            break;
                        }
                    }
                } else {
                    toSort = verticesToSortNext.front();
                    verticesToSortNext.pop();
                }
            }

            return result;
        }

        vector<string> sortBFS(const Graph &graph, const function<bool(const Vertex &, const Vertex &)> &func){
            if(graph.vertices().empty()){
                return {};
            }
            
            vector<string> candiSeeds;
            for(const auto &vertex: graph.vertices()){
                if(graph.edgesIn(vertex.first).empty()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            if(candiSeeds.empty()){
                for(const auto &vertex: graph.vertices()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            random_shuffle(candiSeeds.begin(), candiSeeds.end());
            sort(candiSeeds.begin(), candiSeeds.end(), func);
            return sortBFS(graph, func, candiSeeds[0]);
        }   

        vector<string> sortBFS(const Graph &graph, const function<bool(const Vertex &, const Vertex &)> &func, const string &seed){
            assert(graph.vertices().find(seed) != graph.vertices().end());
            if(graph.vertices().empty()){
                return {};
            }

            vector<string> result;
            unordered_set<string> verticesSorted;
            queue<string> verticesToSortNext;

            verticesToSortNext.push(seed);
            while(verticesSorted.size() < graph.vertices().size()){
                vector<string> verticesSorting;
                vector<string> verticesToSortNextvec;
                while(!verticesToSortNext.empty()){
                    string toSort = verticesToSortNext.front();
                    verticesToSortNext.pop();
                    if(verticesSorted.find(toSort) == verticesSorted.end()){
                        result.push_back(toSort);
                        verticesSorted.insert(toSort);
                        verticesSorting.push_back(toSort);
                    }
                }
                if(verticesSorted.size() == graph.vertices().size()){
                    break;
                }
                for(const auto &vertex: verticesSorting){
                    for(const auto &edge: graph.edgesOut(vertex)){
                        if(verticesSorted.find(edge.to()) == verticesSorted.end()){
                            verticesToSortNextvec.push_back(edge.to());
                        }
                    }
                    for(const auto &edge: graph.edgesIn(vertex)){
                        if(verticesSorted.find(edge.from()) == verticesSorted.end()){
                            verticesToSortNextvec.push_back(edge.from());
                        }
                    }
                }
                sort(verticesToSortNextvec.begin(), verticesToSortNextvec.end(), func);
                for(const auto &vertex: verticesToSortNextvec){
                    verticesToSortNext.push(vertex);
                }
                if(verticesToSortNext.empty()){
                    vector<string> candidates;
                    for(const auto &vertex: graph.vertices()){
                        if(graph.edgesIn(vertex.first).empty() 
                        &&verticesSorted.find(vertex.first) == verticesSorted.end()){
                            candidates.push_back(vertex.first);
                        }
                    }
                    sort(candidates.begin(), candidates.end(), func);
                    verticesToSortNext.push(candidates[0]);
                }
            }

            return result;
        }

        vector<string> sortDFS(const Graph &graph){
            if(graph.vertices().empty()){
                return {};
            }

            vector<string> candiSeeds;
            for(const auto &vertex: graph.vertices()){
                if(graph.edgesIn(vertex.first).empty()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            if(candiSeeds.empty()){
                for(const auto &vertex: graph.vertices()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            const string &seed = candiSeeds[rand() % candiSeeds.size()];
            return sortDFS(graph, seed);
        }

        vector<string> sortDFS(const Graph &graph, const string &seed){
            if(graph.vertices().empty()){
                return {};
            }
            assert(graph.vertices().find(seed) != graph.vertices().end());

            vector<string> result;
            unordered_set<string> verticesSorted;
            stack<string> verticesToSortNext;

            string toSort = seed;
            while(verticesSorted.size() < graph.vertices().size()){
                if(verticesSorted.find(toSort) == verticesSorted.end()){
                    result.push_back(toSort);
                    verticesSorted.insert(toSort);
                }
                if(verticesSorted.size() == graph.vertices().size()){
                    break;
                }
                for(const auto &edge: graph.edgesIn(toSort)){
                    if(verticesSorted.find(edge.from()) == verticesSorted.end()){
                        verticesToSortNext.push(edge.from());
                    }
                }
                for(const auto &edge: graph.edgesOut(toSort)){
                    if(verticesSorted.find(edge.to()) == verticesSorted.end()){
                        verticesToSortNext.push(edge.to());
                    }
                }
                if(verticesToSortNext.empty()){
                    for(const auto &vertex: graph.vertices()){
                        if(graph.edgesIn(vertex.first).empty() 
                        &&verticesSorted.find(vertex.first) == verticesSorted.end()){
                            toSort = vertex.first;
                            break;
                        }
                    }
                } else {
                    toSort = verticesToSortNext.top();
                    verticesToSortNext.pop();
                }
            }

            return result;
        }

        vector<string> sortDFS(const Graph &graph, const function<bool(const Vertex &, const Vertex &)> &func){
            if(graph.vertices().empty()){
                return {};
            }
            
            vector<string> candiSeeds;
            for(const auto &vertex: graph.vertices()){
                if(graph.edgesIn(vertex.first).empty()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            if(candiSeeds.empty()){
                for(const auto &vertex: graph.vertices()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            random_shuffle(candiSeeds.begin(), candiSeeds.end());
            sort(candiSeeds.begin(), candiSeeds.end(), func);
            return sortDFS(graph, func, candiSeeds[0]);
        }   

        vector<string> sortDFS(const Graph &graph, const function<bool(const Vertex &, const Vertex &)> &func, const string &seed){
            if(graph.vertices().empty()){
                return {};
            }
            assert(graph.vertices().find(seed) != graph.vertices().end());

            vector<string> result;
            unordered_set<string> verticesSorted;
            stack<string> verticesToSortNext;

            auto func1 = [&](const Vertex &a, const Vertex &b){
                return func(b, a);
            };

            string toSort = seed;
            while(verticesSorted.size() < graph.vertices().size()){
                if(verticesSorted.find(toSort) == verticesSorted.end()){
                    result.push_back(toSort);
                    verticesSorted.insert(toSort);
                }
                if(verticesSorted.size() == graph.vertices().size()){
                    break;
                }
                vector<string> verticesToSortNextvec;
                for(const auto &edge: graph.edgesIn(toSort)){
                    if(verticesSorted.find(edge.from()) == verticesSorted.end()){
                        verticesToSortNextvec.push_back(edge.from());
                    }
                }
                for(const auto &edge: graph.edgesOut(toSort)){
                    if(verticesSorted.find(edge.to()) == verticesSorted.end()){
                        verticesToSortNextvec.push_back(edge.to());
                    }
                }
                sort(verticesToSortNextvec.begin(), verticesToSortNextvec.end(), func1);
                for(const auto &vertex: verticesToSortNextvec){
                    verticesToSortNext.push(vertex);
                }
                if(verticesToSortNext.empty()){
                    for(const auto &vertex: graph.vertices()){
                        if(graph.edgesIn(vertex.first).empty() 
                        &&verticesSorted.find(vertex.first) == verticesSorted.end()){
                            toSort = vertex.first;
                            break;
                        }
                    }
                } else {
                    toSort = verticesToSortNext.top();
                    verticesToSortNext.pop();
                }
            }

            return result;
        }

        vector<string> sortTVS(const Graph &graph){
            if(graph.vertices().empty()){
                return {};
            }

            vector<string> candiSeeds;
            for(const auto &vertex: graph.vertices()){
                if(graph.edgesOut(vertex.first).empty()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            if(candiSeeds.empty()){
                for(const auto &vertex: graph.vertices()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            const string &seed = candiSeeds[rand() % candiSeeds.size()];
            return sortTVS(graph, seed);
        }

        vector<string> sortTVS(const Graph &graph, const string &seed){
            if(graph.vertices().empty()){
                return {};
            }
            assert(graph.vertices().find(seed) != graph.vertices().end());

            queue<string> outVertices;
            for(const auto &vertex: graph.vertices()){
                if(graph.edgesOut(vertex.first).empty()){
                    outVertices.push(vertex.first);
                }
            }

            vector<string> result;
            unordered_set<string> verticesSorted;
            bool isUp = true;

            string toSort = seed;
            while(verticesSorted.size() < graph.vertices().size()){
                if(verticesSorted.find(toSort) == verticesSorted.end()){
                    result.push_back(toSort);
                    verticesSorted.insert(toSort);
                }
                if(verticesSorted.size() == graph.vertices().size()){
                    break;
                }
                string nextToSort = "";
                if(isUp){
                    if(graph.edgesOut(toSort).size() > 1){
                        for(const auto &edge: graph.edgesOut(toSort)){
                            if(verticesSorted.find(edge.to()) == verticesSorted.end()){
                                isUp = false;
                                nextToSort = edge.to();
                                break;
                            }
                        }
                    }
                    if(isUp){
                        for(const auto &edge: graph.edgesIn(toSort)){
                            if(verticesSorted.find(edge.from()) == verticesSorted.end()){
                                nextToSort = edge.from();
                                break;
                            }
                        }
                    }
                } else {
                    if(graph.edgesIn(toSort).size() > 1){
                        for(const auto &edge: graph.edgesIn(toSort)){
                            if(verticesSorted.find(edge.from()) == verticesSorted.end()){
                                isUp = true;
                                nextToSort = edge.from();
                                break;
                            }
                        }
                    }
                    if(!isUp){
                        for(const auto &edge: graph.edgesOut(toSort)){
                            if(verticesSorted.find(edge.to()) == verticesSorted.end()){
                                nextToSort = edge.to();
                                break;
                            }
                        }
                    }
                }
                if(!nextToSort.empty()){
                    toSort = nextToSort;
                } else {
                    while(!outVertices.empty()){
                        string vertex = outVertices.front();
                        outVertices.pop();
                        if(verticesSorted.find(vertex) == verticesSorted.end()){
                            nextToSort = vertex;
                            isUp = true;
                            break;
                        }
                    }
                    if(nextToSort.empty()){
                        for(const auto &vertex: verticesSorted){
                            for(const auto &edge: graph.edgesIn(vertex)){
                                if(verticesSorted.find(edge.from()) == verticesSorted.end()){
                                    nextToSort = edge.from();
                                    isUp = true;
                                }
                            }
                            for(const auto &edge: graph.edgesOut(vertex)){
                                if(verticesSorted.find(edge.to()) == verticesSorted.end()){
                                    nextToSort = edge.to();
                                    isUp = false;
                                }
                            }
                        }
                    }
                    if(nextToSort.empty()){
                        for(const auto &vertex: graph.vertices()){
                            if(verticesSorted.find(vertex.first) == verticesSorted.end()
                            &&graph.edgesIn(vertex.first).empty()){
                                nextToSort = vertex.first;
                                break;
                            }
                        }
                    }
                    if(nextToSort.empty()){
                        for(const auto &vertex: graph.vertices()){
                            if(verticesSorted.find(vertex.first) == verticesSorted.end()){
                                nextToSort = vertex.first;
                                break;
                            }
                        }
                    }
                    assert(!nextToSort.empty());
                    toSort = nextToSort;
                }

            }

            return result;
        }

        vector<string> sortSTB(const Graph &graph){
            if(graph.vertices().empty()){
                return {};
            }

            vector<string> candiSeeds;
            for(const auto &vertex: graph.vertices()){
                if(graph.edgesIn(vertex.first).empty()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            if(candiSeeds.empty()){
                for(const auto &vertex: graph.vertices()){
                    candiSeeds.push_back(vertex.first);
                }
            }
            const string &seed = candiSeeds[rand() % candiSeeds.size()];
            return sortSTB(graph, seed);
        }

        vector<string> sortSTB(const Graph &graph, const string &seed){
            if(graph.vertices().empty()){
                return {};
            }
            assert(graph.vertices().find(seed) != graph.vertices().end());

            queue<string> inVertices;
            for(const auto &vertex: graph.vertices()){
                if(graph.edgesIn(vertex.first).empty()){
                    inVertices.push(vertex.first);
                }
            }

            vector<string> result;
            unordered_set<string> verticesSorted;
            queue<string> queue1;
            queue<string> queue2;

            string toSort = seed;
            while(verticesSorted.size() < graph.vertices().size()){
                if(verticesSorted.find(toSort) == verticesSorted.end()){
                    result.push_back(toSort);
                    verticesSorted.insert(toSort);
                }
                if(verticesSorted.size() == graph.vertices().size()){
                    break;
                }
                for(const auto &edge: graph.edgesIn(toSort)){
                    if(verticesSorted.find(edge.from()) == verticesSorted.end()){
                        queue1.push(edge.from());
                    }
                }
                for(const auto &edge: graph.edgesOut(toSort)){
                    if(verticesSorted.find(edge.to()) == verticesSorted.end()){
                        queue2.push(edge.to());
                    }
                }
                string nextToSort = "";
                while(!queue1.empty()){
                    string vertex = queue1.front();
                    queue1.pop();
                    if(verticesSorted.find(vertex) == verticesSorted.end()){
                        nextToSort = vertex;
                        break;
                    }
                }
                if(nextToSort.empty()){
                    while(!queue2.empty()){
                        string vertex = queue2.front();
                        queue2.pop();
                        if(verticesSorted.find(vertex) == verticesSorted.end()){
                            nextToSort = vertex;
                            break;
                        }
                    }
                }
                if(nextToSort.empty()){
                    while(!inVertices.empty()){
                        string vertex = inVertices.front();
                        inVertices.pop();
                        if(verticesSorted.find(vertex) == verticesSorted.end()){
                            nextToSort = vertex;
                            break;
                        }
                    }
                }
                if(nextToSort.empty()){
                    for(const auto &vertex: graph.vertices()){
                        if(verticesSorted.find(vertex.first) == verticesSorted.end()
                        &&graph.edgesIn(vertex.first).empty()){
                            nextToSort = vertex.first;
                            break;
                        }
                    }
                }
                if(nextToSort.empty()){
                    for(const auto &vertex: graph.vertices()){
                        if(verticesSorted.find(vertex.first) == verticesSorted.end()){
                            nextToSort = vertex.first;
                            break;
                        }
                    }
                }
                assert(!nextToSort.empty());
                toSort = nextToSort;
            }

            return result;
        }

    }
}
