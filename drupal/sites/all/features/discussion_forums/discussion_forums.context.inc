<?php

/**
 * Implementation of hook_context_default_contexts().
 */
function discussion_forums_context_default_contexts() {
  $export = array();

  $context = new stdClass;
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'forum_topics';
  $context->description = 'Highlight the Community menu when viewing forum topics';
  $context->tag = 'community';
  $context->conditions = array(
    'node' => array(
      'values' => array(
        'forum' => 'forum',
      ),
      'options' => array(
        'node_form' => '1',
      ),
    ),
  );
  $context->reactions = array(
    'menu' => 'forum',
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('Highlight the Community menu when viewing forum topics');
  t('community');
  $export['forum_topics'] = $context;

  return $export;
}
