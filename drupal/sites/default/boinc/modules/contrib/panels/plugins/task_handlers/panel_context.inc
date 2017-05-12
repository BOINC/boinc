<?php

/**
 * @file
 *
 * This is the task handler plugin to handle attaching a panel to any
 * task that advertises itself as a 'context' type, which all of the
 * basic page tasks provided by page_manager.module do by default.
 */

// Plugin definition
$plugin = array(
  // is a 'context' handler type, meaning it supports the API of the
  // context handlers provided by ctools context plugins.
  'handler type' => 'context',
  'visible' => TRUE, // may be added up front.

  // Administrative fields.
  'title' => t('Panel'),
  'admin summary' =>'panels_panel_context_admin_summary',
  'admin title' => 'panels_panel_context_title',
  'operations' => array(
    'settings' => array(
      'title' => t('General'),
      'description' => t('Change general settings about this variant.'),
      'form' => 'panels_panel_context_edit_settings',
    ),
    'criteria' => array(
      'title' => t('Selection rules'),
      'description' => t('Control the criteria used to decide whether or not this variant is used.'),
      'ajax' => FALSE,
      'form' => array(
        'order' => array(
          'form' => t('Selection rules'),
        ),
        'forms' => array(
          'form' => array(
            'include' => drupal_get_path('module', 'ctools') . '/includes/context-task-handler.inc',
            'form id' => 'ctools_context_handler_edit_criteria',
          ),
        ),
      ),
    ),
    'context' => array(
      'title' => t('Contexts'),
      'ajax' => FALSE,
      'description' => t('Add additional context objects to this variant that can be used by the content.'),
      'form' => array(
        'order' => array(
          'form' => t('Context'),
        ),
        'forms' => array(
          'form' => array(
            'include' => drupal_get_path('module', 'ctools') . '/includes/context-task-handler.inc',
            'form id' => 'ctools_context_handler_edit_context',
          ),
        ),
      ),
    ),
    'layout' => array(
      'title' => t('Layout'),
      'description' => t('Change the layout of this panel.'),
      // No AJAX so we get our CSS loaded.
      'ajax' => FALSE,
      'form' => array(
        'order' => array(
          'choose' => t('Change layout'),
          'move' => t('Move content from old layout'),
        ),
        'forms' => array(
          'choose' => array(
            'form id' => 'panels_panel_context_edit_layout',
          ),
          'move' => array(
            'include' => array(
              drupal_get_path('module', 'panels') . '/includes/display-layout.inc',
            ),
            'form id' => 'panels_panel_context_edit_move',
            'submit' => 'panels_change_layout_submit',
          ),
        ),
      ),
    ),
    'content' => array(
      'title' => t('Content'),
      'description' => t('Add content items and change their location with a drag and drop interface.'),
      'ajax' => FALSE,
      'form' => array(
        'order' => array(
          'form' => t('Content'),
        ),
        'forms' => array(
          'form' => array(
            'include' => array(
              drupal_get_path('module', 'panels') . '/includes/display-edit.inc',
            ),
            'form id' => 'panels_panel_context_edit_content',
            'no blocks' => TRUE,
          ),
        ),
      ),
    ),
    'preview' => array(
      'title' => t('Preview'),
      'description' => t('Get a preview of what this variant will look like.'),
      'form' => 'panels_panel_context_edit_preview',
      'ajax' => FALSE,
      'silent' => TRUE,
      'form info' => array('finish text' => t('Preview')),
      'no update and save' => TRUE,
    ),
  ),

  'tab operation' => 'panels_panel_context_tab_operation',

  // Callback to render the data.
  'render' => 'panels_panel_context_render',

  // Various callbacks for operations performed on the handler to ensure
  // related data is updated properly.
  'save' => 'panels_panel_context_save',
  'delete' => 'panels_panel_context_delete',
  'export' => 'panels_panel_context_export',
  'clone' => 'panels_panel_context_clone',

  'add features' => array(
    'criteria' => t('Selection rules'),
    'context' => t('Contexts'),
  ),
  // Where to go when finished.
  'add finish' => 'content',

  'required forms' => array(
    'choose' => t('Choose layout'),
    'settings' => t('Panel settings'),
    'content' => t('Panel content'),
  ),

  'edit forms' => array(
    'content' => t('Panel content'),
    'criteria' => t('Selection rules'),
    'settings' => t('General'),
    'context' => t('Contexts'),
    'layout' => t('Change layout'),
    'move' => '', // no title makes it a 'hidden' edit form.
  ),
  'forms' => array(
    'settings' => array(
      'form id' => 'panels_panel_context_edit_settings',
    ),
    'choose' => array(
      'form id' => 'panels_panel_context_edit_choose',
      'no back validate' => TRUE,
    ),
    'layout' => array(
      'no return' => TRUE,
      'form id' => 'panels_panel_context_edit_layout',
    ),
    'move' => array(
      'include' => array(
        drupal_get_path('module', 'panels') . '/includes/display-layout.inc',
      ),
      'form id' => 'panels_panel_context_edit_move',
      'submit' => 'panels_change_layout_submit',
    ),
    'content' => array(
      'include' => array(
        drupal_get_path('module', 'panels') . '/includes/display-edit.inc',
      ),
      'form id' => 'panels_panel_context_edit_content',
      'no blocks' => TRUE,
    ),
    'context' => array(
      'include' => drupal_get_path('module', 'ctools') . '/includes/context-task-handler.inc',
      'form id' => 'ctools_context_handler_edit_context',
    ),
    'criteria' => array(
      'include' => drupal_get_path('module', 'ctools') . '/includes/context-task-handler.inc',
      'form id' => 'ctools_context_handler_edit_criteria',
    ),
  ),
  'default conf' => array(
    'title' => t('Panel'),
    'no_blocks' => FALSE,
    'pipeline' => 'standard',
    'css_id' => '',
    'css' => '',
    'contexts' => array(),
    'relationships' => array(),
  ),
);

