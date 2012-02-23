<?php

/**
 * @file
 * Page wizard that can create a variant on the node_view to take over a node
 * for a particular type.
 *
 * This wizard does a lot that's cut and pasted from exports. We can get away
 * with this because we know exports tend to remain relatively backward
 * compatible, and because we know that our context IDs are locked in the
 * node_view page.
 */
$plugin = array(
  'title' => t('Node template'),
  'page title' => t('Node template wizard'),
  'description' => t('The node page wizard can help you override the node page for a type of node.'),

  'type' => 'panels',

  'form info' => array(
    'order' => array(
      'type' => t('Select node type'),
      'content' => t('Content'),
    ),

    'forms' => array(
      'type' => array(
        'form id' => 'panels_node_override_basic',
      ),
      'content' => array(
        'form id' => 'panels_node_override_content',
      ),
    ),
  ),

  'default cache' => 'panels_node_override_new_page',

  'start' => 'panels_node_override_start',
  'finish' => 'panels_node_override_finish',
);

/**
 * Provide defaults for a new cache.
 *
 * The cache will store all our temporary data; it isn't really a page
 * in itself, but it does contain everything we need to make one at the end.
 */
function panels_node_override_new_page(&$cache) {
  $cache->type = '';
  $cache->display = panels_new_display();
  $cache->display->layout = 'flexible';
}

/**
 * Callback called prior to the wizard starting up on every page
 * load.
 */
function panels_node_override_start($form_info, $step, &$form_state) {
  $form_state['page'] = page_manager_get_page_cache('node_view');
  if (!empty($form_state['page']->locked)) {
    $account  = user_load($form_state['page']->locked->uid);
    $name     = theme('username', $account);
    $lock_age = format_interval(time() - $form_state['page']->locked->updated);
    $break    = url(page_manager_edit_url($form_state['page']->task_name, array('actions', 'break-lock')));

    drupal_set_message(t('WARNING! The node_view is being edited by user !user, and is therefore locked from editing by others. This wizard cannot create a new node override while this page is locked. This lock is !age old. Click here to <a href="!break">break this lock</a>.', array('!user' => $name, '!age' => $lock_age, '!break' => $break)), 'warning');
  }
}

/**
 * First page of our page creator wizard.
 */
function panels_node_override_basic(&$form, &$form_state) {
  $types = node_get_types();
  $form_state['types'] = $types;

  $already_done = array();
  // Figure out which types already have variants assigned to them.
  foreach ($form_state['page']->handlers as $name => $handler) {
    if ($handler->handler == 'panel_context' && !empty($handler->conf['access']['plugins'])) {
      foreach ($handler->conf['access']['plugins'] as $plugin) {
        if ($plugin['name'] == 'node_type') {
          foreach ($plugin['settings']['type'] as $type) {
            $already_done[$type] = $name;
          }
        }
      }
    }
  }

  if ($already_done) {
    $items = array();
    foreach ($already_done as $type => $handler_id) {
      $items[] = check_plain($types[$type]->name) . ' ' . l(t('[Edit]'), page_manager_edit_url($form_state['page']->task_name, array('handlers', $handler_id, 'content')));
    }

    $form['already_done'] = array(
      '#type' => 'item',
      '#title' => t('Existing node templates'),
      '#value' => theme('item_list', $items),
    );
  }

  $options = array();
  foreach ($types as $name => $type) {
    if (empty($already_done[$name])) {
      $options[$name] = $type->name;
    }
  }

  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Node type'),
    '#options' => $options,
    '#default_value' => $form_state['cache']->type,
  );

  ctools_include('page-wizard', 'panels');
  panels_page_wizard_add_layout($form, $form_state);
}

/**
 * Submit function to store the form data in our cache.
 */
