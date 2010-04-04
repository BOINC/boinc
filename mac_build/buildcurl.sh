#!/bin/sh

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2008 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#
#
# Script to build Macintosh Universal Binary library of curl-7.19.7 for
# use in building BOINC.
#
# Note: reverted to c-ares 1.6.0 from 1.7.0 because the newer c-ares has 
# problems resolving host names on OS 10.6 with default settings when used 
# with AT&T U-Verse 2Wire gateway routers and Airport.
#
# by Charlie Fenton 7/21/06
# Updated 12/3/09 for OS 10.6 Snow Leopard and XCode 3.2.1
# Updated 4/3/10
#
## In Terminal, CD to the curl-7.19.7 directory.
##     cd [path]/curl-7.19.7/
## then run this script:
##     source [path]/buildcurl.sh [ -clean ]
##
## the -clean argument will force a full rebuild.
##

if [ "$1" != "-clean" ]; then
    if [ -f lib/.libs/libcurl_ppc.a ] && [ -f lib/.libs/libcurl_i386.a ] && [ -f lib/.libs/libcurl_x86_64.a ] && [ -f lib/.libs/libcurl.a ]; then
        echo "curl-7.19.7 already built"
        return 0
    fi
fi

if [ ! -d /Developer/SDKs/MacOSX10.4u.sdk/ ]; then
    echo "ERROR: System 10.4u SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

if [ ! -d /Developer/SDKs/MacOSX10.5.sdk/ ]; then
    echo "ERROR: System 10.5 SDK is missing.  For details, see build instructions at"
    echo "boinc/mac_build/HowToBuildBOINC_XCode.rtf or http://boinc.berkeley.edu/trac/wiki/MacBuild"
    return 1
fi

# Update to libcurl 7.19.7 (DNS Cache and Sync DNS patches)
# Patch lib/hostip.c
if [ ! -f lib/hostip.c.orig ]; then
cat >> /tmp/hostip_c_diff << ENDOFFILE
--- curl-7.19.7/lib/hostip.c	Thu Nov  5 16:31:07 2009
+++ moka5/lib/hostip.c	Fri Nov  6 16:26:09 2009
@@ -123,6 +123,10 @@
 static struct curl_hash hostname_cache;
 static int host_cache_initialized;
 
+#ifdef CURLDEBUG
+static int ndns = 0;
+#endif
+
 static void freednsentry(void *freethis);
 
 /*
@@ -232,14 +236,7 @@
     (struct hostcache_prune_data *) datap;
   struct Curl_dns_entry *c = (struct Curl_dns_entry *) hc;
 
-  if((data->now - c->timestamp < data->cache_timeout) ||
-      c->inuse) {
-    /* please don't remove */
-    return 0;
-  }
-
-  /* fine, remove */
-  return 1;
+  return (data->now - c->timestamp >= data->cache_timeout);
 }
 
 /*
@@ -339,7 +336,6 @@
   size_t entry_len;
   struct Curl_dns_entry *dns;
   struct Curl_dns_entry *dns2;
-  time_t now;
 
   /* Create an entry id, based upon the hostname and port */
   entry_id = create_hostcache_id(hostname, port);
@@ -357,23 +353,22 @@
 
   dns->inuse = 0;   /* init to not used */
   dns->addr = addr; /* this is the address(es) */
+  time(&dns->timestamp);
+  if(dns->timestamp == 0)
+    dns->timestamp = 1;   /* zero indicates that entry isn't in hash table */
 
-  /* Store the resolved data in our DNS cache. This function may return a
-     pointer to an existing struct already present in the hash, and it may
-     return the same argument we pass in. Make no assumptions. */
+  /* Store the resolved data in our DNS cache. */ 
   dns2 = Curl_hash_add(data->dns.hostcache, entry_id, entry_len+1,
                        (void *)dns);
   if(!dns2) {
-    /* Major badness, run away. */
     free(dns);
     free(entry_id);
     return NULL;
   }
-  time(&now);
-  dns = dns2;
 
-  dns->timestamp = now; /* used now */
+  dns = dns2;
   dns->inuse++;         /* mark entry as in-use */
+  DEBUGF(ndns++);
 
   /* free the allocated entry_id again */
   free(entry_id);