/**
 * Provide the operation trail for the 'Edit panel' link.
 *
 * When editing the panel, go directly to the content tab.
 */
function panels_panel_context_tab_operation($handler, $contexts, $args) {
  return array('handlers', $handler->name, 'content');
}

/**
 * Get the display for a task handler.
 *
 * There are three methods that the display can be found.
 * - In the database. $handler->conf['did'] will be set in this case,
 *   and $handler->conf['display'] won't be.
 * - In $handler->conf['display'], with $handler->conf['did'] empty. This
 *   will be true for a default/imported task handler as well as a handler
 *   that has just been created but has not yet been saved.
 * - in $handler->conf['display'] with $handler->conf['did' populated. This
 *   simply means that the display has been modified and is awaiting
 *   save. The modified one should always be used for editing purposes.
 * - If none of the above is true, then a new display needs to be created
 *   for the handler and pla
 */
function &panels_panel_context_get_display(&$handler) {
  if (isset($handler->conf['display'])) {
    return $handler->conf['display'];
  }

  if (isset($handler->conf['did'])) {
    $handler->conf['display'] = panels_load_display($handler->conf['did']);

    // Check for a valid display. If no valid display can be loaded, something
    // is wrong and we'll create a new one.
    if (!empty($handler->conf['display'])) {
      return $handler->conf['display'];
    }
  }

  $handler->conf['display'] = panels_new_display();

  return $handler->conf['display'];
}

/**
 * Check selection rules and, if passed, render the contexts.
 */
