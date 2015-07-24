<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function global_search_solr_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "apachesolr" && $api == "apachesolr_environments") {
    return array("version" => 1);
  }
  elseif ($module == "apachesolr_search" && $api == "apachesolr_search_defaults") {
    return array("version" => 3);
  }
  elseif ($module == "facetapi" && $api == "facetapi_defaults") {
    return array("version" => 1);
  }
}
