Code style disclaimer: the style can be a little inconsistent, as there is a migration taking place. See notes below for target style.

This implements k-opt in the style of Lin-Kernighan-Helsgaun (LKH),
    but uses a quadtree to dynamically limit the search.
This approach is novel because fixed search neighborhoods are typically used.
Original LK limited search neighborhoods to 5 nearest points.
LKH limits search to 5 "alpha-nearest" points.
Alpha-nearness measures the similarity of new edges to those in a minimum spanning tree.
Computing alpha-nearness is done once before hill-climbing and takes O(n^2) work.

LKH gives up sequential-move k-optimality. This implementation preserves sequential-move k-optimality.
Note that sequential-move k-optimality is equivalent to k-optimality for k in [2, 3].

Proof of efficacy of current method:
1. Using 3-opt hill-climber on first world record (by LKH) of lrb744710 quickly returns improvements.
    Although it is an old world record, it proves that LKH (and LK) gives up k-optimality by arbitrarily fixing search neighborhoods.
    http://www.math.uwaterloo.ca/tsp/vlsi/lrb744710.log.html

Using a quad tree has the following advantages:
1. All possible improving steps (deletion of one edge and addition of one edge) are considered.
    Search neighborhoods change any time the tour changes, which is especially useful for
    cycle-merging improvements (LKH-2 does not take into account new segments).
2. O(n * (log(n) ^ (k - 1))) work complexity. Keep in mind k for each move is decided during search.

The efficacy of LK can be attributed to the fact that k-opt moves contain at least
one addition and one deletion of edges that occur at a common point, with net cost improvement.
LK looks directly for this feature instead of expensively trying all k-opt combinations.

Coming soon: This repo also implements k-opt swaps that can produce an arbitrary number of cycles.
In the case of multiple cycles, they are rejoined one at a time via a 2-opt move.
LKH also allows for multiple cycles to be formed and rejoined, but does not allow for
new joining edges to be replaced, as in the present one-at-a-time 2-opt join.
However, the implementation of non-sequential moves is complex and expensive,
    and finding good joining moves is more difficult.

Compilation:
1. Make sure "CXX" in "makefile" is set to the desired compiler.
2. Run "make".

Running:
1. Run "./k-opt.out" for usage details.

Style notes:
1. Namespaces follow directory structure. If an entire namespace is in a single header file, the header file name will be the namespace name.
2. Headers are grouped from most to least specific to this repo (e.g. repo header files will come before standard library headers).
3. Put one line break in between function definitions for convenient vim navigation via ctrl + { and ctrl + }.
4. Filenames are lower case and underscore-separated.
5. Header extension is .hh and source extension is .cc.
6. Reference and pointer symbols are right-associated, e.g. "T &ref" vs. "T& ref".
7. Exceptions are not caught.
8. Compile-time constants are capitalized with underscores.
9. Member variables are indicated as such with trailing underscores.
10. Scope start brace is not new-line separated.

TODO:
1. Memoize moves between 2 points that are better than the direct distance between them.
2. Check combinations of non-feasible non-sequential moves that may make a feasible non-sequential move.

