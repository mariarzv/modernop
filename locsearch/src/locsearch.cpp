#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <random>
#include <unordered_set>
#include <algorithm>
using namespace std;


class MaxCliqueTabuSearch
{
public:
    static int GetRandom(int a, int b)
    {
        static mt19937 generator;
        uniform_int_distribution<int> uniform(a, b);
        return uniform(generator);
    }

    void ReadGraphFile(string filename)
    {
        ifstream fin(filename);
        string line;
        int vertices = 0, edges = 0;
        while (getline(fin, line))
        {
            if (line[0] == 'c')
            {
                continue;
            }

            stringstream line_input(line);
            char command;
            if (line[0] == 'p')
            {
                string type;
                line_input >> command >> type >> vertices >> edges;
                neighbour_sets.resize(vertices);
                qco.resize(vertices);
                index.resize(vertices, -1);
                non_neighbours.resize(vertices);
            }
            else
            {
                int start, finish;
                line_input >> command >> start >> finish;
                // Edges in DIMACS file can be repeated, but it is not a problem for our sets
                neighbour_sets[start - 1].insert(finish - 1);
                neighbour_sets[finish - 1].insert(start - 1);
            }
        }
        for (int i = 0; i < vertices; ++i)
        {
            for (int j = 0; j < vertices; ++j)
            {
                if (neighbour_sets[i].count(j) == 0 && i != j)
                    non_neighbours[i].insert(j);
            }
        }
    }

    void RunSearch(int starts, int randomization)
    {
        for (int iter = 0; iter < starts; ++iter)
        {
            ClearClique();
            for (size_t i = 0; i < neighbour_sets.size(); ++i)
            {
                qco[i] = i;
                index[i] = i;
            }
            RunInitialHeuristic(randomization);
            c_border = q_border;
            int swaps = 0;
            while (swaps < 100)
            {
                if (!Move())
                {
                    if (!Swap1To1())
                    {
                        break;
                    }
                    else
                    {
                        ++swaps;
                    }
                }
            }
            if (q_border > best_clique.size())
            {
                best_clique.clear();
                for (int i = 0; i < q_border; ++i)
                    best_clique.insert(qco[i]);
            }
        }
    }

    const unordered_set<int>& GetClique()
    {
        return best_clique;
    }

    bool Check()
    {
        for (int i : best_clique)
        {
            for (int j : best_clique)
            {
                if (i != j && neighbour_sets[i].count(j) == 0)
                {
                    cout << "Returned subgraph is not clique\n";
                    return false;
                }
            }
        }
        return true;
    }

    void ClearClique()
    {
        q_border = 0;
        c_border = 0;
    }

private:
    vector<unordered_set<int>> neighbour_sets;
    vector<unordered_set<int>> non_neighbours;
    unordered_set<int> best_clique;
    vector<int> qco;
    vector<int> index;
    int q_border = 0;
    int c_border = 0;

    int ComputeTightness(int vertex)
    {
        int tightness = 0;
        for (int i = 0; i < q_border; ++i)
        {
            if (non_neighbours[qco[i]].count(vertex))
                ++tightness;
        }
        return tightness;
    }

    void SwapVertices(int vertex, int border)
    {
        int vertex_at_border = qco[border];
        swap(qco[index[vertex]], qco[border]);
        swap(index[vertex], index[vertex_at_border]);
    }

    void InsertToClique(int i)
    {
        for (int j : non_neighbours[i])
        {
            if (ComputeTightness(j) == 0)
            {
                --c_border;
                SwapVertices(j, c_border);
            }
        }
        SwapVertices(i, q_border);
        ++q_border;
    }

    void RemoveFromClique(int k)
    {
        for (int j : non_neighbours[k])
        {
            if (ComputeTightness(j) == 1)
            {
                SwapVertices(j, c_border);
                c_border++;
            }
        }
        --q_border;
        SwapVertices(k, q_border);
    }