function panels_panel_context_render($handler, $base_contexts, $args, $test = TRUE) {
  // Go through arguments and see if they match.
  ctools_include('context');
  ctools_include('context-task-handler');
  ctools_include('plugins', 'panels');

  // Add my contexts
  $contexts = ctools_context_handler_get_handler_contexts($base_contexts, $handler);

  // Test.
  if ($test && !ctools_context_handler_select($handler, $contexts)) {
    return;
  }

  if (isset($handler->handler)) {
    ctools_context_handler_pre_render($handler, $contexts, $args);
  }

  // Load the display
  $display = panels_panel_context_get_display($handler);

  $display->context = $contexts;
  $display->args = $args;
  $display->css_id = $handler->conf['css_id'];
  $task_name = page_manager_make_task_name($handler->task, $handler->subtask);

  $display->cache_key = 'panel_context:' . $task_name . ':' . $handler->name;

  // Check to see if there is any CSS.
  if (!empty($handler->conf['css'])) {
    ctools_include('css');
    $css_id = 'panel_context:' . $handler->name;
    $filename = ctools_css_retrieve($css_id);
    if (!$filename) {
      $filename = ctools_css_store($css_id, $handler->conf['css']);
    }
    ctools_css_add_css($filename);
  }

  // With an argument, this actually sets the display.
  panels_get_current_page_display($display);

  // Handle backward compatibility with the IPE checkbox.
  if (empty($handler->conf['pipeline'])) {
    $handler->conf['pipeline'] = !empty($handler->conf['use_ipe']) ? 'ipe' : 'standard';
  }

  $renderer = panels_get_renderer($handler->conf['pipeline'], $display);

  $info = array(
    'content' => panels_render_display($display, $renderer),
    'no_blocks' => !empty($handler->conf['no_blocks']),
  );

  $info['title'] = $display->get_title();

  return $info;
}

/**
 * Callback to allow the handler to react to being saved.
 *
 * When a handler with a display is saved, two things have to happen.
 * First, we have to save the display so that it becomes a real display,
 * not the fake one we started with. Second, we have to cache
 * any CSS that the display is using. This CSS can get re-cached
 * later if the file disappears, but it's imperative that we do it here
 * to make sure that old, dirty CSS cache gets removed.
 */
function panels_panel_context_save(&$handler, $update) {
  // Only save the display if we believe it has been modified.
  if (isset($handler->conf['display'])) {
    panels_save_display($handler->conf['display']);
    $handler->conf['did'] = $handler->conf['display']->did;
    unset($handler->conf['display']);
  }

  // Delete any previous CSS cache file.
  ctools_include('css');
  ctools_css_clear('panel_context:' . $handler->name);

  if (isset($page->conf['temp_layout'])) {
    unset($page->conf['temp_layout']);
  }
}

/**
 * Special handling for exporting a panel task handler.
 *
 * When a panel is exported, we need to export the display separately
 * rather than just letting its object be unpacked, which does not work
 * very well.
 */
function panels_panel_context_export(&$handler, $indent) {
  $display = panels_panel_context_get_display($handler);
  foreach (array('display', 'did', 'css_cache', 'temp_layout') as $item) {
    if (isset($handler->conf[$item])) {
      unset($handler->conf[$item]);
    }
  }

  $output = panels_export_display($display, $indent);
  $output .= $indent . '$handler->conf[\'display\'] = $display' . ";\n";
  return $output;
}

/**
 * When a handler is cloned, we have to clone the display.
 */
  function panels_panel_context_clone(&$handler) {
  $old_display = panels_panel_context_get_display($handler);
  $code = panels_export_display($old_display);
  eval($code);
  foreach (array('display', 'did', 'css_cache', 'temp_layout') as $item) {
    if (isset($handler->conf[$item])) {
      unset($handler->conf[$item]);
    }
  }
  $display->did = 'new';
  $handler->conf['display'] = $display;
}

/**
 * Callback to delete the display when a handler is deleted.
 */
function panels_panel_context_delete(&$handler) {
  if (!empty($handler->conf['did'])) {
    panels_delete_display($handler->conf['did']);
  }
}

/**
 * Set up a title for the panel based upon the selection rules.
 */
function panels_panel_context_title($handler, $task, $subtask) {
  if (isset($handler->conf['title'])) {
    return check_plain($handler->conf['title']);
  }
  else {
    return t('Panel');
  }
}

/**
 * Provide a nice little summary of what's in a panel.
 *
 * The task handler manager provides a summary of a given handler in a
 * collapsible div. This callback provides that. For a Panel, we
 * provide a summary of the layout type and content on one side, and
 * a summary of the contexts in use on the other.
 */
