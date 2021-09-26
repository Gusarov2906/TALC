#pragma once
#include <vector>

#include "State.h"

class Transition
{
public:
    Transition(State from, State to, char symbol);

private:
    std::vector<State> m_from;
    std::vector<State> m_to;
    char m_symbol;
};

