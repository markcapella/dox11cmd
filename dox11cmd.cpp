/** *********************************************************************
 ** Main dox11cmd.
 **/

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <iostream>

#include <string>
using namespace std;

#include <vector>
#include <algorithm>


// Forward declare supported commands.
void doDisplayUseage();

void doListStackedWindowNames();
void doRaiseWindow(string);
void doLowerWindow(string);
void doMapWindow(string);
void doUnmapWindow(string);

// Forward declare helpers.
Window getWindowMatchName(string);
unsigned long getRootWindowProperty(Atom, Window**);
unsigned long getX11StackedWindowsList(Window**);


// Globals consts.
vector<string> mCmdListStrings
{"list", "raise", "lower", "map", "unmap"};
enum M_COMMAND_STRING
{ LIST,   RAISE,   LOWER,   MAP,   UNMAP};

// Global vars.
string mCmdString = "";
string mArgString = "";

Display* mDisplay;


/** *********************************************************************
 ** Module Entry.
 **/
int main(int argc, char **argv) {
    // String, and guard the inputs.
    if (argc > 1) {
        mCmdString = string(argv[1]);
        if (argc > 2) {
            mArgString = string(argv[2]);
        }
    }

    // Get global Display.
    mDisplay = XOpenDisplay(":0");
    if (!mDisplay) {
        fprintf(stdout, "Cannot open X11 display ?? FATAL.\n");
        exit(1);
    }

    // String switch on users command.
    switch (distance(mCmdListStrings.begin(),
            find(mCmdListStrings.begin(), mCmdListStrings.end(),
        mCmdString))) {
        case LIST:
            doListStackedWindowNames();
            break;
        case RAISE:
            doRaiseWindow(mArgString);
            break;
        case LOWER:
            doLowerWindow(mArgString);
            break;
        case MAP:
            doMapWindow(mArgString);
            break;
        case UNMAP:
            doUnmapWindow(mArgString);
            break;

        default:
            doDisplayUseage();
            doListStackedWindowNames();
    }

    XCloseDisplay(mDisplay);
}

/** *********************************************************************
 ** Display useage (All Supported Commands).
 **/
void doDisplayUseage() {
    cout << "\nUseage: dox11cmd commmand windowName." << endl << endl;

    cout << "Available commands are: " << endl << endl;
    for (auto thisCmd = mCmdListStrings.begin();
        thisCmd != mCmdListStrings.end(); ++thisCmd){
        cout << *thisCmd << "  ";
    }
    cout << "\n" << endl;
}

/** *********************************************************************
 ** Supported Commands - list.
 **/
void doListStackedWindowNames() {
    Window* stackedWins;
    int numberOfStackedWins = getX11StackedWindowsList(&stackedWins);

    cout << "Available Windows in Stacked Order above desktop:\n" << endl;
    for (int i = numberOfStackedWins - 1; i >= 0; i--) {
        XTextProperty titleBarName;
        XGetWMName(mDisplay, stackedWins[i], &titleBarName);

        fprintf(stdout, "wmctrl: [window: 0x%08lx]   windowName: %s\n",
            stackedWins[i], titleBarName.value);

        XFree(titleBarName.value);
    }
}

/** *********************************************************************
 ** Supported Commands - raise.
 **/
void doRaiseWindow(string argString) {
    Window window = getWindowMatchName(argString);
    if (!window) {
        fprintf(stdout, "\nCannot find a Window by that name.\n");
        doListStackedWindowNames();
        return;
    }

    if(!XRaiseWindow(mDisplay, window)) {
        fprintf(stdout, "Error encountered trying to raise "
            "the Window ?? FATAL.\n");
        return;
    }
}

/** *********************************************************************
 ** Supported Commands - lower.
 **/
void doLowerWindow(string argString) {
    Window window = getWindowMatchName(argString);
    if (!window) {
        fprintf(stdout, "\nCannot find a Window by that name.\n");
        doListStackedWindowNames();
        return;
    }

    // Lower target, by raising all other windows above it.
    // Ignore Desktop @ [0].
    Window* stackedWins;
    int numberOfStackedWins = getX11StackedWindowsList(&stackedWins);
    for (int i = 1; i < numberOfStackedWins; i++) {
        if (window == stackedWins[i]) {
            continue;
        }
        if (!XRaiseWindow(mDisplay, stackedWins[i])) {
            fprintf(stdout, "Error trying to lower the Window.\n");
            return;
        }
    }
}

/** *********************************************************************
 ** Supported Commands - map.
 **/
void doMapWindow(string argString) {
    Window window = getWindowMatchName(argString);
    if (!window) {
        fprintf(stdout, "\nCannot find a Window by that name.\n");
        doListStackedWindowNames();
        return;
    }

    if(!XMapWindow(mDisplay, window)) {
        fprintf(stdout, "Error encountered trying to map "
            "the Window ?? FATAL.\n");
        return;
    }
}

/** *********************************************************************
 ** Supported Commands unmap.
 **/
void doUnmapWindow(string argString) {
    Window window = getWindowMatchName(argString);
    if (!window) {
        fprintf(stdout, "\nCannot find a Window by that name.\n");
        doListStackedWindowNames();
        return;
    }

    if(!XUnmapWindow(mDisplay, window)) {
        fprintf(stdout, "Error encountered trying to unmap "
            "the Window ?? FATAL.\n");
        return;
    }
}

/** *********************************************************************
 ** Helper to search for Window Id whose name matches the users request.
 **/
Window getWindowMatchName(string name) {
    Window* stackedWins;
    int numberOfStackedWins = getX11StackedWindowsList(&stackedWins);

    for (int i = numberOfStackedWins - 1; i >= 0; i--) {
        XTextProperty titleBarName;
        XGetWMName(mDisplay, stackedWins[i], &titleBarName);

        // Modern strings for compare.
        string titleBarString(reinterpret_cast<char const*>
            (titleBarName.value));
        string nameString(name);

        // Exact match includes empty nameString.
        if (titleBarString == nameString) {
            XFree(titleBarName.value);
            return stackedWins[i];
        }

        // Else grab partial.
        if (!nameString.empty() &&
            titleBarString.find(nameString) != string::npos) {
            XFree(titleBarName.value);
            return stackedWins[i];
        }

        XFree(titleBarName.value);
    }

    return None;
}

/** *********************************************************************
 ** Helper method gets the X11 stacked windows list.
 **/
unsigned long
getX11StackedWindowsList(Window** windows) {
    return getRootWindowProperty(XInternAtom(mDisplay,
        "_NET_CLIENT_LIST_STACKING", False), windows);
}

/** *********************************************************************
 ** Helper method gets a requested root window property.
 **/
unsigned long
getRootWindowProperty(Atom property, Window** windows) {
    Atom da;
    int di;
    unsigned long len;
    unsigned long dl;
    unsigned char* list;

    if (XGetWindowProperty(mDisplay, DefaultRootWindow(mDisplay),
            property, 0L, 1024, False, XA_WINDOW, &da, &di,
            &len, &dl, &list) != Success) {
        *windows = NULL;
        return 0;
    }

    *windows = (Window*) list;
    return len;
}
