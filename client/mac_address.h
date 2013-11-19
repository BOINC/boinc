#ifndef MAC_ADDRESS_H
#define MAC_ADDRESS_H

// Get the MAC address of a network interface.
// If there's more than one, prefer eth0
// Note: the code on Mac OS X requires the following linkage flags
//			-framework CoreFoundation -lIOKit
//
int get_mac_address(char* address);

#endif // MAC_ADDRESS_H
