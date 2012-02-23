<?php
// $Id: admin.inc,v 1.161.2.12 2010/06/17 02:45:36 merlinofchaos Exp $
/**
 * @file admin.inc
 * Provides the Views' administrative interface.
 */

/**
 * Page callback to list views in the system.
 */
function views_ui_list_views($arg = NULL) {
  if ($arg != NULL) {
    return drupal_not_found();
  }

  $output = theme('views_ui_list_views');
  views_ui_check_advanced_help();
  return $output;
}

/**
 * Check to see if the advanced help module is installed, and if not put up
 * a message.
 *
 * Only call this function if the user is already in a position for this to
 * be useful.
 */
function views_ui_check_advanced_help() {
  if (variable_get('views_hide_help_message', FALSE)) {
    return;
  }

  if (!module_exists('advanced_help')) {
    $filename = db_result(db_query("SELECT filename FROM {system} WHERE type = 'module' AND name = 'advanced_help'"));
    if ($filename && file_exists($filename)) {
      drupal_set_message(t('If you <a href="@modules">enable the advanced help module</a>, Views will provide more and better help. <a href="@hide">Hide this message.</a>', array('@modules' => url('admin/build/modules'),'@hide' => url('admin/build/views/tools'))));
    }
    else {
      drupal_set_message(t('If you install the advanced help module from !href, Views will provide more and better help. <a href="@hide">Hide this message.</a>', array('!href' => l('http://drupal.org/project/advanced_help', 'http://drupal.org/project/advanced_help'), '@hide' => url('admin/build/views/tools'))));
    }
  }
}

/**
 * Preprocess the list views theme
 */
function template_preprocess_views_ui_list_views(&$vars) {
  $items = array();
  $sorts = array();

  $views = views_get_all_views();

  $token_enable = drupal_get_token('views-enable');
  $token_disable = drupal_get_token('views-disable');

  // Respond to a reset command by clearing session and doing a drupal goto
  // back to the base URL.
  if (isset($_GET['op']) && $_GET['op'] == t('Reset')) {
    unset($_SESSION['views']['#admin']);
    drupal_goto('admin/build/views');
  }
  if (count($_GET) <= 1) {
    if (isset($_SESSION['views']['#admin']) && is_array($_SESSION['views']['#admin'])) {
      $_GET += $_SESSION['views']['#admin'];
    }
  }
  else {
    $_SESSION['views']['#admin'] = $_GET;
    unset($_SESSION['views']['#admin']['q']);
  }

  $form_state = array(
    'views' => $views,
    'input' => $_GET,
    'method' => 'get',
    'rerender' => TRUE,
    'no_redirect' => TRUE,
  );

  $vars['widgets'] = drupal_build_form('views_ui_list_views_form', $form_state);

  $vars['help_type_icon'] = theme('advanced_help_topic', 'views', 'view-type');

  $base_tables = views_fetch_base_tables();

  foreach ($views as $view) {
    if ($form_state['values']['tag'] != 'all') {
      if ($form_state['values']['tag'] == 'none') {
        if (!empty($view->tag)) {
          continue;
        }
      }
      else if ($form_state['values']['tag'] != $view->tag) {
        continue;
      }
    }
    if ($form_state['values']['type'] != 'all' && $form_state['values']['type'] != $view->type) {
      continue;
    }

    if ($form_state['values']['base'] != 'all' && $form_state['values']['base'] != $view->base_table) {
      continue;
    }

    if ($form_state['values']['display'] != 'all' && empty($view->display[$form_state['values']['display']])) {
      continue;
    }

    $item = new stdClass();
    $item->ops = array();
    if (empty($view->disabled)) {
      $item->ops[] = l(t('Edit'), "admin/build/views/edit/$view->name");
      $item->ops[] = l(t('Export'), "admin/build/views/export/$view->name");
      $item->ops[] = l(t('Clone'), "admin/build/views/clone/$view->name");
    }
    if ($view->type != t('Default')) {
      $text = $view->type == t('Overridden') ? t('Revert') : t('Delete');
      $item->ops[] = l($text, "admin/build/views/delete/$view->name");
    }
    else {
      if (empty($view->disabled)) {
        $item->ops[] = l(t('Disable'), "admin/build/views/disable/$view->name", array('query' => drupal_get_destination() . '&token=' . $token_disable));
      }
      else {
        $item->ops[] = l(t('Enable'), "admin/build/views/enable/$view->name", array('query' => drupal_get_destination() . '&token=' . $token_enable));
      }
    }

    $item->ops = implode(' | ', $item->ops);
    if (empty($view->display)) {
      $item->path = t('Warning! Broken view!');
    }
    else {
      $item->path = $raw_path = $view->get_path();
      $item->path = $item->path && empty($view->disabled) && strpos($item->path, '%') === FALSE ? l($item->path, $item->path) : check_plain($item->path);
    }

    $item->type = $view->type;
    $item->name = $view->name;

    if (!empty($view->tag)) {
      $item->tag = $view->tag;
    }

    $item->title = $view->get_title();
    $item->base = !empty($base_tables[$view->base_table]['title']) ? $base_tables[$view->base_table]['title'] : t('Broken');

    $item->displays = array();
    foreach ($view->display as $display) {
      if (!empty($display->handler->definition['admin'])) {
        $item->displays[$display->handler->definition['admin']] = TRUE;
      }
    }

    if ($item->displays) {
      ksort($item->displays);
      $item->displays = implode(', ', array_keys($item->displays));
    }

    $item->description = check_plain($view->description);
    $item->classes = empty($view->disabled) ? 'view-enabled' : 'view-disabled';
    $items[] = $item;

    $sort = intval(empty($view->disabled) xor $form_state['values']['sort'] == 'asc');

    switch ($form_state['values']['order']) {
      case 'name':
      default:
        $sort .= strtolower($view->name);
        break;
      case 'title':
        $sort .= strtolower($item->title);
        break;
      case 'path':
        $sort .= strtolower($raw_path); // $path;
        break;
      case 'type':
        $sort .= $view->type . $view->name;
        break;
      case 'tag':
        $sort .= strtolower($view->tag);
        break;
      case 'desc':
        $sort .= strtolower($view->description);
        break;
    }

    $sorts[] = $sort;
  }

  if ($form_state['values']['sort'] == 'desc') {
    arsort($sorts);
  }
  else {
    asort($sorts);
  }

  $i = array();
  foreach ($sorts as $id => $title) {
    $i[] = $items[$id];
  }

  views_add_css('views-list');
  $vars['views'] = $i;

  $getting_started = theme('advanced_help_topic', 'views', 'getting-started', 'title');
  if (!$getting_started) {
    $getting_started = t('Install the advanced help module for the getting started');
  }

  $vars['help'] = t('Not sure what to do? Try the "!getting-started" page.', array('!getting-started' => $getting_started));
}

/**
 * Provide a form for sorting and filtering the list of views.
 */
function views_ui_list_views_form(&$form_state) {
  if (!variable_get('clean_url', FALSE)) {
    $form['q'] = array(
      '#type' => 'hidden',
      '#value' => $_GET['q'],
    );
  }

  $all = array('all' => t('<All>'));
  $none = array('none' => t('<None>'));

  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Storage'),
    '#options' => array(
      'all' => t('<All>'),
      t('Normal') => t('Normal'),
      t('Default') => t('Default'),
      t('Overridden') => t('Overridden'),
    ),
    '#default_value' => 'all',
  );

  $bases = array();
  foreach (views_fetch_base_tables() as $table => $info) {
    $bases[$table] = $info['title'];
  }

  $form['base'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => array_merge($all, $bases),
    '#default_value' => 'all',
  );

  $tags = array();

  $extras = array();
  foreach ($form_state['views'] as $name => $view) {
    if (!empty($view->tag)) {
      $tags[$view->tag] = $view->tag;
    }
  }

  asort($tags);

  $form['tag'] = array(
    '#type' => 'select',
    '#title' => t('Tag'),
    '#options' => array_merge($all, $none, $tags),
    '#default_value' => 'all',
  );

  $displays = array();
  foreach (views_fetch_plugin_data('display') as $id => $info) {
    if (!empty($info['admin'])) {
      $displays[$id] = $info['admin'];
    }
  }

  asort($displays);

  $form['display'] = array(
    '#type' => 'select',
    '#title' => t('Displays'),
    '#options' => array_merge($all, $displays),
    '#default_value' => 'all',
  );

  $form['order'] = array(
    '#type' => 'select',
    '#title' => t('Sort by'),
    '#options' => array(
      'name' => t('Name'),
      'title' => t('Title'),
      'tag' => t('Tag'),
      'path' => t('Path'),
      'type' => t('Type'),
      'desc' => t('Description'),
    ),
    '#default_value' => 'name',
  );

  $form['sort'] = array(
    '#type' => 'select',
    '#title' => t('Order'),
    '#options' => array(
      'asc' => t('Up'),
      'desc' => t('Down'),
    ),
    '#default_value' => 'asc',
  );

  $form['submit'] = array(
    '#name' => '', // so it won't in the $_GET args
    '#type' => 'submit',
    '#id' => 'edit-views-apply',
    '#value' => t('Apply'),
  );

  if (!empty($_SESSION['views']['#admin'])) {
    $form['reset'] = array(
      '#type' => 'submit',
      '#id' => 'edit-views-reset',
      '#value' => t('Reset'),
    );
  }

  $form['#theme'] = array('views_ui_list_views_form');
  return $form;
}

function theme_views_ui_list_views_form($form) {
  // Don't render these:
  unset($form['form_id']);
  unset($form['form_build_id']);
  unset($form['form_token']);
  return drupal_render($form);
}

/**
 * Page callback for the live preview.
 *
 * @todo make this use a template
 */
