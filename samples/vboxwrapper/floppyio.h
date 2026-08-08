//  This file is part Floppy I/O, a Virtual Machine - Hypervisor intercommunication system.
//  Copyright (C) 2011 Ioannis Charalampidis
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

//  File:   FloppyIO.h
//  Author: Ioannis Charalampidis <ioannis.charalampidis AT cern DOT ch>
//  License: GNU Lesser General Public License - Version 3.0
//
//  Hypervisor-Virtual machine bi-directional communication
//  through floppy disk.
//
//  This class provides the hypervisor-side of the script.
//  For the guest-side, check the perl scripts that
//  were available with this code.
//
//  Here is the layout of the floppy disk image (Example of 28k):
//
//  +-----------------+------------------------------------------------+
//  | 0x0000 - 0x37FF |  Hypervisor -> Guest Buffer                    |
//  | 0x3800 - 0x6FFE |  Guest -> Hypervisor Buffer                    |
//  |     0x6FFF      |  "Data available for guest" flag byte          |
//  |     0x7000      |  "Data available for hypervisor" flag byte     |
//  +-----------------+------------------------------------------------+
//
//  Updated at January 5, 2012, 13:06 PM
//

#ifndef FLOPPYIO_H
#define	FLOPPYIO_H

namespace FloppyIONS {

    // Do not initialize (reset) floppy disk image at open.
    // (Flag used by the FloppyIO constructor)
    #define F_NOINIT 1

    // Do not create the filename (assume it exists)
    // (Flag used by the FloppyIO constructor)
    #define F_NOCREATE 2

    // Synchronize I/O.
    // This flag will block the script until the guest has read/written the data.
    // (Flag used by the FloppyIO constructor)
    #define F_SYNCHRONIZED 4

    // Use exceptions instead of error codes
    // (Flag used by the FloppyIO constructor)
    #define F_EXCEPTIONS 8

    // Initialize FloppyIO in client mode.
    // This flag will swap the input/output buffers, making the script usable from
    // within the virtual machine.
    // (Flag used by the FloppyIO constructor)
    #define F_CLIENT 16

    //
    // Error code constants
    //
    #define FPIO_NOERR          0  // No error occurred
    #define FPIO_ERR_IO        -1  // There was an I/O error on the strea,
    #define FPIO_ERR_TIMEOUT   -2  // The operation timed out
    #define FPIO_ERR_CREATE    -3  // Unable to create the floppy file
    #define FPIO_ERR_NOTREADY  -4  // The I/O object is not ready

    //
    // Structure of the synchronization control byte.
    //
    // This byte usually resides at the beginning of the
    // floppy file for the receive buffer and at the end
    // of the file for the sending buffer.
    //
    // It's purpose is to force the entire floppy image
    // to be re-written/re-read by the hypervisor/guest OS and
    // to synchronize the I/O in case of large amount of
    // data being exchanged.
    //
    typedef struct fpio_ctlbyte {
        unsigned short bDataPresent  : 1;
        unsigned short bReserved     : 7;
    } fpio_ctlbytex;

    // Default floppy disk size (In bytes)
    //
    // VirtualBox complains if bigger than 28K
    // It's supposed to go till 1474560 however (1.44 Mb)

    #define DEFAULT_FIO_FLOPPY_SIZE 28672

    // Default synchronization timeout (seconds).
    // This constant defines how long we should wait for synchronization
    // feedback from the guest before aborting.

    #define DEFAULT_FIO_SYNC_TIMEOUT 5

    //
    // Floppy I/O Communication class
    //
    class FloppyIO {
    public:

        // Construcors
        FloppyIO(const char * filename, int flags = 0);
        virtual ~FloppyIO();

        // Functions
        void        reset();
        int         send(std::string strData);
        std::string receive();
        int         receive(std::string* strBuffer);

        // Topology info
        int         ofsInput;   // Input buffer offset & size
        int         szInput;
        int         ofsOutput;  // Output buffer offset & size
        int         szOutput;

        int         ofsCtrlByteIn;  // Control byte offset for input
        int         ofsCtrlByteOut; // Control byte offset for output

        // Synchronization stuff
        bool        synchronized;   // The read/writes are synchronized
        int         syncTimeout;    // For how long should we wait

        // Error reporting and checking
        int         error;
        std::string errorStr;
        bool        useExceptions;  // If TRUE errors will raise exceptions

        void        clear();        // Clear errors
        bool        ready();        // Returns TRUE if there are no errors

    private:

        // Floppy Info
        std::fstream* fIO;
        int         szFloppy;

        // Functions
        int         waitForSync(int controlByteOffset, char state, int timeout);
        int         setError(int code, std::string message);

    };

    //
    // Floppy I/O Exceptions
    //
    class FloppyIOException: public std::exception {
    public:

        int         code;
        std::string message;
        std::string full_message;

        // Default constructor/destructor
        FloppyIOException() {
            init(0, "");
        };
        virtual ~FloppyIOException() throw() { };

        // Get description
        virtual const char* what() const throw() {
            return full_message.c_str();
        }

        // Change the message and return my instance
        // (Used for singleton format)
        FloppyIOException * set(int _code, std::string _message) {
            init(_code, _message);
            return this;
        }

    private:

        void init(int _code, std::string _message) {
            code = _code;
            message = _message;
            std::stringstream ss;
            ss << _message << ". Error code = " << _code;
            full_message = ss.str();
        }

    };

};

#endif	// FLOPPYIO_H

