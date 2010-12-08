// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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


// Provide cross-platform interfaces for making changes to VirtualBox

#ifndef _VM_H_
#define _VM_H_

struct VM {
    std::string stdin_filename;
    std::string stdout_filename;
    std::string stderr_filename;
    // name of checkpoint file, if any
    std::string checkpoint_filename;
    // name of file where app will write its fraction done
    std::string fraction_done_filename;
    bool suspended;

    int parse( XML_PARSER& );
    void poll();
    int run();
    void stop();
    void pause();
    void resume();
};

extern VM vm;

#endif