    bool Swap1To1()
    {
        for (int counter = 0; counter < q_border; ++counter)
        {
            int vertex = qco[counter];
            for (int i : non_neighbours[vertex])
            {
                if (ComputeTightness(i) == 1)
                {
                    RemoveFromClique(vertex);
                    InsertToClique(i);
                    return true;
                }
            }
        }
        return false;
    }

    bool Move()
    {
        if (c_border == q_border)
            return false;
        int vertex = qco[q_border];
        InsertToClique(vertex);
        return true;
    }

    void RunInitialHeuristic(int randomization)
    {
        static mt19937 generator;
        vector<int> candidates(neighbour_sets.size());
        for (size_t i = 0; i < neighbour_sets.size(); ++i)
        {
            candidates[i] = i;
        }
        shuffle(candidates.begin(), candidates.end(), generator);
        while (!candidates.empty())
        {
            int last = candidates.size() - 1;
            int rnd = GetRandom(0, min(randomization - 1, last));
            int vertex = candidates[rnd];
            SwapVertices(vertex, q_border);
            ++q_border;
            for (int c = 0; c < candidates.size(); ++c)
            {
                int candidate = candidates[c];
                if (neighbour_sets[vertex].count(candidate) == 0)
                {
                    // Move the candidate to the end and pop it
                    swap(candidates[c], candidates[candidates.size() - 1]);
                    candidates.pop_back();
                    --c;
                }
            }
            shuffle(candidates.begin(), candidates.end(), generator);
        }
    }
};

string GetProjectFolder() {
    string path = __FILE__;
    size_t found = path.find_last_of("\\/");

    // get current project folder
    string folderPath = path.substr(0, found);
    return folderPath;
}

int main()
{
    string projFolder = GetProjectFolder();
    string cliqueF = projFolder + "\\..\\cliques\\";

    int iterations;
    cout << "Number of iterations: ";
    cin >> iterations;
    int randomization;
    cout << "Randomization: ";
    cin >> randomization;
    vector<string> files = {
        cliqueF + "brock200_1.clq", cliqueF + "brock200_2.clq", cliqueF + "brock200_3.clq", cliqueF + "brock200_4.clq",
        cliqueF + "brock400_1.clq", cliqueF + "brock400_2.clq", cliqueF + "brock400_3.clq", cliqueF + "brock400_4.clq",
        cliqueF + "C125.9.clq",
        cliqueF + "gen200_p0.9_44.clq", cliqueF + "gen200_p0.9_55.clq",
        cliqueF + "hamming8-4.clq",
        cliqueF + "johnson16-2-4.clq", cliqueF + "johnson8-2-4.clq",
        cliqueF + "keller4.clq",
        cliqueF + "MANN_a27.clq", cliqueF + "MANN_a9.clq",
        cliqueF + "p_hat1000-1.clq", cliqueF + "p_hat1000-2.clq", cliqueF + "p_hat1500-1.clq", cliqueF + "p_hat300-3.clq", cliqueF + "p_hat500-3.clq",
        cliqueF + "san1000.clq", cliqueF + "sanr200_0.9.clq", cliqueF + "sanr400_0.7.clq"
    };
    ofstream fout("clique_tabu.csv");
    fout << "File; Clique; Time (sec)\n";
    for (string file : files)
    {
        MaxCliqueTabuSearch problem;
        problem.ReadGraphFile(file);
        clock_t start = clock();
        problem.RunSearch(iterations, randomization);
        if (!problem.Check())
        {
            cout << "*** WARNING: incorrect clique ***\n";
            fout << "*** WARNING: incorrect clique ***\n";
        }
        fout << file << "; " << problem.GetClique().size() << "; " << double(clock() - start) / CLOCKS_PER_SEC << '\n';
        cout << file << ", result - " << problem.GetClique().size() << ", time - " << double(clock() - start) / CLOCKS_PER_SEC << '\n';
    }
    fout.close();
    return 0;
}
