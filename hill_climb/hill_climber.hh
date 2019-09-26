#pragma once

#include "GenericFinder.h"

namespace hill_climb {

class HillClimber : public GenericFinder<HillClimber>
{
    using base = GenericFinder<HillClimber>;
 public:
    using base::base;
};

}  // namespace hill_climb
