<?php
// $Id: flag.admin.inc,v 1.1.4.6 2010/05/07 23:40:25 quicksketch Exp $

/**
 * @file
 * Contains administrative pages for creating, editing, and deleting flags.
 */

/**
 * Flag administration page. Display a list of existing flags.
 */
function flag_admin_page() {
  $flags = flag_get_flags();
  $default_flags = flag_get_default_flags(TRUE);
  return theme('flag_admin_page', $flags, $default_flags);
}

/**
 * Theme the output for the main flag administration page.
 */
function theme_flag_admin_page($flags, $default_flags) {
  $output = '<p>' . t('This page lists all the <em>flags</em> that are currently defined on this system. You may <a href="@add-url">add new flags</a>.', array('@add-url' => url('admin/build/flags/add'))) . '</p>';

  // Build out the list of normal, database flags.
  foreach ($flags as $flag) {
    $ops = theme('links', array(
      'flags_edit' =>  array('title' => t('edit'), 'href' => "admin/build/flags/edit/". $flag->name),
      'flags_delete' =>  array('title' => t('delete'), 'href' => "admin/build/flags/delete/". $flag->name),
    ));

    $roles = array_flip(array_intersect(array_flip(user_roles()), $flag->roles));
    $rows[] = array(
      $flag->name,
      $flag->content_type,
      empty($flag->roles) ? '<em>' . t('No roles') . '</em>' : implode(', ', $roles),
      $flag->types ? implode(', ', $flag->types) : '-',
      $flag->global ? t('Yes') : t('No'),
      $ops,
    );
  }
  if (!$flags) {
    $rows[] = array(
      array('data' => t('No flags are currently defined.'), 'colspan' => 6),
    );
  }

  $header = array(t('Flag'), t('Flag type'), t('Roles'), t('Node types'), t('Global?'), t('Operations'));
  $output .= theme('table', $header, $rows);

  // Build a list of disabled, module-based flags.
  $rows = array();
  foreach ($default_flags as $name => $flag) {
    if (!isset($flags[$name])) {
      $ops = theme('links', array(
        'flags_enable' =>  array('title' => t('enable'), 'href' => "admin/build/flags/edit/". $flag->name),
      ));

      $roles = array_flip(array_intersect(array_flip(user_roles()), $flag->roles));
      $rows[] = array(
        $flag->name,
        $flag->module,
        $flag->content_type,
        $ops,
      );
    }
  }

  if (!empty($rows)) {
    $header = array(t('Disabled Flags'), t('Module'), t('Flag type'), t('Operations'));
    $output .= theme('table', $header, $rows);
  }

  if (!module_exists('views')) {
    $output .= '<p>' . t('The <a href="@views-url">Views</a> module is not installed, or not enabled. It is recommended that you install the Views module to be able to easily produce lists of flagged content.', array('@views-url' => url('http://drupal.org/project/views'))) . '</p>';
  }
  else {
    $output .= '<p>';
    $output .= t('Lists of flagged content can be displayed using views. You can configure these in the <a href="@views-url">Views administration section</a>.', array('@views-url' => url('admin/build/views')));
    if (flag_get_flag('bookmarks')) {
      $output .= ' ' . t('Flag module automatically provides a few <a href="@views-url">default views for the <em>bookmarks</em> flag</a>. You can use these as templates by cloning these views and then customizing as desired.', array('@views-url' => url('admin/build/views', array('query' => 'tag=flag'))));
    }
    $output .= ' ' . t('The <a href="@flag-handbook-url">Flag module handbook</a> contains extensive <a href="@customize-url">documentation on creating customized views</a> using flags.', array('@flag-handbook-url' => 'http://drupal.org/handbook/modules/flag', '@customize-url' => 'http://drupal.org/node/296954'));
    $output .= '</p>';
  }

  if (!module_exists('flag_actions')) {
    $output .= '<p>' . t('Flagging an item may trigger <em>actions</em>. However, you don\'t have the <em>Flag actions</em> module <a href="@modules-url">enabled</a>, so you won\'t be able to enjoy this feature.', array('@actions-url' => url('admin/build/flags/actions'), '@modules-url' => url('admin/build/modules'))) . '</p>';
  }
  else {
    $output .= '<p>' . t('Flagging an item may trigger <a href="@actions-url">actions</a>.', array('@actions-url' => url('admin/build/flags/actions'))) . '</p>';
  }

  if (!module_exists('rules')) {
    $output .= '<p>' . t('Flagging an item may trigger <em>rules</em>. However, you don\'t have the <a href="@rule-url">Rules</a> module enabled, so you won\'t be able to enjoy this feature. The Rules module is a more extensive solution than Flag actions.', array('@rules-url' => url('http://drupal.org/node/407070'))) . '</p>';
  }
  else {
    $output .= '<p>' . t('Flagging an item may trigger <a href="@rules-url">rules</a>.', array('@rules-url' => url('admin/rules/trigger'))) . '</p>';
  }

  $output .= '<p>' . t('To learn about the various ways to use flags, please check out the <a href="@handbook-url">Flag module handbook</a>.', array('@handbook-url' => 'http://drupal.org/handbook/modules/flag')) . '</p>';

  return $output;
}

