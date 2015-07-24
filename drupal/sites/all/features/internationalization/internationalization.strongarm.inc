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
    1 => 5,
  );
  $export['i18nstrings_allowed_formats'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'i18ntaxonomy_vocabulary';
  $strongarm->value = array(
    2 => '1',
    1 => '0',
  );
  $export['i18ntaxonomy_vocabulary'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'language_negotiation';
  $strongarm->value = '2';
  $export['language_negotiation'] = $strongarm;

  return $export;
}
