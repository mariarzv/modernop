#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <time.h>
using namespace std;

class Vertex {
public:
    Vertex(int idx) : index(idx), color(-1), colored(false) {}

    int GetIndex() const { return index; }
    int GetColor() const { return color; }
    int GetNumNeighbors() const { return neighborsnum; }
    bool IsColored() const { return colored; }
    const std::vector<int>& GetNeighbors() const { return neighbors; }

    void SetColor(int c) { color = c; colored = true; }
    void AddNeighbor(int neighbor_idx) { neighbors.push_back(neighbor_idx); }
    void SetNumNeighbors(int n) { neighborsnum = n; }

private:
    int index;             
    int neighborsnum;     
    int color;            
    bool colored;          
    std::vector<int> neighbors;   
};

class ColoringProblem
{
public:
    int GetRandom(int a, int b)
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
                colors.resize(vertices + 1);
            }
            else
            {
                int start, finish;
                line_input >> command >> start >> finish;
                // edges in DIMACS file can be repeated, but it is not a problem for our sets
                neighbour_sets[start - 1].insert(finish - 1);
                neighbour_sets[finish - 1].insert(start - 1);
            }
        }
    }

    void GreedyGraphColoring()
    {
        vector<int> uncolored_vertices(neighbour_sets.size());
        for (size_t i = 0; i < uncolored_vertices.size(); ++i)
            uncolored_vertices[i] = i;

        while (!uncolored_vertices.empty())
        {
            int index = GetRandom(0, uncolored_vertices.size() - 1);
            int vertex = uncolored_vertices[index];
            int color = GetRandom(1, maxcolor);
            for (int neighbour : neighbour_sets[vertex])
            {
                if (color == colors[neighbour])
                {
                    color = ++maxcolor;
                    break;
                }
            }
            colors[vertex] = color;
            // Move the colored vertex to the end and pop it
            swap(uncolored_vertices[uncolored_vertices.size() - 1], uncolored_vertices[index]);
            uncolored_vertices.pop_back();
        }
    }

    void WelshPowellGraphColoring()
    {
        sorted_vertices[0]->SetColor(1);

        for (int i = 1; i < sorted_vertices.size(); i++) {
            Vertex* v = sorted_vertices[i];
            if (!v->IsColored()) {
                // check if v is connected to any colored vertices
                bool connected_to_colored = false;
                for (int j = 0; j < v->GetNeighbors().size(); j++) {
                    int neighbor_idx = v->GetNeighbors()[j];
                    Vertex* neighbor = &vertices[neighbor_idx];
                    if (neighbor->IsColored()) {
                        connected_to_colored = true;
                        break;
                    }
                }

                if (!connected_to_colored) {
                    // color of v = 1
                    v->SetColor(1);
                }
                else {
                    // find the smallest available color that is not being used by v's neighbors
                    unordered_set<int> neighbor_colors;
                    for (int j = 0; j < v->GetNeighbors().size(); j++) {
                        int neighbor_idx = v->GetNeighbors()[j];
                        Vertex* neighbor = &vertices[neighbor_idx];
                        if (neighbor->IsColored()) {
                            neighbor_colors.insert(neighbor->GetColor());
                        }
                    }

                    int new_color = 1;
                    while (neighbor_colors.count(new_color) > 0) {
                        new_color++;
                    }

                    // set the color of v to the smallest available color
                    v->SetColor(new_color);
                    maxcolor = max(maxcolor, new_color);
                }
            }
        }

        for (int i = 0; i < vertices.size(); i++) {
            colors[i] = vertices[i].GetColor();
        }
    }

    bool Check()
    {
        for (size_t i = 0; i < neighbour_sets.size(); ++i)
        {
            if (colors[i] == 0)
            {
                cout << "Vertex " << i + 1 << " is not colored\n";
                return false;
            }
            for (int neighbour : neighbour_sets[i])
            {
                if (colors[neighbour] == colors[i])
                {
                    cout << "Neighbour vertices " << i + 1 << ", " << neighbour + 1 << " have the same color\n";
                    return false;
                }
            }
        }
        return true;
    }

    int GetNumberOfColors()
    {
        return maxcolor;
    }

    const vector<int>& GetColors()
    {
        return colors;
    }

    void ConstructVertexClassList(){
        for (int i = 0; i < neighbour_sets.size(); i++) {
            vertices.push_back(Vertex(i)); 
            Vertex& v = vertices.back(); 
            v.SetNumNeighbors(neighbour_sets[i].size()); 
            for (int neighbor_idx : neighbour_sets[i]) {
                v.AddNeighbor(neighbor_idx); 
            }
        }
    }

    void SortVertexClassObjects() {
        for (Vertex& v : vertices) {
            sorted_vertices.push_back(&v);
        }
        sort(sorted_vertices.begin(), sorted_vertices.end(), [](const Vertex* v1, const Vertex* v2) {
            return v1->GetNumNeighbors() > v2->GetNumNeighbors();
        });
    }

private:
    vector<int> colors;
    int maxcolor = 1;
    vector<unordered_set<int>> neighbour_sets;
    vector<Vertex> vertices;
    vector<Vertex*> sorted_vertices;
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
    string graphF = projFolder + "\\..\\graphs\\";

    vector<string> files = { graphF + "myciel3.col", graphF + "myciel7.col", 
        graphF + "school1.col", graphF + "school1_nsh.col",
        graphF + "anna.col", 
        graphF + "miles1000.col", graphF + "miles1500.col",
        graphF + "le450_5a.col", graphF + "le450_15b.col", 
        graphF + "queen11_11.col"};
    ofstream fout("color.csv");
    fout << "Instance; Colors; Time (sec)\n";
    cout << "Instance; Colors; Time (sec)\n";
    for (string file : files)
    {
        ColoringProblem problem;
        problem.ReadGraphFile(file);
        clock_t start = clock();
        problem.ConstructVertexClassList();
        problem.SortVertexClassObjects();

        //problem.GreedyGraphColoring();
        problem.WelshPowellGraphColoring();
        if (!problem.Check())
        {
            fout << "*** WARNING: incorrect coloring: ***\n";
            cout << "*** WARNING: incorrect coloring: ***\n";
        }
        fout << file << "; " << problem.GetNumberOfColors() << "; " << double(clock() - start) / CLOCKS_PER_SEC << "; " << problem.GetColors()[0] << '\n';
        cout << file << "; " << problem.GetNumberOfColors() << "; " << double(clock() - start) / CLOCKS_PER_SEC << '\n';
    }
    fout.close();
    return 0;
}