<?php

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
  $flag_admin_listing = drupal_get_form('flag_admin_listing', $flags);
  return theme('flag_admin_page', $flags, $default_flags, $flag_admin_listing);
}

/**
 * A form for ordering the weights of all the active flags in the system.
 */
function flag_admin_listing(&$form_state, $flags) {
  $form['#flags'] = $flags;
  $form['#tree'] = TRUE;

  foreach ($flags as $flag) {
    $form['flags'][$flag->name]['weight'] = array(
      '#type' => 'weight',
      '#delta' => count($flags) + 5,
      '#default_value' => $flag->weight,
      '#attributes' => array('class' => 'flag-weight'),
    );
  }

  if (count($flags) == 1) {
    // Don't show weights with only one flag.
    unset($form['flags'][$flag->name]['weight']);
  }
  else if (count($flags) > 1){
    // Only show the form button if there are several flags.
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save flag order'),
    );
  }

  return $form;
}

/**
 * Submit handler for the flag_admin_listing form. Save flag weight ordering.
 */
function flag_admin_listing_submit($form, &$form_state) {
  foreach ($form['#flags'] as $flag) {
    if ($flag->weight != $form_state['values']['flags'][$flag->name]['weight']) {
      $flag->weight = $form_state['values']['flags'][$flag->name]['weight'];
      $flag->save();
    }
  }
}

/**
 * Theme the output of the normal, database flags into a table.
 */
function theme_flag_admin_listing($form) {
  $flags = $form['#flags'];

  $output = '';

  foreach ($flags as $flag) {
    $ops = array(
      'flags_edit' =>  array('title' => t('edit'), 'href' => $flag->admin_path('edit')),
      'flags_delete' =>  array('title' => t('delete'), 'href' => $flag->admin_path('delete')),
      'flags_export' =>  array('title' => t('export'), 'href' => $flag->admin_path('export')),
    );

    $roles = array_flip(array_intersect(array_flip(user_roles()), $flag->roles['flag']));
    $row = array();
    $row[] = $flag->name;
    if (count($flags) > 1) {
      $row[] = drupal_render($form['flags'][$flag->name]['weight']);
    }
    $row[] = $flag->content_type;
    $row[] = empty($flag->roles['flag']) ? '<em>' . t('No roles') . '</em>' : implode(', ', $roles);
    $row[] = $flag->types ? implode(', ', $flag->types) : '-';
    $row[] = $flag->global ? t('Yes') : t('No');
    $row[] = theme('links', $ops);

    $rows[] = array(
      'data' => $row,
      'class' => 'draggable',
    );
  }
  if (!$flags) {
    $rows[] = array(
      array('data' => t('No flags are currently defined.'), 'colspan' => 7),
    );
  }
  else if (count($flags) > 1) {
    drupal_add_tabledrag('flag-admin-listing-table', 'order', 'sibling', 'flag-weight');
  }

  $header = array(t('Flag'));
  if (count($flags) > 1) {
    $header[] = t('Weight');
  }
  $header = array_merge($header, array(t('Flag type'), t('Roles'), t('Entity bundles'), t('Global?'), t('Operations')));
  $output .= theme('table', $header, $rows, array('id' => 'flag-admin-listing-table'));
  $output .= drupal_render($form);

  return $output;
}

/**
 * Theme the list of disabled flags into a table.
 */
function theme_flag_admin_listing_disabled($flags, $default_flags) {
  $output = '';

  // Build a list of disabled, module-based flags.
  $rows = array();
  foreach ($default_flags as $name => $flag) {
    if (!isset($flags[$name])) {
      $ops = array();
      if (!$flag->is_compatible()) {
        $flag_updates_needed = TRUE;
        $ops['flags_update'] = array('title' => '<strong>' . t('update code') . '</strong>', 'href' => $flag->admin_path('update'), 'html' => TRUE);
      }
      else {
        $ops['flags_enable'] = array('title' => t('enable'), 'href' => $flag->admin_path('edit'));
      }
      // $flag->roles['flag'] not exist on older flags.
      $roles = array_flip(array_intersect(array_flip(user_roles()), !empty($flag->roles['flag']) ? $flag->roles['flag'] : array()));
      $rows[] = array(
        $flag->name,
        $flag->module,
        $flag->content_type,
        theme('links', $ops),
      );
    }
  }

  if (isset($flag_updates_needed)) {
    drupal_set_message(t('Some flags provided by modules need to be updated to a new format before they can be used with this version of Flag. See the disabled flags for a list of flags that need updating.'), 'warning');
  }

  if (!empty($rows)) {
    $header = array(t('Disabled Flags'), t('Module'), t('Flag type'), t('Operations'));
    $output .= theme('table', $header, $rows);
  }

  return $output;
}

