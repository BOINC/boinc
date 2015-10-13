<?php

// Define the lifetime of public php pages.
// define to zero to turn of caching for these pages.
define('TEAM_PAGE_TTL', 3600);
define('USER_PAGE_TTL', 3600);
define('USER_HOST_TTL', 3600);
define('USER_PROFILE_TTL', 3600);
define('TOP_PAGES_TTL', 43200);
define('INDEX_PAGE_TTL', 3600);
define('STATUS_PAGE_TTL', 3600);
define('REMOTE_PROJECTS_TTL', 86400);

// max allowed cache usage
//
define('MAX_CACHE_USAGE', 104857600);

// Number of page views between cache size checks
define('CACHE_SIZE_CHECK_FREQ', 1000);

?>