/**
 * Menu callback for adding a new flag.
 */
function flag_add_page($type = NULL, $name = NULL) {
  if (isset($type) && isset($name)) {
    return drupal_get_form('flag_form', $name, $type);
  }
  else {
    return drupal_get_form('flag_add_form');
  }
}

/**
 * Present a form for creating a new flag, setting the type of flag.
 */
function flag_add_form(&$form_state) {
  $form = array();

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag name'),
    '#description' => t('The machine-name for this flag. It may be up to 32 characters long and my only contain lowercase letters, underscores, and numbers. It will be used in URLs and in all API calls.'),
    '#maxlength' => 32,
    '#required' => TRUE,
  );

  $types = array();
  foreach (flag_fetch_definition() as $type => $info) {
    $types[$type] = $info['title'] . '<div class="description">' . $info['description'] . '</div>';
  }

  $form['type'] = array(
    '#type' => 'radios',
    '#title' => t('Flag type'),
    '#default_value' => 'node',
    '#description' => t('The type of content this flag will affect. An individual flag can only affect one type of content. This cannot be changed once the flag is created.'),
    '#required' => TRUE,
    '#options' => $types,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

function flag_add_form_validate($form, &$form_state) {
  $flag = flag_flag::factory_by_content_type($form_state['values']['type']);
  $flag->name = $form_state['values']['name'];
  $flag->validate_name();
}

function flag_add_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/flags/add/' . $form_state['values']['type'] . '/' . $form_state['values']['name'];
}

/**
 * Add/Edit flag page.
 */
