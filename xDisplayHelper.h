
#pragma once

/**
 * x11 App helper in matters Wayland.
 */

// Std C and c++.

// X11.
#include <X11/Xlib.h>

/**
 * Class def.
 */
class xDisplayHelper {
    public:
        xDisplayHelper();
        ~xDisplayHelper();

        Display* getDisplay();
        char* getSessionType();

    private:
        Display* mDisplay;
        char* mSessionType;
};
