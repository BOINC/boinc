<?php

/**
 * Implementation of hook_views_default_views().
 */
function features_test_views_default_views() {
  $views = array();

  // Exported view: features_test
  $view = new view;
  $view->name = 'features_test';
  $view->description = 'Test view provided by Features testing module.';
  $view->tag = 'testing';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('access', array(
    'type' => 'none',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Test');
  $handler->override_option('row_plugin', 'node');

  $views[$view->name] = $view;

  return $views;
}