function panels_panel_context_admin_summary($handler, $task, $subtask, $page, $show_title = TRUE) {
  $task_name = page_manager_make_task_name($task['name'], $subtask['name']);
  $output = '';

  $display = panels_panel_context_get_display($handler);

  ctools_include('plugins', 'panels');
  ctools_include('context');
  ctools_include('context-task-handler');

  // Get the operations
  $operations = page_manager_get_operations($page);

  // Get operations for just this handler.
  $operations = $operations['handlers']['children'][$handler->name]['children']['actions']['children'];
  $args = array('handlers', $handler->name, 'actions');
  $rendered_operations = page_manager_render_operations($page, $operations, array(), array('class' => 'actions'), 'actions', $args);

  $layout = panels_get_layout($display->layout);

  $plugin = page_manager_get_task_handler($handler->handler);

  $object = ctools_context_handler_get_task_object($task, $subtask, $handler);
  $display->context = ctools_context_load_contexts($object, TRUE);

  $access = ctools_access_group_summary(!empty($handler->conf['access']) ? $handler->conf['access'] : array(), $display->context);
  if ($access) {
    $access = t('This panel will be selected if @conditions.', array('@conditions' => $access));
  }
  else {
    $access = t('This panel will always be selected.');
  }

  $rows = array();

  $type = $handler->type == t('Default') ? t('In code') : $handler->type;
  $rows[] = array(
    array('class' => t('page-summary-label'), 'data' => t('Storage')),
    array('class' => t('page-summary-data'), 'data' => $type),
    array('class' => t('page-summary-operation'), 'data' => ''),
  );

  if (!empty($handler->disabled)) {
    $link = l(t('Enable'), page_manager_edit_url($task_name, array('handlers', $handler->name, 'actions', 'enable')));
    $text = t('Disabled');
  }
  else {
    $link = l(t('Disable'), page_manager_edit_url($task_name, array('handlers', $handler->name, 'actions', 'disable')));
    $text = t('Enabled');
  }

  $rows[] = array(
    array('class' => t('page-summary-label'), 'data' => t('Status')),
    array('class' => t('page-summary-data'), 'data' => $text),
    array('class' => t('page-summary-operation'), 'data' => $link),
  );

  $link = l(t('Edit'), page_manager_edit_url($task_name, array('handlers', $handler->name, 'criteria')));
  $rows[] = array(
    array('class' => t('page-summary-label'), 'data' => t('Selection rule')),
    array('class' => t('page-summary-data'), 'data' => $access),
    array('class' => t('page-summary-operation'), 'data' => $link),
  );

  $link = l(t('Change layout'), page_manager_edit_url($task_name, array('handlers', $handler->name, 'layout')));
  $link .= '<br />' . l(t('Edit content'), page_manager_edit_url($task_name, array('handlers', $handler->name, 'content')));
  $link .= '<br />' . l(t('Preview'), page_manager_edit_url($task_name, array('handlers', $handler->name, 'preview')));
  $rows[] = array(
    array('class' => t('page-summary-label'), 'data' => t('Layout')),
    array('class' => t('page-summary-data'), 'data' => check_plain($layout['title'])),
    array('class' => t('page-summary-operation'), 'data' => $link),
  );

  $content_link = ' [' . l(t('Edit'), page_manager_edit_url($task_name, array('handlers', $handler->name, 'content'))) . ']';
  $context_link = ' [' . l(t('Edit'), page_manager_edit_url($task_name, array('handlers', $handler->name, 'context'))) . ']';

  $info = theme('table', array(), $rows, array('class' => 'page-manager-handler-summary'));

/*
  $content = theme('panels_common_content_list', $display);
  if (empty($contents)) {
    $contents = t('This panel has no content.');
  }
  $contexts = theme('ctools_context_list_no_table', $object);
  if (empty($contexts)) {
    $contexts = t('This panel has no contexts.');
  }
*/

  $title = $handler->conf['title'];
  if ($title != t('Panel')) {
    $title = t('Panel: @title', array('@title' => $title));
  }

  $output .= '<div class="clear-block">';
  if ($show_title) {
  $output .= '<div class="handler-title clear-block">';
    $output .= '<div class="actions handler-actions">' . $rendered_operations['actions'] . '</div>';
    $output .= '<span class="title-label">' . $title . '</span>';
  }

  $output .= '</div>';
  $output .= $info;
  $output .= '</div>';
/*
  $output .= '<div class="right-container">';
  $output .= '<h3 class="context-title">' . t('Contexts') . $context_link . '</h3>';
  $output .= $contexts;
  $output .= '</div>';

  $output .= '<div class="left-container">';
//  $output .= $icon;
  $output .= '<h3 class="handler-title">' . t('Content') . $content_link . '</h3>';
  $output .= $content;
  $output .= '</div>';
*/
  return $output;
}

