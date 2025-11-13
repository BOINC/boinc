// BOINC version definitions.
// version.h is automatically updated by `configure` on *nix systems
// Do not edit version.h directly
//
// To update versions run:
// - for the client/manager: set-client-version.py
// - for the vboxwrapper: set-vboxwrapper-version.py
// - for the worker: set-worker-version.py
// - for the wrapper: set-wrapper-version.py

#ifndef BOINC_VERSION_H
#define BOINC_VERSION_H

// Major part of client version number
#define BOINC_MAJOR_VERSION 8

// Minor part of client version number
#define BOINC_MINOR_VERSION 3

// Release part of client version number
#define BOINC_RELEASE 0

// wrapper version number
#define WRAPPER_RELEASE 26019

// vboxwrapper version number
#define VBOXWRAPPER_RELEASE 26211

// worker version number
#define WORKER_RELEASE 6

// dockerwrapper version number
#define DOCKERWRAPPER_RELEASE 15

// multi_thread version number
#define MULTITHREAD_RELEASE 2

// client version number as string
#define BOINC_VERSION_STRING "8.3.0"

// Package is a pre-release (Alpha/Beta) package
#define BOINC_PRERELEASE 1

#if (defined(_WIN32) || defined(__APPLE__))
// Name of package */
#define PACKAGE "boinc"

// address where bug reports for this package should be sent.
#define PACKAGE_BUGREPORT ""

// full name of this package.
#define PACKAGE_NAME "BOINC"

// full name and version of this package.
#define PACKAGE_STRING "BOINC 8.3.0"

// short name of this package.
#define PACKAGE_TARNAME "boinc"

// version of this package.
#define PACKAGE_VERSION "8.3.0"

#endif /* #if (defined(_WIN32) || defined(__APPLE__)) */

#endif /* #ifndef BOINC_VERSION_H */

