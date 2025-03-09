/** ********************************************************
 ** Main dox11cmd.
 **/

#include <algorithm>
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>

using namespace std;


/** ********************************************************
 ** Wininfo object.
 **/
typedef struct {
        Window id;         // id.
        long ws;           // workspace.

        bool sticky;       // visible on all workspaces?
        bool dock;         // is a "dock" (panel)?
        bool hidden;       // is hidden / iconized?

        int x, y;          // x,y coordinates.
        int xa, ya;        // x,y coordinates absolute.
        unsigned int w, h; // width, height.
} WinInfo;

/** ********************************************************
 * Module Method stubs.
 */
// Forward declare helpers.
void doDisplayUseage();

// Forward declare supported commands.
void doListStackedWindowNames();
void doRaiseWindow(string);
void doLowerWindow(string);
void doMapWindow(string);
void doUnmapWindow(string);

Window getWindowWithBestName(string);
Window getWindowWithExactName(string name);
Window getWindowWithPartialName(string name);

unsigned long getX11StackedWindowsList(Window**);
unsigned long getRootWindowProperty(Atom, Window**);
long int getWindowWorkspace(Window window);

bool isWindow_Sticky(long workSpace, WinInfo*);
bool isWindow_Dock(WinInfo*);
bool isWindow_Hidden(Window window, int windowMapState);

bool isDesktop_Visible();
bool isNetWM_Hidden(Window window);
bool isWM_Hidden(Window window);

int handleX11ErrorEvent(Display*, XErrorEvent*);


/** ********************************************************
 ** Module globals and consts.
 **/

// Minor styling.
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_NORMAL "\033[0m"

// Pattern for string switch-statement.
vector<string> mCmdListStrings {
    "list", "raise", "lower", "map", "unmap"};
enum M_COMMAND_STRING {
    LIST, RAISE, LOWER, MAP, UNMAP};

string mCmdString = "";
string mWindow = "";
string mAltWindow = "";

Display* mDisplay;

#define MAX_TITLE_STRING_LENGTH 40
#define MAX_ERROR_MESSAGE_LENGTH 60

int mX11LastErrorCode = 0;


/** ********************************************************
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

    // Check for Wayland as error.
    const bool isWaylandPresent = getenv("WAYLAND_DISPLAY") &&
        getenv("WAYLAND_DISPLAY") [0];
    if (isWaylandPresent) {
        cout << COLOR_YELLOW << "\ndox11cmd: Wayland desktop is detected."
            << COLOR_NORMAL << endl;
    }

    // X11 Initialization.
    mDisplay = XOpenDisplay(NULL);
    if (!mDisplay) {
        cout << COLOR_RED << "\ndox11cmd: X11 Does not seem to be "
            "available." << COLOR_NORMAL << endl;
        exit(1);
    }

    XSynchronize(mDisplay, 0);
    XSetErrorHandler(handleX11ErrorEvent);

    // Execute users command.
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
            cout << COLOR_YELLOW <<
                "\ndox11cmd: That\'s not a valid VERB." <<
                COLOR_NORMAL << endl;
            doDisplayUseage();
            doListStackedWindowNames();
    }

    XCloseDisplay(mDisplay);
}

/** ********************************************************
 ** Display useage (All Supported Commands).
 **/
void doDisplayUseage() {
    cout << COLOR_BLUE <<
        "\nUseage: dox11cmd VERB [WINDOW]" <<
        COLOR_NORMAL << endl << endl;

    cout << COLOR_GREEN << "   VERBs are:" <<
        COLOR_NORMAL << endl << endl;

    cout << "      list" << endl;
    cout << "      raise WINDOW" << endl;
    cout << "      lower WINDOW" << endl;
    cout << "      map WINDOW" << endl;
    cout << "      unmap WINDOW" << endl;
    cout << endl;

    cout << COLOR_GREEN << "   WINDOWs are:" << COLOR_NORMAL <<
        endl << endl << "      Requested by a portion of their "
        "TitleBar Name." << endl;
}

/** ********************************************************
 ** Supported Commands - list.
 **/