// --------------------------------------------------------------------------
// Forms

/**
 * General notes about forms: The handler is automatically cached by the form
 * wizard, so anything we store on $form_state['handler'] anywhere will get
 * saved and appear on the next form. The cache is a 'working' cache and
 * if the user hits cancel on any page of the multi-page wizard, all
 * changes since the last 'update/finish' click will be flushed away.
 *
 * Many of the Panels forms call through to the real Panels cousins. These
 * forms are smart enough to know that they're being wrapped in another
 * form and act appropriately. Some of them are so smart that we just let
 * their submit and validate handlers do the work rather than writing
 * additional ones here.
 */

/**
 * Choose a layout for this panel.
 *
 * This is only called during 'add', when we know that there isn't a
 * previous layout to choose from. a different, only slightly different
 * variant is called to change a pre-existing layout.
 */
function panels_panel_context_edit_choose(&$form, &$form_state) {
  ctools_include('common', 'panels');
  ctools_include('display-layout', 'panels');
  ctools_include('plugins', 'panels');

  // @todo -- figure out where/how to deal with this.
  $form_state['allowed_layouts'] = 'panels_page';

  $form_state['display'] = &panels_panel_context_get_display($form_state['handler']);

  // Grab the storage_type and storage_id and inject it into the display.
  if (empty($form_state['display']->storage_type)) {
    if (!isset($form_state[$form_state['task_id']]->storage_type)) {
      watchdog('panels', "Unable to find the storage type for specified storage. Read 'Upgrading task handlers' in CHANGELOG.txt", array(), WATCHDOG_ERROR);
      $form_state['display']->storage_type = 'unknown';
    }
    else {
      $form_state['display']->storage_type = $form_state[$form_state['task_id']]->storage_type;
    }
    // When adding variants, we don't know the handler id yet. In that case,
    // Mark it as new. We'll assign it later.
    $form_state['display']->storage_id = !empty($form_state['handler_id']) ? $form_state['handler_id'] : 'new';
  }
  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form['#id'] = 'panels-choose-layout';
  $form = array_merge($form, panels_choose_layout($form_state));
}

/**
 * Validate that a layout was chosen.
 */
function panels_panel_context_edit_choose_validate(&$form, &$form_state) {
  if (empty($form_state['values']['layout'])) {
    form_error($form['layout'], t('You must select a layout.'));
  }
}

/**
 * A layout has been selected, set it up.
 */
function panels_panel_context_edit_choose_submit(&$form, &$form_state) {
  $form_state['display']->layout = $form_state['values']['layout'];
  $form_state['handler']->conf['display'] = $form_state['display'];
  if (isset($form_state['page']->display_cache[$form_state['handler_id']])) {
    $form_state['page']->display_cache[$form_state['handler_id']]->display = $form_state['display'];
  }
}

/**
 * Change the layout for this panel.
 *
 * This form is only used if a layout already exists and the user wants
 * to change to a different one. The submit handler changes the next form
 * to the move content form, which is 'hidden' so it won't be accessed
 * directly.
 */
function panels_panel_context_edit_layout(&$form, &$form_state) {
  ctools_include('common', 'panels');
  ctools_include('display-layout', 'panels');
  ctools_include('plugins', 'panels');

  // @todo -- figure out where/how to deal with this.
  $form_state['allowed_layouts'] = 'panels_page';

  $form_state['display'] = &panels_panel_context_get_display($form_state['handler']);

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form['#id'] = 'panels-choose-layout';
  $form = array_merge($form, panels_choose_layout($form_state));
}

/**
 * Validate that a layout was chosen.
 */
