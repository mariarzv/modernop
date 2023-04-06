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


class MaxCliqueProblem
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
    }

    void FindClique(int randomization, int iterations)
    {
        static mt19937 generator;
        for (int iteration = 0; iteration < iterations; ++iteration)
        {
            vector<int> clique;
            vector<int> candidates(neighbour_sets.size());
            for (int i = 0; i < neighbour_sets.size(); ++i)
            {
                candidates[i] = i;
            }
            shuffle(candidates.begin(), candidates.end(), generator);
            while (!candidates.empty())
            {
                int last = candidates.size() - 1;
                int rnd = GetRandom(0, min(randomization - 1, last));
                int vertex = candidates[rnd];
                clique.push_back(vertex);
                for (int c = 0; c < candidates.size(); ++c)
                {
                    int candidate = candidates[c];
                    if (neighbour_sets[vertex].count(candidate) == 0)
                    {
                        // Move the candidate to the end and pop it
                        swap(candidates[c], candidates[last]);
                        candidates.pop_back();
                        --c;
                    }
                }
                shuffle(candidates.begin(), candidates.end(), generator);
            }
            if (clique.size() > best_clique.size())
            {
                best_clique = clique;
            }
        }
    }

    void BasicGreedyRandomizedAlgorithm(int randomization, int iterations)
    {
        vector<int> best_clique;
        static mt19937 generator;

        // Repeat the algorithm for the given number of iterations
        for (int iteration = 0; iteration < iterations; ++iteration)
        {
            vector<int> clique;
            vector<int> candidates(neighbour_sets.size());

            // Initialize the candidates vector
            for (int i = 0; i < neighbour_sets.size(); ++i)
            {
                candidates[i] = i;
            }

            // Shuffle the candidates vector randomly
            shuffle(candidates.begin(), candidates.end(), generator);

            // Construct the clique
            while (!candidates.empty())
            {
                int last = candidates.size() - 1;
                int rnd = GetRandom(0, min(randomization - 1, last));
                int vertex = candidates[rnd];
                clique.push_back(vertex);

                // Remove vertices that are not adjacent to the last added vertex
                for (int c = 0; c < candidates.size(); ++c)
                {
                    int candidate = candidates[c];
                    if (neighbour_sets[vertex].count(candidate) == 0)
                    {
                        swap(candidates[c], candidates[last]);
                        candidates.pop_back();
                        --c;
                    }
                }

                // Shuffle the candidates vector randomly again
                shuffle(candidates.begin(), candidates.end(), generator);
            }

            // Update the best clique found so far
            if (clique.size() > best_clique.size())
            {
                best_clique = clique;
            }
        }

    }

    const vector<int>& GetClique()
    {
        return best_clique;
    }

    bool Check()
    {
        if (unique(best_clique.begin(), best_clique.end()) != best_clique.end())
        {
            cout << "Duplicated vertices in the clique\n";
            return false;
        }
        for (int i : best_clique)
        {
            for (int j : best_clique)
            {
                if (i != j && neighbour_sets[i].count(j) == 0)
                {
                    cout << "Returned subgraph is not a clique\n";
                    return false;
                }
            }
        }
        return true;
    }

private:
    vector<unordered_set<int>> neighbour_sets;
    vector<int> best_clique;
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

    int iterations = 10000;
    //cout << "Number of iterations: ";
    //cin >> iterations;
    int randomization = 10;
    //cout << "Randomization: ";
    //cin >> randomization;
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
    ofstream fout("clique.csv");
    fout << "File; Clique; Time (sec)\n";
    for (string file : files)
    {
        MaxCliqueProblem problem;
        problem.ReadGraphFile(file);
        clock_t start = clock();
        problem.BasicGreedyRandomizedAlgorithm(randomization, iterations);
        //problem.FindClique(randomization, iterations);
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
