//
// Created by hejn on 29.05.16.
//

#ifndef FOOSAT_PARSE_H
#define FOOSAT_PARSE_H

#include <stdio.h>
#include <set>
#include <vector>
#include <map>
#include <utility>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "../minisat/utils/ParseUtils.h"
#include "../minisat/mtl/Vec.h"
#include "../minisat/core/SolverTypes.h"

using namespace std;
using namespace Minisat;

namespace fooSAT {

    struct paircomp {
        bool operator()(const pair<int, int> &lhs, const pair<int, int> &rhs) const {
            return (lhs.second < rhs.second || lhs.first < rhs.first);
        }
    };

    static void appendClaus(ofstream *cnf, vector<string> lits, map<string, int> *mapper);

    static int getNumClaus(int num_vert);

    static int getNumVars(int num_vert);

    static int generateInput(const char *path, const char *cnf_path, vector<string> *rev_mapper);

    static int readGraphFile(const char *path, set<string> *edges);

    static void addEdge(set<string> *edge, int i, int j);

    static void interpretResult(vec<lbool> *model, int numVars, vector<string> mapper);

    inline bool exists(const string &name);
}

#endif //FOOSAT_PARSE_H
