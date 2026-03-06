
/**
 * x11 App helper in matters Wayland.
 */

// Std C and c++.
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

// X11.
#include <X11/Xlib.h>

// Application.
#include "xDisplayHelper.h"


/**
 * Class instantiation.
 */
xDisplayHelper::xDisplayHelper() {
    mDisplay = nullptr;
    mSessionType = nullptr;

    // Check for display error.
    const char* WAYLAND_DISPLAY = getenv("WAYLAND_DISPLAY");
    if (WAYLAND_DISPLAY && strlen(WAYLAND_DISPLAY) > 0) {
        //const char* TEMP = WAYLAND_DISPLAY ?
        //    WAYLAND_DISPLAY : "";
        //cout << XCOLOR_RED << endl << "xDisplayHelper: Wayland "
        //    "Display Manager is detected, FATAL." <<
        //    XCOLOR_NORMAL << endl;
        //cout << XCOLOR_YELLOW << "xDisplayHelper: env var "
        //    "$WAYLAND_DISPLAY: \"" << TEMP <<
        //    "\"." << XCOLOR_NORMAL << endl;
        return;
    }

    // Check for session error.
    mSessionType = getenv("XDG_SESSION_TYPE");
    if (strcmp(mSessionType, "x11") != 0) {
        //cout << endl << XCOLOR_RED << "xDisplayHelper: No X11 "
        //    "Session type is detected, FATAL." <<
        //    XCOLOR_NORMAL << endl;
        //cout << XCOLOR_YELLOW << "xDisplayHelper: env var "
        //    "$XDG_SESSION_TYPE: \"" << mSessionType <<
        //    "\"." << XCOLOR_NORMAL << endl;
        return;
    }

    // Check for display access.
    mDisplay = XOpenDisplay(NULL);
    //if (mDisplay == NULL) {
        //cout << XCOLOR_RED << "xDisplayHelper: X11 Display "
        //    "does not seem to be available (Are you Wayland?) "
        //    "FATAL." << XCOLOR_NORMAL << endl;
    //  return;
    //}
}

Display* xDisplayHelper::getDisplay() {
    return mDisplay;
}

char* xDisplayHelper::getSessionType() {
    return mSessionType;
}
