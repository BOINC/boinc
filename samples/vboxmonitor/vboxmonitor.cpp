// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010-2012 University of California
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

// vboxmonitor [options]     BOINC VirtualBox monitor
// see: http://boinc.berkeley.edu/trac/wiki/VboxApps
// Options:
//
//
// Duplicate everything sent to stdin to the console and the VM Guest Log.
//
// You can see the VM Guest Log on the host machine by executing:
// vboxmanage showvminfo <vm_name> --log 0
//
// As a matter of course, vboxwrapper dumps the last 16k of the guest log to
// the stderr log which is uploaded to the project server in case of an error.
//
//

#include <sys/io.h>
#include <iostream>
#include <string>

void guestlog(int c)
{
    asm volatile ("out %%al,%%dx\n" : : "a"(c), "d"(0x504));
}

int main() {
    std::string buffer;

    // Dump input stream to string buffer
    std::cin >> buffer;

    // root access required
    iopl(3);

    // Write output to stdout as well as VirtualBox's Guest VM Log
    for (int i = 0; i < buffer.size(); ++i) {
        // Virtualbox
        guestlog((int)buffer[i]);

        // Console
        putc((int)buffer[i], stdout);
    }

    return 0;
}
