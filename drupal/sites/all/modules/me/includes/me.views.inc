<?php

/**
 * @file
 *
 * Provides views intergration for the me module.
 */

/**
 * Implementation of hook_views_plugins
 */
function me_views_plugins() {
  // Generate our plugin data.
  return array(
    'argument validator' => array(
      'me' => array(
        'title' => t('Me'),
        'handler' => 'me_plugin_argument_validate_me_alias',
        'path' => drupal_get_path('module', 'me') . '/includes',
        'parent' => 'user',
      ),
    ),
  );
}

/**
 * Implementation of Views' hook_views_pre_execute().
 *
 * See http://drupal.org/project/views for module and hook information.
 */
function me_views_pre_execute($view, $do_redirect = FALSE) {
  static $redirect = FALSE;

  // If $view is NULL, then we are setting the redirect.
  if (is_null($view)) {
    $redirect = $do_redirect;
  }
  elseif (!empty($redirect)) {
    // We should always be able to redirect here regardless, as our handler has to
    // have run for our option to be set, which means we need to redirect anyway.

    // Loop over the argument handlers to get the arguments we need. We also keep this
    // consistent with any extra arguments that may have been passed in.
    $arguments = $view->args;
    foreach (array_values($view->argument) as $key => $argument) {
      if (isset($argument->argument))  {
        $arguments[$key] = $argument->argument;
      }
    }

    // Redirect to the path.
    drupal_goto($view->get_url($arguments));
  }
}