function views_ui_preview($js, $view) {
  // Take off the items we know so that we can have just the args passed
  // in for later use.
  $func_args = func_get_args();
  array_shift($func_args); // $js
  array_shift($func_args); // $view
  $display_id = (count($func_args)) ? array_shift($func_args) : 'default';

  $form_state = array(
    'display_id' => $display_id,
    'view_args' => $func_args ? implode('/', $func_args) : '',
    'rerender' => TRUE,
    'no_redirect' => TRUE,
    'view' => &$view,
    'ajax' => $js
  );

  $output = drupal_build_form('views_ui_preview_form', $form_state);
  $args = array();
  if (isset($form_state['view_args']) && $form_state['view_args'] !== '') {
    $args = explode('/', $form_state['view_args']);
  }

  $errors = $view->validate();
  if ($errors === TRUE) {
    $view->ajax = $js;
    $view->live_preview = TRUE;

    // Store the current view URL for later use:
    $view->set_display($form_state['display_id']);
    $view->set_arguments($args);

    if ($view->display_handler->get_option('path')) {
      $path = $view->get_url();
    }

    // Make view links come back to preview.
    $view->override_path = 'admin/build/views/nojs/preview/' . $view->name . '/' . $form_state['display_id'];

    // also override $_GET['q'] so we get the pager
    $_GET['q'] = $view->override_path;
    if ($form_state['view_args']) {
      $_GET['q'] .= '/' . $form_state['view_args'];
    }

    $preview = $view->preview($form_state['display_id'], $args);

    // Get information from the preview for display.
    if (!empty($view->build_info['query'])) {
      $rows = array();
      $query = db_prefix_tables($view->build_info['query']);
      if ($view->build_info['query_args']) {
        _db_query_callback($view->build_info['query_args'], TRUE);
        $query = preg_replace_callback(DB_QUERY_REGEXP, '_db_query_callback', $query);
      }
      $rows[] = array('<strong>' . t('Query') . '</strong>', '<pre>' . check_plain($query) . '</pre>');
      if (!empty($view->additional_queries)) {
        $queries = '<strong>' . t('These queries were run during view rendering:') . '</strong>';
        foreach ($view->additional_queries as $query) {
          if ($queries) {
            $queries .= "\n";
          }
          $queries .= t('[@time ms]', array('@time' => intval($query[1] * 100000) / 100)) . ' ' . check_plain($query[0]);
        }

        $rows[] = array('<strong>' . t('Other queries') . '</strong>', '<pre>' . $queries . '</pre>');
      }

      $rows[] = array('<strong>' . t('Title') . '</strong>', filter_xss_admin($view->get_title()));
      if (isset($path)) {
        $path = l($path, $path);
      }
      else {
        $path = t('This display has no path.');
      }

      $rows[] = array('<strong>' . t('Path') . '</strong>', $path);

      $rows[] = array('<strong>' . t('Query build time') . '</strong>', t('@time ms', array('@time' => intval($view->build_time * 100000) / 100)));
      $rows[] = array('<strong>' . t('Query execute time') . '</strong>', t('@time ms', array('@time' => intval($view->execute_time * 100000) / 100)));
      $rows[] = array('<strong>' . t('View render time') . '</strong>', t('@time ms', array('@time' => intval($view->render_time * 100000) / 100)));
      drupal_alter('views_preview_info', $rows, $view);

      $info = theme('table', array(), $rows);
    }
    else {
      $info = theme('table', array(), array(array('<strong>' . t('Query') . '</strong>', t('No query was run'))));
    }
  }
  else {
    foreach ($errors as $error) {
      drupal_set_message($error, 'error');
    }
    $preview = t('Unable to preview due to validation errors.');
    $info = '';
  }

  $info = '<div class="views-query-info">' . $info . '</div>';

  if (variable_get('views_ui_query_on_top', FALSE)) {
    $output .= $info . $preview;
  }
  else {
    $output .= $preview . $info;
  }

  if (!$js) {
    views_add_css('views-admin');
    drupal_set_title($view->get_title());
    return $output;
  }
  else {
    views_include('ajax');
    $object = new stdClass();
    if (!empty($view->js_settings)) {
      $object->js = $view->js_settings;
    }
    $object->display = '';
    if ($messages = theme('status_messages')) {
      $object->display = '<div class="views-messages">' . $messages . '</div>';
    }
    $object->display .= $output;
    $object->title = $view->get_title();
    views_ajax_render($object);
  }
}

/**
 * Form for generating argument information for the live preview.
 */
function views_ui_preview_form(&$form_state) {
  $view = &$form_state['view'];
  $view->init_display();
  $options = array();
  foreach ($view->display as $id => $display) {
    $options[$id] = $display->display_title;
  }

  $form['#attributes'] = array(
    'class' => 'clear-block',
  );

  $form['display_id'] = array(
    '#type' => 'select',
    '#title' => t('Display'),
    '#options' => $options,
    '#default_value' => $form_state['display_id'],
    '#id' => 'preview-display-id',
  );

  $form['args'] = array(
    '#type' => 'textfield',
    '#title' => t('Arguments'),
    '#default_value' => $form_state['view_args'],
    '#description' => t('Separate arguments with a / as though they were a URL path.'),
    '#id' => 'preview-args',
  );

  $form['preview'] = array(
    '#type' => 'submit',
    '#value' => t('Preview'),
    '#id' => 'preview-submit',
  );


  $form['live_preview'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatic live preview'),
    '#default_value' => !variable_get('views_ui_disable_live_preview', 0),
  );

  $form['#action'] = url("admin/build/views/nojs/preview/$view->name");
  return $form;
}

/**
 * Submit the preview form.
 *
 * This just takes the data and stores it on the form state in a
 * known location. The caller will be responsible for using it.
 */
function views_ui_preview_form_submit(&$form, &$form_state) {
  $form_state['display_id'] = $form_state['values']['display_id'];
  $form_state['view_args'] = $form_state['values']['args'];
}

/**
 * Page callback to add a new view.
 */
function views_ui_add_page() {
  $form_state = array(
    'view' => NULL
  );

  return drupal_build_form('views_ui_add_form', $form_state);
}

/**
 * Page callback to add a new view.
 */
function views_ui_clone_page($view) {
  $form_state = array(
    'view' => $view->copy(),
  );

  drupal_set_title(t('Clone view %view', array('%view' => $view->name)));
  return drupal_build_form('views_ui_add_form', $form_state);
}

/**
 * Form constructor callback to create the views Add Form, phase 1.
 */
function views_ui_add_form(&$form_state) {
  $view = $form_state['view'];
  $form = array();

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('View name'),
    '#description' => t('This is the unique name of the view. It must contain only alphanumeric characters and underscores; it is used to identify the view internally and to generate unique theming template names for this view. If overriding a module provided view, the name must not be changed or instead a new view will be created.'),
    '#required' => TRUE,
    '#maxlength' => 32,
    '#default_value' => $view ? $view->name : '',
    '#attributes' => array('dir'=>'ltr'),
  );

  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('View description'),
    '#description' => t('This description will appear on the Views administrative UI to tell you what the view is about.'),
    '#default_value' => $view ? $view->description : '',
  );

  $form['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('View tag'),
    '#description' => t('Enter an optional tag for this view; it is used only to help sort views on the administrative page.'),
    '#default_value' => $view ? $view->tag : '',
    '#autocomplete_path' => 'admin/views/ajax/autocomplete/tag',
  );

  $base_tables = array();
  foreach (views_fetch_base_tables() as $table => $info) {
    $base_tables[$table] = $info['title'] . '<div class="description">' . $info['description'] . '</div>';
  }

  $form['base_table'] = array(
    '#type' => 'radios',
    '#title' => t('View type'),
    '#description' => t('The view type is the primary table for which information is being retrieved. The view type controls what arguments, fields, sort criteria and filters are available, so once this is set it <strong>cannot be changed</strong>.'),
    '#default_value' => $view ? $view->base_table : 'node',
    '#options' => $base_tables,
  );

  if ($view) {
    $form['base_table']['#disabled'] = TRUE;
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
    '#validate' => array('views_ui_add_form_validate'),
    '#submit' => array('views_ui_add_form_submit'),
  );

  return $form;
}

/**
 * Validate the add view form.
 */
function views_ui_add_form_validate($form, &$form_state) {
  $name = $form_state['values']['name'];

  // View name must be alphanumeric or underscores, no other punctuation.
  if (preg_match('/[^a-zA-Z0-9_]/', $name)) {
    form_error($form['name'], t('View name must be alphanumeric or underscores only.'));
  }

  // View name must already exist.
  $view = views_get_view($form_state['values']['name']);
  if ($view && $view->type != t('Default')) {
    form_error($form['name'], t('You must use a unique name for this view.'));
  }
}

/**
 * Process the add view form
 */
function views_ui_add_form_submit($form, &$form_state) {
  $view = $form_state['view'] ? $form_state['view'] : views_new_view();
  $view->name = $form_state['values']['name'];
  $view->description = $form_state['values']['description'];
  $view->tag = $form_state['values']['tag'];
  if (empty($form['base_table']['#disabled'])) {
    $view->base_table = $form_state['values']['base_table'];
  }

  views_ui_cache_set($view);
  $form_state['redirect'] ='admin/build/views/edit/' . $view->name;
}

/**
 * Page to delete a view.
 */
function views_ui_delete_confirm(&$form_state, $view) {
  $form_state['view'] = &$view;
  $form = array();

  $cancel = 'admin/build/views';
  if (!empty($_REQUEST['cancel'])) {
    $cancel = $_REQUEST['cancel'];
  }

  if ($view->type == t('Overridden')) {
    $title = t('Are you sure you want to revert the view %name?', array('%name' => $view->name));
    $desc = t('Reverting the view will delete the view that is in the database, reverting it to the original default view. Any changes you have made will be lost and cannot be recovered.');
    $button = t('Revert');
  }
  else {
    $title = t('Are you sure you want to delete the view %name?', array('%name' => $view->name));
    $desc = t('Deleting a view cannot be undone.');
    $button = t('Delete');
  }

  return confirm_form($form,
                  $title,
                  $cancel,
                  $desc,
                  $button,
                  t('Cancel'));
}

/**
 * Submit handler to delete a view.
 */
function views_ui_delete_confirm_submit(&$form, &$form_state) {
  $form_state['view']->delete();
  views_object_cache_clear('view', $form_state['view']->name);
  drupal_set_message(t('The view has been deleted.'));
  $form_state['redirect'] = 'admin/build/views';
}

/**
 * Page to delete a view.
 */
function views_ui_break_lock_confirm(&$form_state, $view) {
  $form_state['view'] = &$view;
  $form = array();

  if (empty($view->locked)) {
    return t('There is no lock on view %view to break.', array('%name' => $view->name));
  }

  $cancel = 'admin/build/views/edit/' . $view->name;
  if (!empty($_REQUEST['cancel'])) {
    $cancel = $_REQUEST['cancel'];
  }

  $account = user_load($view->locked->uid);
  return confirm_form($form,
                  t('Are you sure you want to break the lock on view %name?',
                  array('%name' => $view->name)),
                  $cancel,
                  t('By breaking this lock, any unsaved changes made by !user will be lost!', array('!user' => theme('username', $account))),
                  t('Break lock'),
                  t('Cancel'));
}

/**
 * Submit handler to break_lock a view.
 */
function views_ui_break_lock_confirm_submit(&$form, &$form_state) {
  db_query("DELETE FROM {views_object_cache} WHERE obj = 'view' AND name = '%s'", $form_state['view']->name);
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
  drupal_set_message(t('The lock has been broken and you may now edit this view.'));
}

/**
 * The main view edit page
 */
function views_ui_edit_page($view) {
  drupal_set_title(t('Edit view %view', array('%view' => $view->name)));
  $output = theme('views_ui_edit_view', $view);
  views_ui_check_advanced_help();
  return $output;
}

/**
 * Export a view for cut & paste.
 */
function views_ui_export_page(&$form_state, $view) {
  $code = $view->export();
  $lines = substr_count($code, "\n");
  $form['code'] = array(
    '#type' => 'textarea',
    '#title' => $view->name,
    '#default_value' => $code,
    '#rows' => $lines,
  );
  return $form;
}

/**
 * Import a view from cut & paste
 */
function views_ui_import_page(&$form_state) {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('View name'),
    '#description' => t('Enter the name to use for this view if it is different from the source view. Leave blank to use the name of the view.'),
  );

  $form['view'] = array(
    '#type' => 'textarea',
    '#title' => t('Paste view code here'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => array('views_ui_import_submit'),
    '#validate' => array('views_ui_import_validate'),
  );
  return $form;
}

/**
 * Validate handler to import a view
 */
