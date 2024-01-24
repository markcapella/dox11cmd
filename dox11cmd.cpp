/** *********************************************************************
 ** Main dox11cmd.
 **/

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/Xlib-xcb.h>

#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <iostream>

#include <string>
using namespace std;

#include <vector>
#include <algorithm>


/***********************************************************
 * App Window helper objects.
 */
typedef struct _WinInfo {
        Window id;

        int x, y; // x,y coordinates
        int xa, ya; // x,y coordinates absolute

        unsigned int w, h; // width, height
        long ws; // workspace

        Bool sticky; // is visible on all workspaces
        Bool dock; // is a "dock" (panel)
        Bool hidden; // is hidden / iconized
} WinInfo;


// Forward declare helpers.
void doDisplayUseage();

// Forward declare supported commands.
void doListStackedWindowNames();
void doRaiseWindow(string);
void doLowerWindow(string);
void doMapWindow(string);
void doUnmapWindow(string);

Window getWindowMatchName(string);
Window getWindowNameMatchExact(string name);
Window getWindowNameMatchPartial(string name);

unsigned long getX11StackedWindowsList(Window**);
unsigned long getRootWindowProperty(Atom, Window**);
long int getWindowWorkspace(Window window);

Bool isWindow_Sticky(long workSpace, WinInfo*);
Bool isWindow_Dock(WinInfo*);
Bool isWindow_Hidden(Window window, int windowMapState);

Bool isDesktop_Visible();
Bool isNetWM_Hidden(Window window);
Bool isWM_Hidden(Window window);


// Pattern for string switch-statement.
vector<string> mCmdListStrings {
    "list", "raise", "lower", "map", "unmap"};
enum M_COMMAND_STRING {
    LIST, RAISE, LOWER, MAP, UNMAP};

// Global vars.
string mCmdString = "";
string mWindow = "";
string mAltWindow = "";

Display* mDisplay;


/** *********************************************************************
 ** Module Entry.
 **/
int main(int argc, char **argv) {
    // String, and guard the inputs.
    if (argc > 1) {
        mCmdString = string(argv[1]);
        if (argc > 2) {
            mWindow = string(argv[2]);
            if (argc > 3) {
                mAltWindow = string(argv[3]);
            }
        }
    }

    // Get global Display.
    mDisplay = XOpenDisplay(NULL);
    if (!mDisplay) {
        fprintf(stdout, "dox11cmd: Can\'t open default display ... ");
        fprintf(stdout, "FATAL.\n");
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
            doRaiseWindow(mWindow);
            break;

        case LOWER:
            doLowerWindow(mWindow);
            break;

        case MAP:
            doMapWindow(mWindow);
            break;

        case UNMAP:
            doUnmapWindow(mWindow);
            break;

        default:
            cout << "\ndox11cmd: That\'s not a valid VERB." << endl;
            doDisplayUseage();
    }

    XCloseDisplay(mDisplay);
}

/** *********************************************************************
 ** Display useage (All Supported Commands).
 **/
void doDisplayUseage() {
    cout << "\nUseage: dox11cmd VERB WINDOW" << endl << endl;

    cout << "VERBs are: " << endl << endl;
    cout << "   list" << endl;
    cout << "   raise WINDOW" << endl;
    cout << "   lower WINDOW" << endl;
    cout << "   map WINDOW" << endl;
    cout << "   unmap WINDOW" << endl;

    cout << "\nWINDOWs are: Requested by a portion of their "
        "TitleBar name." << endl;
}

/** *********************************************************************
 ** Supported Commands - list.
 **/
void doListStackedWindowNames() {
    Window* stackedWins;
    int numberOfStackedWins = getX11StackedWindowsList(&stackedWins);

    cout << "\nWindows in Stacked Order above desktop:" << endl;

    for (int i = numberOfStackedWins - 1; i >= 0; i--) {
        WinInfo* winInfoItem = (WinInfo*) malloc(sizeof(WinInfo));

        winInfoItem->id = stackedWins[i];
        winInfoItem->ws = getWindowWorkspace(stackedWins[i]);
        winInfoItem->sticky = isWindow_Sticky(winInfoItem->ws, winInfoItem);
        winInfoItem->dock = isWindow_Dock(winInfoItem);

        XWindowAttributes windowAttributes;
        XGetWindowAttributes(mDisplay, stackedWins[i], &windowAttributes);

        winInfoItem->hidden = isWindow_Hidden(stackedWins[i],
            windowAttributes.map_state);
        winInfoItem->w = windowAttributes.width;
        winInfoItem->h = windowAttributes.height;

        XTextProperty titleBarName;
        XGetWMName(mDisplay, stackedWins[i], &titleBarName);

        fprintf(stdout, "window: [0x%08lx]  ws: %2li  dock: %d  "
            "sticky: %d  hidden: %d  windowName: %s\n",
            winInfoItem->id, winInfoItem->ws, winInfoItem->dock,
            winInfoItem->sticky, winInfoItem->hidden, titleBarName.value);

        XFree(titleBarName.value);
    }
}

