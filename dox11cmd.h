
#pragma once

/**
 * Small cmdline tool to examine and handle x11 Windows.
 */
// Std C and c++.
#include <string>

using namespace std;

/**
 * Module Types, Enums, & Defines.
 */
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

#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_NORMAL "\033[0m"

#define MAX_TITLE_STRING_LENGTH 40
#define MAX_ERROR_MESSAGE_LENGTH 60


/**
 * Module Method stubs.
 */

// Main init & helpers.
void doDisplayUseage();

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