function views_ui_import_validate($form, &$form_state) {
  $view = '';
  views_include('view');
  ob_start();
  eval($form_state['values']['view']);
  ob_end_clean();

  if (!is_object($view)) {
    return form_error($form['view'], t('Unable to interpret view code.'));
  }

  if (empty($view->api_version) || $view->api_version < 2) {
    // Check for some value that would only exist on a Views 1 view.
    if (isset($view->url) || isset($view->page) || isset($view->block)) {
      views_include('convert');
      $view = views1_import($view);
      drupal_set_message(t('You are importing a view created in Views version 1. You may need to adjust some parameters to work correctly in version 2.'), 'warning');
    }
    else {
      form_error($form['view'], t('That view is not compatible with this version of Views.'));
    }
  }
  elseif ($view->api_version > views_api_version()) {
    form_error($form['view'], t('That view is created for the version @import_version of views, but you only have @api_version', array(
      '@import_version' => $view->api_version,
      '@api_version' => views_api_version())));
  }

  // View name must be alphanumeric or underscores, no other punctuation.
  if (!empty($form_state['values']['name']) && preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['name'])) {
    form_error($form['name'], t('View name must be alphanumeric or underscores only.'));
  }

  if ($form_state['values']['name']) {
    $view->name = $form_state['values']['name'];
  }

  $test = views_get_view($view->name);
  if ($test && $test->type != t('Default')) {
    form_set_error('', t('A view by that name already exists; please choose a different name'));
  }

  $view->init_display();

  $broken = FALSE;
  // Make sure that all plugins and handlers needed by this view actually exist.
  foreach ($view->display as $id => $display) {
    if (empty($display->handler) || !empty($display->handler->broken)) {
      drupal_set_message(t('Display plugin @plugin is not available.', array('@plugin' => $display->display_plugin)), 'error');
      $broken = TRUE;
      continue;
    }

    $plugin = views_get_plugin('style', $display->handler->get_option('style_plugin'));
    if (!$plugin) {
      drupal_set_message(t('Style plugin @plugin is not available.', array('@plugin' => $display->handler->get_option('style_plugin'))), 'error');
      $broken = TRUE;
    }
    else if ($plugin->uses_row_plugin()) {
      $plugin = views_get_plugin('row', $display->handler->get_option('row_plugin'));
      if (!$plugin) {
        drupal_set_message(t('Row plugin @plugin is not available.', array('@plugin' => $display->handler->get_option('row_plugin'))), 'error');
        $broken = TRUE;
      }
    }

    foreach (views_object_types() as $type => $info) {
      $handlers = $display->handler->get_handlers($type);
      if ($handlers) {
        foreach ($handlers as $id => $handler) {
          if ($handler->broken()) {
            drupal_set_message(t('@type handler @table.@field is not available.', array(
              '@type' => $info['stitle'],
              '@table' => $handler->table,
              '@field' => $handler->field,
            )), 'error');
            $broken = TRUE;
          }
        }
      }
    }
  }

  if ($broken) {
    form_set_error('', t('Unable to import view.'));
  }

  $form_state['view'] = &$view;
}

/**
 * Submit handler for view import
 */
function views_ui_import_submit($form, &$form_state) {
  // Store in cache and then go to edit.
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * The main edit view form, which is really just a save/cancel/delete button.
 */
function views_ui_edit_view_form(&$form_state, $view) {
  $form['buttons']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#validate' => array('views_ui_edit_view_form_validate'),
    '#submit' => array('views_ui_edit_view_form_submit'),
  );

  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('views_ui_edit_view_form_cancel'),
  );

  if (is_numeric($view->vid)) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => $view->type == t('Overridden') ? t('Revert') : t('Delete'),
      '#submit' => array('views_ui_edit_view_form_delete'),
    );
  }

  $form_state['view'] = &$view;
  return $form;
}

/**
 * Validate that a view is complete and whole.
 */
function views_ui_edit_view_form_validate($form, &$form_state) {
  // Do not validate cancel or delete or revert.
  if (empty($form_state['clicked_button']['#value']) || $form_state['clicked_button']['#value'] != t('Save')) {
    return;
  }

  $errors = $form_state['view']->validate();
  if ($errors !== TRUE) {
    foreach ($errors as $error) {
      form_set_error('', $error);
    }
  }
}

/**
 * Submit handler for the edit view form.
 */
function views_ui_edit_view_form_submit($form, &$form_state) {
  // Go through and remove displayed scheduled for removal.
  foreach ($form_state['view']->display as $id => $display) {
    if (!empty($display->deleted)) {
      unset($form_state['view']->display[$id]);
    }
  }

  $form_state['view']->save();
  drupal_set_message(t('The view %name has been saved.', array('%name' => $form_state['view']->name)));

  // Make sure menu items get rebuilt as neces
  menu_rebuild();

  // Clear the views cache.
  cache_clear_all('*', 'cache_views');

  // Clear the page cache.
  cache_clear_all();

  // Remove this view from cache so we can edit it properly.
  views_object_cache_clear('view', $form_state['view']->name);
}

/**
 * Submit handler for the edit view form.
 */
function views_ui_edit_view_form_cancel($form, &$form_state) {
  // Remove this view from cache so edits will be lost.
  views_object_cache_clear('view', $form_state['view']->name);
  if (empty($form['view']->vid)) {
    // I seem to have to drupal_goto here because I can't get fapi to
    // honor the redirect target. Not sure what I screwed up here.
    drupal_goto('admin/build/views');
  }
}

function views_ui_edit_view_form_delete($form, &$form_state) {
  unset($_REQUEST['destination']);
  // Redirect to the delete confirm page
  $form_state['redirect'] = array('admin/build/views/delete/' . $form_state['view']->name, 'cancel=admin/build/views/edit/' . $form_state['view']->name);
}

/**
 * Preprocess the view edit page.
 */
function template_preprocess_views_ui_edit_view(&$vars) {
  $view = &$vars['view'];

  $vars['save_button'] = drupal_get_form('views_ui_edit_view_form', $view);

  $table = views_fetch_data($view->base_table);
  $vars['base_table'] = !empty($table['table']['base']['title']) ?
    $table['table']['base']['title'] : t('Unknown or missing table name');

  views_include('tabs');
  $tabs = new views_tabset;

  $vars['message'] = '<div class="message">' . t("Click on an item to edit that item's details.") . '</div>';

  if (!$view->set_display('default')) {
    drupal_set_message(t('This view has a broken default display and cannot be used.'), 'error');
  }

  foreach ($view->display as $display) {
    list($title, $body) = views_ui_display_tab($view, $display);
    // The first display is the default.
    $tabs->set($display->id, $title, $body);
  }

  // This is the area that will render beneath the links
  $form_state = array(
    'view' => &$view,
    'ajax' => FALSE,
  );

  $display_button = drupal_build_form('views_ui_add_display_form', $form_state);
  $analyze_button = drupal_get_form('views_ui_analyze_view_button', $view);
  $tabs->add_extra($display_button . $analyze_button);

  $vars['tabs'] = $tabs->render();

  $form_state = array(
    'display_id' => 'default',
    'view_args' => '',
    'rerender' => FALSE,
    'no_redirect' => TRUE,
    'view' => &$view,
    'input' => array(),
  );
  $vars['preview'] = drupal_build_form('views_ui_preview_form', $form_state);

  $vars['locked'] = NULL;
  if (isset($view->locked) && is_object($view->locked)) {
    $account = user_load($view->locked->uid);
    $vars['locked'] = theme('username', $account);
    $vars['lock_age'] = format_interval(time() - $view->locked->updated);
    $vars['break'] = url('admin/build/views/break-lock/' . $view->name);
  }

  $vars['quick_links_raw'] = array(
    array(
      'title' => t('Export'),
      'alt' => t("Export this view"),
      'href' => "admin/build/views/export/$view->name",
    ),
    array(
      'title' => t('Clone'),
      'alt' => t("Create a copy of this view"),
      'href' => "admin/build/views/clone/$view->name",
    ),
  );

  $paths = array();
  foreach ($view->display as $id => $display) {
    if (!empty($display->handler) && $display->handler->has_path()) {
      $path = $display->handler->get_path();
      if (strpos($path, '%') === FALSE && !isset($paths[$path])) {
        $vars['quick_links_raw'][] = array(
          'title' => t('View "@display"', array('@display' => $display->display_title)),
          'alt' => t("Go to the real page for this display"),
          'href' => $path,
        );
        // Displays can have the same path; no point in showing more than one link.
        $paths[$path] = TRUE;
      }
    }
  }

  $vars['quick_links'] = theme('links', $vars['quick_links_raw']);
  views_add_css('views-admin');
  views_add_js('ajax');
  drupal_add_js('misc/jquery.form.js');

  // Also add any js files required by plugins:
  $plugins = views_fetch_plugin_data();
  foreach ($plugins as $type => $type_plugins) {
    foreach ($type_plugins as $name => $plugin) {
      if (!empty($plugin['js'])) {
        foreach ($plugin['js'] as $file) {
          drupal_add_js($file);
        }
      }
    }
  }

  $settings = array('views' => array('ajax' => array(
    'id' => '#views-ajax-pad',
    'title' => '#views-ajax-title',
    'defaultForm' => $vars['message'],
  )));

  drupal_add_js($settings, 'setting');
}

function template_preprocess_views_ui_edit_tab(&$vars) {
  $view = $vars['view'];
  $display = $vars['display'];
  $plugin = $display->handler->definition;

  $top = $left = $middle = $right = '';

  // If this form was submitted it was already handled, so force it not to
  // submit again.

  $vars['remove'] = '';
  if (empty($plugin['no remove'])) {
    if (!empty($_POST['form_id']) && $_POST['form_id'] == 'views_ui_remove_display_form') {
      unset($_POST['form_id']);
    }
    $form_state = array('view' => &$view, 'display_id' => $display->id, 'ajax' => FALSE);
    $vars['remove'] = drupal_build_form('views_ui_remove_display_form', $form_state);
  }

  // basic fields
  $vars['title'] = check_plain($display->display_title);
  $vars['description'] = check_plain($plugin['help']);

  // Special fields if tihs is the default display.
  $vars['default'] = ($display->id == 'default');
  $vars['details_class'] = views_ui_item_css('details');
  if (!empty($view->changed_sections['details'])) {
    $vars['details_changed'] = TRUE;
  }

  $tag = empty($view->tag) ? t('None') : $view->tag;
  $vars['details'] = t('Description') . '/' . t('Tag') . ': ' . l($tag, "admin/build/views/nojs/details/$view->name", array('attributes' => array('class' => 'views-ajax-link')));

  // Calculate options from display plugin.
  $options = $categories = array();
  $display->handler->options_summary($categories, $options);

  // Build all of the options we were returned and put them into the
  // category data fields.
  foreach ($options as $id => $option) {
    if (empty($categories[$option['category']]['data'])) {
      $categories[$option['category']]['data'] = array();
    }
    $categories[$option['category']]['data'][$id] = array();
    $data = &$categories[$option['category']]['data'][$id];
    $data['content'] = '';
    $data['links'] = '';
    $data['overridden'] = FALSE;
    $data['defaulted'] = FALSE;

    // If there are optional links, build them first so they float properly.
    if (!empty($option['links'])) {
      foreach ($option['links'] as $link_id => $link_value) {
        $data['links'] .= $display->handler->option_link($link_value, $link_id, 'views-button-configure');
      }
    }
    if (!empty($option['title'])) {
      $data['content'] .= $option['title'] . ': ';
    }

    $data['content'] .= $display->handler->option_link($option['value'], $id, '', empty($option['desc']) ? '' : $option['desc']);
    if (!empty($display->handler->options['defaults'][$id])) {
      $display_id = 'default';
      $data['defaulted'] = TRUE;
    }
    else {
      $display_id = $display->id;
      if (!$display->handler->is_default_display()) {
        if ($display->handler->defaultable_sections($id)) {
          $data['overridden'] = TRUE;
        }
      }
    }
    $data['class'] = views_ui_item_css($display_id . '-' . $id);
    if (!empty($view->changed_sections[$display_id . '-' . $id])) {
      $data['changed'] = TRUE;
    }
  }

  $vars['categories'] = $categories;

  // Add a help icon
  if (isset($plugin['help topic'])) {
    $vars['display_help_icon'] = theme('advanced_help_topic', $plugin['module'], $plugin['help topic']);
  }
  else {
    $vars['display_help_icon'] = '';
  }

  // Fetch style plugin info because it has some effect on how/what we render.
  $style_plugin = $display->handler->get_plugin();

  $vars['fields'] = '';
  $vars['fields'] = theme('views_ui_edit_item', 'field', $view, $display, !($style_plugin && $style_plugin->uses_fields()));
  $vars['relationships'] = theme('views_ui_edit_item', 'relationship', $view, $display);
  $vars['arguments'] = theme('views_ui_edit_item', 'argument', $view, $display);
  $vars['filters'] = theme('views_ui_edit_item', 'filter', $view, $display);
  $vars['sorts'] = theme('views_ui_edit_item', 'sort', $view, $display);
}