/** *********************************************************************
 ** Supported Commands - raise.
 **/
void doRaiseWindow(string windowString) {
    Window window = getWindowMatchName(windowString);
    if (!window) {
        fprintf(stdout, "\ndox11cmd: Cannot find a Window "
            "by that name.\n");
        return;
    }

    if(!XRaiseWindow(mDisplay, window)) {
        fprintf(stdout, "dox11cmd: Error encountered trying to "
            "raise the Window ?? FATAL.\n");
        return;
    }
}

/** *********************************************************************
 ** Supported Commands - lower.
 **/
void doLowerWindow(string windowString) {
    Window window = getWindowMatchName(windowString);
    if (!window) {
        fprintf(stdout, "\ndox11cmd: Cannot find a Window "
            "by that name.\n");
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
            fprintf(stdout, "dox11cmd: Error trying to "
                "lower the Window.\n");
            return;
        }
    }
}

/** *********************************************************************
 ** Supported Commands - map.
 **/
void doMapWindow(string windowString) {
    Window window = getWindowMatchName(windowString);
    if (!window) {
        fprintf(stdout, "\ndox11cmd: Cannot find a Window "
            "by that name.\n");
        return;
    }

    if(!XMapWindow(mDisplay, window)) {
        fprintf(stdout, "dox11cmd: Error encountered trying to "
            "map the Window ?? FATAL.\n");
        return;
    }
}

/** *********************************************************************
 ** Supported Commands unmap.
 **/
void doUnmapWindow(string windowString) {
    Window window = getWindowMatchName(windowString);
    if (!window) {
        fprintf(stdout, "\ndox11cmd: Cannot find a Window "
            "by that name.\n");
        return;
    }

    if(!XUnmapWindow(mDisplay, window)) {
        fprintf(stdout, "dox11cmd: Error encountered trying to "
            "unmap the Window ?? FATAL.\n");
        return;
    }
}

/** *********************************************************************
 ** Helper to search for Window Id whose name matches the users request.
 **/
Window getWindowMatchName(string name) {
    Window result = getWindowNameMatchExact(name);
    if (result) {
        return result;
    }

    return getWindowNameMatchPartial(name);
}

/** *********************************************************************
 ** Helper to search for Window Id whose name matches the users request.
 **/
Window getWindowNameMatchExact(string name) {
    Window* stackedWins;
    int numberOfStackedWins = getX11StackedWindowsList(&stackedWins);

    for (int i = numberOfStackedWins - 1; i >= 0; i--) {
        XTextProperty titleBarName;
        XGetWMName(mDisplay, stackedWins[i], &titleBarName);

        // Exact match includes empty nameString.
        string titleBarString(reinterpret_cast<char const*>
            (titleBarName.value));
        string nameString(name);
        if (titleBarString == nameString) {
            XFree(titleBarName.value);
            return stackedWins[i];
        }

        XFree(titleBarName.value);
    }
    return None;
}

/** *********************************************************************
 ** Helper to search for Window Id whose name matches the users request.
 **/
