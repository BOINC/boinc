#include "mac_address.h"

#include <string.h>

#if defined(__linux__)
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#elif defined(_WIN32)
#include <windows.h>
#include <Iphlpapi.h>
#elif defined(__APPLE__)
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <paths.h>
#include <sysexits.h>
#include <sys/param.h>

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

    #elif defined(__linux__)
    char          buf[1024];
    struct ifconf ifc;
    struct ifreq *ifr;
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
    ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
    if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
    {
        perror("ioctl(SIOCGIFCONF)");

        return false;
    }
    /* Iterate through the list of interfaces. */
    ifr         = ifc.ifc_req;
    nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
    strcpy(addresses, "");
    char delimiter[2] = "\0";

    for(i = 0; i < nInterfaces; i++)
    {
        struct ifreq *item = &ifr[i];
        /* Get the MAC address */
        if(ioctl(sck, SIOCGIFHWADDR, item) < 0)
        {
            perror("ioctl(SIOCGIFHWADDR)");

            return false;
        }
        strcat(addresses, delimiter);
        delimiter[0] = ':';
        delimiter[1] = '\0';
        strcat(addresses, ether_ntoa((struct ether_addr *)item->ifr_hwaddr.sa_data));
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
    #endif
}

