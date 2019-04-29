<?php

/**
 * @file
 * Contains cron hooks for the object cache tool.
 *
 * We use this to clean up old object caches.
 */

function ctools_object_cache_cron() {
  if (variable_get('ctools_last_cron', 0) < time() - 86400) {
    variable_set('ctools_last_cron', time());
    ctools_include('object-cache');
    ctools_object_cache_clean();
  }
}
