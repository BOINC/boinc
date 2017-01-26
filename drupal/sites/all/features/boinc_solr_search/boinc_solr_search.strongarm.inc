<?php

/**
 * Implementation of hook_strongarm().
 */
function boinc_solr_search_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'boinc_solr_comments_nodetypes';
  $strongarm->value = array(
    'forum' => 'forum',
    'news' => 'news',
    'page' => 'page',
    'story' => 'story',
    'team_forum' => 'team_forum',
  );
  $export['boinc_solr_comments_nodetypes'] = $strongarm;

  return $export;
}