function flag_form(&$form_state, $name, $type = NULL) {
  if (isset($type)) {
    // Adding a new flag.
    $flag = flag_flag::factory_by_content_type($type);
    $flag->name = $name;
    drupal_set_title(t('Add new flag'));
  }
  else {
    // Editing an existing flag.
    $flag = flag_get_flag($name);
    if (empty($flag)) {
      // Check if we're overriding a default flag.
      $default_flags = flag_get_default_flags(TRUE);
      if (isset($default_flags[$name])) {
        $flag = $default_flags[$name];
      }
      else {
        drupal_goto('admin/build/flags');
      }
    }
    drupal_set_title(t('Edit @title flag', array('@title' => $flag->get_title())));
  }

  $form['#flag'] = $flag;

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $flag->name,
    '#description' => t('The machine-name for this flag. It may be up to 32 characters long and my only contain lowercase letters, underscores, and numbers. It will be used in URLs and in all API calls.'),
    '#maxlength' => 32,
    '#required' => TRUE,
    '#access' => empty($flag->locked['name']),
  );

  if (!empty($flag->fid)) {
    $form['name']['#description'] .= ' <strong>'. t('Change this value only with great care.') .'</strong>';
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $flag->title,
    '#description' => t('A short, descriptive title for this flag. It will be used in administrative interfaces to refer to this flag, and in page titles and menu items of some <a href="@insite-views-url">views</a> this module provides (theses are customizable, though). Some examples could be <em>Bookmarks</em>, <em>Favorites</em>, or <em>Offensive</em>.', array('@insite-views-url' => url('admin/build/views'))),
    '#maxlength' => 255,
    '#required' => TRUE,
    '#access' => empty($flag->locked['title']),
  );

  $form['flag_short'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag link text'),
    '#default_value' => $flag->flag_short,
    '#description' => t('The text for the "flag this" link for this flag.'),
    '#required' => TRUE,
    '#access' => empty($flag->locked['flag_short']),
  );

  $form['flag_long'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag link description'),
    '#default_value' => $flag->flag_long,
    '#description' => t('The description of the "flag this" link. Usually displayed on mouseover.'),
    '#access' => empty($flag->locked['flag_long']),
  );

  $form['flag_confirmation'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag confirmation message'),
    '#default_value' => $flag->flag_confirmation,
    '#description' => t('Message displayed if the user has clicked the "flag this" link and confirmation is required. Usually presented in the form of a question such as, "Are you sure you want to flag this content?"'),
    '#access' => empty($flag->locked['flag_confirmation']),
  );

  $form['flag_message'] = array(
    '#type' => 'textfield',
    '#title' => t('Flagged message'),
    '#default_value' => $flag->flag_message,
    '#description' => t('Message displayed after flagging content. If JavaScript is enabled, it will be displayed below the link. If not, it will be displayed in the message area.'),
    '#access' => empty($flag->locked['flag_message']),
  );

  $form['unflag_short'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag link text'),
    '#default_value' => $flag->unflag_short,
    '#description' => t('The text for the "unflag this" link for this flag.'),
    '#required' => TRUE,
    '#access' => empty($flag->locked['unflag_short']),
  );

  $form['unflag_long'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag link description'),
    '#default_value' => $flag->unflag_long,
    '#description' => t('The description of the "unflag this" link. Usually displayed on mouseover.'),
    '#access' => empty($flag->locked['unflag_long']),
  );

  $form['unflag_confirmation'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag confirmation message'),
    '#default_value' => $flag->unflag_confirmation,
    '#description' => t('Message displayed if the user has clicked the "unflag this" link and confirmation is required. Usually presented in the form of a question such as, "Are you sure you want to unflag this content?"'),
    '#access' => empty($flag->locked['unflag_confirmation']),
  );

  $form['unflag_message'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflagged message'),
    '#default_value' => $flag->unflag_message,
    '#description' => t('Message displayed after content has been unflagged. If JavaScript is enabled, it will be displayed below the link. If not, it will be displayed in the message area.'),
    '#access' => empty($flag->locked['unflag_message']),
  );

  if (module_exists('token')) {
    $form['token_help'] = array(
      '#title' => t('Token replacement'),
      '#type' => 'fieldset',
      '#description' => t('The above six options may contain the following wildcard replacements. For example, "Mark Link" could be entered as "Add [title] to your flags" or "Add this [type-name] to your flags". These wildcards will be replaced with the appropriate field from the node.') . theme('flag_token_help', $flag->get_labels_token_types()),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
  }
  else {
    $form['token_help'] = array(
      '#value' => '<em>' . t('Note: You don\'t have the <a href="@token-url">Token</a> module installed. If you have it installed, and enabled, you\'ll be able to embed tokens in the six labels above.', array('@token-url' => 'http://drupal.org/project/token')) . '</em>',
    );
  }

  $form['global'] = array(
    '#type' => 'checkbox',
    '#title' => t('Global flag'),
    '#default_value' => $flag->global,
    '#description' => t('If checked, flag is considered "global" and each node is either flagged or not. If unchecked, each user has individual flags on content.'),
    '#weight' => 1,
    '#access' => empty($flag->locked['global']),
  );

  $form['roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Roles that may use this flag'),
    '#options' => user_roles(TRUE),
    '#default_value' => $flag->roles,
    '#description' => t('Checking <em>authenticated user</em> will allow all logged-in users to flag content with this flag. Anonymous users may not flag content.'),
    '#weight' => 5,
    '#access' => empty($flag->locked['roles']),
  );

  // Disabled access breaks checkboxes unless #value is hard coded.
  if (!empty($flag->locked['roles'])) {
    $form['roles']['#value'] = $flag->roles;
  }

  $form['types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('What nodes this flag may be used on'),
    '#options' => array_map('check_plain', node_get_types('names')),
    '#default_value' => $flag->types,
    '#description' => t('Check any node types that this flag may be used on. You must check at least one node type.'),
    '#required' => TRUE,
    '#weight' => 10,
    '#access' => empty($flag->locked['types']),
  );

  // Disabled access breaks checkboxes unless #value is hard coded.
  if (!empty($flag->locked['types'])) {
    $form['types']['#value'] = $flag->types;
  }

  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display options'),
    '#description' => t('Flags are usually controlled through links that allow users to toggle their behavior. You can choose how users interact with flags by changing options here. It is legitimate to have none of the following checkboxes ticked, if, for some reason, you wish <a href="@placement-url">to place the the links on the page yourself</a>.', array('@placement-url' => 'http://drupal.org/node/295383')),
    '#tree' => FALSE,
    '#weight' => 20,
  );

  $form['display']['link_type'] = array(
    '#type' => 'radios',
    '#title' => t('Link type'),
    '#options' => _flag_link_type_options(),
    '#default_value' => $flag->link_type,
    '#weight' => 2,
    '#access' => empty($flag->locked['link_type']),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    // We put this button on the form before calling $flag->options_form()
    // to give the flag handler a chance to remove it (e.g. flag_broken).
    '#weight' => 999,
  );

  $flag->options_form($form);

  return $form;
}