/**
 * Generate the summary output for a single display to render in a tab.
 */
function views_ui_display_tab($view, $display) {
  if (isset($display->handler)) {
    $plugin = $display->handler->definition;
  }
  if (empty($plugin)) {
    $title = isset($display->display_title) ? $display->display_title : t('Invalid');
    return array($title, t("Error: Display @display refers to a plugin named '@plugin', but that plugin doesn't exist!", array('@display' => $display->id, '@plugin' => $display->display_plugin)));

    // @todo We can do a better 'plugin does not exist' tab.
  }

  // The display should always be initialized prior to this call.
  if (empty($display->handler)) {
    return FALSE;
  }

  $body = theme('views_ui_edit_tab', $view, $display);
  return array($display->display_title, $body);
}

/**
 * Add information about a section to a display.
 */
function template_preprocess_views_ui_edit_item(&$vars) {
  $type = $vars['type'];
  $view = $vars['view'];
  $display = $vars['display'];

  $types = views_object_types();

  $vars['overridden'] = FALSE;
  $vars['defaulted'] = FALSE;

  if ($vars['no_fields']) {
    $vars['title'] = $types[$type]['title'];
    $vars['item_help_icon'] = theme('advanced_help_topic', 'views', $type);
    $vars['rearrange'] = NULL;
    $vars['add'] = NULL;
    return;
  }

  $vars['rearrange'] = l('<span>' . t('Rearrange') . '</span>', "admin/build/views/nojs/rearrange/$view->name/$display->id/$type", array('attributes' => array('class' => 'views-button-rearrange views-ajax-link', 'title' => t('Rearrange')), 'html' => true));

  $vars['add'] = l('<span>' . t('Add') . '</span>', "admin/build/views/nojs/add-item/$view->name/$display->id/$type", array('attributes' => array('class' => 'views-button-add views-ajax-link', 'title' => t('Add')), 'html' => true));

  if (!$display->handler->is_default_display()) {
    if (!$display->handler->is_defaulted($types[$type]['plural'])) {
      $vars['overridden'] = TRUE;
    }
    else {
      $vars['defaulted'] = TRUE;
    }
  }

  $vars['title'] = l($types[$type]['title'], "admin/build/views/nojs/config-type/$view->name/$display->id/$type", array('attributes' => array('class' => 'views-ajax-link')));
//  $vars['title'] = l($types[$type]['title'], "admin/build/views/nojs/config-type/$view->name/$display->id/$type", array('attributes' => array('class' => 'views-ajax-link')));

  $fields = array();

  static $relationships = NULL;
  if (!isset($relationships)) {
    // Get relationship labels
    $relationships = array();
    // @todo: get_handlers()
    foreach ($display->handler->get_option('relationships') as $id => $relationship) {
      $handler = views_get_handler($relationship['table'], $relationship['field'], 'relationship');
      if (empty($handler)) {
        continue;
      }
      $handler->init($view, $relationship);
      $relationships[$id] = $handler->label();
    }
  }

  // @todo: get_handlers()
  foreach ($display->handler->get_option($types[$type]['plural']) as $id => $field) {
    $fields[$id] = array();

    $handler = views_get_handler($field['table'], $field['field'], $type);
    if (empty($handler)) {
      $fields[$id]['class'] = 'broken';
      $field_name = t('Broken/missing handler: @table > @field', array('@table' => $field['table'], '@field' => $field['field']));
      $fields[$id]['title'] = l($field_name, "admin/build/views/nojs/config-item/$view->name/$display->id/$type/$id", array('attributes' => array('class' => 'views-ajax-link'), 'html' => TRUE));
      $fields[$id]['info'] = '';
      continue;
    }
    $handler->init($view, $field);

    $field_name = $handler->ui_name(TRUE);
    if (!empty($field['relationship']) && !empty($relationships[$field['relationship']])) {
      $field_name = '(' . $relationships[$field['relationship']] . ') ' . $field_name;
    }

    $fields[$id]['title'] = l($field_name, "admin/build/views/nojs/config-item/$view->name/$display->id/$type/$id", array('attributes' => array('class' => 'views-ajax-link'), 'html' => TRUE));
    $fields[$id]['class'] = views_ui_item_css($display->id . '-' . $type . '-' . $id);
    if (!empty($view->changed_sections[$display->id . '-' . $type . '-' . $id])) {
      $fields[$id]['changed'] = TRUE;
    }
    $fields[$id]['info'] = $handler->admin_summary();

    if ($handler->has_extra_options()) {
      $fields[$id]['links'] = l('<span>' . t('Settings') . '</span>', "admin/build/views/nojs/config-item-extra/$view->name/$display->id/$type/$id", array('attributes' => array('class' => 'views-button-configure views-ajax-link', 'title' => t('Settings')), 'html' => true));
    }

    if ($handler->needs_style_plugin()) {
      $style_plugin = views_fetch_plugin_data('style', $handler->options['style_plugin']);
      $style_title = empty($style_plugin['title']) ? t('Missing style plugin') : $style_plugin['title'];
      $pid = $id . '-style-plugin';

      if (!empty($style_plugin['uses options'])) {
        $fields[$pid]['links'] = l('<span>' . t('Change settings for this style') . '</span>', "admin/build/views/nojs/config-style/$view->name/$display->id/$type/$id", array('attributes' => array('class' => 'views-button-configure views-ajax-link', 'title' => t('Settings')), 'html' => true));
      }

      $fields[$pid]['title'] = ' ' . t('&nbsp; Style: !style', array('!style' => l($style_title, "admin/build/views/nojs/change-style/$view->name/$display->id/$type/$id", array('attributes' => array('class' => 'views-ajax-link')))));
      $fields[$pid]['class'] = views_ui_item_css($display->id . '-' . $type . '-' . $pid);
      if (!empty($view->changed_sections[$display->id . '-' . $type . '-' . $pid])) {
        $fields[$pid]['changed'] = TRUE;
      }
      $fields[$pid]['info'] = '';
    }
  }

  $vars['fields'] = $fields;
  $vars['item_help_icon'] = theme('advanced_help_topic', 'views', $type);
}

/**
 * Regenerate the tabs for AJAX updates.
 */
function views_ui_regenerate_tabs(&$view, $display_id = NULL, $object = NULL) {
  if (empty($display_id)) {
    $displays = array_keys($view->display);
  }
  elseif (!is_array($display_id)) {
    $displays = array($display_id);
    if ($display_id != 'default') {
      $displays[] = 'default';
    }
  }
  else {
    $displays = $display_id;
  }

  if (!$view->set_display('default')) {
    views_ajax_render(t('Invalid display id found while regenerating tabs'));
  }

  if (!is_object($object)) {
    $object = new stdClass();
  }

  $object->replace = array();
  foreach ($displays as $id) {
    list($title, $body) = views_ui_display_tab($view, $view->display[$id]);
    $object->replace['#views-tab-' . $id] = $body;
    $object->replace['#views-tab-title-' . $id] = check_plain($title);
  }

  if (!empty($view->changed)) {
    $object->changed = TRUE;
  }

  views_ajax_render($object);
}

/**
 * Provide standard buttons for the forms to make it easy. Also provide
 * a hidden op operator because the forms plugin doesn't seem to properly
 * provide which button was clicked.
 */
function views_ui_standard_form_buttons(&$form, &$form_state, $form_id, $name = NULL, $third = NULL, $submit = NULL) {
  $form['buttons'] = array(
    '#prefix' => '<div class="clear-block"><div class="form-buttons">',
    '#suffix' => '</div></div>',
  );

  if (empty($name)) {
    $name = t('Update');
  }
  // remove default validate handler
  $form['#validate'] = array();

  if (empty($form_state['ok_button'])) {
    // but be sure submit button validates!
    $form['buttons']['submit'] = array(
      '#type' => 'submit',
      '#value' => $name,
      '#submit' => array($form_id . '_submit'),
      '#validate' => array('views_ui_standard_submit', $form_id . '_validate'),
    );
  }

  $cancel_submit = function_exists($form_id . '_cancel') ? $form_id . '_cancel' : 'views_ui_standard_cancel';
  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => empty($form_state['ok_button']) ? t('Cancel') : t('Ok'),
    '#submit' => array($cancel_submit),
    '#validate' => array(),
  );

  if ($third) {
    if (empty($submit)) {
      $submit = 'third';
    }
    $third_submit = function_exists($form_id . '_' . $submit) ? $form_id . '_' . $submit : 'views_ui_standard_cancel';

    $form['buttons'][$submit] = array(
      '#type' => 'submit',
      '#value' => $third,
      '#validate' => array(),
      '#submit' => array($third_submit),
    );
  }

  // Compatibility, to be removed later:
  // We used to set these items on the form, but now we want them on the $form_state:
  if (isset($form['#title'])) {
    $form_state['title'] = $form['#title'];
  }
  if (isset($form['#help_topic'])) {
    $form_state['help_topic'] = $form['#help_topic'];
  }
  if (isset($form['#help_module'])) {
    $form_state['help_module'] = $form['#help_module'];
  }
  if (isset($form['#url'])) {
    $form_state['url'] = $form['#url'];
  }
  if (isset($form['#js'])) {
    if (!empty($form_state['js settings']) && is_array($form_state['js settings'])) {
      $form_state['js settings'] = array_merge($form_state['js settings'], $form['#js']);
    }
    else {
      $form_state['js settings'] = $form['#js'];
    }
  }
  if (isset($form['#section'])) {
    $form_state['#section'] = $form['#section'];
  }
  // Finally, we never want these cached -- our object cache does that for us.
  $form['#no_cache'] = TRUE;

  // If this isn't an ajaxy form, then we want to set the title.
  if (!empty($form['#title'])) {
    drupal_set_title($form['#title']);
  }
  views_add_css('views-admin');
}

