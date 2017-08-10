<?php

/*******************************************************************************
 * DRUSH SUPPORT
 ******************************************************************************/

/**
 * Implementation of hook_drush_command().
 */
function elysia_cron_drush_command() {
  $items = array();

  $items['elysia-cron'] = array(
    'description' => "Run all cron hooks in all active modules for specified site using elysia cron system. Use this instead of \"core-cron\" if elysia_cron module is installed",
    'callback'    => 'drush_elysia_cron_run_wrapper',
    'arguments' => array(),
  );

  return $items;
}

/**
 * A drush command callback.
 *
 * wraps the elysia_cron_run function, passing manual = true
 */
function drush_elysia_cron_run_wrapper() {
  return elysia_cron_run(TRUE);
}
