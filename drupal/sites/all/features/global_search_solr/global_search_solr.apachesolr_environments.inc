<?php

/**
 * Implementation of hook_apachesolr_environments().
 */
function global_search_solr_apachesolr_environments() {
  $export = array();

  $environment = new stdClass;
  $environment->api_version = 1;
  $environment->env_id = 'solr';
  $environment->name = 'localhost server';
  $environment->url = 'http://localhost:8983/solr';
  $environment->service_class = '';
  $environment->conf = array();
  $environment->index_bundles = array(
    'node' => array(
      0 => 'forum',
      1 => 'page',
      2 => 'panel',
      3 => 'profile',
      4 => 'story',
    ),
  );
  $export['solr'] = $environment;

  return $export;
}
