cmake_minimum_required(VERSION 3.5)
project(fooSAT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cc minisat/core/Solver.cc minisat/core/SolverTypes.h minisat/core/Dimacs.h minisat/utils/Options.cc minisat/utils/ParseUtils.h minisat/utils/System.cc minisat/simp/SimpSolver.cc
        minisat/mtl/Alg.h minisat/mtl/Alloc.h minisat/mtl/Heap.h minisat/mtl/IntMap.h minisat/mtl/IntTypes.h minisat/mtl/Map.h minisat/mtl/Queue.h minisat/mtl/Rnd.h minisat/mtl/Sort.h minisat/mtl/Vec.h minisat/mtl/XAlloc.h foo/Parse.cc foo/Parse.h)
add_executable(fooSAT ${SOURCE_FILES})

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(fooSAT ${ZLIB_LIBRARIES})
endif (ZLIB_FOUND)