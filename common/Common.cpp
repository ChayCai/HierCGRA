#include "Common.h"

using namespace std; 

namespace FastCGRA
{

std::string readText(const std::string &filename)
{
    ifstream fin(filename); 
    if(!fin)
    {
        return ""; 
    }
    
    stringstream sin; 
    sin << fin.rdbuf(); 
    
    return sin.str(); 
}

std::unordered_map<std::string, std::unordered_set<std::string>> readSets(const std::string &filename)
{
    unordered_map<string, unordered_set<string>> result; 
    ifstream fin(filename);
    if(!fin)
    {
        return {}; 
    }
    while(!fin.eof())
    {
        string line; 
        getline(fin, line); 
        if(line.empty())
        {
            continue; 
        }
        istringstream sin(line); 
        string tmp; 
        sin >> tmp; 
        if(tmp.empty())
        {
            continue; 
        }
        result[tmp] = unordered_set<string>(); 
        while(!sin.eof())
        {
            string tmp2;
            sin >> tmp2; 
            if(tmp2.empty())
            {
                continue; 
            }
            result[tmp].insert(tmp2); 
        }
    }

    return result; 
}

bool writeSets(const std::unordered_map<std::string, std::unordered_set<std::string>> &sets, const std::string &filename)
{
    ofstream fout(filename); 
    for(const auto &item: sets)
    {
        fout << item.first; 
        for(const auto &elem: item.second)
        {
            fout << " " << elem; 
        }
        fout << endl; 
    }
    fout.close(); 

    return true; 
}

std::string getPrefix(const std::string &name){
    size_t posDot = name.rfind("."); 
    string result; 
    if(posDot == name.npos)
    {
        result = name; 
    }
    else
    {
        result = name.substr(0, posDot); 
    }
    return result; 
}

std::string getPostfix(const std::string &name){
    size_t posDot = name.rfind("."); 
    string result; 
    if(posDot == name.npos)
    {
        result = ""; 
    }
    else
    {
        result = name.substr(posDot+1, name.size()-1-posDot); 
    }
    return result; 
}; 

std::string getFront(const std::string &name){
    size_t posDot = name.find("."); 
    string result; 
    if(posDot == name.npos)
    {
        result = name; 
    }
    else
    {
        result = name.substr(0, posDot); 
    }
    return result; 
}

std::vector<std::string> split(const std::string &str, const std::string &sep)
{
    string tmp = str; 
    vector<string> result; 
    while(tmp.find(sep) != string::npos)
    {
        string tmp2 = tmp.substr(0, tmp.find(sep)); 
        result.push_back(tmp2); 
        tmp = tmp.substr(tmp2.size() + sep.size()); 
    }
    if(tmp.size() > 0)
    {
        result.push_back(tmp); 
    }
    return result; 
}

std::string removeSpace(const std::string &str)
{
    size_t idx = 0;
    string result = str;
    for (size_t jdx = 0; jdx < str.size(); jdx++) {
        if (str[jdx] != ' ') {
            result[idx++] = str[jdx];
        }
    }
    return result.substr(0, idx);
}

std::string removeLineBreak(const std::string &str)
{
    size_t idx = 0;
    string result = str;
    for (size_t jdx = 0; jdx < str.size(); jdx++) {
        if (str[jdx] != '\n' && str[jdx] != '\r') {
            result[idx++] = str[jdx];
        }
    }
    return result.substr(0, idx);
}

std::string join(const std::vector<std::string> &strs, const std::string &sep)
{
    if(strs.size() < 1)
    {
        return ""; 
    }
    string result = strs[0];  
    for(size_t idx = 1; idx < strs.size(); idx++)
    {
        result += sep + strs[idx]; 
    }
    
    return result; 
}

std::vector<std::string> parseArr(const std::string &str)
{
    string tmp; 
    for(const auto &ch: str)
    {
        if(ch == '(' || ch == ')' || ch == ',')
        {
            continue; 
        }
        tmp += ch; 
    }

    return split(tmp, " "); 
}

std::vector<double> parseVec(const std::string &str)
{
    string tmp; 
    for(const auto &ch: str)
    {
        if(ch == '(' || ch == ')' || ch == ',')
        {
            continue; 
        }
        tmp += ch; 
    }

    vector<string> splited = split(tmp, " "); 
    vector<double> results(splited.size()); 
    for(size_t idx = 0; idx < splited.size(); idx++)
    {
        results[idx] = as<double>(splited[idx]); 
    }

    return results; 
}

std::string replaceChar(const std::string &str, char a, char b)
{
    string newStr = str; 
    for(auto &ch: newStr)
    {
        if(ch == a)
        {
            ch = b; 
        }
    }
    
    return newStr; 
}

RandomSelector::RandomSelector(size_t size): _accum(size, 1.0)
{
    assert(size > 0); 
    for(size_t idx = 1; idx < _accum.size(); idx++)
    {
        _accum[idx] += _accum[idx-1]; 
    }
    for(size_t idx = 0; idx < _accum.size(); idx++)
    {
        _accum[idx] /= _accum[_accum.size()-1]; 
    }
    if(_accum[_accum.size()-1] < 1.0)
    {
        _accum[_accum.size()-1] = 1.0 + 1e-3; 
    }
}

RandomSelector::RandomSelector(const vector<double> &prob): _accum(prob)
{
    assert(prob.size() > 0); 
    for(size_t idx = 1; idx < _accum.size(); idx++)
    {
        _accum[idx] += _accum[idx-1]; 
    }
    for(size_t idx = 0; idx < _accum.size(); idx++)
    {
        _accum[idx] /= _accum[_accum.size()-1]; 
    }
    if(_accum[_accum.size()-1] < 1.0)
    {
        _accum[_accum.size()-1] = 1.0 + 1e-3; 
    }
}

size_t RandomSelector::select() const
{
    double randNum = static_cast<double>(rand()) / static_cast<double>(RAND_MAX); 
    for(size_t idx = 0; idx < _accum.size(); idx++)
    {
        if(randNum < _accum[idx])
        {
            return idx; 
        }
    }
    return static_cast<size_t>(-1); 
}

vector<int> assignment(vector<vector<int>> a) {
    const int INF = 1000 * 1000 * 1000;

    int n = a.size();
    int m = n * 2 + 2;
    vector<vector<int>> f(m, vector<int>(m));
    int s = m - 2, t = m - 1;
    int cost = 0;
    while (true) {
        vector<int> dist(m, INF);
        vector<int> p(m);
        vector<bool> inq(m, false);
        queue<int> q;
        dist[s] = 0;
        p[s] = -1;
        q.push(s);
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            inq[v] = false;
            if (v == s) {
                for (int i = 0; i < n; ++i) {
                    if (f[s][i] == 0) {
                        dist[i] = 0;
                        p[i] = s;
                        inq[i] = true;
                        q.push(i);
                    }
                }
            } else {
                if (v < n) {
                    for (int j = n; j < n + n; ++j) {
                        if (f[v][j] < 1 && dist[j] > dist[v] + a[v][j - n]) {
                            dist[j] = dist[v] + a[v][j - n];
                            p[j] = v;
                            if (!inq[j]) {
                                q.push(j);
                                inq[j] = true;
                            }
                        }
                    }
                } else {
                    for (int j = 0; j < n; ++j) {
                        if (f[v][j] < 0 && dist[j] > dist[v] - a[j][v - n]) {
                            dist[j] = dist[v] - a[j][v - n];
                            p[j] = v;
                            if (!inq[j]) {
                                q.push(j);
                                inq[j] = true;
                            }
                        }
                    }
                }
            }
        }

        int curcost = INF;
        for (int i = n; i < n + n; ++i) {
            if (f[i][t] == 0 && dist[i] < curcost) {
                curcost = dist[i];
                p[t] = i;
            }
        }
        if (curcost == INF)
            break;
        cost += curcost;
        for (int cur = t; cur != -1; cur = p[cur]) {
            int prev = p[cur];
            if (prev != -1)
                f[cur][prev] = -(f[prev][cur] = 1);
        }
    }

    vector<int> answer(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (f[i][j + n] == 1)
                answer[i] = j;
        }
    }
    return answer;
}

void copyfile(const string &from, const string &to)
{
    if(from == to){
        return;
    }
    ifstream fin(from); 
    ofstream fout(to);
    if(!fin)
    {
        cerr << "Common::copyfile WARN: cannot open the file:" << from; 
        return ; 
    }
    string buf;
    while(getline(fin, buf)){
        fout << buf << endl;
    }
    fin.close();
    fout.close();
}


}


