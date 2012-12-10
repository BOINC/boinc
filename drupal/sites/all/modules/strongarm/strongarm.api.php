<?php
// $Id$

/**
 * @file
 *  Hooks provided by Strongarm.
 */

/**
 * Implementation of hook_ctools_plugin_api().
 *
 * Example of a CTools plugin hook that needs to be implemented to make
 * hook_strongarm() discoverable by CTools and Strongarm. The hook specifies
 * that the hook_strongarm() returns Strongarm API version 1 style plugins.
 */
function hook_ctools_plugin_api() {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
}

/**
 * Defines one or more variables for exportables-style import.
 *
 * These values are loaded if the variable is not present in the database.
 *
 * @return
 *  Array of objects containing variable value definitions.
 *  - disabled: Set to TRUE to ignore this value, set to FALSE to use it.
 *  - api_version: Specify which version of the Strongarm API your code uses.
 *  - name: The name of the variable, such as 'site_offline' or 'clean_urls'.
 *  - value: The value to specify for the variable.
 *
 */
function hook_strongarm() {
  $export = array();

  $variable = new stdClass;
  $variable->disabled = FALSE;
  $variable->api_version = 1;
  $variable->name = 'variable_name';
  $variable->value = 'variable_value';
  $export[$variable->name] = $variable;
  
  return $export;
}

/**
 * Adjust the definition of a Strongarm variable before it is imported.
 *
 * Note that this is applied before caching.
 *
 * @param $strongarms
 *  Array of all strongarm-exported variable definitions. Keyed on variable name.
 *  The elements of each strongarm object in the array are:
 *  - disabled: Whether the strongarm variable should be ignored.
 *  - api_version: The version of the Strongarm API for the variable definition.]
 *  - name: Name of the variable, repeated.
 *  - value: The value specified for the variable.
 *  - export_module: The name of the module that is exporting the variable.
 */
function hook_strongarm_alter(&$strongarms) {
  $strongarms['variable_name']->value = 'variable_better_value';
}