@@ -436,6 +431,7 @@
 
   if(dns) {
     dns->inuse++; /* we use it! */
+    DEBUGF(ndns++);
     rc = CURLRESOLV_RESOLVED;
   }
 
@@ -688,6 +684,12 @@
     Curl_share_lock(data, CURL_LOCK_DATA_DNS, CURL_LOCK_ACCESS_SINGLE);
 
   dns->inuse--;
+  DEBUGF(ndns--);
+  /* only free if nobody is using AND it is not in hostcache (timestamp == 0) */
+  if (dns->inuse == 0 && dns->timestamp == 0) {
+    Curl_freeaddrinfo(dns->addr);
+    free(dns);
+  }
 
   if(data->share)
     Curl_share_unlock(data, CURL_LOCK_DATA_DNS);
@@ -700,11 +702,20 @@
 {
   struct Curl_dns_entry *p = (struct Curl_dns_entry *) freethis;
 
-  if(p) {
-    Curl_freeaddrinfo(p->addr);
-    free(p);
-  }
+  /* mark the entry as not in hostcache */
+  p->timestamp = 0;
+  if (p->inuse == 0) {
+     Curl_freeaddrinfo(p->addr);
+     free(p);
+  }   
 }
+
+#ifdef CURLDEBUG
+int curl_get_ndns(void)
+{
+  return ndns;
+}
+#endif
 
 /*
  * Curl_mk_dnscache() creates a new DNS cache and returns the handle for it.
ENDOFFILE

patch -bfi /tmp/hostip_c_diff lib/hostip.c

rm -f /tmp/hostip_c_diff
else
    echo "hostip.c already patched"
fi


# Patch lib/hostip.h
if [ ! -f lib/hostip.h.orig ]; then
cat >> /tmp/hostip_h_diff << ENDOFFILE
--- curl-7.19.7/lib/hostip.h	Thu Nov  5 16:31:07 2009
+++ moka5/lib/hostip.h	Fri Nov  6 16:21:20 2009
@@ -121,6 +121,8 @@
 
 struct Curl_dns_entry {
   Curl_addrinfo *addr;
+  /* timestamp == 0 -- entry not in hostcache
+     timestamp != 0 -- entry is in hostcache */
   time_t timestamp;
   long inuse;      /* use-counter, make very sure you decrease this
                       when you're done using the address you received */
ENDOFFILE
### ' (Fix syntax coloring for easier readability)

patch -bfi /tmp/hostip_h_diff lib/hostip.h

rm -f /tmp/hostip_h_diff
else
    echo "hostip.h already patched"
fi


# Patch lib/hash.c
if [ ! -f lib/hash.c.orig ]; then
cat >> /tmp/hash_c_diff << ENDOFFILE
--- curl-7.19.7/lib/hash.c	Thu Nov  5 16:31:07 2009
+++ moka5/lib/hash.c	Wed Nov  4 13:46:15 2009
@@ -140,8 +140,8 @@
 
 #define FETCH_LIST(x,y,z) x->table[x->hash_func(y, z, x->slots)]
 
