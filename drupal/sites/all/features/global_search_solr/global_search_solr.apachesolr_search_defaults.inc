<?php

/**
 * Implementation of hook_apachesolr_search_default_searchers().
 */
function global_search_solr_apachesolr_search_default_searchers() {
  $export = array();

  $searcher = new stdClass;
  $searcher->disabled = FALSE; /* Edit this to true to make a default searcher disabled initially */
  $searcher->api_version = 3;
  $searcher->page_id = 'core_search';
  $searcher->label = 'Core Search';
  $searcher->description = 'Core Search';
  $searcher->search_path = 'search/site';
  $searcher->page_title = bts('Search site', array(), NULL, 'boinc:search-page');
  $searcher->env_id = 'solr';
  $searcher->settings = array(
    'fq' => array(),
    'apachesolr_search_custom_enable' => 0,
    'apachesolr_search_search_type' => 'custom',
    'apachesolr_search_search_box' => TRUE,
    'apachesolr_search_per_page' => '10',
    'apachesolr_search_browse' => 'none',
    'apachesolr_search_spellcheck' => 1,
    'apachesolr_search_allow_user_input' => 0,
  );
  $export['core_search'] = $searcher;

  return $export;
}