function panels_node_override_basic_submit(&$form, &$form_state) {
  $cache = &$form_state['cache'];
  $cache->display->layout = $form_state['values']['layout'];
  $cache->type = $form_state['values']['type'];

  // Create a new handler object and cache it; this way we can use the
  // handler object for retrieving contexts properly.
  // Create the the panel context variant configured with our display
  $plugin = page_manager_get_task_handler('panel_context');

  // Create a new handler.
  $cache->handler = page_manager_new_task_handler($plugin);
  $cache->handler->conf['title'] = $form_state['types'][$cache->type]->name;
  $cache->handler->conf['pipeline'] = 'ipe';
  $cache->handler->conf['access'] = array(
    'plugins' => array(
      0 => array(
        'name' => 'node_type',
        'settings' => array(
          'type' => array(
            $cache->type => $cache->type,
          ),
        ),
        'context' => 'argument_nid_1',
        'not' => FALSE,
      ),
    ),
    'logic' => 'and',
  );

  // Find a region by trying some basic main content region IDs.
  $layout = panels_get_layout($form_state['values']['layout']);
  $regions = panels_get_regions($layout, $cache->display);
  foreach (array('center', 'middle', 'content', 'main') as $candidate) {
    if (!empty($regions[$candidate])) {
      $region = $candidate;
      break;
    }
  }

  // If all of the above failed, use the first region.
  if (empty($region)) {
    $keys = array_keys($regions);
    $region = reset($keys);
  }

  // Populate the layout with content. This is from an export, with minor
  // changes to ensure defaults are correct and to add stuff to the proper region.
  $pane = new stdClass;
  $pane->pid = 'new-1';
  $pane->panel = $region;
  $pane->type = 'node_content';
  $pane->subtype = 'node_content';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'links' => 1,
    'page' => 1,
    'no_extras' => 0,
    'override_title' => 0,
    'override_title_text' => '',
    'identifier' => '',
    'link' => 0,
    'leave_node_title' => 0,
    'context' => 'argument_nid_1',
    'build_mode' => 'full',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 0;
  $cache->display->content['new-1'] = $pane;
  $cache->display->panels[$region][0] = 'new-1';
  $pane = new stdClass;
  $pane->pid = 'new-2';
  $pane->panel = $region;
  $pane->type = 'node_comments';
  $pane->subtype = 'node_comments';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'mode' => variable_get('comment_default_mode', COMMENT_MODE_THREADED_EXPANDED),
    'order' => variable_get('comment_default_order', COMMENT_ORDER_NEWEST_FIRST),
    'comments_per_page' => variable_get('comment_default_per_page', '50'),
    'context' => 'argument_nid_1',
    'override_title' => 0,
    'override_title_text' => '',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 1;
  $cache->display->content['new-2'] = $pane;
  $cache->display->panels[$region][1] = 'new-2';
  $pane = new stdClass;
  $pane->pid = 'new-3';
  $pane->panel = $region;
  $pane->type = 'node_comment_form';
  $pane->subtype = 'node_comment_form';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array(
    'anon_links' => 1,
    'context' => 'argument_nid_1',
    'override_title' => 0,
    'override_title_text' => '',
  );
  $pane->cache = array();
  $pane->style = array(
    'settings' => NULL,
  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 2;
  $cache->display->content['new-3'] = $pane;
  $cache->display->panels[$region][2] = 'new-3';

  $task = page_manager_get_task('node_view');
  ctools_include('context');
  ctools_include('context-task-handler');
  $cache->context = ctools_context_handler_get_all_contexts($task, NULL, $cache->handler);

}

/**
 * Second page of our wizard. This one provides a layout and lets the
 * user add content.
 */
function panels_node_override_content(&$form, &$form_state) {
  ctools_include('page-wizard', 'panels');
  panels_page_wizard_add_content($form, $form_state);
}

/**
 * Store changes to the display.
 */
function panels_node_override_content_submit(&$form, &$form_state) {
  panels_page_wizard_add_content_submit($form, $form_state);
}

/**
 * Complete the wizard, create a new variant, and send them to the
 * edit screen of that variant.
 */
function panels_node_override_finish(&$form_state) {
  $page = &$form_state['page'];
  $cache = &$form_state['cache'];

  // Add the new handler to the page
  $cache->handler->conf['display'] = $cache->display;
  page_manager_handler_add_to_page($page, $cache->handler);

  // Save it
  page_manager_save_page_cache($page);

  // Send us to the page manager edit form for this.
  $form_state['redirect'] = url(page_manager_edit_url('node_view', array('handlers', $cache->handler->name, 'content')));
  drupal_set_message(t('Your node template has been created.'));
}
