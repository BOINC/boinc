<?php

/**
 * Implementation of hook_default_fe_block_settings().
 */
function global_search_solr_default_fe_block_settings() {
  $export = array();

  // boinc
  $theme = array();

  $theme['apachesolr_search-sort'] = array(
    'module' => 'apachesolr_search',
    'delta' => 'sort',
    'theme' => 'boinc',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-7ML0OK3vgYXeSgMyfPmmpnz2TBw9jZ7J'] = array(
    'module' => 'facetapi',
    'delta' => '7ML0OK3vgYXeSgMyfPmmpnz2TBw9jZ7J',
    'theme' => 'boinc',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-GiIy4zr9Gu0ZSa0bumw1Y9qIIpIDf1wu'] = array(
    'module' => 'facetapi',
    'delta' => 'GiIy4zr9Gu0ZSa0bumw1Y9qIIpIDf1wu',
    'theme' => 'boinc',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-WtseAVugB1EhxGslwR3XIwoa0sVTttYt'] = array(
    'module' => 'facetapi',
    'delta' => 'WtseAVugB1EhxGslwR3XIwoa0sVTttYt',
    'theme' => 'boinc',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-wWWinJ0eOefOtAMbjo2yl86Mnf1rO12j'] = array(
    'module' => 'facetapi',
    'delta' => 'wWWinJ0eOefOtAMbjo2yl86Mnf1rO12j',
    'theme' => 'boinc',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $theme['boinc_solr_search-boinc_solr_search_0'] = array(
    'module' => 'boinc_solr_search',
    'delta' => 'boinc_solr_search_0',
    'theme' => 'boinc',
    'status' => '1',
    'weight' => '0',
    'region' => 'sidebar_second',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'search/*',
    'title' => '',
    'cache' => '1',
  );

  $export['boinc'] = $theme;

  // einstein
  $theme = array();

  $theme['apachesolr_search-sort'] = array(
    'module' => 'apachesolr_search',
    'delta' => 'sort',
    'theme' => 'einstein',
    'status' => '1',
    'weight' => '-21',
    'region' => 'sidebar_second',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-7ML0OK3vgYXeSgMyfPmmpnz2TBw9jZ7J'] = array(
    'module' => 'facetapi',
    'delta' => '7ML0OK3vgYXeSgMyfPmmpnz2TBw9jZ7J',
    'theme' => 'einstein',
    'status' => 0,
    'weight' => '-24',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-GiIy4zr9Gu0ZSa0bumw1Y9qIIpIDf1wu'] = array(
    'module' => 'facetapi',
    'delta' => 'GiIy4zr9Gu0ZSa0bumw1Y9qIIpIDf1wu',
    'theme' => 'einstein',
    'status' => '1',
    'weight' => '-24',
    'region' => 'sidebar_second',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-WtseAVugB1EhxGslwR3XIwoa0sVTttYt'] = array(
    'module' => 'facetapi',
    'delta' => 'WtseAVugB1EhxGslwR3XIwoa0sVTttYt',
    'theme' => 'einstein',
    'status' => '1',
    'weight' => '-23',
    'region' => 'sidebar_second',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-wWWinJ0eOefOtAMbjo2yl86Mnf1rO12j'] = array(
    'module' => 'facetapi',
    'delta' => 'wWWinJ0eOefOtAMbjo2yl86Mnf1rO12j',
    'theme' => 'einstein',
    'status' => 0,
    'weight' => '-23',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $theme['boinc_solr_search-boinc_solr_search_0'] = array(
    'module' => 'boinc_solr_search',
    'delta' => 'boinc_solr_search_0',
    'theme' => 'einstein',
    'status' => '1',
    'weight' => '-20',
    'region' => 'sidebar_second',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'search/*',
    'title' => '',
    'cache' => '1',
  );

  $export['einstein'] = $theme;

  // garland
  $theme = array();

  $theme['apachesolr_search-sort'] = array(
    'module' => 'apachesolr_search',
    'delta' => 'sort',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-7ML0OK3vgYXeSgMyfPmmpnz2TBw9jZ7J'] = array(
    'module' => 'facetapi',
    'delta' => '7ML0OK3vgYXeSgMyfPmmpnz2TBw9jZ7J',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-GiIy4zr9Gu0ZSa0bumw1Y9qIIpIDf1wu'] = array(
    'module' => 'facetapi',
    'delta' => 'GiIy4zr9Gu0ZSa0bumw1Y9qIIpIDf1wu',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-WtseAVugB1EhxGslwR3XIwoa0sVTttYt'] = array(
    'module' => 'facetapi',
    'delta' => 'WtseAVugB1EhxGslwR3XIwoa0sVTttYt',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => 'search/team/*',
    'title' => '',
    'cache' => '-1',
  );

  $theme['facetapi-wWWinJ0eOefOtAMbjo2yl86Mnf1rO12j'] = array(
    'module' => 'facetapi',
    'delta' => 'wWWinJ0eOefOtAMbjo2yl86Mnf1rO12j',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $theme['boinc_solr_search-boinc_solr_search_0'] = array(
    'module' => 'boinc_solr_search',
    'delta' => 'boinc_solr_search_0',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'search/*',
    'title' => '',
    'cache' => '1',
  );

  $export['garland'] = $theme;

  $theme_default = variable_get('theme_default', 'garland');
  $themes = list_themes();
  foreach ($export as $theme_key => $settings) {
    if ($theme_key != $theme_default && empty($themes[$theme_key]->status)) {
      unset($export[$theme_key]);
    }
  }
  return $export;
}