/**
 * Basic submit handler applicable to all 'standard' forms
 */
function views_ui_standard_submit($form, &$form_state) {
  if (!empty($form['#section'])) {
    $form_state['view']->changed_sections[$form['#section']] = TRUE;
  }
}

/**
 * Submit handler for cancel button
 */
function views_ui_standard_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

// --------------------------------------------------------------------------
// Various subforms for editing the pieces of a view.

function views_ui_ajax_forms($key = NULL) {
  $forms = array(
    'display' => array(
      'form_id' => 'views_ui_edit_display_form',
      'args' => array('section'),
    ),
    'remove-display' => array(
      'form_id' => 'views_ui_remove_display_form',
      'args' => array(),
    ),
    'config-type' => array(
      'form_id' => 'views_ui_config_type_form',
      'args' => array('type'),
    ),
    'rearrange' => array(
      'form_id' => 'views_ui_rearrange_form',
      'args' => array('type'),
    ),
    'add-item' => array(
      'form_id' => 'views_ui_add_item_form',
      'args' => array('type'),
    ),
    'config-item' => array(
      'form_id' => 'views_ui_config_item_form',
      'args' => array('type', 'id'),
    ),
    'config-item-extra' => array(
      'form_id' => 'views_ui_config_item_extra_form',
      'args' => array('type', 'id'),
    ),
    'change-style' => array(
      'form_id' => 'views_ui_change_style_form',
      'args' => array('type', 'id'),
    ),
    'config-style' => array(
      'form_id' => 'views_ui_config_style_form',
      'args' => array('type', 'id'),
    ),
  );

  if ($key) {
    return !empty($forms[$key]) ? $forms[$key] : NULL;
  }

  return $forms;
}

/**
 * Build a form identifier that we can use to see if one form
 * is the same as another. Since the arguments differ slightly
 * we do a lot of spiffy concenation here.
 */
function views_ui_build_identifier($key, $view, $display_id, $args) {
  $form = views_ui_ajax_forms($key);
  $identifier = implode('-', array($key, $view->name, $display_id));

  foreach ($form['args'] as $id) {
    $arg = (!empty($args)) ? array_shift($args) : NULL;
    $identifier .= '-' . $arg;
  }
  return $identifier;
}

/**
 * Build up a $form_state object suitable for use with drupal_build_form
 * based on known information about a form.
 */
function views_ui_build_form_state($js, $key, &$view, $display_id, $args) {
  $form = views_ui_ajax_forms($key);
  // Build up form state
  $form_state = array(
    'form_key' => $key,
    'form_id' => $form['form_id'],
    'view' => &$view,
    'ajax' => $js,
    'display_id' => $display_id,
    'no_redirect' => TRUE,
  );

  foreach ($form['args'] as $id) {
    $form_state[$id] = (!empty($args)) ? array_shift($args) : NULL;
  }

  return $form_state;
}

/**
 * Create the URL for one of our standard AJAX forms based upon known
 * information about the form.
 */
function views_ui_build_form_url($form_state) {
  $form = views_ui_ajax_forms($form_state['form_key']);
  $ajax = empty($form_state['ajax']) ? 'nojs' : 'ajax';
  $name = $form_state['view']->name;
  $url = "admin/build/views/$ajax/$form_state[form_key]/$name/$form_state[display_id]";
  foreach ($form['args'] as $arg) {
    $url .= '/' . $form_state[$arg];
  }
  return $url;
}

/**
 * Add another form to the stack; clicking 'update' will go to this form
 * rather than closing the ajax pad.
 */
function views_ui_add_form_to_stack($key, &$view, $display_id, $args, $top = FALSE) {
  if (empty($view->stack)) {
    $view->stack = array();
  }

  $stack = array(views_ui_build_identifier($key, $view, $display_id, $args), $key, &$view, $display_id, $args);
  if ($top) {
    array_unshift($view->stack, $stack);
  }
  else {
    $view->stack[] = $stack;
  }
}

/**
 * Generic entry point to handle forms.
 *
 * We do this for consistency and to make it easy to chain forms
 * together. This only works for forms that use both $view
 * and $display_id, so we have a couple of ajax forms that we don't
 * use with this system.
 */
function views_ui_ajax_form($js, $key, $view, $display_id) {
  $form = views_ui_ajax_forms($key);
  if (empty($form)) {
    return drupal_not_found();
  }

  views_include('ajax');
  $args = func_get_args();
  // Remove the known args
  array_splice($args, 0, 4);

  $form_state = views_ui_build_form_state($js, $key, $view, $display_id, $args);
  // check to see if this is the top form of the stack. If it is, pop
  // it off; if it isn't, the user clicked somewhere else and the stack is
  // now irrelevant.
  if (!empty($view->stack)) {
    $identifier = views_ui_build_identifier($key, $view, $display_id, $args);
    $top = array_shift($view->stack);
    if (array_shift($top) != $identifier) {
      $view->stack = array();
    }
  }

  $output = views_ajax_form_wrapper($form_state['form_id'], $form_state);

  if (!$output) {
    // Sometimes we need to re-generate the form for multi-step type operations.
    $object = NULL;
    if (!empty($view->stack)) {
      $stack = $view->stack; // copy so the next shift doesn't break the array
      $top = array_shift($stack);
      $top[0] = $js; // change identifier into $js setting
      $form_state = call_user_func_array('views_ui_build_form_state', $top);
      $form_state['input'] = array(); // this is a new form, make sure it
      // doesn't try to inherit $_POST info.
      if (!$js) {
        return drupal_goto(views_ui_build_form_url($form_state));
      }
      $object = views_ajax_form_wrapper($form_state['form_id'], $form_state);
      $object->url = url(views_ui_build_form_url($form_state));
    }
    else if (!$js) {
      // if nothing on the stack, non-js forms just go back to the main view editor.
      return drupal_goto("admin/build/views/edit/$view->name");
    }
    // regenerate all tabs because changes to the default tab could ripple.
    return views_ui_regenerate_tabs($view, NULL, $object);
  }

  return ($js) ? views_ajax_render($output) : $output;
}

/**
 * AJAX callback to add a display.
 */
function views_ui_add_display($js, $view) {
  views_include('ajax');
  $form_state = array(
    'view' => &$view,
    'ajax' => $js,
  );

  $output = views_ajax_form_wrapper('views_ui_add_display_form', $form_state);

  if ($js) {
    // If we don't have an output object, it was submitted. Set up the submission.
    if (empty($output)) {
      $id = $form_state['id'];

      // Make sure the new display is active
      if (!$view->set_display('default')) {
        views_ajax_render(t('Unable to initialize default display'));
      }

      // Render the new display
      list($title, $body) = views_ui_display_tab($view, $view->display[$id]);

      // Instruct the javascript on the browser to render the new tab.
      $output = new stdClass;
      $output->tab = array('#views-tab-' . $id => array('title' => $title, 'body' => $body));
    }
    // Render the command object. This automatically exits.
    views_ajax_render($output);
  }

  // But the non-js variant will return output if it didn't redirect us.
  return $output;
}

/**
 * Form to add a display to a view.
 */
function views_ui_add_display_form(&$form_state) {
  $view = &$form_state['view'];

  $form['display']['display'] = array(
    '#type' => 'select',
    '#options' => views_fetch_plugin_names('display'),
    '#default_value' => 'page',
  );

  $form['display']['add_display'] = array(
    '#type' => 'submit',
    '#value' => t('Add display'),
    '#submit' => array('views_ui_add_display_form_submit'),
  );

  $form['#id'] = 'views-add-display-form';
  $form['#attributes'] = array('class' => 'views-ajax-form');
  $form['#action'] = url("admin/build/views/nojs/add-display/$view->name");

  return $form;
}

/**
 * Submit handler to add a display to a view.
 */
function views_ui_add_display_form_submit($form, &$form_state) {
  // Create the new display
  $plugin = $form_state['values']['display'];
  $form_state['id'] = $form_state['view']->add_display($plugin);

  // Store in cache
  views_ui_cache_set($form_state['view']);

  // Send it back
  $form_state['redirect'] = array('admin/build/views/edit/' . $form_state['view']->name, NULL, 'views-tab-' . $form_state['id']);
}

/**
 * Form to remove a display from a view.
 */
function views_ui_remove_display_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];

  if (empty($view->display[$display_id]->deleted)) {
    $form['display'] = array(
      '#prefix' => '<div class="display-button remove-display">',
      '#suffix' => '</div>',
    );
    $form['remove_display'] = array(
      '#type' => 'submit',
      '#value' => t('Remove display'),
      '#submit' => array('views_ui_remove_display_form_submit'),
    );
  }
  else {
    $form['display'] = array(
      '#prefix' => '<div class="display-button restore-display">',
      '#suffix' => '</div>',
    );
    $form['restore_display'] = array(
      '#type' => 'submit',
      '#value' => t('Restore display'),
      '#submit' => array('views_ui_remove_display_form_restore'),
    );
  }
  $form['#action'] = url("admin/build/views/nojs/remove-display/$view->name/$display_id");
  $form['#attributes'] = array('class' => 'views-ajax-form');

  return $form;
}

/**
 * Submit handler to add a remove to a display from a view.
 */
function views_ui_remove_display_form_submit($form, &$form_state) {
  // Create the new display
  $plugin = views_fetch_plugin_data('display', $form_state['view']->display[$form_state['display_id']]->display_plugin);
  if (empty($plugin['no remove'])) {
    $id = $form_state['display_id'];
    $form_state['view']->display[$id]->deleted = TRUE;

    // Store in cache
    views_ui_cache_set($form_state['view']);
  }
}

/**
 * Submit handler to add a restore a removed display to a view.
 */
function views_ui_remove_display_form_restore($form, &$form_state) {
  // Create the new display
  $id = $form_state['display_id'];
  $form_state['view']->display[$id]->deleted = FALSE;

  // Store in cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Page callback to display analysis information on a view.
 */
function views_ui_analyze_view($js, $view) {
  views_include('ajax');
  $form_state = array(
    'view' => &$view,
    'ajax' => $js,
  );

  $output = views_ajax_form_wrapper('views_ui_analyze_view_form', $form_state);

  if ($js) {
    // If we don't have an output object, it was submitted. Set up the submission.
    if (empty($output)) {
      return views_ui_regenerate_tabs($view);
    }
    return views_ajax_render($output);

  }
  return $output;
}

/**
 * This form doesn't particularly do much; it's really just providing a link
 * but a button seems like it would be nicer here.
 *
 * It has no submit or anything, as we will never actually submit this form
 * where the form is placed.
 */
function views_ui_analyze_view_button(&$form_state, $view) {
  $form['#action'] = url("admin/build/views/nojs/analyze/$view->name");
  $form['#attributes'] = array('class' => 'views-ajax-form');
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Analyze'),
  );

  return $form;
}

/**
 * Form constructor callback to display analysis information on a view
 */
