This implements Lin-Kernighan-Helsgaun-style k-opt, but using a quad tree to limit the search.
The original LK algorithm limited search neighborhoods for each point to 5 nearest neighbors.
The LKH algorithm uses "alpha-nearness" to determine search neighborhoods,
which essentially considers points neighbors if new edges are similar enough to 1-tree edges.
Constructing the candidate set is done once before hill-climbing and takes O(n ^ 2) work.

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

TODO:
1. Check if output directory exists before writing out paths to file (currently silently fails).
2. Length table reset (to remove storage of distance calculations no longer used).
3. Storage of point sets (these only change upon segment length changes).


