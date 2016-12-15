#include "common.hpp"
#include "nfa.hpp"

#pragma once

NFA kleene(const NFA & a);
NFA cat(const NFA & a, const NFA & b);
