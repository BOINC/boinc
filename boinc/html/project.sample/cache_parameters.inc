<?php

// Define the lifetime of public php pages.
// define to zero to turn of caching for these pages.
define('TEAM_PAGE_TTL', 3600);
define('USER_PAGE_TTL', 3600);
define('USER_HOST_TTL', 3600);
define('USER_PROFILE_TTL', 3600);
define('TOP_PAGES_TTL', 43200);
define('INDEX_PAGE_TTL', 3600);

// Define the max allowed cache usage and the min allowed free space
// Both default to 100M.
define('MAX_CACHE_USAGE', 104857600);
define('MIN_FREE_SPACE', 104857600);

// Number of page views between cache size checks
define('CACHE_SIZE_CHECK_FREQ', 1000);

?>
