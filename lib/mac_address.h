#ifndef MAC_ADDRESS_H
#define MAC_ADDRESS_H

// Note: the code on Mac OS X requires the following linkage flags
//			-framework CoreFoundation -lIOKit
bool
get_mac_addresses(char* addresses);

#endif // MAC_ADDRESS_H
