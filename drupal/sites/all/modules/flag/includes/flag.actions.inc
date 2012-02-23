<?php
// $Id: flag.actions.inc,v 1.1.4.2 2009/03/17 02:07:34 quicksketch Exp $

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
