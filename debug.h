#include <iostream>

enum Option {
    DEBUG_DRAW_NORMS = 0x01,
    DEBUG_FRAMERATE = 0x02,
    DEBUG_FREEHAND_CAMERA = 0x04,
    DEBUG_VERBOSE = 0x08,
};

unsigned int options;

bool DebugActive(Option option)
{
    return options & option;
}

void SetDebug(Option option, bool setActive = true)
{
    if (setActive) {
        options |= option;
    }
    else
    {
        options &= ~option;
    }
}

void PrintLog(string log, bool verbose = true)
{
    if (!verbose || DebugActive(DEBUG_VERBOSE))
        std::cout << "log: " << log << std::endl;
}