/**
 * Add/Edit flag form validate.
 */
function flag_form_validate($form, &$form_state) {
  $form_values = $form_state['values'];

  if ($form_values['link_type'] == 'confirm') {
    if (empty($form_values['flag_confirmation'])) {
      form_set_error('flag_confirmation', t('A flag confirmation message is required when using the confirmation link type.'));
    }
    if (empty($form_values['unflag_confirmation'])) {
      form_set_error('unflag_confirmation', t('An unflag confirmation message is required when using the confirmation link type.'));
    }
  }


  $flag = $form['#flag'];
  $flag->form_input($form_values);
  $flag->validate();
}

/**
 * Add/Edit flag form submit.
 */
function flag_form_submit($form, &$form_state) {
  $flag = $form['#flag'];
  $flag->form_input($form_state['values']);
  $flag->save();
  $flag->enable();
  drupal_set_message(t('Flag @name has been saved.', array('@name' => $flag->get_title())));
  _flag_clear_cache();
  $form_state['redirect'] = 'admin/build/flags';
}

/**
 * Delete flag page.
 */
function flag_delete_confirm(&$form_state, $name) {
  $flag = flag_get_flag($name);
  if (empty($flag)) {
    drupal_goto('admin/build/flags');
  }

  $form['fid'] = array('#type' => 'value', '#value' => $flag->fid);

  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $flag->get_title())),
    !empty($_GET['destination']) ? $_GET['destination'] : 'admin/build/flags',
    isset($flag->module) ? t('This flag is provided by the @module module. It will loose any customizations and be disabled.', array('@module' => node_get_types('name', $flag->module))) : t('This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

function flag_delete_confirm_submit($form, &$form_state) {
  $flag = flag_get_flag(NULL, $form_state['values']['fid']);
  if ($form_state['values']['confirm']) {
    $flag->delete();
    $flag->disable();
    _flag_clear_cache();
  }
  drupal_set_message(t('Flag @name has been deleted.', array('@name' => $flag->get_title())));
  $form_state['redirect'] = 'admin/build/flags';
}

/**
 * Clears various caches when a flag is modified.
 */
function _flag_clear_cache() {
  if (module_exists('views')) {
    views_invalidate_cache();
  }
  // Reset our flags cache, thereby making the following code aware of the
  // modifications.
  flag_get_flags(NULL, NULL, NULL, TRUE);
  // The title of a flag may appear in the menu (indirectly, via our "default
  // views"), so we need to clear the menu cache. This call also clears the
  // page cache, which is desirable too because the flag labels may have
  // changed.
  menu_rebuild();
}
