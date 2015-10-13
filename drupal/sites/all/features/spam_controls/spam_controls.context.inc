<?php

/**
 * Implementation of hook_context_default_contexts().
 */
function spam_controls_context_default_contexts() {
  $export = array();

  $context = new stdClass;
  $context->disabled = FALSE; /* Edit this to true to make a default context disabled initially */
  $context->api_version = 3;
  $context->name = 'account_info';
  $context->description = '';
  $context->tag = 'account';
  $context->conditions = array(
    'path' => array(
      'values' => array(
        'account/info/edit/profile' => 'account/info/edit/profile',
      ),
    ),
    'user_page' => array(
      'values' => array(
        'form' => 'form',
      ),
      'options' => array(
        'mode' => 'current',
      ),
    ),
  );
  $context->reactions = array(
    'menu' => 'user/me/edit',
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('account');
  $export['account_info'] = $context;

  return $export;
}
