// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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


// find a MAC address for this host

#include <string.h>

#if defined(_WIN32)
#include <boinc_win.h>
#include <Iphlpapi.h>
#elif defined(__APPLE__)
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <paths.h>
#include <sysexits.h>
#include <sys/param.h>
#else  // used to be if defined(__linux__)
#include "config.h"
#include <cstdio>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_NET_ARP_H
#include <net/arp.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif
#ifdef HAVE_NETINET_ETHER_H
#include <netinet/ether.h>
#endif
#if defined(__FreeBSD__)
#include <ifaddrs.h>
#include <net/if_dl.h>
#endif

#include "mac_address.h"

#endif

using std::perror;

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IOEthernetController.h>

// Returns an iterator across all known Ethernet interfaces. Caller is responsible for
// releasing the iterator when iteration is complete.
static kern_return_t
FindEthernetInterfaces(io_iterator_t *matchingServices)
{
    kern_return_t       kernResult;
    mach_port_t         masterPort;
    CFMutableDictionaryRef  classesToMatch;

    kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    if (KERN_SUCCESS != kernResult) fprintf(stderr, "IOMasterPort returned %d\n", kernResult);
    // Ethernet interfaces are instances of class kIOEthernetInterfaceClass
    classesToMatch = IOServiceMatching(kIOEthernetInterfaceClass);
    // Note that another option here would be: classesToMatch = IOBSDMatching("enX");
    // where X is a number from 0 to the number of Ethernet interfaces on the system - 1.
    if (classesToMatch == NULL) fprintf(stderr, "IOServiceMatching returned a NULL dictionary.\n");

    kernResult = IOServiceGetMatchingServices(masterPort, classesToMatch, matchingServices);
    if (KERN_SUCCESS != kernResult) fprintf(stderr, "IOServiceGetMatchingServices returned %d\n", kernResult);

    return kernResult;
}

// Given an iterator across a set of Ethernet interfaces, return the MAC address of the first one.
// If no interfaces are found the MAC address is set to an empty string.
static kern_return_t
GetMACAddress(io_iterator_t intfIterator, char* buffer)
{
    io_object_t     intfService;
    io_object_t     controllerService;
    kern_return_t   kernResult = KERN_FAILURE;

    while ((intfService = IOIteratorNext(intfIterator)))
    {
        CFTypeRef   MACAddressAsCFData;
        // IONetworkControllers can't be found directly by the IOServiceGetMatchingServices call,
        // matching mechanism. So we've found the IONetworkInterface and will get its parent controller
        // by asking for it specifically.
        kernResult = IORegistryEntryGetParentEntry( intfService, kIOServicePlane, &controllerService );

        if (KERN_SUCCESS != kernResult) fprintf(stderr, "IORegistryEntryGetParentEntry returned 0x%08x\n", kernResult);
        else {
            MACAddressAsCFData = IORegistryEntryCreateCFProperty( controllerService, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0);

            if (MACAddressAsCFData)
            {
                const __CFData* refData = (const __CFData*)MACAddressAsCFData;
                UInt8 MACAddress[ kIOEthernetAddressSize ];

                CFDataGetBytes(refData, CFRangeMake(0,CFDataGetLength(refData)), MACAddress);
                sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x",
                        MACAddress[0], MACAddress[1], MACAddress[2], MACAddress[3], MACAddress[4], MACAddress[5]);
                CFRelease(MACAddressAsCFData);
            }
            (void) IOObjectRelease(controllerService);
        }
        // We have sucked this service dry of information so release it now.
        (void) IOObjectRelease(intfService);
        // We're just interested in the first interface so exit the loop.
        break;
    }
    return kernResult;
}
#endif

int get_mac_address(char* address) {
#if defined(_WIN32)
    IP_ADAPTER_INFO AdapterInfo[16]; // Allocate information for up to 16 NICs
    DWORD dwBufLen = sizeof(AdapterInfo); // Save memory size of buffer
    // Call GetAdapterInfo
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

    if(dwStatus != ERROR_SUCCESS) {
        return -1;
    }
    strcpy(address, "");
    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to current adapter info
    while (pAdapterInfo) {
        sprintf(address, "%02x:%02x:%02x:%02x:%02x:%02x",
            pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2],
            pAdapterInfo->Address[3], pAdapterInfo->Address[4], pAdapterInfo->Address[5]
        );
        if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET) break;
        pAdapterInfo = pAdapterInfo->Next;
    }
    return 0;

