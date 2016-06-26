/*****************************************************************************************[Main.cc]
Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007,      Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include <zlib.h>
#include <bits/unique_ptr.h>

#include "minisat/utils/System.h"
#include "minisat/utils/ParseUtils.h"
#include "minisat/utils/Options.h"
#include "minisat/core/Dimacs.h"
#include "minisat/simp/SimpSolver.h"
#include "foo/Parse.h"
#include "foo/Parse.cc"

using namespace Minisat;
using namespace fooSAT;

//=================================================================================================


static Solver *solver;

// Terminate by notifying the solver and back out gracefully. This is mainly to have a test-case
// for this feature of the Solver as it may take longer than an immediate call to '_exit()'.
static void SIGINT_interrupt(int) { solver->interrupt(); }

// Note that '_exit()' rather than 'exit()' has to be used. The reason is that 'exit()' calls
// destructors and may cause deadlocks if a malloc/free function happens to be running (these
// functions are guarded by locks for multithreaded use).
static void SIGINT_exit(int) {
    printf("\n");
    printf("*** INTERRUPTED ***\n");
    if (solver->verbosity > 0) {
        solver->printStats();
        printf("\n");
        printf("*** INTERRUPTED ***\n");
    }
    _exit(1);
}


//=================================================================================================
// Main:

int main(int argc, char **argv) {
    const char *tmp = "./tmp.cnf";

    try {
        setX86FPUPrecision();

        // Extra options:
        //
        IntOption verb("MAIN", "verb", "Verbosity level (0=silent, 1=some, 2=more).", 0, IntRange(0, 2));
        BoolOption pre("MAIN", "pre", "Completely turn on/off any preprocessing.", true);
        BoolOption solve("MAIN", "solve", "Completely turn on/off solving after preprocessing.", true);
        StringOption dimacs("MAIN", "dimacs", "If given, stop after preprocessing and write the result to this file.");
        IntOption cpu_lim("MAIN", "cpu-lim", "Limit on CPU time allowed in seconds.\n", 0, IntRange(0, INT32_MAX));
        IntOption mem_lim("MAIN", "mem-lim", "Limit on memory usage in megabytes.\n", 0, IntRange(0, INT32_MAX));
        BoolOption strictp("MAIN", "strict", "Validate DIMACS header during parsing.", true);

        char *input_graph = argv[1];
        if (input_graph == NULL || !exists(input_graph)) {
            printf("input file: %s\n", input_graph);
            printf("FILE NOT FOUND\n");
            exit(0);
        } else {

            vector<string> reverse_mapper;
            int nVars = generateInput(input_graph, tmp, &reverse_mapper);

            parseOptions(argc, argv, true);

            SimpSolver S;
            double initial_time = cpuTime();

            if (!pre) S.eliminate(true);

            S.verbosity = verb;

            solver = &S;
            // Use signal handlers that forcibly quit until the solver will be able to respond to
            // interrupts:
            sigTerm(SIGINT_exit);

            // Try to set resource limits:
            if (cpu_lim != 0) limitTime((uint32_t) cpu_lim);
            if (mem_lim != 0) limitMemory((uint64_t) mem_lim);

            gzFile in = gzopen(tmp, "rb");

            if (S.verbosity > 0) {
                printf("============================[ Problem Statistics ]=============================\n");
                printf("|                                                                             |\n");
            }

            //if (in == NULL)
            //    printf("|  >> failed to load input file!                                              |\n");

            parse_DIMACS(in, S, (bool) strictp);
//        parse_DIMACS_foo(input, S, (bool) strictp);
            gzclose(in);

            if (S.verbosity > 0) {
                printf("|  Number of variables:  %12d                                         |\n", S.nVars());
                printf("|  Number of clauses:    %12d                                         |\n", S.nClauses());
            }

            double parsed_time = cpuTime();
            if (S.verbosity > 0)
                printf("|  Parse time:           %12.2f s                                       |\n",
                       parsed_time - initial_time);

            // Change to signal-handlers that will only notify the solver and allow it to terminate
            // voluntarily:
            sigTerm(SIGINT_interrupt);

            S.eliminate(true);
            double simplified_time = cpuTime();
            if (S.verbosity > 0) {
                printf("|  Simplification time:  %12.2f s                                       |\n",
                       simplified_time - parsed_time);
                printf("|                                                                             |\n");
            }

            if (!S.okay()) {
                if (S.verbosity > 0) {
                    printf("===============================================================================\n");
                    printf("Solved by simplification\n");
                    S.printStats();
                    printf("\n");
                }
                printf("s UNSATISFIABLE");
                exit(20);
            }

            lbool ret = l_Undef;

            if (solve) {
                vec<Lit> dummy;
                ret = S.solveLimited(dummy);
            } else if (S.verbosity > 0)
                printf("===============================================================================\n");

            if (dimacs && ret == l_Undef)
                S.toDimacs((const char *) dimacs);

            if (S.verbosity > 0) {
                S.printStats();
                printf("\n");
            }

            printf(ret == l_True ? "s SATISFIABLE\n" : ret == l_False ? "s UNSATISFIABLE\n" : "s INDETERMINATE\n");

            if (ret == l_True) {
//            for (int i = 0; i < S.nVars(); i++)
//            if (S.model[i] != l_Undef)
//                fprintf(res, "%s%s%d", (i == 0) ? "" : " ", (S.model[i] == l_True) ? "" : "-", i + 1);

                interpretResult(&(S.model), nVars, reverse_mapper);
            }

            if(exists(tmp)) {
               remove(tmp);
            }

#ifdef NDEBUG
            exit(ret == l_True ? 10 : ret == l_False ? 20 : 0);     // (faster than "return", which will invoke the destructor for 'Solver')
#else
            return (ret == l_True ? 10 : ret == l_False ? 20 : 0);
#endif
        }
    } catch (OutOfMemoryException &) {
        printf("===============================================================================\n");
        printf("INDETERMINATE\n");
        exit(0);
    }
}
