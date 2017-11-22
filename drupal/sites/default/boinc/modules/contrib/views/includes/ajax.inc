<?php

/**
 * @file ajax.inc
 *
 * Handles the server side AJAX interactions of Views.
 *
 * @defgroup ajax Views ajax library
 * @{
 */

/**
 * Menu callback to load a view via AJAX.
 */
function views_ajax() {
  if (isset($_REQUEST['view_name']) && isset($_REQUEST['view_display_id'])) {
    $name = $_REQUEST['view_name'];
    $display_id = $_REQUEST['view_display_id'];
    $args = isset($_REQUEST['view_args']) && $_REQUEST['view_args'] !== '' ? explode('/', $_REQUEST['view_args']) : array();
    $path = isset($_REQUEST['view_path']) ? $_REQUEST['view_path'] : NULL;
    $dom_id = isset($_REQUEST['view_dom_id']) ? intval($_REQUEST['view_dom_id']) : NULL;
    $pager_element = isset($_REQUEST['pager_element']) ? intval($_REQUEST['pager_element']) : NULL;
    views_include('ajax');
    $object = new stdClass();

    $object->status = FALSE;
    $object->display = '';

    $arg = explode('/', $_REQUEST['view_path']);

    if ($arg[0] == 'admin' || (variable_get('node_admin_theme', '0') && $arg[0] == 'node' && ($arg[1] == 'add' || $arg[2] == 'edit'))) {
        global $custom_theme;
        $custom_theme = variable_get('admin_theme', '0');
        drupal_add_css(drupal_get_path('module', 'system') .'/admin.css', 'module');
    }
    // Load the view.
    if ($view = views_get_view($name)) {
      if ($view->access($display_id) && $view->set_display($display_id) && $view->display_handler->use_ajax()) {

        // Fix 'q' for paging.
        if (!empty($path)) {
          $_GET['q'] = $path;
        }

        // Override the display's pager_element with the one actually used.
        if (isset($pager_element)) {
          $view->display[$display_id]->handler->set_option('pager_element', $pager_element);
        }
        // Reuse the same DOM id so it matches that in Drupal.settings.
        $view->dom_id = $dom_id;

        $object->status = TRUE;
        $object->display .= $view->preview($display_id, $args);
        // Get the title after the preview call, to let it set
        // up both the view's current display and arguments
        $object->title = $view->get_title();
        // Register the standard JavaScript callback.
        $object->__callbacks = array('Drupal.Views.Ajax.ajaxViewResponse');
        // Allow other modules to extend the data returned.
        drupal_alter('ajax_data', $object, 'views', $view);
      }
    }
    $messages = theme('status_messages');
    $object->messages = $messages ? '<div class="views-messages">' . $messages . '</div>' : '';

    views_ajax_render($object);
  }
}

/**
 * Simple render function to make sure output is what we want.
 *
 * This function renders an object into JSON, and that object contains
 * commands to the ajax response parser on the other side. The actual
 * commands that can be sent are completely dependent upon the client
 * javascript parser, which can be anything, but this function assumes
 * that 'display', at least, will be displayed in some kind of ajax
 * spot or popup.
 */
function views_ajax_render($output = NULL, $title = NULL, $url = NULL, $js = NULL) {
  if (empty($output)) {
    $output->display = t('Server reports invalid input error.');
    $output->title = t('Error');
  }
  elseif (!is_object($output)) {
    $temp = new stdClass();
    $temp->display = $output;
    $temp->title = $title;
    $temp->url = $url;
    $output = $temp;
  }
  if (!empty($js)) {
    $output->js = $js;
  }

  drupal_json($output);
  exit;
}

/**
 * Wrapper around drupal_build_form to handle some AJAX stuff automatically.
 * This makes some assumptions about the client.
 */
function views_ajax_form_wrapper($form_id, &$form_state) {
  // This won't override settings already in.
  $form_state += array(
    'rerender' => FALSE,
    'no_redirect' => !empty($form_state['ajax']),
  );

  $output = drupal_build_form($form_id, $form_state);
  if (!empty($form_state['ajax']) && empty($form_state['executed'])) {
    // If the form didn't execute and we're using ajax, build up a
    // json command object to render.
    $object = new stdClass();
    $object->display = '';
    if ($messages = theme('status_messages')) {
      $object->display = '<div class="views-messages">' . $messages . '</div>';
    }
    $object->display .= $output;

    $object->title = empty($form_state['title']) ? '' : $form_state['title'];
    if (!empty($form_state['help_topic'])) {
      $module = !empty($form_state['help_module']) ? $form_state['help_module'] : 'views';
      $object->title = theme('advanced_help_topic', $module, $form_state['help_topic']) . $object->title;
    }
    $object->url = empty($form_state['url']) ? url($_GET['q'], array('absolute' => TRUE)) : $form_state['url'];
    $object->js = empty($form_state['js settings']) ? NULL : $form_state['js settings'];
    if (!empty($form_state['#section'])) {
      $object->hilite = '.' . views_ui_item_css($form_state['#section']);
    }

    $output = $object;
  }

  // These forms have the title built in, so set the title here:
  if (empty($form_state['ajax']) && !empty($form_state['title'])) {
    drupal_set_title($form_state['title']);
  }

  return $output;
}


/**
 * Page callback for views user autocomplete
 */
function views_ajax_autocomplete_user($string = '') {
  // The user enters a comma-separated list of tags. We only autocomplete the last tag.
  $array = drupal_explode_tags($string);

  // Fetch last tag
  $last_string = trim(array_pop($array));
  $matches = array();
  if ($last_string != '') {
    $prefix = count($array) ? implode(', ', $array) . ', ' : '';

    if (strpos('anonymous', strtolower($last_string)) !== FALSE) {
      $matches[$prefix . 'Anonymous'] = 'Anonymous';
    }
    $result = db_query_range("SELECT name FROM {users} WHERE LOWER(name) LIKE LOWER('%s%%')", $last_string, 0, 10);

    while ($account = db_fetch_object($result)) {
      $n = $account->name;
      // Commas and quotes in terms are special cases, so encode 'em.
      if (strpos($account->name, ',') !== FALSE || strpos($account->name, '"') !== FALSE) {
        $n = '"' . str_replace('"', '""', $account->name) . '"';
      }
      $matches[$prefix . $n] = check_plain($account->name);
    }
  }

  drupal_json($matches);
}

/**
 * @}
 */
