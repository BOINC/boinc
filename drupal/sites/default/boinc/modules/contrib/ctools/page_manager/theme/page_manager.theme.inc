<?php
// $Id

/**
 * @file
 * Preprocess functions for page manager editing templates.
 */

/**
 * Preprocess the page manager edit page.
 */
function template_preprocess_page_manager_edit_page(&$vars) {
  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();
  ctools_add_js('dependent');

  drupal_add_css(drupal_get_path('module', 'page_manager') . '/css/page-manager.css');
  ctools_add_css('wizard');

  $task = $vars['page']->task;
  $task_handler_plugins = page_manager_get_task_handler_plugins($task);
  foreach ($task_handler_plugins as $id => $plugin) {
    if (isset($plugin['admin css'])) {
      foreach ($plugin['admin css'] as $file) {
        drupal_add_css($file);
      }
    }
    if (isset($plugin['admin js'])) {
      foreach ($plugin['admin js'] as $file) {
        drupal_add_js($file);
      }
    }
  }

  $page = &$vars['page'];

  $vars['locked'] = '';
  $vars['changed'] = '';
  if (!empty($page->locked)) {
    $vars['locked'] = theme('page_manager_lock', $page);
    $vars['changed'] = theme('page_manager_changed', t('Locked'), t('This page is being edited by another user and you cannot make changes to it.'));
  }
  else if (!empty($page->new)) {
    $vars['changed'] = theme('page_manager_changed', t('New'), t('This page is newly created and has not yet been saved to the database. It will not be available until you save it.'));
  }
  else if (!empty($page->changed)) {
    $vars['changed'] = theme('page_manager_changed', t('Changed'), t('This page has been modified, but these modifications are not yet live. While modifying this page, it is locked from modification by other users.'));
  }

  $form_state = array(
    'page' => &$vars['page'],
  );

  $active = $vars['content']['active'];
  if ($active[0] == 'handlers' && isset($vars['operations'][$active[1]])) {
    $vars['operations']['secondary'] = $vars['operations'][$active[1]];
  }
}

/**
 * Remove some items from the form so they don't submit.
 */
function theme_page_manager_list_pages_form($form) {
  // Don't render these:
/*
  unset($form['form_id']);
  unset($form['form_build_id']);
  unset($form['form_token']);
*/
  return '<div class="clear-block">' . drupal_render($form) . '</div>';
}

/**
 * Turn the rearrange form into a table with tablesorting on.
 */
function theme_page_manager_handler_rearrange($form) {
  // Assemble the data for a table from everything in $form['handlers']
  foreach (element_children($form['handlers']) as $id) {
    // provide a reference shortcut.
    $element = &$form['handlers'][$id];
    if (isset($element['title'])) {
      $row = array();

      $row[] = array(
        'data' => drupal_render($element['title']),
        'class' => 'page-manager-handler',
      );

      $element['weight']['#attributes']['class'] = 'weight';
      $row[] = drupal_render($element['weight']);

      $rows[] = array('data' => $row, 'class' => 'draggable', 'id' => 'page-manager-row-' . $id);
    }
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No task handlers are defined for this task.'), 'colspan' => '5'));
  }

  $header = array(
    array('data' => t('Variant'), 'class' => 'page-manager-handler'),
    t('Weight'),
  );

  drupal_add_tabledrag('page-manager-arrange-handlers', 'order', 'sibling', 'weight');

  $output = theme('table', $header, $rows, array('id' => 'page-manager-arrange-handlers'));
  $output .= drupal_render($form);
  return $output;
}

/**
 * Draw the "this task is locked from editing" box.
 */
function theme_page_manager_lock($page) {
  $account  = user_load($page->locked->uid);
  $name     = theme('username', $account);
  $lock_age = format_interval(time() - $page->locked->updated);
  $break    = url(page_manager_edit_url($page->task_name, array('actions', 'break-lock')));

  ctools_add_css('ctools');
  $output = '<div class="ctools-locked">';
  $output .= t('This page is being edited by user !user, and is therefore locked from editing by others. This lock is !age old. Click here to <a href="!break">break this lock</a>.', array('!user' => $name, '!age' => $lock_age, '!break' => $break));
  $output .= '</div>';
  return $output;
}

/**
 * Draw the "you have unsaved changes and this task is locked." message.
 */
function theme_page_manager_changed($text, $description) {
  ctools_add_css('ctools');
  $output = '<div class="page-manager-changed" title="' . $description . '">';
  $output .= $text;
  $output .= '</div>';

  return $output;
}
