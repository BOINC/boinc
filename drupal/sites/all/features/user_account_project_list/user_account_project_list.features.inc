<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function user_account_project_list_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "panels_mini" && $api == "panels_default") {
    return array("version" => 1);
  }
}
