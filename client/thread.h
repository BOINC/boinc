// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOINC_THREAD_H
#define BOINC_THREAD_H

#ifdef _WIN32
#else
#include <pthread.h>
#endif

struct THREAD {
    void* arg;
    bool quit_flag;
#ifdef _WIN32
    int run(LPTHREAD_START_ROUTINE, void*);
#else
    int run(void*(*func)(void*), void*);
#endif
    void quit();
};

struct THREAD_LOCK {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
    void lock();
    void unlock();

    THREAD_LOCK();
};

#endif
