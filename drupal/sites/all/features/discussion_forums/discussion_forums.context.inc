<?php

/**
 * Implementation of hook_context_default_contexts().
 */
function discussion_forums_context_default_contexts() {
  $export = array();

  $context = new stdClass;
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'community_default';
  $context->description = 'Highlight the Message Boards submenu when viewing the top level community page';
  $context->tag = 'community';
  $context->conditions = array(
    'path' => array(
      'values' => array(
        'community' => 'community',
      ),
    ),
  );
  $context->reactions = array(
    'menu' => 'community/forum',
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('Highlight the Message Boards submenu when viewing the top level community page');
  t('community');
  $export['community_default'] = $context;

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