void doListStackedWindowNames() {
    cout << COLOR_BLUE << "\nWindows in Stacked Order "
        "above Desktop:" << COLOR_NORMAL << endl;

    cout << COLOR_GREEN << "\n---window---  Titlebar Name"
        "                             WS   "
        "---Position-- -----Size----  Attributes" <<
        COLOR_NORMAL << endl;

    Window* stackedWins;
    int numberOfStackedWins =
        getX11StackedWindowsList(&stackedWins);

    for (int i = numberOfStackedWins - 1; i >= 0; i--) {
        // Get window attributes.
        XWindowAttributes windowAttributes;
        windowAttributes.map_state = -1;
        windowAttributes.width = -1;
        windowAttributes.height = -1;
        windowAttributes.x = -1;
        windowAttributes.y = -1;

        XGetWindowAttributes(mDisplay, stackedWins[i],
            &windowAttributes);

        // Get window coordinates.
        int xCoord = -1;
        int yCoord = -1;
        Window child_return = None;

        XTranslateCoordinates(mDisplay, stackedWins[i],
            DefaultRootWindow(mDisplay), 0, 0, &xCoord,
            &yCoord, &child_return);

        // Get window title.
        // Create a formatted title (name) c-string with a hard length,
        // replacing unprintables with SPACE, padding right with SPACE,
        // and preserving null terminator.
        char outputTitle[MAX_TITLE_STRING_LENGTH + 1];
        int outP = 0;

        XTextProperty titleBarName;
        if (XGetWMName(mDisplay, stackedWins[i], &titleBarName) != 0) {
            const char* nameP = (char*) titleBarName.value;
            const int nameL = strlen(nameP);
            for (; outP < nameL && outP < MAX_TITLE_STRING_LENGTH; outP++) {
                outputTitle[outP] = isprint(*(nameP + outP)) ?
                    *(nameP + outP) : ' ';
            }
            XFree(titleBarName.value);
        }

        for (; outP < MAX_TITLE_STRING_LENGTH; outP++) {
            outputTitle[outP] = ' ';
        }
        outputTitle[outP] = '\0';


        // Create a WinInfo struct.
        WinInfo* winInfoItem = (WinInfo*)
            malloc(sizeof(WinInfo));

        winInfoItem->id = stackedWins[i];
        winInfoItem->ws = getWindowWorkspace(stackedWins[i]);

        winInfoItem->sticky = isWindow_Sticky(winInfoItem->ws,
            winInfoItem);
        winInfoItem->dock = isWindow_Dock(winInfoItem);
        winInfoItem->hidden = isWindow_Hidden(stackedWins[i],
            windowAttributes.map_state);

        winInfoItem->w  = windowAttributes.width;
        winInfoItem->h  = windowAttributes.height;

        winInfoItem->x  = windowAttributes.x;
        winInfoItem->y  = windowAttributes.y;

        winInfoItem->xa = xCoord - winInfoItem->x;
        winInfoItem->ya = yCoord - winInfoItem->y;

        // Log a WinInfo struct.
        fprintf(stdout, "[0x%08lx]  %s  %2li  "
            " %5d , %-5d %5d x %-5d  %s%s%s\n",
            winInfoItem->id, outputTitle,
            winInfoItem->ws,
            winInfoItem->xa, winInfoItem->ya,
            winInfoItem->w, winInfoItem->h,
            winInfoItem->dock ? "dock " : "",
            winInfoItem->sticky ? "sticky " : "",
            winInfoItem->hidden ? "hidden" : "");
    }
}

/** ********************************************************
 ** Supported Commands - raise.
 **/
void doRaiseWindow(string windowString) {
    Window window = getWindowWithBestName(windowString);
    if (!window) {
        cout << COLOR_RED << "\ndox11cmd: Cannot find a Window "
            "by that name." << COLOR_NORMAL << endl;
        return;
    }

    if(!XRaiseWindow(mDisplay, window)) {
        cout << COLOR_RED << "dox11cmd: Error encountered trying to "
            "raise the Window ?? FATAL." << COLOR_NORMAL << endl;
        return;
    }
}

