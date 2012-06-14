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

// The world's smallest discrete event simulator.
// Uses the STL "heap" data structure for efficient event storage.

#ifndef _DES_
#define _DES_

#include <vector>
#include <algorithm>

using std::vector;

// base class for events.
// t is the time when the event occurs.
// handle() is the function to call then.
//
struct EVENT {
    double t;
    virtual void handle(){}
};

inline bool compare(EVENT* e1, EVENT* e2) {
    return (e1->t > e2->t);
}

struct SIMULATOR {
    vector<EVENT*> events;
    double now;

    // add an event
    //
    void insert(EVENT* e) {
        //printf("adding %x\n", e);
        events.push_back(e);
        push_heap(events.begin(), events.end(), compare);
    }

    // remove an event
    //
    void remove(EVENT* e) {
        vector<EVENT*>::iterator i;
        //printf("removing %x\n", e);
        for (i=events.begin(); i!=events.end(); i++) {
            if (*i == e) {
                events.erase(i);
                make_heap(events.begin(), events.end(), compare);
                //printf("removed %x\n", e);
                return;
            }
        }
        //printf("%x not found\n", e);
    }

    // run the simulator for the given time period
    //
    void simulate(double dur) {
        while (events.size()) {
            EVENT* e = events.front();
            pop_heap(events.begin(), events.end(), compare);
            events.pop_back();
            now = e->t;
            if (now > dur) break;
            e->handle();
        }
    }
};

extern SIMULATOR sim;

#endif
