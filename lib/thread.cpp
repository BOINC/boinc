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

#include "thread.h"

#ifdef _WIN32
int THREAD::run(LPTHREAD_START_ROUTINE func, void* _arg) {
    CreateThread(NULL, 0, func, this, 0, NULL);
#else
int THREAD::run(void*(*func)(void*), void* _arg) {
    pthread_t id;
    pthread_attr_t thread_attrs;
    pthread_attr_init(&thread_attrs);
    pthread_create(&id, &thread_attrs, func, this);
#endif
    arg = _arg;
    return 0;
}

void THREAD::quit() {
    quit_flag = true;
}

THREAD_LOCK::THREAD_LOCK() {
#ifdef _WIN32
    InitializeCriticalSection(&mutex);
#else
    pthread_mutex_init(&mutex, NULL);
#endif
}

void THREAD_LOCK::lock() {
#ifdef _WIN32
    EnterCriticalSection(&mutex);
#else
    pthread_mutex_lock(&mutex);
#endif
}

void THREAD_LOCK::unlock() {
#ifdef _WIN32
    LeaveCriticalSection(&mutex);
#else
    pthread_mutex_lock(&mutex);
#endif
}
