<?php

// Some web pages need lots of DB access to generate.
// To reduce DB server load, these pages
// (actually, the data used to create them) can be cached.
// Each type of page has a 'time to live' (TTL) parameter.
// If the age of a cached page exceeds the TTL,
// the page is regenerated.
//
// TTL = 0 means don't cache that type of page

require_once('../inc/util_basic.inc');

define('TEAM_PAGE_TTL', 3600);
define('USER_PAGE_TTL', 3600);
define('USER_PROFILE_TTL', 3600);
define('TOP_PAGES_TTL', 0);
    // host/user/team leader boards
    // caching these causes inconsistent results
define('INDEX_PAGE_TTL', 3600);
define('STATUS_PAGE_TTL', 3600);
define('REMOTE_PROJECTS_TTL', 86400);

// max cache usage, bytes
//
define('MAX_CACHE_USAGE', 10*MEGA);

// the cache can grow without bound;
// every so often we need to check its size and delete old entries.
// We do this randomly, on average every N page views

define('CACHE_SIZE_CHECK_FREQ', 1000);

?>