Window getWindowNameMatchPartial(string name) {
    Window* stackedWins;
    int numberOfStackedWins = getX11StackedWindowsList(&stackedWins);

    for (int i = numberOfStackedWins - 1; i >= 0; i--) {
        XTextProperty titleBarName;
        XGetWMName(mDisplay, stackedWins[i], &titleBarName);

        // Else grab partial.
        string titleBarString(reinterpret_cast<char const*>
            (titleBarName.value));
        string nameString(name);
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

/** *********************************************************************
 ** This method determines if a window is visible on a workspace.
 **/
long int getWindowWorkspace(Window window) {
    Bool result = 0;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char *properties = NULL;

    XGetWindowProperty(mDisplay, window,
        XInternAtom(mDisplay, "_NET_WM_DESKTOP", False), 0, 1, False,
        AnyPropertyType, &type, &format, &nitems, &unusedBytes, &properties);

    if (type != XA_CARDINAL) {
        if (properties) {
            XFree(properties);
        }
        properties = NULL;
        XGetWindowProperty(mDisplay, window,
            XInternAtom(mDisplay, "_WIN_WORKSPACE", False), 0, 1,
            False, AnyPropertyType, &type, &format, &nitems, &unusedBytes,
            &properties);
    }

    if (properties) {
        result = *(long *) (void *) properties;
        if (properties) {
            XFree(properties);
        }
    }

    return result;
}

/** *********************************************************************
 ** This method determines if a window state is sticky.
 **/
Bool isWindow_Sticky(long workSpace, WinInfo* winInfoItem) {
    // Needed in KDE and LXDE.
    if (workSpace == -1) {
        return true;
    }

    Bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char *properties = NULL;

    XGetWindowProperty(mDisplay, winInfoItem->id,
        XInternAtom(mDisplay, "_NET_WM_STATE", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (type == XA_ATOM) {
        for (unsigned long int i = 0; i < nitems; i++) {
            char *nameString = XGetAtomName(mDisplay,
                ((Atom *) (void *) properties) [i]);
            if (strcmp(nameString, "_NET_WM_STATE_STICKY") == 0) {
                result = true;
                if (nameString) {
                    XFree(nameString);
                }
                break;
            }
            if (nameString) {
                XFree(nameString);
            }
        }
    }

    if (properties) {
        XFree(properties);
    }

    return result;
}

/** *********************************************************************
 ** This method ...
 **/
Bool isWindow_Dock(WinInfo* winInfoItem) {
    Bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char *properties = NULL;

    XGetWindowProperty(mDisplay, winInfoItem->id,
        XInternAtom(mDisplay, "_NET_WM_WINDOW_TYPE", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (format == 32) {
        for (int i = 0; (unsigned long)i < nitems; i++) {
            char *nameString = XGetAtomName(mDisplay,
                ((Atom *) (void *) properties) [i]);
            if (strcmp(nameString, "_NET_WM_WINDOW_TYPE_DOCK") == 0) {
                result = true;
                if (nameString) {
                    XFree(nameString);
                }
                break;
            }
            if (nameString) {
                XFree(nameString);
            }
        }
    }

    if (properties) {
        XFree(properties);
    }

    return result;
}

/** *********************************************************************
 ** This method determines if a window state is hidden.
 **/
Bool isWindow_Hidden(Window window, int windowMapState) {
    if (!isDesktop_Visible()) {
        return true;
    }
    if (windowMapState != IsViewable) {
        return true;
    }

    if (isNetWM_Hidden(window)) {
        return true;
    }
    if (isWM_Hidden(window)) {
        return true;
    }

    return false;
}

/** *********************************************************************
 ** This method checks "_NET_WM_STATE" for window HIDDEN attribute.
 **/
Bool isNetWM_Hidden(Window window) {
    Bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char *properties = NULL;

    XGetWindowProperty(mDisplay, window,
        XInternAtom(mDisplay, "_NET_WM_STATE", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (format == 32) {
        for (unsigned long i = 0; i < nitems; i++) {
            char *nameString = XGetAtomName(mDisplay,
                ((Atom *) (void *) properties) [i]);
            if (strcmp(nameString, "_NET_WM_STATE_HIDDEN") == 0) {
                result = true;
                if (nameString) {
                    XFree(nameString);
                }
                break;
            }

            if (nameString) {
                XFree(nameString);
            }
        }
    }

    if (properties) {
        XFree(properties);
    }

    return result;
}

/** *********************************************************************
 ** This method checks "WM_STATE" for window HIDDEN attribute.
 **/
Bool isWM_Hidden(Window window) {
    Bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char *properties = NULL;

    XGetWindowProperty(mDisplay, window,
        XInternAtom(mDisplay, "WM_STATE", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (format == 32 && nitems >= 1) {
        if (* (long*) (void*) properties != NormalState) {
            result = true;
        }
    }

    if (properties) {
        XFree(properties);
    }

    return result;
}

/** *********************************************************************
 ** This method ...
 **/
Bool isDesktop_Visible() {
    Bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char *properties = NULL;

    XGetWindowProperty(mDisplay, DefaultRootWindow(mDisplay),
        XInternAtom(mDisplay, "_NET_SHOWING_DESKTOP", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (format == 32 && nitems >= 1) {
        if (*(long *) (void *) properties != 1) {
            result = true;
        }
    }

    if (properties) {
        XFree(properties);
    }

    return result;
}
