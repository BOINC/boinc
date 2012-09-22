<?php

/**
 * @file
 * Hooks for flag actions.
 */

/**
 * Implementation of hook_hook_info().
 */
function flag_hook_info() {
  return array(
    'flag' => array(
      'flag' => array(
        'flag' => array(
          'runs when' => t('Content has been flagged'),
        ),
        'unflag' => array(
          'runs when' => t('Content has been unflagged')
        ),
      ),
    ),
  );
}
