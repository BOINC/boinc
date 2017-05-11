<?php
/**
 * @file
 * views-revert - Drush command to revert views overridden in the system.
 */

/**
 * Implement hook_drush_help().
 */
function views_revert_drush_help($section) {
  switch ($section) {
    case 'drush:revert-views':
      return dt('Reverts all views in the drupal installation that have been overriden. Careful, use with care.');
  }
}

/**
 * Implement hook_drush_command().
 */
function views_revert_drush_command() {
  $items = array();

  $items['views-revert'] = array(
    'callback' => 'views_revert_views',
    'drupal dependencies' => array('views'),
    'description' => dt('Revert overridden views to their default state. Make sure to backup first.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'aliases' => array('vr'),
  );

  return $items;
}

/**
 * Callback function for views-revert command.
 */
function views_revert_views() {
  $views = views_get_all_views();
  $i = 0;
  // The provided view names specified in the command.
  $viewnames = _convert_csv_to_array(func_get_args());

  // Find all overridden views.
  foreach ($views as $view) {
    if ($view->disabled) {
      continue;
    }
    if ($view->type == dt('Overridden')) {
      $overridden[$view->name] = $view->name;
    }
  }

  // Return early if there are no views overridden in the system.
  if (empty($overridden)) {
    return drush_set_error(dt('There are no overridden views in the system.'));
  }

  // If the user specified in the command the views to be overridden.
  if (!empty($viewnames)) {
    foreach ($viewnames as $key => $viewname) {
      $is_overridden = key_exists($viewname, $overridden);
      // Check if the provided view name is in the system
      if ($viewname && !key_exists($viewname, $views)) {
        drush_set_error(dt("'@viewname' view is not present in the system.", array('@viewname' => $viewname)));
      }
      // Check if the provided view is overridden.
      elseif (!$is_overridden) {
        drush_set_error(dt("The view specified '@viewname' is not overridden.", array('@viewname' => $viewname)));
      }
      // If the view is overriden, revert it.
      elseif ($is_overridden){
        views_revert_view($views[$viewname]);
        $i++;
      }
      // We should never get here but well...
      else {
        drush_set_error(dt("The view specified '@viewname' is not provided in code, and thus cannot be reverted.", array('@viewname' => $viewname)));
      }
    }
  }

  // The user did not specify any views in the command, prompt the user
  else {
    // list of choices for the user
    $overridden['all'] = dt('Revert all overridden views'); // add a choice at the end
    $choice = drush_choice($overridden, 'Enter a number to choose which view to revert.', '!key'); // prompt the user

    if ($choice !== FALSE) {
      // revert all views option
      if ($choice == 'all') {
        $i = views_revert_allviews($views);
      }
      // else the user specified a single view
      else {
        views_revert_view($views[$choice]);
        $i++;
      }
    }

  }

  // final results output
  if ($i == 0) {
    drush_log(dt('No views were reverted.'), 'ok');
  }
  else {
    drush_log(dt('Reverted a total of @count views.', array('@count' => $i)), 'ok');
  }
}

/**
 * Reverts all views
 * @param $views
 * All views in the system as provided by views_get_all_views().
 */
function views_revert_allviews($views) {
  $i = 0;
  foreach ($views as $view) {
    if ($view->disabled) {
      continue;
    }

    if ($view->type == t('Overridden')) {
      views_revert_view($view);
      $i++;
    }
  }
  return $i;
}

/**
 * Revert a specified view
 * @param $view
 * The view object to be reverted
 *
 * Checks on wether or not the view is overridden is handled in views_revert_views_revert()
 * We perform a check here anyway in case someone somehow calls this function on their own...
 */
function views_revert_view($view) {
  // check anyway just in case
  if ($view->type == t('Overridden')) {
    // Revert the view.
    $view->delete();
    // Clear its cache.
    views_object_cache_clear('view', $view->name);
    // Give feedback.
    $message = dt("Reverted the view '@viewname'", array('@viewname' => $view->name));
    drush_log($message, 'success');
    // Reverted one more view.
  }
  else {
    drush_set_error(dt("The view '@viewname' is not overridden.", array('@viewname' => $view->name)));
  }
}
