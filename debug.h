enum Option {
    DEBUG_DRAW_NORMS = 0x01,
    DEBUG_FRAMERATE = 0x02,
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