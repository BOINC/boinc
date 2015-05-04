<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function work_and_host_stats_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "page_manager" && $api == "pages_default") {
    return array("version" => 1);
  }
}

/**
 * Implementation of hook_views_api().
 */
function work_and_host_stats_views_api() {
  return array(
    'api' => '2',
  );
}
