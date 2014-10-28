<?php

/**
 * Implementation of hook_strongarm().
 */
function internationalization_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'i18nstrings_allowed_formats';
  $strongarm->value = array(
    0 => 4,
  );
  $export['i18nstrings_allowed_formats'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'language_count';
  $strongarm->value = 2;
  $export['language_count'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'language_negotiation';
  $strongarm->value = '2';
  $export['language_negotiation'] = $strongarm;

  return $export;
}