-/* Return the data in the hash. If there already was a match in the hash,
-   that data is returned. */
+/* Insert the data in the hash. If there already was a match in the hash,
+   that data is replaced. */
 void *
 Curl_hash_add(struct curl_hash *h, void *key, size_t key_len, void *p)
 {
@@ -152,8 +152,9 @@
   for (le = l->head; le; le = le->next) {
     he = (struct curl_hash_element *) le->ptr;
     if(h->comp_func(he->key, he->key_len, key, key_len)) {
-      h->dtor(p);     /* remove the NEW entry */
-      return he->ptr; /* return the EXISTING entry */
+      Curl_llist_remove(l, le, (void *)h); 
+      --h->size;
+      break;
     }
   }
ENDOFFILE

patch -bfi /tmp/hash_c_diff lib/hash.c

rm -f /tmp/hash_c_diff
else
    echo "hash.c already patched"
fi

export PATH=/usr/local/bin:$PATH

CURL_DIR=`pwd`
# curl configure and make expect a path to _installed_ c-ares-1.6.0
# so temporarily install c-ares at a path that does not contain spaces.
cd ../c-ares-1.6.0
make install 
cd "${CURL_DIR}"

export SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.4

rm -f lib/.libs/libcurl.a
rm -f lib/.libs/libcurl_ppc.a
rm -f lib/.libs/libcurl_i386.a
rm -f lib/.libs/libcurl_x86_64.a

# cURL configure creates a different curlbuild.h file for each architecture
rm -f include/curl/curlbuild.h
rm -f include/curl/curlbuild_ppc.h
rm -f include/curl/curlbuild_i386.h
rm -f include/curl/curlbuild_x86_64.h

export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS=" -isysroot /Developer/SDKs/MacOSX10.4u.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch ppc"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc"

# c-ares configure creates a different ares_build.h file for each architecture
cp -f ../c-ares-1.6.0/ares_build_ppc.h /tmp/installed-c-ares/include/ares_build.h

./configure --enable-shared=NO --enable-ares=/tmp/installed-c-ares --host=ppc
if [  $? -ne 0 ]; then return 1; fi

make clean

make
if [  $? -ne 0 ]; then return 1; fi
mv -f include/curl/curlbuild.h include/curl/curlbuild_ppc.h
mv -f lib/.libs/libcurl.a lib/libcurl_ppc.a

make clean
if [  $? -ne 0 ]; then return 1; fi

##export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
export SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.4

# c-ares configure creates a different ares_build.h file for each architecture
cp -f ../c-ares-1.6.0/ares_build_i386.h /tmp/installed-c-ares/include/ares_build.h

./configure --enable-shared=NO --enable-ares=/tmp/installed-c-ares --host=i386
if [  $? -ne 0 ]; then return 1; fi

make
if [  $? -ne 0 ]; then return 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

# Build for x86_64 architecture using OS 10.5 SDK
mv -f include/curl/curlbuild.h include/curl/curlbuild_i386.h
mv -f lib/.libs/libcurl.a lib/libcurl_i386.a

make clean
if [  $? -ne 0 ]; then return 1; fi

##export PATH=/usr/local/bin:$PATH
export CC=/usr/bin/gcc-4.0;export CXX=/usr/bin/g++-4.0
export LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export CPPFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export CFLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64"
export SDKROOT="/Developer/SDKs/MacOSX10.5.sdk"
export MACOSX_DEPLOYMENT_TARGET=10.5

# c-ares configure creates a different ares_build.h file for each architecture
cp -f ../c-ares-1.6.0/ares_build_x86_64.h /tmp/installed-c-ares/include/ares_build.h

./configure --enable-shared=NO --enable-ares=/tmp/installed-c-ares --host=x86_64
if [  $? -ne 0 ]; then return 1; fi

make
if [  $? -ne 0 ]; then return 1; fi

export CC="";export CXX=""
export LDFLAGS=""
export CPPFLAGS=""
export CFLAGS=""
export SDKROOT=""

mv -f include/curl/curlbuild.h include/curl/curlbuild_x86_64.h

mv -f lib/.libs/libcurl.a lib/.libs/libcurl_x86_64.a
mv -f lib/libcurl_ppc.a lib/.libs/
mv -f lib/libcurl_i386.a lib/.libs/
lipo -create lib/.libs/libcurl_i386.a lib/.libs/libcurl_x86_64.a lib/.libs/libcurl_ppc.a -output lib/.libs/libcurl.a
if [  $? -ne 0 ]; then return 1; fi

# Delete temporarily installed c-ares.
rm -Rf /tmp/installed-c-ares/

rm -f include/curl/curlbuild.h

# Create a custom curlbuild.h file which directs BOINC builds 
# to the correct curlbuild_xxx.h file for each architecture.
cat >> include/curl/curlbuild.h << ENDOFFILE
/***************************************************************************
*
* This file was created for BOINC by the buildcurl.sh script
*
* You should not need to modify it manually
*
 ***************************************************************************/

#ifndef __BOINC_CURLBUILD_H
#define __BOINC_CURLBUILD_H

#ifndef __APPLE__
#error - this file is for Macintosh only
#endif

#ifdef __x86_64__
#include "curl/curlbuild_x86_64.h"
#elif defined(__ppc__)
#include "curl/curlbuild_ppc.h"
#elif defined(__i386__)
#include "curl/curlbuild_i386.h"
#else
#error - unknown architecture
#endif

#endif /* __BOINC_CURLBUILD_H */
ENDOFFILE

return 0