function views_ui_analyze_view_form(&$form_state) {
  $view = &$form_state['view'];

  $form['#title'] = t('View analysis');
  $form['#section'] = 'analyze';

  views_include('analyze');
  $messages = views_analyze_view($view);

  $form['analysis'] = array(
    '#prefix' => '<div class="form-item">',
    '#suffix' => '</div>',
    '#value' => views_analyze_format_result($view, $messages),
  );

  // Inform the standard button function that we want an OK button.
  $form_state['ok_button'] = TRUE;
  views_ui_standard_form_buttons($form, $form_state, 'views_ui_analyze_view_form');
  return $form;
}

/**
 * Submit handler for views_ui_analyze_view_form
 */
function views_ui_analyze_view_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Page callback to edit details of a view.
 */
function views_ui_edit_details($js, $view) {
  views_include('ajax');
  $form_state = array(
    'view' => &$view,
    'ajax' => $js,
  );

  $output = views_ajax_form_wrapper('views_ui_edit_details_form', $form_state);

  if ($js) {
    // If we don't have an output object, it was submitted. Set up the submission.
    if (empty($output)) {
      return views_ui_regenerate_tabs($view);
    }
    return views_ajax_render($output);

  }
  return $output;
}

/**
 * Form constructor callback to edit details of a view
 */
function views_ui_edit_details_form(&$form_state) {
  $view = &$form_state['view'];

  $form['#title'] = t('View details');
  $form['#section'] = 'details';

  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('View description'),
    '#description' => t('This description will appear on the Views administrative UI to tell you what the view is about.'),
    '#default_value' => $view->description,
  );

  $form['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('View tag'),
    '#description' => t('Enter an optional tag for this view; it is used only to help sort views on the administrative page.'),
    '#default_value' => $view->tag,
    '#autocomplete_path' => 'admin/views/ajax/autocomplete/tag',
  );

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_edit_details_form');
  return $form;
}

/**
 * Submit handler for views_ui_edit_details_form
 */
function views_ui_edit_details_form_submit($form, &$form_state) {
  $form_state['view']->description = $form_state['values']['description'];
  $form_state['view']->tag = $form_state['values']['tag'];
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/build/views/edit/' . $form_state['view']->name;
}

/**
 * Form constructor callback to edit display of a view
 */
function views_ui_edit_display_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $section = $form_state['section'];

  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $display = &$view->display[$display_id];

  // Get form from the handler.
  $display->handler->options_form($form, $form_state);
  $name = NULL;
  if (isset($form_state['update_name'])) {
    $name = $form_state['update_name'];
  }

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_edit_display_form', $name);
  return $form;
}

/**
 * Validate handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_validate($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_validate($form, $form_state);
}

/**
 * Submit handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_submit($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_submit($form, $form_state);

  views_ui_cache_set($form_state['view']);
}

/**
 * Override handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_override($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_override($form, $form_state);

  views_ui_cache_set($form_state['view']);
  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
}
/**
 * Form to config items in the views UI.
 */
function views_ui_config_type_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];

  $types = views_object_types();
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $display = &$view->display[$display_id];
  $form['#title'] = check_plain($display->display_title) . ': ';
  $form['#title'] .= t('Configure @type', array('@type' => $types[$type]['ltitle']));
  $form['#section'] = $display_id . 'config-item';

  if ($display->handler->defaultable_sections($types[$type]['plural'])) {
    $form_state['section'] = $types[$type]['plural'];
    $display->handler->add_override_button($form, $form_state, $form_state['section']);
  }

  if (!empty($types[$type]['options']) && function_exists($types[$type]['options'])) {
    $options = $type . '_options';
    $form[$options] = array('#tree' => TRUE);
    $types[$type]['options']($form, $form_state);
  }

  $name = NULL;
  if (isset($form_state['update_name'])) {
    $name = $form_state['update_name'];
  }

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_type_form', $name);
  return $form;
}

/**
 * Submit handler for type configuration form
 */
function views_ui_config_type_form_submit($form, &$form_state) {
  $types = views_object_types();
  $display = &$form_state['view']->display[$form_state['display_id']];

  // Store in cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Configure settings particular to filters.
 */
function views_ui_config_filters_form(&$form, &$form_state) {

}

/**
 * Form to rearrange items in the views UI.
 */
function views_ui_rearrange_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];

  $types = views_object_types();
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $display = &$view->display[$display_id];
  $form['#title'] = check_plain($display->display_title) . ': ';
  $form['#title'] .= t('Rearrange @type', array('@type' => $types[$type]['ltitle']));
  $form['#section'] = $display_id . 'rearrange-item';

  if ($display->handler->defaultable_sections($types[$type]['plural'])) {
    $form_state['section'] = $types[$type]['plural'];
    $display->handler->add_override_button($form, $form_state, $form_state['section']);
  }

  $count = 0;

  // Get relationship labels
  $relationships = array();
  // @todo: get_handlers()
  foreach ($display->handler->get_option('relationships') as $id => $relationship) {
    $handler = views_get_handler($relationship['table'], $relationship['field'], 'relationship');
    if (empty($handler)) {
      continue;
    }
    $handler->init($view, $relationship);
    $relationships[$id] = $handler->label();
  }

  // @todo: get_handlers()
  foreach ($display->handler->get_option($types[$type]['plural']) as $id => $field) {
    $form[$id] = array('#tree' => TRUE);
    $form[$id]['weight'] = array(
      '#type' => 'weight',
      '#delta' => 200,
      '#default_value' => ++$count,
    );
    $handler = views_get_handler($field['table'], $field['field'], $type);
    if ($handler) {
      $handler->init($view, $field);
      $name = $handler->ui_name() . ' ' . $handler->admin_summary();
      if (!empty($field['relationship']) && !empty($relationships[$field['relationship']])) {
        $name = '(' . $relationships[$field['relationship']] . ') ' . $name;
      }

      $form[$id]['name'] = array(
        '#value' => $name,
      );
    }
    else {
      $form[$id]['name'] = array('#value' => t('Broken field @id', array('@id' => $id)));
    }
    $form[$id]['removed'] = array(
      '#type' => 'checkbox',
      '#id' => 'views-removed-' . $id,
      '#attributes' => array('class' => 'views-remove-checkbox'),
      '#default_value' => 0,
    );
  }

  // Add javascript settings that will be added via $.extend for tabledragging
  $form['#js']['tableDrag']['arrange']['weight'][0] = array(
    'target' => 'weight',
    'source' => NULL,
    'relationship' => 'sibling',
    'action' => 'order',
    'hidden' => TRUE,
    'limit' => 0,
  );

  $name = NULL;
  if (isset($form_state['update_name'])) {
    $name = $form_state['update_name'];
  }

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_rearrange_form');
  return $form;
}

/**
 * Turn the rearrange form into a proper table
 */
function theme_views_ui_rearrange_form($form) {
  $rows = array();
  foreach (element_children($form) as $id) {
    if (isset($form[$id]['name'])) {
      $row = array();
      $row[] = drupal_render($form[$id]['name']);
      $form[$id]['weight']['#attributes']['class'] = 'weight';
      $row[] = drupal_render($form[$id]['weight']);
      $row[] = drupal_render($form[$id]['removed']) . l('<span>' . t('Remove') . '</span>', 'javascript:void()', array('attributes' => array('id' => 'views-remove-link-' . $id, 'class' => 'views-button-remove views-remove-link', 'alt' => t('Remove this item'), 'title' => t('Remove this item')), 'html' => true));

      $rows[] = array('data' => $row, 'class' => 'draggable', 'id' => 'views-row-' . $id);
    }
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => t('No fields available.'), 'colspan' => '2'));
  }

  $header = array('', t('Weight'), t('Remove'));
  drupal_add_tabledrag('arrange', 'order', 'sibling', 'weight');
  $output = drupal_render($form['override']);
  $output .= theme('table', $header, $rows, array('id' => 'arrange'));
  $output .= drupal_render($form);
  return $output;

}

/**
 * Submit handler for rearranging form
 */
function views_ui_rearrange_form_submit($form, &$form_state) {
  $types = views_object_types();
  $display = &$form_state['view']->display[$form_state['display_id']];

  $old_fields = $display->handler->get_option($types[$form_state['type']]['plural']);
  $new_fields = $order = array();

  // Make an array with the weights
  foreach ($form_state['values'] as $field => $info) {
    // add each value that is a field with a weight to our list, but only if
    // it has had its 'removed' checkbox checked.
    if (is_array($info) && isset($info['weight']) && empty($info['removed'])) {
      $order[$field] = $info['weight'];
    }
  }

  // Sort the array
  asort($order);

  // Create a new list of fields in the new order.
  foreach (array_keys($order) as $field) {
    $new_fields[$field] = $old_fields[$field];
  }
  $display->handler->set_option($types[$form_state['type']]['plural'], $new_fields);

  // Store in cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Form to add_item items in the views UI.
 */
function views_ui_add_item_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];

  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $display = &$view->display[$display_id];

  $types = views_object_types();
  $form['#title'] = check_plain($display->display_title) . ': ';
  $form['#title'] .= t('Add @type', array('@type' => $types[$type]['ltitle']));
  $form['#section'] = $display_id . 'add-item';

  // Figure out all the base tables allowed based upon what the relationships provide.
  $base_tables = $view->get_base_tables();
  $options = views_fetch_fields(array_keys($base_tables), $type);

  if (!empty($options)) {
    $groups = array('all' => t('<All>'));
    $form['group'] = array(
      '#type' => 'select',
      '#title' => t('Groups'),
      '#options' => array(),
      '#attributes' => array('class' => 'views-master-dependent'),
    );

    $form['name'] = array(
      '#prefix' => '<div class="views-radio-box form-checkboxes">',
      '#suffix' => '</div>',
      '#tree' => TRUE,
      '#default_value' => 'all',
    );

    // Group options first to simplify the DOM objects that Views
    // dependent JS will act upon.
    $grouped_options = array();
    foreach ($options as $key => $option) {
      $group = preg_replace('/[^a-z0-9]/', '-', strtolower($option['group']));
      $groups[$group] = $option['group'];
      $grouped_options[$group][$key] = $option;
    }

    foreach ($grouped_options as $group => $group_options) {
      $form['name'][$group . '_start'] = array('#type' => 'markup', '#value' => '<div class="views-dependent-all views-dependent-' . $group . '">');
      foreach ($group_options as $key => $option) {
        $form['name'][$key] = array(
          '#type' => 'checkbox',
          '#title' => t('!group: !field', array('!group' => $option['group'], '!field' => $option['title'])),
          '#description' => $option['help'],
          '#return_value' => $key,
        );
      }
      $form['name'][$group . '_end'] = array('#type' => 'markup', '#value' => '</div>');
    }

    $form['group']['#options'] = $groups;
  }
  else {
    $form['markup'] = array(
      '#value' => '<div class="form-item">' . t('There are no @types available to add.', array('@types' =>  $types[$type]['ltitle'])) . '</div>',
    );
  }
  views_ui_standard_form_buttons($form, $form_state, 'views_ui_add_item_form', t('Add'));

  return $form;
}

/**
 * Submit handler for adding new item(s) to a view.
 */
