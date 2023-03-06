#ifndef __FASTCGRA_COMMON__
#define __FASTCGRA_COMMON__

#define TIME_SEED

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <assert.h>

#include <sys/time.h>
#include <unistd.h>

#include <omp.h>

namespace FastCGRA
{
    std::string readText(const std::string &filename); 
    std::unordered_map<std::string, std::unordered_set<std::string>> readSets(const std::string &filename); 
    bool writeSets(const std::unordered_map<std::string, std::unordered_set<std::string>> &sets, const std::string &filename); 
    std::string getPrefix(const std::string &name); 
    std::string getPostfix(const std::string &name); 
    std::string getFront(const std::string &name);
    std::vector<std::string> split(const std::string &str, const std::string &sep); 
    std::string removeSpace(const std::string &str);
    std::string removeLineBreak(const std::string &str);
    std::string join(const std::vector<std::string> &strs, const std::string &sep); 
    std::vector<std::string> parseArr(const std::string &str); 
    std::vector<double> parseVec(const std::string &str); 
    std::string replaceChar(const std::string &str, char a, char b); 
    std::vector<int> assignment(std::vector<std::vector<int>> a);
    void copyfile(const std::string &from, const std::string &to);

    class RandomSelector
    {
    private: 
        std::vector<double> _accum; 
    public: 
        RandomSelector() = delete; 
        RandomSelector(size_t size); 
        RandomSelector(const std::vector<double> &prob); 
        size_t select() const; 
    }; 

    template<typename Number>
    std::string num2str(Number n)
    {
        std::ostringstream sout; 
        sout << n; 
        return sout.str(); 
    }

    template<typename Number>
    Number str2num(const std::string &str)
    {
        std::istringstream sin(str); 
        Number n; 
        sin >> n; 
        return n; 
    }

    template<typename Number>
    Number as(const std::string &str)
    {
        std::istringstream sin(str); 
        Number n; 
        sin >> n; 
        return n; 
    }

    template<typename Type1, typename Type2>
    std::ostream &operator << (std::ostream &out, const std::pair<Type1, Type2> &p)
    {
        out << "(" << p.first << ", " << p.second << ")"; 

        return out; 
    }

    template<typename TypeValue>
    std::ostream &operator << (std::ostream &out, const std::unordered_map<std::string, TypeValue> &hashTable)
    {
        for(const auto &item: hashTable)
        {
            out << " -> " << item.first << ": " << item.second << std::endl; 
        }

        return out; 
    }

    template<typename TypeValue>
    std::ostream &operator << (std::ostream &out, const std::unordered_multimap<std::string, TypeValue> &hashTable)
    {
        for(const auto &item: hashTable)
        {
            out << " -> " << item.first << ": " << item.second << std::endl; 
        }

        return out; 
    }

    template<typename TypeValue>
    std::ostream &operator << (std::ostream &out, const std::unordered_set<TypeValue> &setValues)
    {
        out << "("; 
        for(const auto &item: setValues)
        {
            out << item << ", "; 
        }
        out << ")"; 

        return out; 
    }

    template<typename TypeValue>
    std::ostream &operator << (std::ostream &out, const std::unordered_multiset<TypeValue> &setValues)
    {
        out << "("; 
        for(const auto &item: setValues)
        {
            out << item << ", "; 
        }
        out << ")"; 

        return out; 
    }

    template<typename TypeValue>
    std::ostream &operator << (std::ostream &out, const std::vector<TypeValue> &values)
    {
        out << "("; 
        for(const auto &item: values)
        {
            out << item << ", "; 
        }
        out << ")"; 

        return out; 
    }

    template<typename TypeValue>
    std::vector<TypeValue> &operator << (std::vector<TypeValue> &vec, const TypeValue &value)
    {
        vec.push_back(value); 

        return vec; 
    }

    template<typename TypeValue>
    std::vector<TypeValue> operator + (const std::vector<TypeValue> &vec, const TypeValue &value)
    {
        std::vector<TypeValue> result = vec; 
        result.push_back(value); 

        return result; 
    }

    template<typename TypeValue>
    std::vector<TypeValue> operator + (const TypeValue &value, const std::vector<TypeValue> &vec)
    {
        std::vector<TypeValue> result; 
        result.push_back(value); 
        for(const auto &elem: vec)
        {
            result.push_back(elem); 
        }

        return result; 
    }

}

#endif
