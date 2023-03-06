#ifndef __GRAPHSORT__
#define __GRAPHSORT__

#include "common/Common.h"
#include "common/HyperGraph.h"

namespace FastCGRA
{
    namespace GraphSort
    {
        std::vector<std::string> sortBFS(const Graph &graph);
        std::vector<std::string> sortBFS(const Graph &graph, const std::string &seed);
        std::vector<std::string> sortBFS(const Graph &graph, const std::function<bool(const Vertex &, const Vertex &)> &func);
        std::vector<std::string> sortBFS(const Graph &graph, const std::function<bool(const Vertex &, const Vertex &)> &func, const std::string &seed);

        std::vector<std::string> sortDFS(const Graph &graph);
        std::vector<std::string> sortDFS(const Graph &graph, const std::string &seed);
        std::vector<std::string> sortDFS(const Graph &graph, const std::function<bool(const Vertex &, const Vertex &)> &func);
        std::vector<std::string> sortDFS(const Graph &graph, const std::function<bool(const Vertex &, const Vertex &)> &func, const std::string &seed);

        std::vector<std::string> sortTVS(const Graph &graph);//Traversal
        std::vector<std::string> sortTVS(const Graph &graph, const std::string &seed);//Traversal

        std::vector<std::string> sortSTB(const Graph &graph);//Search Tree based
        std::vector<std::string> sortSTB(const Graph &graph, const std::string &seed);//Search Tree based
    }
}

#endif