function panels_panel_context_edit_layout_validate(&$form, &$form_state) {
  $display = &panels_panel_context_get_display($form_state['handler']);

  if (empty($form_state['values']['layout'])) {
    form_error($form['layout'], t('You must select a layout.'));
  }
  if ($form_state['values']['layout'] == $display->layout) {
    form_error($form['layout'], t('You must select a different layout if you wish to change layouts.'));
  }
}

/**
 * A layout has been selected, set it up.
 */
function panels_panel_context_edit_layout_submit(&$form, &$form_state) {
  $display = &panels_panel_context_get_display($form_state['handler']);

  if ($form_state['values']['layout'] != $display->layout) {
    $form_state['handler']->conf['temp_layout'] = $form_state['values']['layout'];
  }
}

/**
 * When a layout is changed, the user is given the opportunity to move content.
 */
function panels_panel_context_edit_move(&$form, &$form_state) {
  $form_state['display'] = &panels_panel_context_get_display($form_state['handler']);
  $form_state['layout'] = $form_state['handler']->conf['temp_layout'];

  ctools_include('common', 'panels');
  ctools_include('display-layout', 'panels');
  ctools_include('plugins', 'panels');

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form = array_merge($form, panels_change_layout($form_state));

  // Change the 'back' button to just go directly to the previous form
//  $task_id = $form_state['task']['name'];
//  $handler_id = $form_state['handler']->handler;
//  $name = $form_state['handler']->name;

  // This form is outside the normal wizard list, so we need to specify the
  // previous/next forms.
  $form['buttons']['previous']['#next'] = 'layout';
  $form['buttons']['next']['#next'] = 'content';

  $form_state['form_info']['return path'] = page_manager_edit_url($form_state['page']->task_name, array('handlers', $form_state['handler_id'], 'content'));
}

/**
 * Present the panels drag & drop editor to edit the display attached
 * to the task handler.
 */
function panels_panel_context_edit_content(&$form, &$form_state) {
  ctools_include('ajax');
  ctools_include('plugins', 'panels');
  ctools_include('common', 'panels');
  ctools_include('context');
  ctools_include('context-task-handler');

  $cache = panels_edit_cache_get('panel_context:' . $form_state['task_name'] . ':' . $form_state['handler_id']);

  $form_state['renderer'] = panels_get_renderer_handler('editor', $cache->display);
  $form_state['renderer']->cache = &$cache;

  $form_state['display'] = &$cache->display;
  $form_state['content_types'] = $cache->content_types;
  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;
  $form_state['display_title'] = !empty($cache->display_title);
  $form_state['no preview'] = TRUE;
  $form_state['page']->display_cache[$form_state['handler_id']] = $cache;

  $form = array_merge($form, panels_edit_display_form($form_state));
  // Make sure the theme will work since our form id is different.
  $form['#theme'] = 'panels_edit_display_form';

  if (!isset($form_state['type']) || $form_state['type'] != 'add' && !empty($form_state['handler_id']) && !empty($form['buttons'])) {
    $form['buttons']['preview'] = $form['buttons']['return'];
    $form['buttons']['preview']['#value'] = t('Update and preview');
  }
}

function panels_panel_context_edit_content_submit(&$form, &$form_state) {
  // Update the storage_id if this is a new variant before saving.
  if ($form_state['display']->storage_id == 'new') {
    $form_state['display']->storage_id = $form_state['handler_id'];
  }
  panels_edit_display_form_submit($form, $form_state);
  $handler = &$form_state['handler'];

  // update the cached display:
  $display = $form_state['page']->display_cache[$form_state['handler_id']]->display;
  $handler->conf['display'] = $display;
  unset($form_state['page']->display_cache[$form_state['handler_id']]);

  if ($form_state['clicked_button']['#value'] == t('Update and preview')) {
    $form_state['new trail'] = array('handlers', $form_state['handler_id'], 'preview');
  }
}

/**
 * General settings for the panel
 */