/** ********************************************************
 ** Supported Commands - lower.
 **/
void doLowerWindow(string windowString) {
    Window window = getWindowWithBestName(windowString);
    if (!window) {
        cout << COLOR_RED << "\ndox11cmd: Cannot find a Window "
            "by that name." << COLOR_NORMAL << endl;
        return;
    }

    // Lower target, by raising all other windows above it.
    // Ignore desktop @ [0].
    Window* stackedWins;
    int numberOfStackedWins = getX11StackedWindowsList(&stackedWins);
    for (int i = 1; i < numberOfStackedWins; i++) {
        if (window == stackedWins[i]) {
            continue;
        }
        if (!XRaiseWindow(mDisplay, stackedWins[i])) {
            cout << COLOR_RED << "dox11cmd: Error trying to "
                "lower the Window." << COLOR_NORMAL << endl;
            return;
        }
    }
}

/** ********************************************************
 ** Supported Commands - map.
 **/
void doMapWindow(string windowString) {
    Window window = getWindowWithBestName(windowString);
    if (!window) {
        cout << COLOR_RED << "\ndox11cmd: Cannot find a Window "
            "by that name." << COLOR_NORMAL << endl;
        return;
    }

    if(!XMapWindow(mDisplay, window)) {
        cout << COLOR_RED << "dox11cmd: Error "
            "trying to map the Window." << COLOR_NORMAL << endl;
        return;
    }
}

/** ********************************************************
 ** Supported Commands unmap.
 **/
void doUnmapWindow(string windowString) {
    Window window = getWindowWithBestName(windowString);
    if (!window) {
        cout << COLOR_RED << "\ndox11cmd: Cannot find a Window "
            "by that name." << COLOR_NORMAL << endl;
        return;
    }

    if(!XUnmapWindow(mDisplay, window)) {
        cout << COLOR_RED << "dox11cmd: Error trying to "
            "unmap the Window ?? FATAL." << COLOR_NORMAL << endl;
        return;
    }
}

/** ********************************************************
 ** Helper to search for Window Id whose name matches.
 **/
Window getWindowWithBestName(string name) {
    Window result = getWindowWithExactName(name);
    if (result) {
        return result;
    }

    return getWindowWithPartialName(name);
}

/** ********************************************************
 ** Helper to search for Window Id whose name matches.
 **/