#elif defined(__APPLE__)
    kern_return_t   kernResult = KERN_SUCCESS; // on PowerPC this is an int (4 bytes)
    /*
     *  error number layout as follows (see mach/error.h and IOKitLib/IOReturn.h):
     *
     *  hi            lo
     *  | system(6) | subsystem(12) | code(14) |
     */
    io_iterator_t   intfIterator;
    int retval = 0;

    kernResult = FindEthernetInterfaces(&intfIterator);
    if (KERN_SUCCESS != kernResult) {
        fprintf(stderr, "FindEthernetInterfaces returned 0x%08x\n", kernResult);
        retval = -1;
    } else {
        kernResult = GetMACAddress(intfIterator, address);
        if (KERN_SUCCESS != kernResult) {
            fprintf(stderr, "GetMACAddress returned 0x%08x\n", kernResult);
            retval = -1;
        }
    }
    IOObjectRelease(intfIterator);
    return retval;

#elif defined(SIOCGIFCONF) || defined(SIOCGLIFCONF)
    char          buf[1024];
#ifdef HAVE_STRUCT_LIFCONF
    struct lifconf ifc;
    struct lifreq *ifr;
#else
    struct ifconf ifc;
    struct ifreq *ifr;
#endif
    int           sck;
    int           nInterfaces;
    int           i;
    /* Get a socket handle. */
    sck = socket(AF_INET, SOCK_DGRAM, 0);
    if (sck < 0) {
        perror("socket");
        return -1;
    }
    /* Query available interfaces. */
#ifdef HAVE_STRUCT_LIFCONF
    ifc.lifc_len = sizeof(buf);
    ifc.lifc_buf = buf;
    if (ioctl(sck, SIOCGLIFCONF, &ifc) < 0) {
        perror("ioctl(SIOCGLIFCONF)");
        close(sck);
        return -1;
    }
#else
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sck, SIOCGIFCONF, &ifc) < 0) {
        perror("ioctl(SIOCGIFCONF)");
        close(sck);
        return -1;
    }
#endif

#ifdef HAVE_STRUCT_LIFCONF
    /* Iterate through the list of interfaces. */
    ifr         = ifc.lifc_req;
    nInterfaces = ifc.lifc_len / sizeof(struct lifreq);
#else
    ifr        = ifc.ifc_req;
    nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
#endif
    strcpy(address, "");

    for (i = 0; i < nInterfaces; i++) {
#ifdef HAVE_STRUCT_LIFCONF
        struct lifreq *item = &ifr[i];
#else
        struct ifreq *item = &ifr[i];
#endif
        struct ether_addr *hw_addr;
        /* Get the MAC address */
#ifdef SIOCGIFHWADDR
        if(ioctl(sck, SIOCGIFHWADDR, item) < 0) {
            perror("ioctl(SIOCGIFHWADDR)");
            close(sck);
            return -1;
        }
        hw_addr=(struct ether_addr *)(item->ifr_hwaddr.sa_data);
#elif  defined(SIOCGIFARP)
        if(ioctl(sck, SIOCGIFARP, item) < 0) {
            perror("ioctl(SIOCGIFARP)");
            close(sck);
            return -1;
        }
        hw_addr = (struct ether_addr *)&(item->lifr_lifru.lifru_enaddr);
#elif defined(__FreeBSD__)
    struct ifaddrs *ifap, *ifaptr;
    unsigned char *ptr;

    if (getifaddrs(&ifap) == 0) {
        for(ifaptr = ifap; ifaptr != NULL; ifaptr = (ifaptr)->ifa_next) {
            if (!strcmp((ifaptr)->ifa_name,  item->ifr_name) && (((ifaptr)->ifa_addr)->sa_family == AF_LINK)) {
                ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifaptr)->ifa_addr);
                hw_addr = (struct ether_addr *)ptr;
                break;
            }
        }
    } else {
        return -1;
    }
#else
	return -1;
#endif
        strcpy(address, ether_ntoa(hw_addr));
#if	defined(__FreeBSD__)
	freeifaddrs(ifap);
#endif
#ifdef HAVE_STRUCT_LIFCONF
        if (strstr(item->lifr_name, "eth")) break;
#elif  defined(__FreeBSD__)
	break;
#else
        if (strstr(item->ifr_name, "eth")) break;
#endif
    }
    close(sck);
    if (!strcmp(address, "")) return -1;
    if (!strcmp(address, "0:0:0:0:0:0")) return -1;

    return 0;
#else
#warning Don`t know how to obtain MAC address.  get_mac_address() will fail.
    return -1;
#endif
}
