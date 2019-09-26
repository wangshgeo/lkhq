#pragma once

#include "GenericFinder.h"

namespace hill_climb {

class OptimalFinder : public GenericFinder<OptimalFinder>
{
    using base = GenericFinder<OptimalFinder>;
public:
    using base::base;

};

}  // namespace hill_climb