Window getWindowWithExactName(string name) {
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

/** ********************************************************
 ** Helper to search for Window Id whose name matches.
 **/
Window getWindowWithPartialName(string name) {
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

/** ********************************************************
 ** Helper method gets the X11 stacked windows list.
 **/
unsigned long
getX11StackedWindowsList(Window** windows) {
    return getRootWindowProperty(XInternAtom(mDisplay,
        "_NET_CLIENT_LIST_STACKING", False), windows);
}

/** ********************************************************
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

/** ********************************************************
 ** This method determines if a window is visible on a workspace.
 **/
long int getWindowWorkspace(Window window) {
    long int result = 0;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char* properties = NULL;

    XGetWindowProperty(mDisplay, window,
        XInternAtom(mDisplay, "_NET_WM_DESKTOP", False),
        0, 1, False, AnyPropertyType, &type, &format, &nitems,
        &unusedBytes, &properties);

    if (type != XA_CARDINAL) {
        XFree(properties);
        properties = NULL;

        XGetWindowProperty(mDisplay, window,
            XInternAtom(mDisplay, "_WIN_WORKSPACE", False),
            0, 1, False, AnyPropertyType, &type, &format, &nitems,
            &unusedBytes, &properties);
    }

    if (properties) {
        result = *(long*) (void*) properties;
        XFree(properties);
    }

    return result;
}

/** ********************************************************
 ** This method determines if a window state is sticky.
 **/
bool isWindow_Sticky(long workSpace, WinInfo* winInfoItem) {
    // Needed in KDE and LXDE.
    if (workSpace == -1) {
        return true;
    }

    bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char* properties = NULL;

    XGetWindowProperty(mDisplay, winInfoItem->id,
        XInternAtom(mDisplay, "_NET_WM_STATE", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (type == XA_ATOM) {
        for (unsigned long int i = 0; i < nitems; i++) {
            char* nameString = XGetAtomName(mDisplay,
                ((Atom*) (void*) properties) [i]);
            if (strcmp(nameString, "_NET_WM_STATE_STICKY") == 0) {
                result = true;
                XFree(nameString);
                break;
            }
            XFree(nameString);
        }
    }

    XFree(properties);
    return result;
}

/** ********************************************************
 ** This method checks for a _NET_WM_WINDOW_TYPE of
 ** _NET_WM_WINDOW_TYPE_DOCK.
 **/
bool isWindow_Dock(WinInfo* winInfoItem) {
    bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char* properties = NULL;

    XGetWindowProperty(mDisplay, winInfoItem->id,
        XInternAtom(mDisplay, "_NET_WM_WINDOW_TYPE", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (format == 32) {
        for (int i = 0; (unsigned long)i < nitems; i++) {
            char* nameString = XGetAtomName(mDisplay,
                ((Atom*) (void*) properties) [i]);
            if (strcmp(nameString, "_NET_WM_WINDOW_TYPE_DOCK") == 0) {
                result = true;
                XFree(nameString);
                break;
            }
            XFree(nameString);
        }
    }

    XFree(properties);
    return result;
}

/** ********************************************************
 ** This method determines if a window state is hidden.
 **/
bool isWindow_Hidden(Window window, int windowMapState) {
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

/** ********************************************************
 ** This method checks for window _NET_SHOWING_DESKTOP property.
 **/
bool isDesktop_Visible() {
    bool result = true;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char* properties = NULL;

    XGetWindowProperty(mDisplay, DefaultRootWindow(mDisplay),
        XInternAtom(mDisplay, "_NET_SHOWING_DESKTOP", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (format == 32 && nitems >= 1) {
        if (*(long*) (void*) properties == 1) {
            result = false;
        }
    }

    XFree(properties);
    return result;
}

/** ********************************************************
 ** This method checks "_NET_WM_STATE" for window HIDDEN.
 **/
bool isNetWM_Hidden(Window window) {
    bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char* properties = NULL;

    XGetWindowProperty(mDisplay, window,
        XInternAtom(mDisplay, "_NET_WM_STATE", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (format == 32) {
        for (unsigned long i = 0; i < nitems; i++) {
            char* nameString = XGetAtomName(mDisplay,
                ((Atom*) (void*) properties) [i]);
            if (strcmp(nameString, "_NET_WM_STATE_HIDDEN") == 0) {
                result = true;
                XFree(nameString);
                break;
            }
            XFree(nameString);
        }
    }

    XFree(properties);
    return result;
}

/** ********************************************************
 ** This method checks "WM_STATE" for window HIDDEN attribute.
 **/
bool isWM_Hidden(Window window) {
    bool result = false;

    Atom type;
    int format;
    unsigned long nitems, unusedBytes;
    unsigned char* properties = NULL;

    XGetWindowProperty(mDisplay, window,
        XInternAtom(mDisplay, "WM_STATE", False),
        0, (~0L), False, AnyPropertyType, &type, &format,
        &nitems, &unusedBytes, &properties);

    if (format == 32 && nitems >= 1) {
        if (*(long*) (void*) properties != NormalState) {
            result = true;
        }
    }

    XFree(properties);
    return result;
}

/** ********************************************************
 ** This method traps and handles X11 errors.
 **/
int handleX11ErrorEvent(Display* dpy, XErrorEvent* event) {

    // Save error & quit early if simply BadWindow.
    mX11LastErrorCode = event->error_code;
    if (mX11LastErrorCode == BadWindow) {
        return 0;
    }

    // Print the error message of the event.
    char msg[MAX_ERROR_MESSAGE_LENGTH];
    XGetErrorText(dpy, event->error_code, msg, sizeof(msg));

    printf("dox11cmd: Error is %s%s%s.\n",
        COLOR_RED, msg, COLOR_NORMAL);

    return 0;
}