/**
 * Theme the output for the main flag administration page.
 */
function theme_flag_admin_page($flags, $default_flags, $flag_admin_listing) {
  $output = '';

  $output .= $flag_admin_listing;
  $output .= theme('flag_admin_listing_disabled', $flags, $default_flags);

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
    $output .= '<p>' . t('Flagging an item may trigger <em>rules</em>. However, you don\'t have the <a href="@rules-url">Rules</a> module enabled, so you won\'t be able to enjoy this feature. The Rules module is a more extensive solution than Flag actions.', array('@rules-url' => url('http://drupal.org/node/407070'))) . '</p>';
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
  drupal_set_title(t('Add new flag'));
  if (isset($type) && isset($name)) {
    $flag = flag_flag::factory_by_content_type($type);
    $flag->name = $name;
    return drupal_get_form('flag_form', $flag);
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
    '#description' => t('The machine-name for this flag. It may be up to 32 characters long and may only contain lowercase letters, underscores, and numbers. It will be used in URLs and in all API calls.'),
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
    '#value' => t('Add flag'),
  );

  return $form;
}

function flag_add_form_validate($form, &$form_state) {
  $flag = flag_flag::factory_by_content_type($form_state['values']['type']);
  $flag->name = $form_state['values']['name'];
  $errors = $flag->validate_name();
  foreach ($errors as $field => $field_errors) {
    foreach ($field_errors as $error) {
      form_set_error($field, $error['message']);
    }
  }
}

function flag_add_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/flags/add/' . $form_state['values']['type'] . '/' . $form_state['values']['name'];
}

/**
 * Add/Edit flag page.
 */
