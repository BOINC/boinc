<?php

/**
 * Implementation of hook_context_default_contexts().
 */
function team_forums_context_default_contexts() {
  $export = array();

  $context = new stdClass;
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'team_forum_topic';
  $context->description = 'Highlight the Message Boards submenu when viewing a team forum topic';
  $context->tag = 'community';
  $context->conditions = array(
    'node' => array(
      'values' => array(
        'team_forum' => 'team_forum',
      ),
      'options' => array(
        'node_form' => '1',
      ),
    ),
  );
  $context->reactions = array(
    'menu' => 'community/forum',
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('Highlight the Message Boards submenu when viewing a team forum topic');
  t('community');
  $export['team_forum_topic'] = $context;

  $context = new stdClass;
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'team_forums';
  $context->description = 'Highlight the Message Boards submenu when viewing a team forum';
  $context->tag = 'community';
  $context->conditions = array(
    'path' => array(
      'values' => array(
        'community/teams/*/forum/*' => 'community/teams/*/forum/*',
      ),
    ),
  );
  $context->reactions = array(
    'menu' => 'community/forum',
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('Highlight the Message Boards submenu when viewing a team forum');
  t('community');
  $export['team_forums'] = $context;

  return $export;
}
