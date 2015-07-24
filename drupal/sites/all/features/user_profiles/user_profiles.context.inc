<?php

/**
 * Implementation of hook_context_default_contexts().
 */
function user_profiles_context_default_contexts() {
  $export = array();

  $context = new stdClass;
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'account--profile--form';
  $context->description = 'Set the account context for the profile form';
  $context->tag = '';
  $context->conditions = array(
    'path' => array(
      'values' => array(
        'account/info/edit/*' => 'account/info/edit/*',
      ),
    ),
  );
  $context->reactions = array(
    'menu' => 'user/me/edit',
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('Set the account context for the profile form');
  $export['account--profile--form'] = $context;

  return $export;
}