function flag_form(&$form_state, $flag) {
  $form['#flag'] = $flag;

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $flag->name,
    '#description' => t('The machine-name for this flag. It may be up to 32 characters long and may only contain lowercase letters, underscores, and numbers. It will be used in URLs and in all API calls.'),
    '#maxlength' => 32,
    '#required' => TRUE,
    '#access' => empty($flag->locked['name']),
    '#weight' => -3,
  );

  if (!empty($flag->fid)) {
    $form['name']['#description'] .= ' <strong>' . t('Change this value only with great care.') . '</strong>';
  }

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $flag->title,
    '#description' => t('A short, descriptive title for this flag. It will be used in administrative interfaces to refer to this flag, and in page titles and menu items of some <a href="@insite-views-url">views</a> this module provides (theses are customizable, though). Some examples could be <em>Bookmarks</em>, <em>Favorites</em>, or <em>Offensive</em>.', array('@insite-views-url' => url('admin/build/views'))),
    '#maxlength' => 255,
    '#required' => TRUE,
    '#access' => empty($flag->locked['title']),
    '#weight' => -2,
  );

  $form['global'] = array(
    '#type' => 'checkbox',
    '#title' => t('Global flag'),
    '#default_value' => $flag->global,
    '#description' => t('If checked, flag is considered "global" and each node is either flagged or not. If unchecked, each user has individual flags on content.'),
    '#access' => empty($flag->locked['global']),
    '#weight' => -1,
  );

  $form['messages'] = array(
    '#type' => 'fieldset',
    '#title' => t('Messages'),
  );

  $form['messages']['flag_short'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag link text'),
    '#default_value' => $flag->flag_short,
    '#description' => t('The text for the "flag this" link for this flag.'),
    '#required' => TRUE,
    '#access' => empty($flag->locked['flag_short']),
  );

  $form['messages']['flag_long'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag link description'),
    '#default_value' => $flag->flag_long,
    '#description' => t('The description of the "flag this" link. Usually displayed on mouseover.'),
    '#access' => empty($flag->locked['flag_long']),
  );

  $form['messages']['flag_message'] = array(
    '#type' => 'textfield',
    '#title' => t('Flagged message'),
    '#default_value' => $flag->flag_message,
    '#description' => t('Message displayed after flagging content. If JavaScript is enabled, it will be displayed below the link. If not, it will be displayed in the message area.'),
    '#access' => empty($flag->locked['flag_message']),
  );

  $form['messages']['unflag_short'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag link text'),
    '#default_value' => $flag->unflag_short,
    '#description' => t('The text for the "unflag this" link for this flag.'),
    '#required' => TRUE,
    '#access' => empty($flag->locked['unflag_short']),
  );

  $form['messages']['unflag_long'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag link description'),
    '#default_value' => $flag->unflag_long,
    '#description' => t('The description of the "unflag this" link. Usually displayed on mouseover.'),
    '#access' => empty($flag->locked['unflag_long']),
  );

  $form['messages']['unflag_message'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflagged message'),
    '#default_value' => $flag->unflag_message,
    '#description' => t('Message displayed after content has been unflagged. If JavaScript is enabled, it will be displayed below the link. If not, it will be displayed in the message area.'),
    '#access' => empty($flag->locked['unflag_message']),
  );

  if (module_exists('token')) {
    $form['messages']['tokens_help'] = array(
      '#title' => t('Token replacement'),
      '#type' => 'fieldset',
      '#description' =>
        '<p>' . t('The above six texts may contain any of the tokens listed below. For example, <em>"Flag link text"</em> could be entered as:') . '</p>' .
        theme('item_list', array(
          t('Add &lt;em&gt;[title]&lt;/em&gt; to your favorites'),
          t('Add this [type] to your favorites'),
          t('Vote for this proposal ([flag-vote-count] people have already done so)'),
        ), NULL, 'ul', array('class' => 'token-examples')) .
        '<p>' . t('These tokens will be replaced with the appropriate fields from the node (or user, or comment).') . '</p>' .
        theme('flag_tokens_browser', $flag->get_labels_token_types()),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
  }
  else {
    $form['messages']['token_help'] = array(
      '#value' => '<em>' . t('Note: You don\'t have the <a href="@token-url">Token</a> module installed. If you have it installed, and enabled, you\'ll be able to embed tokens in the six labels above.', array('@token-url' => 'http://drupal.org/project/token')) . '</em>',
    );
  }

  $form['access'] = array(
    '#type' => 'fieldset',
    '#title' => t('Flag access'),
    '#tree' => FALSE,
    '#weight' => 10,
  );

  $form['access']['types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Flaggable content'),
    '#options' => array_map('check_plain', node_get_types('names')),
    '#default_value' => $flag->types,
    '#description' => t('Check any node types that this flag may be used on. You must check at least one node type.'),
    '#required' => TRUE,
    '#weight' => 10,
    '#access' => empty($flag->locked['types']),
  );

  // Disabled access breaks checkboxes unless #value is hard coded.
  if (!empty($flag->locked['types'])) {
    $form['access']['types']['#value'] = $flag->types;
  }

  $form['access']['roles'] = array(
    '#title' => t('Roles that may use this flag'),
    '#access' => empty($flag->locked['roles']),
    '#description' => t('Users may only unflag content if they have access to flag the content initially. Checking <em>authenticated user</em> will allow access for all logged-in users.'),
    '#theme' => 'flag_form_roles',
    '#weight' => -2,
  );
  if (module_exists('session_api')) {
    $form['access']['roles']['#description'] .= ' ' . t('Support for anonymous users is being provided by <a href="http://drupal.org/project/session_api">Session API</a>.');
  }
  else {
    $form['access']['roles']['#description'] .= ' ' . t('Anonymous users may flag content if the <a href="http://drupal.org/project/session_api">Session API</a> module is installed.');
  }

  $form['access']['roles']['flag'] = array(
    '#type' => 'checkboxes',
    '#options' => user_roles(!module_exists('session_api')),
    '#default_value' => $flag->roles['flag'],
    '#parents' => array('roles', 'flag'),
  );
  $form['access']['roles']['unflag'] = array(
    '#type' => 'checkboxes',
    '#options' => user_roles(!module_exists('session_api')),
    '#default_value' => $flag->roles['unflag'],
    '#parents' => array('roles', 'unflag'),
  );

  // Disabled access breaks checkboxes unless #value is hard coded.
  if (!empty($flag->locked['roles'])) {
    $form['access']['roles']['#type'] = 'value';
    $form['access']['roles']['#value'] = $flag->roles;
  }

  $form['access']['unflag_denied_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag not allowed text'),
    '#default_value' => $flag->unflag_denied_text,
    '#description' => t('If a user is allowed to flag but not unflag, this text will be displayed after flagging. Often this is the past-tense of the link text, such as "flagged".'),
    '#access' => empty($flag->locked['unflag_denied_text']),
    '#weight' => -1,
  );

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
    '#option_descriptions' => _flag_link_type_descriptions(),
    '#after_build' => array('flag_expand_link_option', 'flag_check_link_types'),
    '#default_value' => $flag->link_type,
    '#weight' => 2,
    '#access' => empty($flag->locked['link_type']),
  );

  $form['display']['link_options_intro'] = array(
    '#value' => '<p id="link-options-intro">' . t('The selected link type may require these additional settings:') . '</p>',
    '#weight' => 20,
  );

  $form['display']['link_options_confirm'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options for the "Confirmation form" link type'),
    // Any "link type" provider module must put its settings fields inside
    // a fieldset whose HTML ID is link-options-LINKTYPE, where LINKTYPE is
    // the machine-name of the link type. This is necessary for the
    // radiobutton's JavaScript dependency feature to work.
    '#attributes' => array('id' => 'link-options-confirm'),
    '#weight' => 21,
  );

  $form['display']['link_options_confirm']['flag_confirmation'] = array(
    '#type' => 'textfield',
    '#title' => t('Flag confirmation message'),
    '#default_value' => isset($flag->flag_confirmation) ? $flag->flag_confirmation : '',
    '#description' => t('Message displayed if the user has clicked the "flag this" link and confirmation is required. Usually presented in the form of a question such as, "Are you sure you want to flag this content?"'),
    '#access' => empty($flag->locked['flag_confirmation']),
  );

  $form['display']['link_options_confirm']['unflag_confirmation'] = array(
    '#type' => 'textfield',
    '#title' => t('Unflag confirmation message'),
    '#default_value' => isset($flag->unflag_confirmation) ? $flag->unflag_confirmation : '',
    '#description' => t('Message displayed if the user has clicked the "unflag this" link and confirmation is required. Usually presented in the form of a question such as, "Are you sure you want to unflag this content?"'),
    '#access' => empty($flag->locked['unflag_confirmation']),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save flag'),
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
  $errors = $flag->validate();
  foreach ($errors as $field => $field_errors) {
    foreach ($field_errors as $error) {
      form_set_error($field, $error['message']);
    }
  }
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
 * Output the access options for roles in a table.
 */
function theme_flag_form_roles($element) {
  drupal_add_css(drupal_get_path('module', 'flag') . '/theme/flag-admin.css', 'module', 'all', FALSE);
  drupal_add_js(drupal_get_path('module', 'flag') . '/theme/flag-admin.js', 'module', 'header', FALSE, TRUE, FALSE);

  $header = array(
    array('class' => 'checkbox', 'data' => t('Flag')),
    array('class' => 'checkbox', 'data' => t('Unflag')),
    t('Role'),
  );
  $rows = array();
  foreach (element_children($element['flag']) as $role) {
    $row = array();
    $role_name = $element['flag'][$role]['#title'];
    unset($element['flag'][$role]['#title']);
    unset($element['unflag'][$role]['#title']);
    $element['flag'][$role]['#attributes']['class'] = 'flag-access';
    $element['unflag'][$role]['#attributes']['class'] = 'unflag-access';
    $row[] = array('class' => 'checkbox', 'data' => drupal_render($element['flag'][$role]));
    $row[] = array('class' => 'checkbox', 'data' => drupal_render($element['unflag'][$role]));
    $row[] = $role_name;
    $rows[] = $row;
  }

  $element['#children'] = theme('table', $header, $rows, array('class' => 'flag-admin-table', 'id' => 'flag-roles'));
  return theme('form_element', $element, $element['#children']);
}

/**
 * Delete flag page.
 */
function flag_delete_confirm(&$form_state, $flag) {
  $form['fid'] = array('#type' => 'value', '#value' => $flag->fid);

  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $flag->get_title())),
    !empty($_GET['destination']) ? $_GET['destination'] : 'admin/build/flags',
    isset($flag->module) ? t('This flag is provided by the %module module. It will lose any customizations and be disabled.', array('%module' => $flag->module)) : t('This action cannot be undone.'),
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
 * FormAPI after_build function to add descriptions to radio buttons.
 */
function flag_expand_link_option($element) {
  drupal_add_js(drupal_get_path('module', 'flag') . '/theme/flag-admin.js', 'module', 'header', FALSE, TRUE, FALSE);

  foreach (element_children($element) as $key) {
    // Add a description to the link option.
    if (isset($element['#option_descriptions'][$key])) {
      $element[$key]['#description'] = $element['#option_descriptions'][$key];
    }
  }
  $element['#attributes']['class'] = 'flag-link-options';

  return $element;
}

/**
 * FormAPI after_build function to check that the link type exists.
 */
function flag_check_link_types($element) {
  $link_types = flag_get_link_types();
  if (!isset($link_types[$element['#value']])) {
    drupal_set_message(t('This flag uses a link type of %type, which does not exist.', array('%type' => $element['#value'])), 'error');
  }
  return $element;
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
