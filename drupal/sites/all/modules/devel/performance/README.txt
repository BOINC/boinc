$Id: README.txt,v 1.1.4.1 2010/05/16 22:14:28 kbahey Exp $

By Khalid Baheyeldin

Copyright 2008 http://2bits.com

Description
-----------
This module provides performance statistics logging for a site, such as page generation
times, and memory usage, for each page load.

This module is useful for developers and site administrators alike to identify pages that
are slow to generate or use excessive memory.

Features include:
* Settings to enable detailed logging or summary logging. The module defaults to no
  logging at all.

* Detailed logging causes one database row to be written for each page load of the site.
  The data includes page generation time in milliseconds, and the number of bytes allocated
  to PHP, time stamp, ...etc.

* Summary logging logs the average and maximum page generation time, average and maximum memory
  usage, last access time, and number of accesses for each path.

* Summary can be logged to memcache, if configured, so as to not cause extra load on the database.
  This works when APC cannot be used (e.g. certain FastCGI configurations, or when you have many
  web servers on different boxes. This mode is recommended for live sites.

* Summary can be logged to APC, if installed, and the APC data cache is shared, so as to not cause
  extra load on the database. This mode is recommended for live sites.

* A settings option is available when using summary mode with APC, to exclude pages with less
  than a certain number of accesses. Useful for large sites.

* Support for normal page cache.

Note that detailed logging is only suitable for a site that is in development or testing. Do NOT
enable detailed logging on a live site.

The memory measurement feature of this module depends on the memory_get_peak_usage() function,
available only in PHP 5.2.x or later.

Only summary logging with Memcache or APC are recommended mode for live sites, with a threshold of
2 or more.

Note on Completeness:
---------------------
Please note that when summary logging to APC or Memcache, the data captured in the summary will 
not be comprehensive reflecting every single page view for every URL.

The reason for this is that there is no atomic locking when updating the data structures that
store per-URL statistics in this module.

This means that the values you get when using these storage caches are only samples, and would
miss some page views, depending on how busy the site is.

For memcache, there is way to implement locking using the $mc->increment and/or $mc->add as well.
However, there is a risk if these are implemented, that there will be less concurrency and we
can cause a site to slow down.

Configuration:
--------------
If you are using memcache, then you need to configure an extra bin for performance.
If you have multiple web server boxes, then it is best to centralize this bin for
all the boxes, so you get combined statistics.

Your settings.php looks like this:

    $conf = array(
      'cache_inc' => './sites/all/modules/memcache/memcache.inc',

      'memcache_servers' => array(
        '127.0.0.1:11211' => 'default',
        // More bins here ....
        '127.0.0.1:11311' => 'performance',
      ),

      'memcache_bins' => array(
        'cache_performance' => 'performance',
      ),
    );

Bugs/Features/Patches:
----------------------
If you want to report bugs, feature requests, or submit a patch, please do so at the project page on
the Drupal web site at http://drupal.org/project/performance

Author
------
Khalid Baheyeldin (http://baheyeldin.com/khalid and http://2bits.com)

If you use this module, find it useful, and want to send the author a thank you note, then use the
Feedback/Contact page at the URL above.

The author can also be contacted for paid customizations of this and other modules.
