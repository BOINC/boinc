<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function global_search_teams_solr_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "apachesolr_search" && $api == "apachesolr_search_defaults") {
    return array("version" => 3);
  }
}
