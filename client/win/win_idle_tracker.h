
/**
 * IdleTracker - a DLL that tracks the user's idle input time
 *               system-wide.
 *
 * Usage
 * =====
 * - call IdleTrackerInit() when you want to start monitoring.
 * - call IdleTrackerTerm() when you want to stop monitoring.
 * - to get the time past since last user input, do the following:
 *    GetTickCount() - IdleTrackerGetLastTickCount()
 *
 * Author: Sidney Chong
 * Date: 25/5/2000
 * Version: 1.0
 **/

__declspec(dllimport) BOOL IdleTrackerInit();
__declspec(dllimport) void IdleTrackerTerm();
__declspec(dllimport) DWORD IdleTrackerGetLastTickCount();
