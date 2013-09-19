<?php

/**
 * Implementation of hook_context_default_contexts().
 */
function news_context_default_contexts() {
  $export = array();

  $context = new stdClass;
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'news_articles';
  $context->description = '';
  $context->tag = 'news';
  $context->conditions = array(
    'node' => array(
      'values' => array(
        'news' => 'news',
      ),
      'options' => array(
        'node_form' => '1',
      ),
    ),
    'path' => array(
      'values' => array(
        'news/*' => 'news/*',
      ),
    ),
  );
  $context->reactions = array(
    'menu' => 'news',
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('news');
  $export['news_articles'] = $context;

  return $export;
}
