<?php

/**
 * Implementation of hook_strongarm().
 */
function default_site_content_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'boinc_default_site_content';
  $strongarm->value = 1;
  $export['boinc_default_site_content'] = $strongarm;

  return $export;
}
