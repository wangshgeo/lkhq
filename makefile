CXX = g++-8
CXX_FLAGS = -std=c++17
CXX_FLAGS += -Wuninitialized -Wall -Wextra -Werror -pedantic -Wfatal-errors # source code quality.
CXX_FLAGS += -O3 -ffast-math # "production" version.
#CXX_FLAGS += -O0 -g # debug version.
CXX_FLAGS += -I./ # include paths.

LINK_FLAGS = -lstdc++fs # filesystem

SRCS = k-opt.cc tour.cc \
	merge/merge.cc merge/edge_map.cc merge/exchange_pair.cc merge/cycle_util.cc \
	hill_climber.cc \
	best_keeper.cc \
	hill_climb/RandomFinder.cc \
    point_quadtree/node.cc \
    point_quadtree/point_quadtree.cc \
    point_quadtree/point_inserter.cc \
    cycle_check.cc \
	MulticycleTour.cc

%.o: %.cc; $(CXX) $(CXX_FLAGS) -o $@ -c $<

OBJS = $(SRCS:.cc=.o)

all: $(OBJS); $(CXX) $^ $(LINK_FLAGS) -o k-opt.out

clean: ; rm -rf k-opt.out $(OBJS) *.dSYM
