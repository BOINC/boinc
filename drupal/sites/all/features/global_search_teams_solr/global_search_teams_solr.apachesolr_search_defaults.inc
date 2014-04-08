<?php

/**
 * Implementation of hook_apachesolr_search_default_searchers().
 */
function global_search_teams_solr_apachesolr_search_default_searchers() {
  $export = array();

  $searcher = new stdClass;
  $searcher->disabled = FALSE; /* Edit this to true to make a default searcher disabled initially */
  $searcher->api_version = 3;
  $searcher->page_id = 'team_search';
  $searcher->label = 'Team search';
  $searcher->description = '';
  $searcher->search_path = 'search/team';
  $searcher->page_title = 'Search teams';
  $searcher->env_id = 'solr';
  $searcher->settings = array(
    'fq' => array(
      0 => 'bundle:team',
    ),
    'apachesolr_search_custom_enable' => 1,
    'apachesolr_search_search_type' => 'custom',
    'apachesolr_search_search_box' => 1,
    'apachesolr_search_per_page' => '10',
    'apachesolr_search_browse' => 'none',
    'apachesolr_search_spellcheck' => 1,
    'apachesolr_search_allow_user_input' => 0,
  );
  $export['team_search'] = $searcher;

  return $export;
}