function views_ui_add_item_form_submit($form, &$form_state) {
  $type = $form_state['type'];
  $types = views_object_types();

  if (!empty($form_state['values']['name']) && is_array($form_state['values']['name'])) {
    // Loop through each of the items that were checked and add them to the view.
    foreach (array_keys(array_filter($form_state['values']['name'])) as $field) {
      list($table, $field) = explode('.', $field, 2);
      $id = $form_state['view']->add_item($form_state['display_id'], $type, $table, $field);

      // check to see if this type has settings, if so add the settings form first
      $handler = views_get_handler($table, $field, $type);
      if ($handler && $handler->has_extra_options()) {
        views_ui_add_form_to_stack('config-item-extra', $form_state['view'], $form_state['display_id'], array($type, $id));
      }
      // Then add the form to the stack
      views_ui_add_form_to_stack('config-item', $form_state['view'], $form_state['display_id'], array($type, $id));
    }
  }

  // Store in cache
  views_ui_cache_set($form_state['view']);
}


/**
 * Form to config_item items in the views UI.
 */
function views_ui_config_item_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item['table'], $item['field'], $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
    }
    else {
      $handler->init($view, $item);
      $types = views_object_types();

      if ($view->display_handler->defaultable_sections($types[$type]['plural'])) {
        $form_state['section'] = $types[$type]['plural'];
        $view->display_handler->add_override_button($form['options'], $form_state, $form_state['section']);
      }

      // A whole bunch of code to figure out what relationships are valid for
      // this item.
      $relationships = $view->display_handler->get_option('relationships');
      $relationship_options = array();

      foreach ($relationships as $relationship) {
        // relationships can't link back to self. But also, due to ordering,
        // relationships can only link to prior relationships.
        if ($type == 'relationship' && $id == $relationship['id']) {
          break;
        }
        $relationship_handler = views_get_handler($relationship['table'], $relationship['field'], 'relationship');
        // ignore invalid/broken relationships.
        if (empty($relationship_handler)) {
          continue;
        }

        // If this relationship is valid for this type, add it to the list.
        $data = views_fetch_data($relationship['table']);
        $base = $data[$relationship['field']]['relationship']['base'];
        $base_fields = views_fetch_fields($base, $form_state['type']);
        if (isset($base_fields[$item['table'] . '.' . $item['field']])) {
          $relationship_handler->init($view, $relationship);
          $relationship_options[$relationship['id']] = $relationship_handler->label();
        }
      }

      if (!empty($relationship_options)) {
        // Make sure the existing relationship is even valid. If not, force
        // it to none.
        $base_fields = views_fetch_fields($view->base_table, $form_state['type']);
        if (isset($base_fields[$item['table'] . '.' . $item['field']])) {
          $relationship_options = array_merge(array('none' => t('Do not use a relationship')), $relationship_options);
        }
        $rel = empty($item['relationship']) ? 'none' : $item['relationship'];
        if (empty($relationship_options[$rel])) {
          // Pick the first relationship.
          $rel = key($relationship_options);
          // We want this relationship option to get saved even if the user
          // skips submitting the form.
          $view->set_item_option($display_id, $type, $id, 'relationship', $rel);
          $temp_view = $view->clone_view();
          views_ui_cache_set($temp_view);
        }

        $form['options']['relationship'] = array(
          '#type' => 'select',
          '#title' => t('Relationship'),
          '#options' => $relationship_options,
          '#default_value' => $rel,
        );
      }
      else {
        $form['options']['relationship'] = array(
          '#type' => 'value',
          '#value' => 'none',
        );
      }

      $form['#title'] = check_plain($view->display[$display_id]->display_title) . ': ';
      $form['#title'] .= t('Configure @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

      $form['#section'] = $display_id . '-' . $type . '-' . $id;

      // Get form from the handler.
      $handler->options_form($form['options'], $form_state);
      $form_state['handler'] = &$handler;
    }

    $name = NULL;
    if (isset($form_state['update_name'])) {
      $name = $form_state['update_name'];
    }

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_item_form', $name, t('Remove'), 'remove');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_form_validate($form, &$form_state) {
  $form_state['handler']->options_validate($form['options'], $form_state);
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['options'], $form_state);
  $item = $form_state['handler']->options;

  // Unset a button
  unset($form_state['values']['options']['expose_button']);

  // Store the data we're given.
  foreach ($form_state['values']['options'] as $key => $value) {
    $item[$key] = $value;
  }

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  $handler = views_get_handler($item['table'], $item['field'], $form_state['type']);
  $handler->init($form_state['view'], $item);
  if ($handler && $handler->needs_style_plugin()) {
    views_ui_add_form_to_stack('change-style', $form_state['view'], $form_state['display_id'], array($form_state['type'], $form_state['id']), TRUE);
  }

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Submit handler for removing an item from a view
 */
function views_ui_config_item_form_remove($form, &$form_state) {
  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], NULL);

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Override handler for views_ui_edit_display_form
 */
function views_ui_config_item_form_expose($form, &$form_state) {
  $item = &$form_state['handler']->options;
  // flip
  $item['exposed'] = empty($item['exposed']);

  // If necessary, set new defaults:
  if ($item['exposed']) {
    $form_state['handler']->expose_options();
  }

  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  views_ui_cache_set($form_state['view']);
  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
  $form_state['force_expose_options'] = TRUE;
}

/**
 * Form to config_item items in the views UI.
 */
function views_ui_config_item_extra_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item['table'], $item['field'], $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
      break;
    }
    else {
      $handler->init($view, $item);
      $types = views_object_types();

      $form['#title'] = check_plain($view->display[$display_id]->display_title) . ': ';
      $form['#title'] .= t('Configure extra settings for @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

      $form['#section'] = $display_id . '-' . $type . '-' . $id;

      // Get form from the handler.
      $handler->extra_options_form($form['options'], $form_state);
        $form_state['handler'] = &$handler;

    }

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_item_extra_form');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_extra_form_validate($form, &$form_state) {
  $form_state['handler']->extra_options_validate($form['options'], $form_state);
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_extra_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->extra_options_submit($form['options'], $form_state);
  $item = $form_state['handler']->options;

  // Store the data we're given.
  foreach ($form_state['values']['options'] as $key => $value) {
    $item[$key] = $value;
  }

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Form to change_style items in the views UI.
 */
function views_ui_change_style_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item['table'], $item['field'], $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
      break;
    }
    $handler->init($view, $item);
    $types = views_object_types();
    $form['#title'] = t('Change summary style for @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

    $form['#section'] = $display_id . '-' . $type . '-' . $id . '-style-plugin';

    $form['style_plugin'] =  array(
      '#type' => 'radios',
      '#options' => views_fetch_plugin_names('style', 'summary'),
      '#default_value' => $item['style_plugin'],
    );

    $form_state['handler'] = &$handler;

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_change_style_form');
  }
  return $form;
}

function views_ui_change_style_form_validate($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_validate($form['options'], $form_state);

  $plugin = views_get_plugin('style', $form_state['values']['style_plugin']);
  if (!$plugin) {
    form_error($form['style_plugin'], t('Internal error: broken plugin.'));
  }
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_change_style_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['options'], $form_state);
  $item = $form_state['handler']->options;

  $plugin = views_get_plugin('style', $form_state['values']['style_plugin']);
  if (!$plugin) {
    drupal_set_message(t('Internal error: broken plugin.'), 'error');
    return;
  }

  $plugin->init($form_state['view'], $form_state['view']->display[$form_state['display_id']]);

  // If changing style plugin, reset options to defaults.
  if (empty($item['style_plugin']) || $item['style_plugin'] != $form_state['values']['style_plugin']) {
    $item['style_options'] = $plugin->options;
  }

  // Store the data we're given.
  $item['style_plugin'] = $form_state['values']['style_plugin'];

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  if (!empty($plugin->definition['uses options'])) {
    views_ui_add_form_to_stack('config-style', $form_state['view'], $form_state['display_id'], array($form_state['type'], $form_state['id']), TRUE);
  }

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Form to config_style items in the views UI.
 */
function views_ui_config_style_form(&$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item['table'], $item['field'], $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
      break;
    }
    $handler->init($view, $item);
    $types = views_object_types();

    $form['#title'] = check_plain($view->display[$display_id]->display_title) . ': ';
    $form['#title'] .= t('Configure summary style for @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

    $form['#section'] = $display_id . '-' . $type . '-style-options';

    $plugin = views_get_plugin('style', $item['style_plugin']);
    if ($plugin) {
      $form['style_options'] = array(
        '#tree' => TRUE,
      );
      $plugin->init($view, $view->display[$display_id], $item['style_options']);

      $plugin->options_form($form['style_options'], $form_state);
    }

    $form_state['handler'] = &$handler;

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_style_form');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_style_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['style_options'], $form_state);
  $item = $form_state['handler']->options;

  // Store the data we're given.
  $item['style_options'] = $form_state['values']['style_options'];

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Get a list of roles in the system.
 */
function views_ui_get_roles() {
  static $roles = NULL;
  if (!isset($roles)) {
    $roles = array();
    $result = db_query("SELECT r.rid, r.name FROM {role} r ORDER BY r.name");
    while ($obj = db_fetch_object($result)) {
      $roles[$obj->rid] = $obj->name;
    }
  }

  return $roles;
}

/**
 * Get a css safe id for a particular section.
 */
function views_ui_item_css($item) {
  return views_css_safe('views-item-' . $item);
}

/**
 * Page callback for the Views enable page.
 */
function views_ui_enable_page($view) {
  if (isset($_GET['token']) && drupal_valid_token($_GET['token'], 'views-enable')) {
    $views_status = variable_get('views_defaults', array());
    $views_status[$view->name] = FALSE; // false is enabled
    variable_set('views_defaults', $views_status);
    views_invalidate_cache();
    menu_rebuild();
    drupal_goto('admin/build/views');
  }
  else {
    return drupal_access_denied();
  }
}

/**
 * Page callback for the Views enable page
 */
function views_ui_disable_page($view) {
  if (isset($_GET['token']) && drupal_valid_token($_GET['token'], 'views-disable')) {
    $views_status = variable_get('views_defaults', array());
    $views_status[$view->name] = TRUE; // True is disabled
    variable_set('views_defaults', $views_status);
    views_invalidate_cache();
    menu_rebuild();
    drupal_goto('admin/build/views');
  }
  else {
    return drupal_access_denied();
  }
}

/**
 * Page callback for the tools - other page
 */
function views_ui_admin_tools() {
  $form['clear_cache'] = array(
    '#type' => 'submit',
    '#value' => t("Clear Views' cache"),
    '#submit' => array('views_ui_tools_clear_cache'),
  );

  $form['views_sql_signature'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add Views signature to all SQL queries'),
    '#description' => t("All Views-generated queries will include a special 'VIEWS' = 'VIEWS' string in the WHERE clause. This makes identifying Views queries in database server logs simpler, but should only be used when troubleshooting."),
    '#default_value' => variable_get('views_sql_signature', FALSE),
  );

  $form['views_skip_cache'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable views data caching'),
    '#description' => t("Views caches data about tables, modules and views available, to increase performance. By checking this box, Views will skip this cache and always rebuild this data when needed. This can have a serious performance impact on your site."),
    '#default_value' => variable_get('views_skip_cache', FALSE),
  );

  $form['views_hide_help_message'] = array(
    '#type' => 'checkbox',
    '#title' => t('Ignore missing advanced help module'),
    '#description' => t("Views uses the advanced help module to provide help text; if this module is not present Views will complain, unless this setting is checked."),
    '#default_value' => variable_get('views_hide_help_message', FALSE),
  );

  $form['views_ui_query_on_top'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show query above live preview'),
    '#description' => t("The live preview feature will show you the output of the view you're creating, as well as the view. Check here to show the query and other information above the view; leave this unchecked to show that information below the view."),
    '#default_value' => variable_get('views_ui_query_on_top', FALSE),
  );

  $form['views_ui_disable_live_preview'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable automatic live preview'),
    '#description' => t("Don't automatically update the preview. This can speed up the editing of views a bit.'"),
    '#default_value' => variable_get('views_ui_disable_live_preview', 0),
  );

  $form['views_show_additional_queries'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show other queries run during render during live preview'),
    '#description' => t("Drupal has the potential to run many queries while a view is being rendered. Checking this box will display every query run during view render as part of the live preview."),
    '#default_value' => variable_get('views_show_additional_queries', FALSE),
  );

  $form['views_no_hover_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not show hover links over views'),
    '#description' => t("To make it easier to administrate your views, Views provides 'hover' links to take you to the edit and export screen of a view whenever the view is used. This can be distracting on some themes, though; if it is problematic, you can turn it off here."),
    '#default_value' => variable_get('views_no_hover_links', FALSE),
  );

  $form['views_devel_output'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable views performance statistics via the Devel module'),
    '#description' => t("Check this to enable some Views query and performance statistics <em>if Devel is installed</em>."),
    '#default_value' => variable_get('views_devel_output', FALSE),
  );

  $form['views_no_javascript'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable javascript with Views'),
    '#description' => t("If you are having problems with the javascript, you can disable it here; the Views UI should degrade and still be usable without javascript, it just not as good."),
    '#default_value' => variable_get('views_no_javascript', FALSE),
  );

  $regions = system_region_list(variable_get('theme_default', 'garland'));
  $regions['watchdog'] = t('Watchdog');

  $form['views_devel_region'] = array(
    '#type' => 'select',
    '#title' => t('Page region to output performance statistics'),
    '#default_value' => variable_get('views_devel_region', 'footer'),
    '#options' => $regions,
  );

  $form['views_exposed_filter_any_label'] = array(
    '#type' => 'select',
    '#title' => t('Label for "Any" value on optional single-select exposed filters'),
    '#options' => array('old_any' => '<Any>', 'new_any' => t('- Any -')),
    '#default_value' => variable_get('views_exposed_filter_any_label', 'old_any'),
  );

  return system_settings_form($form);
}

