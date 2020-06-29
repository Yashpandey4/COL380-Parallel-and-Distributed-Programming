Assignment2 Part 2: Due on Apr 2
--------------------------------

In this assignment, you will learn on your own two new concepts, both coming from Google: the pagerank algorithm and mapreduce paradigm for problem solving.

**Learning part:**

-   For pagerank, check out the sample [implementations](https://github.com/louridas/pagerank) and understand how the implementations follow from the [pagerank algorithm](http://www.ams.org/publicoutreach/feature-column/fcarc-pagerank).
-   For mapreduce, this is the [Wikipedia page ](https://en.wikipedia.org/wiki/MapReduce)and this is [Google's original mapreduce paper ](https://static.googleusercontent.com/media/research.google.com/en//archive/mapreduce-osdi04.pdf). Some examples of particular problems cast as map-reduce are [finding friends](http://stevekrenzel.com/finding-friends-with-mapreduce) and [finding maximum temperature](https://www.ibm.com/analytics/hadoop/mapreduce). As in any COL380 assignment, you will be working on your own laptop. So you need not focus on the distributed computing a.k.a Hadoop part of running mapreduce. Just understand how to cast a problem into map and reduce functions.

**Thinking part:** Combine the understandings above and solve pagerank as a mapreduce problem.

**Coding part:**

-   Implement mapreduce-pagerank using [mapreduce C++ library](https://github.com/cdmh/mapreduce). Name your high level program file mr-pr-cpp.cpp, from which you call the map-reduce library functions. Pagerank benchmarks in java and python are at [tests](https://github.com/louridas/pagerank/tree/master/test). Your executable should be runnable as `./mr-pr-cpp.o ${filename}.txt -o ${filename}-pr-cpp.txt`. Format of the output files should be exactly same as the python (${filename}-pr-p.txt) and java (${filename}-pr-j.txt) output files in the benchmark. Compare correctness of the pagrank output against the given java/python outputs. (4)
-   Implement your own mapreduce library with MPI. All functions in the map-reduce library are unnecessary, implement only the functions needed for pagerank. Use this library again for mapreduce-pagerank, with high level file named as mr-pr-mpi.cpp. Executable should be run as `./mr-pr-mpi.o ${filename}.txt -o ${filename}-pr-mpi.txt`. Compare correctness of the pagerank output against the given java/python outputs. (5)
-   Instead of your own mapreduce library with MPI, use existing [mapreduce MPI library](https://mapreduce.sandia.gov/). Name the high level file mr-pr-mpi-base.cpp and execute as `./mr-pr-mpi-base.o ${filename}.txt -o ${filename}-pr-mpi-base.txt`. Compare correctness of the pagerank output against the given java/python outputs. (3)

**Performance analysis part:** Plot graphs with x-axis as benchmark ID, and y-axis as pagerank runtime, three graphs for your three executables. Compare pagerank latencies across the three implementations and comment on your observations. Prepare a 1-2 page PDF report with these graphs and observations. (3)

**To submit:** Submit a zip file containing the three cpp files, a Makefile that compiles the executables and the PDF.
