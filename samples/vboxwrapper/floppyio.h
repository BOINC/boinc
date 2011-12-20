//  File:   FloppyIO.h
//  Author: Ioannis Charalampidis <ioannis.charalampidis AT cern DOT ch>
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
//  Created on November 24, 2011, 12:30 PM

#ifndef FLOPPYIO_H
#define	FLOPPYIO_H

// Do not initialize (reset) floppy disk image at open.
// (Flag used at FloppyIO constructor)

#define F_NOINIT 1


// Do not create the filename (assume it exists)
// (Flag used at FloppyIO constructor)

#define F_NOCREATE 2


// Synchronize I/O [NOT YET IMPLEMENTED]
// This flag will block the script until the guest has read/written the data.
// (Flag used at FloppyIO constructor)

#define F_SYNCHRONIZED 4

// Default floppy disk size (In bytes)
// 
// VirtualBox complains if bigger than 28K
// It's supposed to go till 1474560 however (!.44 Mb)

#define DEFAULT_FLOPPY_SIZE 28672


// Floppy I/O Communication class

class FloppyIO {
public:
    
    // Constructors
    FloppyIO(const char * filename);
    FloppyIO(const char * filename, int flags);
    virtual ~FloppyIO();
    
    // Functions
    void        reset();
    void        send(std::string strData);
    std::string receive();
    
    // Topology info
    int     ofsInput;   // Input buffer offset & size
    int     szInput;
    int     ofsOutput;  // Output buffer offset & size
    int     szOutput;
    
    int     ofsCtrlByteIn;  // Control byte offset for input
    int     ofsCtrlByteOut; // Control byte offset for output

private:

    // Floppy Info
    std::fstream* fIO;
    int     szFloppy;
    
};

#endif	// FLOPPYIO_H 

