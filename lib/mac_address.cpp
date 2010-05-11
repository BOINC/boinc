#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#elif defined(__APPLE__)
#include "config.h"
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <paths.h>
#include <sysexits.h>
#include <sys/param.h>
#include <string.h>
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
#include <string.h>
#include "unix_util.h"
#endif

#include "mac_address.h"

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
    char            delimiter[2] = "\0";

    while (intfService = IOIteratorNext(intfIterator))
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
                sprintf(buffer, "%s%s%02x:%02x:%02x:%02x:%02x:%02x",
                        buffer, delimiter,
                        MACAddress[0], MACAddress[1], MACAddress[2], MACAddress[3], MACAddress[4], MACAddress[5]);
                CFRelease(MACAddressAsCFData);
                delimiter[0] = ':';
                delimiter[1] = '\0';
            }
            (void) IOObjectRelease(controllerService);
        }
        // We have sucked this service dry of information so release it now.
        (void) IOObjectRelease(intfService);
        // We're just interested in the first interface so exit the loop.
        //break;
    }
    return kernResult;
}
#endif

bool
get_mac_addresses(char* addresses) {
#if defined(_WIN32)
    IP_ADAPTER_INFO AdapterInfo[16]; // Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo); // Save memory size of buffer
	// Call GetAdapterInfo
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

	if(dwStatus == ERROR_SUCCESS)
	{
		strcpy(addresses, "");
		char delimiter[2] = "\0";
		// valid, no buffer overflow
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to current adapter info
		do {
			sprintf(addresses, "%s%s%02x:%02x:%02x:%02x:%02x:%02x", addresses, delimiter,
					pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2], 
					pAdapterInfo->Address[3], pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
			delimiter[0] = ':';
			delimiter[1] = '\0';

			pAdapterInfo = pAdapterInfo->Next;
		}
		while(pAdapterInfo);
	}
	else
	{
		fprintf(stderr, "Adapters information not found\n");

		return false;
	}
	return true;

#elif defined(__APPLE__)
	kern_return_t   kernResult = KERN_SUCCESS; // on PowerPC this is an int (4 bytes)
    /*
     *  error number layout as follows (see mach/error.h and IOKitLib/IOReturn.h):
     *
     *  hi            lo
     *  | system(6) | subsystem(12) | code(14) |
     */
    io_iterator_t   intfIterator;

    kernResult = FindEthernetInterfaces(&intfIterator);
    if (KERN_SUCCESS != kernResult) fprintf(stderr, "FindEthernetInterfaces returned 0x%08x\n", kernResult);
    else
	{
        kernResult = GetMACAddress(intfIterator, addresses);
        if (KERN_SUCCESS != kernResult) fprintf(stderr, "GetMACAddress returned 0x%08x\n", kernResult);
    }
    IOObjectRelease(intfIterator);

	return kernResult;

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
    if(sck < 0)
    {
        perror("socket");

        return false;
    }
    /* Query available interfaces. */
#ifdef HAVE_STRUCT_LIFCONF
    ifc.lifc_len = sizeof(buf);
    ifc.lifc_buf = buf;
    if(ioctl(sck, SIOCGLIFCONF, &ifc) < 0)
    {
        perror("ioctl(SIOCGLIFCONF)");
        return false;
    }
#else
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
    {
        perror("ioctl(SIOCGIFCONF)");
        return false;
    }
#endif

#ifdef HAVE_STRUCT_LIFCONF
    /* Iterate through the list of interfaces. */
    ifr         = ifc.lifc_req;
    nInterfaces = ifc.lifc_len / sizeof(struct lifreq);
#else
    ifr		= ifc.ifc_req;
    nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
#endif
    strcpy(addresses, "");
    char delimiter[2] = "\0";

    for(i = 0; i < nInterfaces; i++)
    {

#ifdef HAVE_STRUCT_LIFCONF
        struct lifreq *item = &ifr[i];
#else
        struct ifreq *item = &ifr[i];
#endif
        struct ether_addr *hw_addr; 
        /* Get the MAC address */
#ifdef SIOCGIFHWADDR
        if(ioctl(sck, SIOCGIFHWADDR, item) < 0)
        {
            perror("ioctl(SIOCGIFHWADDR)");
            return false;
        }
        hw_addr=(struct ether_addr *)(item->ifr_hwaddr.sa_data);
#elif  defined(SIOCGIFARP)
        if(ioctl(sck, SIOCGIFARP, item) < 0)
        {
            perror("ioctl(SIOCGIFARP)");
            return false;
        }
        hw_addr=(struct ether_addr *)&(item->lifr_lifru.lifru_enaddr);  
#endif
        strcat(addresses, delimiter);
        delimiter[0] = ':';
        delimiter[1] = '\0';
        strcat(addresses, ether_ntoa(hw_addr));
    }

    return true;
#else
#warning Don`t know how to obtain mac address.  get_mac_addresses() will return false.
    return false;
#endif
}