function panels_panel_context_edit_settings(&$form, &$form_state) {
  $conf = $form_state['handler']->conf;
  $form['conf']['title'] = array(
    '#type' => 'textfield',
    '#default_value' => $conf['title'],
    '#title' => t('Administrative title'),
    '#description' => t('Administrative title of this variant.'),
  );

  $form['conf']['no_blocks'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['no_blocks'],
    '#title' => t('Disable Drupal blocks/regions'),
    '#description' => t('Check this to have the page disable all regions displayed in the theme. Note that some themes support this setting better than others. If in doubt, try with stock themes to see.'),
  );

  ctools_include('plugins', 'panels');
  $pipelines = panels_get_renderer_pipelines();

  // Handle backward compatibility with the IPE checkbox.
  if (empty($conf['pipeline'])) {
    $conf['pipeline'] = !empty($conf['use_ipe']) ? 'ipe' : 'standard';
  }

  // If there are no pipelines, that probably means we're operating in
  // legacy mode.
  if (empty($pipelines)) {
    // We retain the original pipeline so we don't wreck things by installing
    // old modules.
    $form['conf']['pipeline'] = array(
      '#type' => 'value',
      '#value' => $conf['pipeline'],
    );
  }
  else {
    $options = array();
    foreach ($pipelines as $name => $pipeline) {
      $options[$name] = check_plain($pipeline->admin_title) . '<div class="description">' . check_plain($pipeline->admin_description) . '</div>';
    }

    $form['conf']['pipeline'] = array(
      '#type' => 'radios',
      '#options' => $options,
      '#title' => t('Renderer'),
      '#default_value' => $conf['pipeline'],
    );
  }

  $form['conf']['css_id'] = array(
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => $conf['css_id'],
    '#title' => t('CSS ID'),
    '#description' => t('The CSS ID to apply to this page'),
  );

  $form['conf']['css'] = array(
    '#type' => 'textarea',
    '#title' => t('CSS code'),
    '#description' => t('Enter well-formed CSS code here; this code will be embedded into the page, and should only be used for minor adjustments; it is usually better to try to put CSS for the page into the theme if possible. This CSS will be filtered for safety so some CSS may not work.'),
    '#default_value' => $conf['css'],
  );
}

/**
 * Submit handler for general settings form.
 */
function panels_panel_context_edit_settings_submit(&$form, &$form_state) {
  $form_state['handler']->conf['no_blocks'] = $form_state['values']['no_blocks'];
  $form_state['handler']->conf['pipeline'] = $form_state['values']['pipeline'];
  $form_state['handler']->conf['css_id'] = $form_state['values']['css_id'];
  $form_state['handler']->conf['css'] = $form_state['values']['css'];
  $form_state['handler']->conf['title'] = $form_state['values']['title'];

  // Unset the old checkbox so we don't store needless data.
  if (isset($form_state['handler']->conf['use_ipe'])) {
    unset($form_state['handler']->conf['use_ipe']);
  }
}

/**
 * Form to show a nice preview.
 */
function panels_panel_context_edit_preview(&$form, &$form_state) {
  ctools_include('context');
  ctools_include('context-task-handler');

  $contexts = ctools_context_handler_get_all_contexts($form_state['task'], $form_state['subtask'], $form_state['handler']);
  $form['preview'] = array();
  ctools_context_replace_form($form['preview'], $contexts);

  // automatically preview if there are no argument placeholders.
  if (empty($form['preview'])) {
    $display = panels_panel_context_get_display($form_state['handler']);
    $display->context = $contexts;
    $display->skip_cache = TRUE;
    $output = panels_render_display($display);
    if (isset($form['buttons'])) {
      unset($form['buttons']);
    }
  }
  else {
    $form['preview']['#tree'] = TRUE;
    $form_state['contexts'] = $contexts;
  }

  if (!empty($output)) {
    $form['output'] = array(
      '#value' => $output,
    );
  }

  $form_state['do not cache'] = TRUE;
}

/**
 * Display a preview upon submit if arguments were needed.
 */
function panels_panel_context_edit_preview_submit(&$form, &$form_state) {
  $display = panels_panel_context_get_display($form_state['handler']);
  $display->context = ctools_context_replace_placeholders($form_state['contexts'], $form_state['values']['preview']);

  $form_state['content'] = panels_render_display($display);
  $form_state['redirect'] = FALSE;
  $form_state['rerender'] = TRUE;
}