/**
 * Submit hook to clear the views cache.
 */
function views_ui_tools_clear_cache() {
  views_invalidate_cache();
  drupal_set_message(t('The cache has been cleared.'));
}

/**
 * Submit hook to clear Drupal's theme registry (thereby triggering
 * a templates rescan).
 */
function views_ui_config_item_form_rescan($form, &$form_state) {
  drupal_rebuild_theme_registry();

  // The 'Theme: Information' page is about to be shown again. That page
  // analyzes the output of theme_get_registry(). However, this latter
  // function uses an internal cache (which was initialized before we
  // called drupal_rebuild_theme_registry()) so it won't reflect the
  // current state of our theme registry. The only way to clear that cache
  // is to re-initialize the theme system:
  unset($GLOBALS['theme']);
  init_theme();

  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
}

/**
 * Override handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_change_theme($form, &$form_state) {
  // This is just a temporary variable.
  $form_state['view']->theme = $form_state['values']['theme'];

  views_ui_cache_set($form_state['view']);
  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
}

/**
 * Page callback for views tag autocomplete
 */
function views_ui_autocomplete_tag($string = '') {
  $matches = array();
  // get matches from default views:
  views_include('view');
  $views = views_discover_default_views();
  foreach ($views as $view) {
    if (!empty($view->tag) && strpos($view->tag, $string) === 0) {
      $matches[$view->tag] = $view->tag;
    }
  }

  if ($string) {
    $result = db_query_range("SELECT DISTINCT tag FROM {views_view} WHERE LOWER(tag) LIKE LOWER('%s%%')", $string, 0, 10 - count($matches));
    while ($view = db_fetch_object($result)) {
      $matches[$view->tag] = check_plain($view->tag);
    }
  }

  drupal_json($matches);
}

// ------------------------------------------------------------------
// Get information from the Views data

function _views_weight_sort($a, $b) {
  if ($a['weight'] != $b['weight']) {
    return $a['weight'] < $b['weight'] ? -1 : 1;
  }
  if ($a['title'] != $b['title']) {
    return $a['title'] < $b['title'] ? -1 : 1;
  }

  return 0;
}

/**
 * Fetch a list of all base tables available
 *
 * @return
 *   A keyed array of in the form of 'base_table' => 'Description'.
 */
function views_fetch_base_tables() {
  static $base_tables = array();
  if (empty($base_tables)) {
    $weights = array();
    $tables = array();
    $data = views_fetch_data();
    foreach ($data as $table => $info) {
      if (!empty($info['table']['base'])) {
        $tables[$table] = array(
          'title' => $info['table']['base']['title'],
          'description' => $info['table']['base']['help'],
          'weight' => !empty($info['table']['base']['weight']) ? $info['table']['base']['weight'] : 0,
        );
      }
    }
    uasort($tables, '_views_weight_sort');
    $base_tables = $tables;
  }

  return $base_tables;
}

function _views_sort_types($a, $b) {
  if ($a['group'] != $b['group']) {
    return $a['group'] < $b['group'] ? -1 : 1;
  }

  if ($a['title'] != $b['title']) {
    return $a['title'] < $b['title'] ? -1 : 1;
  }

  return 0;
}

/**
 * Fetch a list of all fields available for a given base type.
 *
 * @return
 *   A keyed array of in the form of 'base_table' => 'Description'.
 */
function views_fetch_fields($base, $type) {
  static $fields = array();
  if (empty($fields)) {
    $data = views_fetch_data();
    $start = views_microtime();
    // This constructs this ginormous multi dimensional array to
    // collect the important data about fields. In the end,
    // the structure looks a bit like this (using nid as an example)
    // $strings['nid']['filter']['title'] = 'string'.
    //
    // This is constructed this way because the above referenced strings
    // can appear in different places in the actual data structure so that
    // the data doesn't have to be repeated a lot. This essentially lets
    // each field have a cheap kind of inheritance.

    foreach ($data as $table => $table_data) {
      $bases = array();
      $strings = array();
      $skip_bases = array();
      foreach ($table_data as $field => $info) {
        // Collect table data from this table
        if ($field == 'table') {
          // calculate what tables this table can join to.
          if (!empty($info['join'])) {
            $bases = array_keys($info['join']);
          }
          // And it obviously joins to itself.
          $bases[] = $table;
          continue;
        }
        foreach (array('field', 'sort', 'filter', 'argument', 'relationship') as $key) {
          if (!empty($info[$key])) {
            if (!empty($info[$key]['skip base'])) {
              foreach ((array) $info[$key]['skip base'] as $base_name) {
                $skip_bases[$field][$key][$base_name] = TRUE;
              }
            }
            elseif (!empty($info['skip base'])) {
              foreach ((array) $info['skip base'] as $base_name) {
                $skip_bases[$field][$key][$base_name] = TRUE;
              }
            }
            foreach (array('title', 'group', 'help', 'base') as $string) {
              // First, try the lowest possible level
              if (!empty($info[$key][$string])) {
                $strings[$field][$key][$string] = $info[$key][$string];
              }
              // Then try the field level
              elseif (!empty($info[$string])) {
                $strings[$field][$key][$string] = $info[$string];
              }
              // Finally, try the table level
              elseif (!empty($table_data['table'][$string])) {
                $strings[$field][$key][$string] = $table_data['table'][$string];
              }
              else {
                if ($string != 'base') {
                  $strings[$field][$key][$string] = t("Error: missing @component", array('@component' => $string));
                }
              }
            }
          }
        }
      }
      foreach ($bases as $base_name) {
        foreach ($strings as $field => $field_strings) {
          foreach ($field_strings as $type_name => $type_strings) {
            if (empty($skip_bases[$field][$type_name][$base_name])) {
              $fields[$base_name][$type_name]["$table.$field"] = $type_strings;
            }
          }
        }
      }
    }
//    vsm('Views UI data build time: ' . (views_microtime() - $start) * 1000 . ' ms');
  }

  // If we have an array of base tables available, go through them
  // all and add them together. Duplicate keys will be lost and that's
  // Just Fine.
  if (is_array($base)) {
    $strings = array();
    foreach ($base as $base_table) {
      if (isset($fields[$base_table][$type])) {
        $strings += $fields[$base_table][$type];
      }
    }
    uasort($strings, '_views_sort_types');
    return $strings;
  }

  if (isset($fields[$base][$type])) {
    uasort($fields[$base][$type], '_views_sort_types');
    return $fields[$base][$type];
  }
  return array();
}

/**
 * Fetch a list of all base tables available
 *
 * @param $type
 *   Either 'display', 'style' or 'row'
 * @param $key
 *   For style plugins, this is an optional type to restrict to. May be 'normal',
 *   'summary', 'feed' or others based on the neds of the display.
 * @param $base
 *   An array of possible base tables.
 *
 * @return
 *   A keyed array of in the form of 'base_table' => 'Description'.
 */
function views_fetch_plugin_names($type, $key = NULL, $base = array()) {
  $data = views_fetch_plugin_data();

  $plugins[$type] = array();

  foreach ($data[$type] as $id => $plugin) {
    // Skip plugins that don't conform to our key.
    if ($key && (empty($plugin['type']) || $plugin['type'] != $key)) {
      continue;
    }
    if (empty($plugin['no ui']) && (empty($base) || empty($plugin['base']) || array_intersect($base, $plugin['base']))) {
      $plugins[$type][$id] = $plugin['title'];
    }
  }

  if (!empty($plugins[$type])) {
    asort($plugins[$type]);
    return $plugins[$type];
  }
  // fall-through
  return array();
}


/**
 * Theme the form for the table style plugin
 */
function theme_views_ui_style_plugin_table($form) {
  $output = drupal_render($form['description_markup']);

  $header = array(
    t('Field'),
    t('Column'),
    t('Separator'),
    array(
      'data' => t('Sortable'),
      'align' => 'center',
    ),
    array(
      'data' => t('Default sort'),
      'align' => 'center',
    ),
  );
  $rows = array();
  foreach (element_children($form['columns']) as $id) {
    $row = array();
    $row[] = drupal_render($form['info'][$id]['name']);
    $row[] = drupal_render($form['columns'][$id]);
    $row[] = drupal_render($form['info'][$id]['separator']);
    if (!empty($form['info'][$id]['sortable'])) {
      $row[] = array(
        'data' => drupal_render($form['info'][$id]['sortable']),
        'align' => 'center',
      );
      $row[] = array(
        'data' => drupal_render($form['default'][$id]),
        'align' => 'center',
      );
    }
    else {
      $row[] = '';
      $row[] = '';
    }
    $rows[] = $row;
  }

  // Add the special 'None' row.
  $rows[] = array(t('None'), '', '', '', array('align' => 'center', 'data' => drupal_render($form['default'][-1])));

  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}

