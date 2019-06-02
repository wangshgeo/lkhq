#pragma once

#include "GenericFinder.h"

class NonsequentialFinder : public GenericFinder<NonsequentialFinder>
{
    using base = GenericFinder<NonsequentialFinder>;
public:
    using base::base;
    void final_move_check();

};

inline void NonsequentialFinder::final_move_check()
{
    base::final_move_check();
}

