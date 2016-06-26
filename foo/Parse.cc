//
// Created by hejn on 29.05.16.
//

#include "Parse.h"

using namespace std;
using namespace Minisat;

namespace fooSAT {

    //static int generateInput(const char *path, stringstream *cnf, vector<string> *rev_mapper) {
    static int generateInput(const char *path, const char *cnf_path, vector<string> *rev_mapper) {

        set<string> edges;
        int num_vert = readGraphFile(path, &edges);

        ofstream a(cnf_path);
        ofstream *cnf = &a;

        *cnf << "p cnf " << getNumVars(num_vert) << " " << getNumClaus(num_vert) << endl;

        map<string, int> mapper;
        for (int i = 0; i < num_vert; i++) {
            for (int j = 0; j < num_vert; j++) {
                string s = "p " + to_string(j) + " " + to_string(i);
                mapper.insert(make_pair(s, mapper.size() + 1));
                rev_mapper->push_back(s);
            }
        }
        for (int i = 0; i < num_vert; i++) {
            for (int j = 0; j < num_vert; j++) {
                if (i != j) {
                    string s = "e " + to_string(i) + " " + to_string(j);
                    mapper.insert(make_pair(s, mapper.size() + 1));
                    rev_mapper->push_back(s);
                }
            }
        }

        vector<string> lits;

        for (int i = 0; i < num_vert; i++) {
            lits.clear();
            for (int j = 0; j < num_vert; j++) {
                lits.push_back("p " + to_string(i) + " " + to_string(j));
            }
            appendClaus(cnf, lits, &mapper);
        }

        for (int i = 0; i < num_vert; i++) {
            for (int j = 0; j < num_vert; j++) {
                for (int k = 0; k < num_vert; k++) {
                    if (k != j)
                        appendClaus(cnf, {"-p " + to_string(j) + " " + to_string(i),
                                          "-p " + to_string(k) + " " + to_string(i)}, &mapper);
                }
            }
        }

        for (int i = 0; i < num_vert; i++) {
            for (int j = 0; j < num_vert; j++) {
                for (int k = 0; k < num_vert; k++) {
                    if (j != i)
                        appendClaus(cnf, {"e " + to_string(i) + " " + to_string(j),
                                          "-p " + to_string(k) + " " + to_string(i),
                                          "-p " + to_string((k + 1) % num_vert) + " " + to_string(j)}, &mapper);
                }
            }
        }

        for (int i = 0; i < num_vert; i++) {
            for (int j = 0; j < num_vert; j++) {
                if (i != j) {
                    string s = edges.find(to_string(i) + " " + to_string(j)) != edges.end() ? "" : "-";
                    s += "e " + to_string(i) + " " + to_string(j);
                    appendClaus(cnf, {s}, &mapper);
                }
            }
        }

        //cnf.close();
        return num_vert * num_vert;
    }

    static void appendClaus(ofstream *cnf, vector<string> lits, map<string, int> *mapper) {
        for (string l :lits) {
            if (l[0] == '-') {
                *cnf << "-" + to_string(mapper->find(l.erase(0, 1))->second) << " ";
            } else {
                *cnf << to_string(mapper->find(l)->second) << " ";
            }
        }
        *cnf << "0" << endl;
    }

    static int getNumClaus(int num_vert) { return 2 * num_vert * num_vert * num_vert - num_vert * num_vert; }

    static int getNumVars(int num_vert) { return 2 * num_vert * num_vert - num_vert; }

    static int readGraphFile(const char *path, set<string> *edges) {

        int vert = 0;

        ifstream graph(path);

        char e;
        int i, j;

        string line;
        while (getline(graph, line)) {
            string f;
            istringstream iss(line);
            if ((iss >> e >> f >> i >> j)) {
                if (e == 'p') {
                    vert = i;
                    //printf("#vert = %d\n", vert);
                    break;
                }
            }
        }

        while (getline(graph, line)) {
            istringstream iss(line);
            if ((iss >> e >> i >> j)) {
                if (e == 'e') {
                    addEdge(edges, i - 1, j - 1);
                }
            }
        }

        graph.close();

        return vert;
    }

    static void addEdge(set<string> *edges, int i, int j) {
        edges->insert(to_string(i) + " " + to_string(j));
        //printf("%d -> %d\n", i, j);
    }

    void interpretResult(vec<lbool> *model, int nVars, vector<string> mapper) {

        set<pair<int, int>, paircomp> s;
        for (int i = 0; i < nVars; ++i) {
            if ((*model)[i] == l_True) {
                istringstream iss(mapper[i]);
                vector<string> parts{istream_iterator<string> {iss},
                                     istream_iterator<string> {}};
                int j = stoi(parts[1]);
                int k = stoi(parts[2]);
                s.insert(make_pair(j, k));
            }
        }
        for (auto f : s) {
            printf("%d ", f.second + 1);
        }

    }

}