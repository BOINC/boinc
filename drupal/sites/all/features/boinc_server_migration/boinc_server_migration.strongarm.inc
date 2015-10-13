<?php

/**
 * Implementation of hook_strongarm().
 */
function boinc_server_migration_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'boincimport_input_format';
  $strongarm->value = '4';
  $export['boincimport_input_format'] = $strongarm;

  return $export;
}
