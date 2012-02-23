<?php
// $Id: content.admin.inc,v 1.181.2.76 2009/11/02 21:21:24 markuspetrux Exp $

/**
 * @file
 * Administrative interface for content type creation.
 */


/**
 * Menu callback; replacement for node_overview_types().
 */
function content_types_overview() {
  $types = node_get_types();
  $names = node_get_types('names');
  $header = array(t('Name'), t('Type'), t('Description'), array('data' => t('Operations'), 'colspan' => '4'),);
  $rows = array();

  foreach ($names as $key => $name) {
    $type = $types[$key];
    if (node_hook($type, 'form')) {
      $type_url_str = str_replace('_', '-', $type->type);
      $row = array(
        check_plain($name),
        check_plain($type->type),
      );
      // Make the description smaller
      $row[] = array('data' => filter_xss_admin($type->description), 'class' => 'description');
      // Set the edit column.
      $row[] = array('data' => l(t('edit'), 'admin/content/node-type/'. $type_url_str));
      // Set links for managing fields.
      // TODO: a hook to allow other content modules to add more stuff?
      $row[] = array('data' => l(t('manage fields'), 'admin/content/node-type/'. $type_url_str .'/fields'));
      // Set the delete column.
      if ($type->custom) {
        $row[] = array('data' => l(t('delete'), 'admin/content/node-type/'. $type_url_str .'/delete'));
      }
      else {
        $row[] = array('data' => '');
      }

      $rows[] = $row;
    }
  }

  // Allow external modules alter the table headers and rows.
  foreach (module_implements('content_types_overview_alter') as $module) {
    $function = $module .'_content_types_overview_alter';
    $function($header, $rows);
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No content types available.'), 'colspan' => '7', 'class' => 'message'));
  }

  return theme('table', $header, $rows) .theme('content_overview_links');
}

function theme_content_overview_links() {
  return '<div class="content-overview-links">'. l(t('» Add a new content type'), 'admin/content/types/add') .'</div>';
}

/**
 * Menu callback; lists all defined fields for quick reference.
 */
function content_fields_list() {
  $fields = content_fields();
  $field_types = _content_field_types();

  // Sort fields by field name.
  ksort($fields);

  $header = array(t('Field name'), t('Field type'), t('Used in'));
  $rows = array();
  foreach ($fields as $field) {
    $row = array();
    $row[] = $field['locked'] ? t('@field_name (Locked)', array('@field_name' => $field['field_name'])) : $field['field_name'];
    $row[] = t($field_types[$field['type']]['label']);

    $types = array();
    $result = db_query("SELECT nt.name, nt.type FROM {". content_instance_tablename() ."} nfi ".
    "LEFT JOIN {node_type} nt ON nt.type = nfi.type_name ".
    "WHERE nfi.field_name = '%s' ".
    // Keep disabled modules out of table.
    "AND nfi.widget_active = 1 ".
    "ORDER BY nt.name ASC", $field['field_name']);
    while ($type = db_fetch_array($result)) {
      $content_type = content_types($type['type']);
      $types[] = l($type['name'], 'admin/content/node-type/'. $content_type['url_str'] .'/fields');
    }
    $row[] = implode(', ', $types);

    $rows[] = array('data' => $row, 'class' => $field['locked'] ? 'menu-disabled' : '');
  }
  if (empty($rows)) {
    $output = t('No fields have been defined for any content type yet.');
  }
  else {
    $output = theme('table', $header, $rows);
  }
  return $output;
}

/**
 * Helper function to display a message about inactive fields.
 */
function content_inactive_message($type_name) {
  $inactive_fields = content_inactive_fields($type_name);
  if (!empty($inactive_fields)) {
    $field_types = _content_field_types();
    $widget_types = _content_widget_types($type_name);
    drupal_set_message(t('This content type has inactive fields. Inactive fields are not included in lists of available fields until their modules are enabled.'), 'error');
    foreach ($inactive_fields as $field_name => $field) {
      drupal_set_message(t('!field (!field_name) is an inactive !field_type field that uses a !widget_type widget.', array(
      '!field' => $field['widget']['label'],
      '!field_name' => $field['field_name'],
      '!field_type' => array_key_exists($field['type'], $field_types) ? $field_types[$field['type']]['label'] : $field['type'],
      '!widget_type' => array_key_exists($field['widget']['type'], $widget_types) ? $widget_types[$field['widget']['type']]['label'] : $field['widget']['type'],
      )));
    }
  }
}

/**
 * Menu callback; listing of fields for a content type.
 *
 * Allows fields to be reordered and nested in fieldgroups using
 * JS drag-n-drop. Non-CCK form elements can also be moved around.
 */
function content_field_overview_form(&$form_state, $type_name) {

  content_inactive_message($type_name);

  // When displaying the form, make sure the list of fields
  // is up-to-date.
  if (empty($form_state['post'])) {
    content_clear_type_cache();
  }

  // Gather type information.
  $type = content_types($type_name);
  $fields = $type['fields'];
  $field_types = _content_field_types();

  $extra = $type['extra'];
  $groups = $group_options = $group_types = array();
  if (module_exists('fieldgroup')) {
    $groups = fieldgroup_groups($type['type']);
    $group_types = fieldgroup_types();
    $group_options = _fieldgroup_groups_label($type['type']);
    // Add the ability to group under the newly created row.
    $group_options['_add_new_group'] = '_add_new_group';
  }

  // Store the default weights as we meet them, to be able to put the
  //'add new' rows after them.
  $weights = array();

  $form = array(
    '#tree' => TRUE,
    '#type_name' => $type['type'],
    '#fields' => array_keys($fields),
    '#groups' => array_keys($groups),
    '#extra' => array_keys($extra),
    '#field_rows' => array(),
    '#group_rows' => array(),
  );

  // Fields.
  foreach ($fields as $name => $field) {
    $weight = $field['widget']['weight'];
    $form[$name] = array(
      'label' => array('#value' => check_plain($field['widget']['label'])),
      'field_name' => array('#value' => $field['field_name']),
      'type' => array('#value' => t($field_types[$field['type']]['label'])),
      'configure' => array('#value' => l(t('Configure'), 'admin/content/node-type/'. $type['url_str'] .'/fields/'. $field['field_name'])),
      'remove' => array('#value' => l(t('Remove'), 'admin/content/node-type/'. $type['url_str'] .'/fields/'. $field['field_name'] .'/remove')),
      'weight' => array('#type' => 'textfield', '#default_value' => $weight, '#size' => 3),
      'parent' => array('#type' => 'select', '#options' => $group_options, '#default_value' => ''),
      'prev_parent' => array('#type' => 'hidden', '#value' => ''),
      'hidden_name' => array('#type' => 'hidden', '#default_value' => $field['field_name']),
      '#leaf' => TRUE,
      '#row_type' => 'field',
      'field' =>  array('#type' => 'value', '#value' => $field),
    );
    if ($field['locked']) {
      $form[$name]['configure'] = array('#value' => t('Locked'));
      $form[$name]['remove'] = array();
      $form[$name]['#disabled_row'] = TRUE;
    }
    $form['#field_rows'][] = $name;
    $weights[] = $weight;
  }

  // Groups.
  foreach ($groups as $name => $group) {
    $weight = $group['weight'];
    $form[$name] = array(
      'label' => array('#value' => check_plain($group['label'])),
      'group_name' => array('#value' => $group['group_name']),
      'group_type' => array('#value' => t($group_types[$group['group_type']])),
      'configure' => array('#value' => l(t('Configure'), 'admin/content/node-type/'. $type['url_str'] .'/groups/'. $group['group_name'])),
      'remove' => array('#value' => l(t('Remove'), 'admin/content/node-type/'. $type['url_str'] .'/groups/'. $group['group_name'] .'/remove')),
      'weight' => array('#type' => 'textfield', '#default_value' => $weight, '#size' => 3),
      'parent' => array('#type' => 'hidden', '#default_value' => ''),
      'hidden_name' => array('#type' => 'hidden', '#default_value' => $group['group_name']),
      '#root' => TRUE,
      '#row_type' => 'group',
      'group' => array('#type' => 'value', '#value' => $group),
    );
    // Adjust child fields rows.
    foreach ($group['fields'] as $field_name => $field) {
      $form[$field_name]['parent']['#default_value'] = $name;
      $form[$field_name]['prev_parent']['#value'] = $name;
    }
    $form['#group_rows'][] = $name;
    $weights[] = $weight;
  }

  // Non-CCK 'fields'.
  foreach ($extra as $name => $label) {
    $weight = $extra[$name]['weight'];
    $form[$name] = array(
      'label' => array('#value' => check_plain(t($extra[$name]['label']))),
      'description' => array('#value' => isset($extra[$name]['description']) ? $extra[$name]['description'] : ''),
      'weight' => array('#type' => 'textfield', '#default_value' => $weight, '#size' => 3),
      'parent' => array('#type' => 'hidden', '#default_value' => ''),
      'configure' => array('#value' => isset($extra[$name]['configure']) ? $extra[$name]['configure'] : ''),
      'remove' => array('#value' => isset($extra[$name]['remove']) ? $extra[$name]['remove'] : ''),
      'hidden_name' => array('#type' => 'hidden', '#default_value' => $name),
      '#leaf' => TRUE,
      '#root' => TRUE,
      '#disabled_row' => TRUE,
      '#row_type' => 'extra',
    );
    $form['#field_rows'][] = $name;
    $weights[] = $weight;
  }

  // Additional row : add new field.
  $weight = max($weights) + 1;
  $field_type_options = content_field_type_options();
  $widget_type_options = content_widget_type_options(NULL, TRUE);
  if ($field_type_options && $widget_type_options) {
    array_unshift($field_type_options, t('- Select a field type -'));
    array_unshift($widget_type_options, t('- Select a widget -'));
    $name = '_add_new_field';
    $form[$name] = array(
      'label' => array(
        '#type' => 'textfield',
        '#size' => 15,
        '#description' => t('Label'),
      ),
      'field_name' => array(
        '#type' => 'textfield',
        // This field should stay LTR even for RTL languages.
        '#field_prefix' => '<span dir="ltr">field_',
        '#field_suffix' => '</span>&lrm;',
        '#attributes' => array('dir'=>'ltr'),
        '#size' => 15,
        // Field names are limited to 32 characters including the 'field_'
        // prefix which is 6 characters long.
        '#maxlength' => 26,
        '#description' => t('Field name (a-z, 0-9, _)'),
      ),
      'type' => array(
        '#type' => 'select',
        '#options' => $field_type_options,
        '#description' => theme('advanced_help_topic', 'content', 'fields') . t('Type of data to store.'),
      ),
      'widget_type' => array(
        '#type' => 'select',
        '#options' => $widget_type_options,
        '#description' => t('Form element to edit the data.'),
      ),
      'weight' => array('#type' => 'textfield', '#default_value' => $weight, '#size' => 3),
      'parent' => array('#type' => 'select', '#options' => $group_options, '#default_value' => ''),
      'hidden_name' => array('#type' => 'hidden', '#default_value' => $name),
      '#leaf' => TRUE,
      '#add_new' => TRUE,
      '#row_type' => 'add_new_field',
    );
    $form['#field_rows'][] = $name;
  }

  // Additional row : add existing field.
  $existing_field_options = content_existing_field_options($type_name);
  if ($existing_field_options && $widget_type_options) {
    $weight++;
    array_unshift($existing_field_options, t('- Select an existing field -'));
    $name = '_add_existing_field';
    $form[$name] = array(
      'label' => array(
        '#type' => 'textfield',
        '#size' => 15,
        '#description' => t('Label'),
      ),
      'field_name' => array(
        '#type' => 'select',
        '#options' => $existing_field_options,
        '#description' => t('Field to share'),
      ),
      'widget_type' => array(
        '#type' => 'select',
        '#options' => $widget_type_options,
        '#description' => t('Form element to edit the data.'),
      ),
      'weight' => array('#type' => 'textfield', '#default_value' => $weight, '#size' => 3),
      'parent' => array('#type' => 'select', '#options' => $group_options, '#default_value' => ''),
      'hidden_name' => array('#type' => 'hidden', '#default_value' => $name),
      '#leaf' => TRUE,
      '#add_new' => TRUE,
      '#row_type' => 'add_existing_field',
    );
    $form['#field_rows'][] = $name;
  }

  // Additional row : add new group.
  if (!empty($group_types)) {
    $weight++;
    $name = '_add_new_group';
    $form[$name] = array(
      'label' => array(
        '#type' => 'textfield',
        '#size' => 15,
        '#description' => t('Label'),
      ),
      'group_name' => array(
        '#type' => 'textfield',
        // This field should stay LTR even for RTL languages.
        '#field_prefix' => '<span dir="ltr">group_',
        '#field_suffix' => '</span>&lrm;',
        '#attributes' => array('dir'=>'ltr'),
        '#size' => 15,
        // Group names are limited to 32 characters including the 'group_'
        // prefix which is 6 characters long.
        '#maxlength' => 26,
        '#description' => t('Group name (a-z, 0-9, _)'),
      ),
      'group_option' => array(
        '#type' => 'hidden',
        '#value' => '',
      ),
      'group_type' => array(
        '#type' => 'hidden',
        '#value' => 'standard',
      ),
      'weight' => array('#type' => 'textfield', '#default_value' => $weight, '#size' => 3),
      'parent' => array('#type' => 'hidden', '#default_value' => ''),
      'hidden_name' => array('#type' => 'hidden', '#default_value' => $name),
      '#root' => TRUE,
      '#add_new' => TRUE,
      '#row_type' => 'add_new_group',
    );
    if (count($group_types) > 1) {
      $form[$name]['group_type'] = array(
        '#type' => 'select',
        '#description' => t('Type of group.'),
        '#options' => $group_types,
        '#default_value' => 'standard',
      );
    }
    $form['#group_rows'][] = $name;
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  return $form;
}

function content_field_overview_form_validate($form, &$form_state) {
  _content_field_overview_form_validate_add_new($form, $form_state);
  _content_field_overview_form_validate_add_existing($form, $form_state);
}

/**
 * Helper function for content_field_overview_form_validate.
 *
 * Validate the 'add new field' row.
 */
function _content_field_overview_form_validate_add_new($form, &$form_state) {
  $field = $form_state['values']['_add_new_field'];

  // Validate if any information was provided in the 'add new field' row.
  if (array_filter(array($field['label'], $field['field_name'], $field['type'], $field['widget_type']))) {
    // No label.
    if (!$field['label']) {
      form_set_error('_add_new_field][label', t('Add new field: you need to provide a label.'));
    }

    // No field name.
    if (!$field['field_name']) {
      form_set_error('_add_new_field][field_name', t('Add new field: you need to provide a field name.'));
    }
    // Field name validation.
    else {
      $field_name = $field['field_name'];

      // Add the 'field_' prefix.
      if (substr($field_name, 0, 6) != 'field_') {
        $field_name = 'field_'. $field_name;
        form_set_value($form['_add_new_field']['field_name'], $field_name, $form_state);
      }

      // Invalid field name.
      if (!preg_match('!^field_[a-z0-9_]+$!', $field_name)) {
        form_set_error('_add_new_field][field_name', t('Add new field: the field name %field_name is invalid. The name must include only lowercase unaccentuated letters, numbers, and underscores.', array('%field_name' => $field_name)));
      }
      if (strlen($field_name) > 32) {
        form_set_error('_add_new_field][field_name', t('Add new field: the field name %field_name is too long. The name is limited to 32 characters, including the \'field_\' prefix.', array('%field_name' => $field_name)));
      }
      // A field named 'field_instance' would cause a tablename clash with {content_field_instance}
      if ($field_name == 'field_instance') {
        form_set_error('_add_new_field][field_name', t("Add new field: the name 'field_instance' is a reserved name."));
      }

      // Field name already exists.
      // We need to check inactive fields as well, so we can't use content_fields().
      module_load_include('inc', 'content', 'includes/content.crud');
      $fields = content_field_instance_read(array(), TRUE);
      $used = FALSE;
      foreach ($fields as $existing_field) {
        $used |= ($existing_field['field_name'] == $field_name);
      }
      if ($used) {
        form_set_error('_add_new_field][field_name', t('Add new field: the field name %field_name already exists.', array('%field_name' => $field_name)));
      }
    }

    // No field type.
    if (!$field['type']) {
      form_set_error('_add_new_field][type', t('Add new field: you need to select a field type.'));
    }

    // No widget type.
    if (!$field['widget_type']) {
      form_set_error('_add_new_field][widget_type', t('Add new field: you need to select a widget.'));
    }
    // Wrong widget type.
    elseif ($field['type']) {
      $widget_types = content_widget_type_options($field['type']);
      if (!isset($widget_types[$field['widget_type']])) {
        form_set_error('_add_new_field][widget_type', t('Add new field: invalid widget.'));
      }
    }
  }
}

/**
 * Helper function for content_field_overview_form_validate.
 *
 * Validate the 'add existing field' row.
 */
function _content_field_overview_form_validate_add_existing($form, &$form_state) {
  // The form element might be absent if no existing fields can be added to
  // this content type
  if (isset($form_state['values']['_add_existing_field'])) {
    $field = $form_state['values']['_add_existing_field'];

    // Validate if any information was provided in the 'add existing field' row.
    if (array_filter(array($field['label'], $field['field_name'], $field['widget_type']))) {
      // No label.
      if (!$field['label']) {
        form_set_error('_add_existing_field][label', t('Add existing field: you need to provide a label.'));
      }

      // No existing field.
      if (!$field['field_name']) {
        form_set_error('_add_existing_field][field_name', t('Add existing field: you need to select a field.'));
      }

      // No widget type.
      if (!$field['widget_type']) {
        form_set_error('_add_existing_field][widget_type', t('Add existing field: you need to select a widget.'));
      }
      // Wrong widget type.
      elseif ($field['field_name'] && ($existing_field = content_fields($field['field_name']))) {
        $widget_types = content_widget_type_options($existing_field['type']);
        if (!isset($widget_types[$field['widget_type']])) {
          form_set_error('_add_existing_field][widget_type', t('Add existing field: invalid widget.'));
        }
      }
    }
  }
}

function content_field_overview_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];

  $type_name = $form['#type_name'];
  $type = content_types($type_name);

  // Update field weights.
  $extra = array();
  foreach ($form_values as $key => $values) {
    // Groups are handled in fieldgroup_content_overview_form_submit().
    if (in_array($key, $form['#fields'])) {
      db_query("UPDATE {". content_instance_tablename() ."} SET weight = %d WHERE type_name = '%s' AND field_name = '%s'",
        $values['weight'], $type_name, $key);
    }
    elseif (in_array($key, $form['#extra'])) {
      $extra[$key] = $values['weight'];
    }
  }

  if ($extra) {
    variable_set('content_extra_weights_'. $type_name, $extra);
  }
  else {
    variable_del('content_extra_weights_'. $type_name);
  }

  content_clear_type_cache();

  $destinations = array();

  // Create new field.
  if (!empty($form_values['_add_new_field']['field_name'])) {
    $field = $form_values['_add_new_field'];
    $field['type_name'] = $type_name;

    module_load_include('inc', 'content', 'includes/content.crud');
    if (content_field_instance_create($field)) {
      // Store new field information for fieldgroup submit handler.
      $form_state['fields_added']['_add_new_field'] = $field['field_name'];
      $destinations[] = 'admin/content/node-type/'. $type['url_str'] .'/fields/'. $field['field_name'];
    }
    else {
      drupal_set_message(t('There was a problem creating field %label.', array(
        '%label' => $field['label'])));
    }
  }

  // Add existing field.
  if (!empty($form_values['_add_existing_field']['field_name'])) {
    $field = $form_values['_add_existing_field'];
    $field['type_name'] = $type_name;
    $existing_field = content_fields($field['field_name']);

    if ($existing_field['locked']) {
      drupal_set_message(t('The field %label cannot be added to a content type because it is locked.', array('%label' => $field['field_name'])));
    }
    else {
      module_load_include('inc', 'content', 'includes/content.crud');
      if (content_field_instance_create($field)) {
        // Store new field information for fieldgroup submit handler.
        $form_state['fields_added']['_add_existing_field'] = $field['field_name'];
        $destinations[] = 'admin/content/node-type/'. $type['url_str'] .'/fields/'. $field['field_name'];
      }
      else {
        drupal_set_message(t('There was a problem adding field %label.', array('%label' => $field['field_name'])));
      }
    }
  }

  if ($destinations) {
    $destinations[] = urldecode(substr(drupal_get_destination(), 12));
    unset($_REQUEST['destination']);
    $form_state['redirect'] = content_get_destinations($destinations);
  }

}

/**
 * Menu callback; presents a listing of fields display settings for a content type.
 *
 * Form includes form widgets to select which fields appear for teaser, full node
 * and how the field labels should be rendered.
 */
function content_display_overview_form(&$form_state, $type_name, $contexts_selector = 'basic') {
  content_inactive_message($type_name);

  // Gather type information.
  $type = content_types($type_name);
  $field_types = _content_field_types();
  $fields = $type['fields'];

  $groups = array();
  if (module_exists('fieldgroup')) {
    $groups = fieldgroup_groups($type['type']);
  }
  $contexts = content_build_modes($contexts_selector);

  $form = array(
    '#tree' => TRUE,
    '#type_name' => $type['type'],
    '#fields' => array_keys($fields),
    '#groups' => array_keys($groups),
    '#contexts' => $contexts_selector,
  );

  if (empty($fields)) {
    drupal_set_message(t('There are no fields configured for this content type. You can add new fields on the <a href="@link">Manage fields</a> page.', array(
      '@link' => url('admin/content/node-type/'. $type['url_str'] .'/fields'))), 'warning');
    return $form;
  }

  // Fields.
  $label_options = array(
    'above' => t('Above'),
    'inline' => t('Inline'),
    'hidden' => t('<Hidden>'),
  );
  foreach ($fields as $name => $field) {
    $field_type = $field_types[$field['type']];
    $defaults = $field['display_settings'];
    $weight = $field['widget']['weight'];

    $form[$name] = array(
      'human_name' => array('#value' => check_plain($field['widget']['label'])),
      'weight' => array('#type' => 'value', '#value' => $weight),
      'parent' => array('#type' => 'value', '#value' => ''),
    );

    // Label
    if ($contexts_selector == 'basic') {
      $form[$name]['label']['format'] = array(
        '#type' => 'select',
        '#options' => $label_options,
        '#default_value' => isset($defaults['label']['format']) ? $defaults['label']['format'] : 'above',
      );
    }

    // Formatters.
    $options = array();
    foreach ($field_type['formatters'] as $formatter_name => $formatter_info) {
      $options[$formatter_name] = $formatter_info['label'];
    }
    $options['hidden'] = t('<Hidden>');

    foreach ($contexts as $key => $value) {
      $form[$name][$key]['format'] = array(
        '#type' => 'select',
        '#options' => $options,
        '#default_value' => isset($defaults[$key]['format']) ? $defaults[$key]['format'] : 'default',
      );
      // exclude from $content
      $form[$name][$key]['exclude'] = array(
        '#type' => 'checkbox',
        '#options' => array(0 => t('Include'), 1 => t('Exclude')),
        '#default_value' => isset($defaults[$key]['exclude']) ? $defaults[$key]['exclude'] : 0,
      );
    }
  }

  // Groups.
  $label_options = array(
    'above' => t('Above'),
    'hidden' => t('<Hidden>'),
  );
  $options = array(
    'no_style' => t('no styling'),
    'simple' => t('simple'),
    'fieldset' => t('fieldset'),
    'fieldset_collapsible' => t('fieldset - collapsible'),
    'fieldset_collapsed' => t('fieldset - collapsed'),
    'hidden' => t('<Hidden>'),
  );
  foreach ($groups as $name => $group) {
    $defaults = $group['settings']['display'];
    $weight = $group['weight'];

    $form[$name] = array(
      'human_name' => array('#value' => check_plain($group['label'])),
      'weight' => array('#type' => 'value', '#value' => $weight),
    );
    if ($contexts_selector == 'basic') {
      $form[$name]['label'] = array(
        '#type' => 'select',
        '#options' => $label_options,
        '#default_value' => isset($defaults['label']) ? $defaults['label'] : 'above',
      );
    }
    foreach ($contexts as $key => $title) {
      $form[$name][$key]['format'] = array(
        '#type' => 'select',
        '#options' => $options,
        '#default_value' => isset($defaults[$key]['format']) ? $defaults[$key]['format'] : 'fieldset',
      );
      // exclude in $content
      $form[$name][$key]['exclude'] = array(
        '#type' => 'checkbox',
        '#options' => array(0 => t('Include'), 1 => t('Exclude')),
        '#default_value' => isset($defaults[$key]['exclude']) ? $defaults[$key]['exclude'] : 0,
      );
    }
    foreach ($group['fields'] as $field_name => $field) {
      $form[$field_name]['parent']['#value'] = $name;
    }
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  return $form;
}

/**
 * Submit handler for the display overview form.
 */
function content_display_overview_form_submit($form, &$form_state) {
  module_load_include('inc', 'content', 'includes/content.crud');
  $form_values = $form_state['values'];
  foreach ($form_values as $key => $values) {
    // Groups are handled in fieldgroup_display_overview_form_submit().
    if (in_array($key, $form['#fields'])) {
      $field = content_fields($key, $form['#type_name']);
      // We have some numeric keys here, so we can't use array_merge.
      $field['display_settings'] = $values + $field['display_settings'];
      content_field_instance_update($field, FALSE);
    }
  }

  // Clear caches and rebuild menu.
  content_clear_type_cache(TRUE);
  menu_rebuild();

  drupal_set_message(t('Your settings have been saved.'));
}

/**
 * Return an array of field_type options.
 */
function content_field_type_options() {
  static $options;

  if (!isset($options)) {
    $options = array();
    $field_types = _content_field_types();
    $field_type_options = array();
    foreach ($field_types as $field_type_name => $field_type) {
      // skip field types which have no widget types.
      if (content_widget_type_options($field_type_name)) {
        $options[$field_type_name] = t($field_type['label']);
      }
    }
    asort($options);
  }
  return $options;
}

/**
 * Return an array of widget type options for a field type.
 *
 * If no field type is provided, returns a nested array of
 * all widget types, keyed by field type human name
 */
function content_widget_type_options($field_type = NULL, $by_label = FALSE) {
  static $options;

  if (!isset($options)) {
    $options = array();
    foreach (_content_widget_types() as $widget_type_name => $widget_type) {
      foreach ($widget_type['field types'] as $widget_field_type) {
        $options[$widget_field_type][$widget_type_name] = t($widget_type['label']);
      }
    }
  }

  if ($field_type) {
    return !empty($options[$field_type]) ? $options[$field_type] : array();
  }
  elseif ($by_label) {
    $field_types = _content_field_types();
    $options_by_label = array();
    foreach ($options as $field_type => $widgets) {
      $options_by_label[t($field_types[$field_type]['label'])] = $widgets;
    }
    return $options_by_label;
  }
  else {
    return $options;
  }
}

/**
 * Return an array of existing field to be added to a node type.
 */
function content_existing_field_options($type_name) {
  $type = content_types($type_name);
  $fields = content_fields();
  $field_types = _content_field_types();

  $options = array();
  foreach ($fields as $field) {
    if (!isset($type['fields'][$field['field_name']]) && !$field['locked']) {
      $field_type = $field_types[$field['type']];
      $text = t('@type: @field (@label)', array('@type' => t($field_type['label']), '@label' => t($field['widget']['label']), '@field' => $field['field_name']));
      $options[$field['field_name']] = (drupal_strlen($text) > 80) ? truncate_utf8($text, 77) . '...' : $text;
    }
  }
  // Sort the list by type, then by field name, then by label.
  asort($options);

  return $options;
}

/**
 * A form element for selecting field, widget, and label.
 */
function content_field_basic_form(&$form_state, $form_values) {
  module_load_include('inc', 'content', 'includes/content.crud');

  $type_name = $form_values['type_name'];
  $type = content_types($form_values['type_name']);
  $field_name = $form_values['field_name'];
  $field_type = $form_values['type'];
  $label = $form_values['label'];

  $form = array();

  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('Edit basic information'),
  );
  $form['basic']['field_name'] = array(
    '#title' => t('Field name'),
    '#type' => 'textfield',
    '#value' => $field_name,
    '#description' => t("The machine-readable name of the field. This name cannot be changed."),
    '#disabled' => TRUE,
  );
  $form['basic']['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => $label,
    '#required' => TRUE,
    '#description' => t('A human-readable name to be used as the label for this field in the %type content type.', array('%type' => $type['name'])),
  );
  $form['basic']['type'] = array(
    '#type' => 'select',
    '#title' => t('Field type'),
    '#options' => content_field_type_options(),
    '#default_value' => $field_type,
    '#description' => t('The type of data you would like to store in the database with this field. This option cannot be changed.'),
    '#disabled' => TRUE,
  );
  $form['basic']['widget_type'] = array(
    '#type' => 'select',
    '#title' => t('Widget type'),
    '#required' => TRUE,
    '#options' => content_widget_type_options($field_type),
    '#default_value' => $form_values['widget_type'],
    '#description' => t('The type of form element you would like to present to the user when creating this field in the %type content type.', array('%type' => $type['name'])),
  );

  $form['type_name'] = array(
    '#type' => 'value',
    '#value' => $type_name,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  $form['#validate'] = array();
  $form['#submit'] = array('content_field_basic_form_submit');

  return $form;
}

/**
 * Create a new field for a content type.
 */
function content_field_basic_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];

  $label = $form_values['label'];

  // Set the right module information
  $field_types = _content_field_types();
  $widget_types = _content_widget_types();
  $form_values['module'] = $field_types[$form_values['type']]['module'];
  $form_values['widget_module'] = $widget_types[$form_values['widget_type']]['module'];

  // Make sure we retain previous values and only over-write changed values.
  module_load_include('inc', 'content', 'includes/content.crud');
  $instances = content_field_instance_read(array('field_name' => $form_values['field_name'], 'type_name' => $form_values['type_name']));
  $field = array_merge(content_field_instance_collapse($instances[0]), $form_values);
  if (content_field_instance_update($field)) {
    drupal_set_message(t('Updated basic settings for field %label.', array(
      '%label' => $label)));
  }
  else {
    drupal_set_message(t('There was a problem updating the basic settings for field %label.', array(
      '%label' => $label)));
  }

  $type = content_types($form_values['type_name']);
  $form_state['redirect'] = 'admin/content/node-type/'. $type['url_str'] .'/fields/'. $form_values['field_name'];
  $form_state['rebuild'] = FALSE;
}

/**
 * Menu callback; present a form for removing a field from a content type.
 */
function content_field_remove_form(&$form_state, $type_name, $field_name) {
  $type = content_types($type_name);
  $field = $type['fields'][$field_name];

  $form = array();
  $form['type_name'] = array(
    '#type' => 'value',
    '#value' => $type_name,
  );
  $form['field_name'] = array(
    '#type' => 'value',
    '#value' => $field_name,
  );

  $output = confirm_form($form,
    t('Are you sure you want to remove the field %field?', array('%field' => $field['widget']['label'])),
    'admin/content/node-type/'. $type['url_str'] .'/fields',
    t('If you have any content left in this field, it will be lost. This action cannot be undone.'),
    t('Remove'), t('Cancel'),
    'confirm'
  );

  if ($field['locked']) {
    unset($output['actions']['submit']);
    $output['description']['#value'] = t('This field is <strong>locked</strong> and cannot be removed.');
  }

  return $output;
}

/**
 * Remove a field from a content type.
 */
function content_field_remove_form_submit($form, &$form_state) {
  module_load_include('inc', 'content', 'includes/content.crud');
  $form_values = $form_state['values'];

  $type = content_types($form_values['type_name']);
  $field = $type['fields'][$form_values['field_name']];
  if ($field['locked']) {
    return;
  }

  if ($type && $field && $form_values['confirm']) {
    if (content_field_instance_delete($form_values['field_name'], $form_values['type_name'])) {
      drupal_set_message(t('Removed field %field from %type.', array(
        '%field' => $field['widget']['label'],
        '%type' => $type['name'])));
    }
    else {
      drupal_set_message(t('There was a problem deleting %field from %type.', array(
        '%field' => $field['widget']['label'],
        '%type' => $type['name'])));
    }
    $form_state['redirect'] = 'admin/content/node-type/'. $type['url_str'] .'/fields';
  }
}

/**
 * Menu callback; presents the field editing page.
 */
function content_field_edit_form(&$form_state, $type_name, $field_name) {
  $output = '';
  $type = content_types($type_name);
  $field = $type['fields'][$field_name];

  if ($field['locked']) {
    $output = array();
    $output['locked'] = array(
       '#value' => t('The field %field is locked and cannot be edited.', array('%field' => $field['widget']['label'])),
    );
    return $output;
  }

  $field_types = _content_field_types();
  $field_type = $field_types[$field['type']];
  $widget_types = _content_widget_types();
  $widget_type = $widget_types[$field['widget']['type']];

  $title = isset($field['widget']['label']) ? $field['widget']['label'] : $field['field_name'];
  drupal_set_title(check_plain($title));

  // See if we need to change the widget type or label.
  if (isset($form_state['change_basic'])) {
    module_load_include('inc', 'content', 'includes/content.crud');
    $field_values = content_field_instance_collapse($field);
    return content_field_basic_form($form_state, $field_values);
  }

  $add_new_sequence = isset($_REQUEST['destinations']);

  // Remove menu tabs when we are in an 'add new' sequence.
  if ($add_new_sequence) {
    menu_set_item(NULL, menu_get_item('node'));
  }

  $form = array();
  $form['#field'] = $field;
  $form['#type'] = $type;

  // Basic iformation : hide when we are in an 'add new' sequence.
  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('%type basic information', array('%type' => $type['name'])),
    '#access' => !$add_new_sequence,
  );
  $form['basic']['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#value' => $field['widget']['label'],
    '#disabled' => TRUE,
  );
  $form['basic']['field_name'] = array(
    '#type' => 'hidden',
    '#title' => t('Field name'),
    '#value' => $field['field_name'],
    '#disabled' => TRUE,
  );
  $form['basic']['type'] = array(
    '#type' => 'hidden',
    '#title' => t('Field type'),
    '#value' => $field['type'],
    '#disabled' => TRUE,
  );
  $widget_options = content_widget_type_options($field['type']);
  $form['basic']['widget_type'] = array(
    '#type' => 'select',
    '#title' => t('Widget type'),
    '#options' => $widget_options,
    '#default_value' => $field['widget']['type'] ? $field['widget']['type'] : key($widget_options),
    '#disabled' => TRUE,
  );
  $form['basic']['change'] = array(
    '#type' => 'submit',
    '#value' => t('Change basic information'),
    '#submit' => array('content_field_edit_form_submit_update_basic'),
  );

  $form['widget'] = array(
    '#type' => 'fieldset',
    '#title' => t('%type settings', array('%type' => $type['name'])),
    '#description' => t('These settings apply only to the %field field as it appears in the %type content type.', array(
      '%field' => $field['widget']['label'],
      '%type' => $type['name'])),
  );
  $form['widget']['weight'] = array(
    '#type' => 'hidden',
    '#default_value' => $field['widget']['weight'],
  );

  $additions = (array) module_invoke($widget_type['module'], 'widget_settings', 'form', $field['widget']);
  drupal_alter('widget_settings', $additions, 'form', $field['widget']);
  $form['widget'] = array_merge($form['widget'], $additions);

  $form['widget']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Help text'),
    '#default_value' => $field['widget']['description'],
    '#rows' => 5,
    '#description' => t('Instructions to present to the user below this field on the editing form.<br />Allowed HTML tags: @tags', array('@tags' => _content_filter_xss_display_allowed_tags())),
    '#required' => FALSE,
  );

  // Add handling for default value if not provided by field.
  if (content_callback('widget', 'default value', $field) == CONTENT_CALLBACK_DEFAULT) {

    // Store the original default value for use in programmed forms.
    // Set '#default_value' instead of '#value' so programmed values
    // can override whatever we set here.
    $default_value = isset($field['widget']['default_value']) ? $field['widget']['default_value'] : array();
    $default_value_php = isset($field['widget']['default_value_php']) ? $field['widget']['default_value_php'] : '';
    $form['widget']['default_value'] = array(
      '#type' => 'value',
      '#default_value' => $default_value,
    );
    $form['widget']['default_value_php'] = array(
      '#type' => 'value',
      '#default_value' => $default_value_php,
    );

    // We can't tell at the time we build the form if this is a programmed
    // form or not, so we always end up adding the default value widget
    // even if we won't use it.
    $form['widget']['default_value_fieldset'] = array(
      '#type' => 'fieldset',
      '#title' => t('Default value'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    // Default value widget.
    $widget_form = array('#node' => (object) array('type' => $type_name));
    $widget_form_state = array('values' => array($field['field_name'] => $default_value));
    // Make sure the default value is not a required field.
    $widget_field = $field;
    $widget_field['required'] = FALSE;
    module_load_include('inc', 'content', 'includes/content.node_form');
    $form_element = content_field_form($widget_form, $widget_form_state, $widget_field, 0);
    $form['widget']['default_value_fieldset']['default_value_widget'] = $form_element;
    $form['widget']['default_value_fieldset']['default_value_widget']['#tree'] = TRUE;
    // Set up form info that the default value widget will need to find in the form.
    $form['#field_info'] = array($widget_field['field_name'] => $widget_field);

    // Advanced: PHP code.
    $form['widget']['default_value_fieldset']['advanced_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('PHP code'),
      '#collapsible' => TRUE,
      '#collapsed' => empty($field['widget']['default_value_php']),
    );

    if (user_access('Use PHP input for field settings (dangerous - grant with care)')) {
      $db_info = content_database_info($field);
      $columns = array_keys($db_info['columns']);
      foreach ($columns as $key => $column) {
        $columns[$key] = t("'@column' => value for @column", array('@column' => $column));
      }
      $sample = t("return array(\n  0 => array(@columns),\n  // You'll usually want to stop here. Provide more values\n  // if you want your 'default value' to be multi-valued:\n  1 => array(@columns),\n  2 => ...\n);", array('@columns' => implode(', ', $columns)));

      $form['widget']['default_value_fieldset']['advanced_options']['default_value_php'] = array(
        '#type' => 'textarea',
        '#title' => t('Code'),
        '#default_value' => isset($field['widget']['default_value_php']) ? $field['widget']['default_value_php'] : '',
        '#rows' => 6,
        '#tree' => TRUE,
        '#description' => t('Advanced usage only: PHP code that returns a default value. Should not include &lt;?php ?&gt; delimiters. If this field is filled out, the value returned by this code will override any value specified above. Expected format: <pre>!sample</pre>To figure out the expected format, you can use the <em>devel load</em> tab provided by <a href="@link_devel">devel module</a> on a %type content page.', array(
          '!sample' => $sample,
          '@link_devel' => 'http://www.drupal.org/project/devel',
          '%type' => $type_name)),
      );
    }
    else {
      $form['widget']['default_value_fieldset']['advanced_options']['markup_default_value_php'] = array(
        '#type' => 'item',
        '#title' => t('Code'),
        '#value' => !empty($field['widget']['default_value_php']) ? '<code>'. check_plain($field['widget']['default_value_php']) .'</code>' : t('&lt;none&gt;'),
        '#description' => empty($field['widget']['default_value_php']) ? t("You're not allowed to input PHP code.") : t('This PHP code was set by an administrator and will override any value specified above.'),
      );
    }
  }

  $form['field'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global settings'),
    '#description' => t('These settings apply to the %field field in every content type in which it appears.', array('%field' => $field['widget']['label'])),
  );
  $form['field']['required'] = array(
    '#type' => 'checkbox',
    '#title' => t('Required'),
    '#default_value' => $field['required'],
  );
  $description = t('Maximum number of values users can enter for this field.');
  if (content_handle('widget', 'multiple values', $field) == CONTENT_HANDLE_CORE) {
    $description .= '<br/>'. t("'Unlimited' will provide an 'Add more' button so the users can add as many values as they like.");
  }
  $description .= '<br/><strong>'. t('Warning! Changing this setting after data has been created could result in the loss of data!') .'</strong>';
  $form['field']['multiple'] = array(
    '#type' => 'select',
    '#title' => t('Number of values'),
    '#options' => array(1 => t('Unlimited'), 0 => 1) + drupal_map_assoc(range(2, 10)),
    '#default_value' => $field['multiple'],
    '#description' => $description,
  );

  $form['field']['previous_field'] = array(
    '#type' => 'hidden',
    '#value' => serialize($field),
  );

  $additions = (array) module_invoke($field_type['module'], 'field_settings', 'form', $field);
  drupal_alter('field_settings', $additions, 'form', $field);
  $form['field'] = array_merge($form['field'], $additions);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save field settings'),
  );
  $form['type_name'] = array(
    '#type' => 'value',
    '#value' => $type_name,
  );
  $form['field_name'] = array(
    '#type' => 'value',
    '#value' => $field_name,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $field['type'],
  );
  $form['module'] = array(
    '#type' => 'value',
    '#value' => $field['module'],
  );
  $form['widget']['label'] = array(
    '#type' => 'value',
    '#value' => $field['widget']['label'],
  );
  $form['widget_module'] = array(
    '#type' => 'value',
    '#value' => $field['widget']['module'],
  );
  $form['columns'] = array(
    '#type' => 'value',
    '#value' => $field['columns'],
  );
  return $form;
}

/**
 * Validate a field's settings.
 */
function content_field_edit_form_validate($form, &$form_state) {
  $form_values = $form_state['values'];
  if (isset($form_state['change_basic']) || $form_values['op'] == t('Change basic information')) {
    return;
  }

  module_load_include('inc', 'content', 'includes/content.crud');
  $previous_field = unserialize($form_values['previous_field']);
  $field = content_field_instance_expand($form_values);
  $field['db_storage'] = content_storage_type($field);

  $field_types = _content_field_types();
  $field_type = $field_types[$field['type']];
  $widget_types = _content_widget_types();
  $widget_type = $widget_types[$field['widget']['type']];

  if ($dropped_data = content_alter_db_analyze($previous_field, $field)) {
    // @TODO
    // This is a change that might result in loss of data.
    // Add a confirmation form here.
    // dsm($dropped_data);
  }

  module_invoke($widget_type['module'], 'widget_settings', 'validate', array_merge($field, $form_values));
  module_invoke($field_type['module'], 'field_settings', 'validate', array_merge($field, $form_values));

  // If content.module is handling the default value,
  // validate the result using the field validation.
  if (content_callback('widget', 'default value', $field) == CONTENT_CALLBACK_DEFAULT) {

    // If this is a programmed form, get rid of the default value widget,
    // we have the default values already.
    if ($form['#programmed']) {
      form_set_value(array('#parents' => array('default_value_widget')), NULL, $form_state);
      return;
    }

    if (isset($form_values['default_value_php']) &&
    ($php = trim($form_values['default_value_php']))) {
      $error = FALSE;
      ob_start();
      $return = eval($php);
      ob_end_clean();
      if (!is_array($return)) {
        $error = TRUE;
      }
      else {
        foreach ($return as $item) {
          if (!is_array($item)) {
            $error = TRUE;
            break;
          }
        }
      }
      if ($error) {
        $db_info = content_database_info($field);
        $columns = array_keys($db_info['columns']);
        foreach ($columns as $key => $column) {
          $columns[$key] = t("'@column' => value for @column", array('@column' => $column));
        }
        $sample = t("return array(\n  0 => array(@columns),\n  // You'll usually want to stop here. Provide more values\n  // if you want your 'default value' to be multi-valued:\n  1 => array(@columns),\n  2 => ...\n);", array('@columns' => implode(', ', $columns)));

        form_set_error('default_value_php', t('The default value PHP code returned an incorrect value.<br/>Expected format: <pre>!sample</pre> Returned value: @value', array(
          '!sample' => $sample,
          '@value' => print_r($return, TRUE))));
        return;
      }
      else {
        $default_value = $return;
        $is_code = TRUE;
        form_set_value(array('#parents' => array('default_value_php')), $php, $form_state);
        form_set_value(array('#parents' => array('default_value')), array(), $form_state);
      }
    }
    elseif (!empty($form_values['default_value_widget'])) {
      // Fields that handle their own multiple values may use an expected
      // value as the top-level key, so just pop off the top element.
      $key = array_shift(array_keys($form_values['default_value_widget']));
      $default_value = $form_values['default_value_widget'][$key];
      $is_code = FALSE;
      form_set_value(array('#parents' => array('default_value_php')), '', $form_state);
      form_set_value(array('#parents' => array('default_value')), $default_value, $form_state);
    }
    if (isset($default_value)) {
      $node = array();
      $node[$form_values['field_name']] = $default_value;
      $field['required'] = FALSE;
      $field_function = $field_type['module'] .'_field';

      $errors_before = form_get_errors();

      // Widget now does its own validation, should be no need
      // to add anything for widget validation here.
      if (function_exists($field_function)) {
        $field_function('validate', $node, $field, $default_value, $form, NULL);
      }
      // The field validation routine won't set an error on the right field,
      // so set it here.
      $errors_after = form_get_errors();
      if (count($errors_after) > count($errors_before)) {
        if (trim($form_values['default_value_php'])) {
          form_set_error('default_value_php', t("The PHP code for 'default value' returned @value, which is invalid.", array(
            '@value' => print_r($default_value, TRUE))));
        }
        else {
          form_set_error('default_value', t('The default value is invalid.'));
        }
      }
    }
  }
}

/**
 * Button submit handler.
 */
function content_field_edit_form_submit_update_basic($form, &$form_state) {
  $form_state['change_basic'] = TRUE;
  $form_state['rebuild'] = TRUE;
}

/**
 * Save a field's settings after editing.
 */
function content_field_edit_form_submit($form, &$form_state) {
  module_load_include('inc', 'content', 'includes/content.crud');
  $form_values = $form_state['values'];
  content_field_instance_update($form_values);

  if (isset($_REQUEST['destinations'])) {
    drupal_set_message(t('Added field %label.', array('%label' => $form_values['label'])));
    $form_state['redirect'] = content_get_destinations($_REQUEST['destinations']);
  }
  else {
    drupal_set_message(t('Saved field %label.', array('%label' => $form_values['label'])));
    $type = content_types($form_values['type_name']);
    $form_state['redirect'] = 'admin/content/node-type/'. $type['url_str'] .'/fields';
  }
}

/**
 * Helper function to handle multipage redirects.
 */
function content_get_destinations($destinations) {
  $query = array();
  $path = array_shift($destinations);
  if ($destinations) {
    $query['destinations'] = $destinations;
  }
  return array($path, $query);
}

/**
 * Content Schema Alter
 *
 * Alter the database schema.
 *
 * TODO figure out an API-safe way to use batching to update the nodes that
 * will be affected by this change so the node_save() hooks will fire.
 *
 */
function content_alter_schema($previous_field, $new_field) {
  content_alter_db($previous_field, $new_field);
}

/**
 * Schema Alter Analyze
 *
 * Analyze if changes will remove columns or delta values, thus losing data.
 * Do this so we can delete the data and fire the necessary hooks, before
 * we actually alter the schema.
 */
function content_alter_db_analyze($previous_field, $new_field) {
  $dropped = array();
  // There is no loss of data if there was no previous data.
  if (empty($previous_field)) {
    return $dropped;
  }

  // Analyze possible data loss from changes in storage type.
  if (!empty($previous_field) && !empty($new_field)) {
    // Changing from multiple to not multiple data, will cause loss of all
    // values greater than zero.
    if ($previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD &&
    $new_field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE) {
      $dropped['delta'] = 0;
    }
    // Changing from one multiple value to another will cause loss of all
    // values for deltas greater than or equal to the new multiple value.
    elseif (isset($previous_field['multiple']) && isset($new_field['multiple'])) {
      if ($previous_field['multiple'] > $new_field['multiple'] &&
      $new_field['multiple'] > 1) {
        $dropped['delta'] = $new_field['multiple'];
      }
    }
  }

  // Analyze possible data loss from changes in field columns.
  $previous_schema = !empty($previous_field) ? content_table_schema($previous_field) : array('fields' => array());
  $new_schema = !empty($new_field) ? content_table_schema($new_field) : array('fields' => array());
  $dropped_columns = array_diff(array_keys($previous_schema['fields']), array_keys($new_schema['fields']));
  if ($dropped_columns) {
    $dropped['columns'] = $dropped_columns;
  }
//  if (empty($new_schema['fields'])) {
//    // No new columns, will lose all columns for a field.
//    foreach ($previous_schema['fields'] as $column => $attributes) {
//      $dropped['columns'][] = $column;
//    }
//  }
//  else {
//    // Check both old and new columns to see if we are deleting some columns for a field.
//    foreach ($previous_schema['fields'] as $column => $attributes) {
//      if (!isset($new_schema['fields'][$column])) {
//        $dropped['columns'][] = $column;
//      }
//    }
//  }

  return $dropped;
}

/**
 * Perform adds, alters, and drops as needed to synchronize the database with
 * new field definitions.
 */
function content_alter_db($previous_field, $new_field) {
  $ret = array();

  // One or the other of these must be valid.
  if (empty($previous_field) && empty($new_field)) {
    return $ret;
  }

  // Gather relevant information : schema, table name...
  $previous_schema = !empty($previous_field) ? content_table_schema($previous_field) : array();
  $new_schema = !empty($new_field) ? content_table_schema($new_field) : array();
  if (!empty($previous_field)) {
    $previous_db_info = content_database_info($previous_field);
    $previous_table = $previous_db_info['table'];
  }
  if (!empty($new_field)) {
    $new_db_info = content_database_info($new_field);
    $new_table = $new_db_info['table'];
  }

  // Deletion of a field instance: drop relevant columns and tables and return.
  if (empty($new_field)) {
    if ($previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD) {
      db_drop_table($ret, $previous_table);
    }
    else {
      foreach ($previous_schema['fields'] as $column => $attributes) {
        if (!in_array($column, array('nid', 'vid', 'delta'))) {
          db_drop_field($ret, $previous_table, $column);
        }
      }
    }
    content_alter_db_cleanup();
    return $ret;
  }

  // Check that content types that have fields do have a per-type table.
  if (!empty($new_field)) {
    $base_tablename = _content_tablename($new_field['type_name'], CONTENT_DB_STORAGE_PER_CONTENT_TYPE);
    if (!db_table_exists($base_tablename)) {
      db_create_table($ret, $base_tablename, content_table_schema());
    }
  }

  // Create new table and columns, if not already created.
  if (!db_table_exists($new_table)) {
    db_create_table($ret, $new_table, $new_schema);
  }
  else {
    // Or add fields and/or indexes to an existing table.
    foreach ($new_schema['fields'] as $column => $attributes) {
      if (!in_array($column, array('nid', 'vid', 'delta'))) {
        // Create the column if it does not exist.
        if (!db_column_exists($new_table, $column)) {
          db_add_field($ret, $new_table, $column, $attributes);
        }
        // Create the index if requested to, and it does not exist.
        if (isset($new_schema['indexes'][$column]) && !content_db_index_exists($new_table, $column)) {
          db_add_index($ret, $new_table, $column, $new_schema['indexes'][$column]);
        }
      }
    }
  }

  // If this is a new field, we're done.
  if (empty($previous_field)) {
    content_alter_db_cleanup();
    return $ret;
  }

  // If the previous table doesn't exist, we're done.
  // Could happen if someone tries to run a schema update from an
  // content.install update function more than once.
  if (!db_table_exists($previous_table)) {
    content_alter_db_cleanup();
    return $ret;
  }

  // If changing data from one schema to another, see if changes require that
  // we drop multiple values or migrate data from one storage type to another.
  $migrate_columns = array_intersect_assoc($new_schema['fields'], $previous_schema['fields']);
  unset($migrate_columns['nid'], $migrate_columns['vid'], $migrate_columns['delta']);

  // If we're going from one multiple value a smaller one or to single,
  // drop all delta values higher than the new maximum delta value.
  // Not needed if the new multiple is unlimited or if the new table is the content table.
  if ($new_table != $base_tablename && $new_field['multiple'] < $previous_field['multiple'] && $new_field['multiple'] != 1) {
    db_query("DELETE FROM {". $new_table ."} WHERE delta >= ". max(1, $new_field['multiple']));
  }

  // If going from multiple to non-multiple, make sure the field tables have
  // the right database structure to accept migrated data.
  if ($new_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD) {
    if ($previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD && count($previous_schema['fields'])) {
      // Already using per-field storage; change multiplicity if needed.
      if ($previous_field['multiple'] > 0 && $new_field['multiple'] == 0) {
        db_drop_field($ret, $new_table, 'delta');
        db_drop_primary_key($ret, $new_table);
        db_add_primary_key($ret, $new_table, array('vid'));
      }
      else if ($previous_field['multiple'] == 0 && $new_field['multiple'] > 0) {
        db_add_field($ret, $new_table, 'delta', array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'default' => 0));
        db_drop_primary_key($ret, $new_table);
        db_add_primary_key($ret, $new_table, array('vid', 'delta'));
      }
    }
  }

  // Migrate data from per-content-type storage.
  if ($previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE &&
  $new_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD) {
    $columns = array_keys($migrate_columns);
    if ($new_field['multiple']) {
      db_query('INSERT INTO {'. $new_table .'} (vid, nid, delta, '. implode(', ', $columns) .') '.
        ' SELECT vid, nid, 0, '. implode(', ', $columns) .' FROM {'. $previous_table .'}');
    }
    else {
      db_query('INSERT INTO {'. $new_table .'} (vid, nid, '. implode(', ', $columns) .') '.
        ' SELECT vid, nid, '. implode(', ', $columns) .' FROM {'. $previous_table .'}');
    }
    foreach ($columns as $column_name) {
      db_drop_field($ret, $previous_table, $column_name);
    }
  }

  // Migrate data from per-field storage, and drop per-field table.
  if ($previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD &&
  $new_field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE) {
    // In order to be able to use drupal_write_record, we need to
    // rebuild the schema now.
    content_alter_db_cleanup();
    if ($previous_field['multiple']) {
      $result = db_query("SELECT * FROM {". $previous_table ."} c JOIN {node} n ON c.nid = n.nid WHERE delta = 0 AND n.type = '%s'", $new_field['type_name']);
    }
    else {
      $result = db_query("SELECT * FROM {". $previous_table ."} c JOIN {node} n ON c.nid = n.nid WHERE n.type = '%s'", $new_field['type_name']);
    }
    $record = array();
    while ($data = db_fetch_array($result)) {
      $record['nid'] = $data['nid'];
      $record['vid'] = $data['vid'];
      if ($previous_field['multiple']) {
        $record['delta'] = $data['delta'];
      }
      foreach ($migrate_columns as $column => $attributes) {
        if (is_null($data[$column])) {
          $record[$column] = NULL;
        }
        else {
          $record[$column] = $data[$column];
          // Prevent double serializtion in drupal_write_record.
          if (isset($attributes['serialize']) && $attributes['serialize']) {
            $record[$column] = unserialize($record[$column]);
          }
        }
      }
      if (db_result(db_query('SELECT COUNT(*) FROM {'. $new_table .
      '} WHERE vid = %d AND nid = %d', $data['vid'], $data['nid']))) {
        $keys = $new_field['multiple'] ? array('vid', 'delta') : array('vid');
        drupal_write_record($new_table, $record, $keys);
      }
      else {
        drupal_write_record($new_table, $record);
      }
    }
    db_drop_table($ret, $previous_table);
  }

  // Change modified columns that don't involve storage changes.
  foreach ($new_schema['fields'] as $column => $attributes) {
    if (isset($previous_schema['fields'][$column]) &&
    $previous_field['db_storage'] == $new_field['db_storage']) {
      if ($attributes != $previous_schema['fields'][$column]) {
        if (!in_array($column, array('nid', 'vid', 'delta'))) {
          db_change_field($ret, $new_table, $column, $column, $attributes);
        }
      }
    }
  }

  // Remove obsolete columns.
  foreach ($previous_schema['fields'] as $column => $attributes) {
    if (!isset($new_schema['fields'][$column])) {
      if (!in_array($column, array('nid', 'vid', 'delta'))) {
        db_drop_field($ret, $previous_table, $column);
      }
    }
  }

  // TODO: debugging stuff - should be removed
  if (module_exists('devel')) {
    //dsm($ret);
  }
  return $ret;
}

/**
 * Helper function for handling cleanup operations when schema changes are made.
 */
function content_alter_db_cleanup() {
  // Rebuild the whole database schema.
  // TODO: this could be optimized. We don't need to rebuild in *every case*...
  // Or do we? This affects the schema and menu and may have unfortunate
  // delayed effects if we don't clear everything out at this point.
  content_clear_type_cache(TRUE);
}

/**
 * Helper function to order fields and groups when theming (preprocessing)
 * overview forms.
 *
 * The $form is passed by reference because we assign depths as parenting
 * relationships are sorted out.
 */
function _content_overview_order(&$form, $field_rows, $group_rows) {
  // Put weight and parenting values into a $dummy render structure
  // and let drupal_render figure out the corresponding row order.
  $dummy = array();
  // Group rows: account for weight.
  if (module_exists('fieldgroup')) {
    foreach ($group_rows as $name) {
      $dummy[$name] = array('#weight' => $form[$name]['weight']['#value'], '#value' => $name .' ');
    }
  }
  // Field rows : account for weight and parenting.
  foreach ($field_rows as $name) {
    $dummy[$name] = array('#weight' => $form[$name]['weight']['#value'], '#value' => $name .' ');
    if (module_exists('fieldgroup')) {
      if ($parent = $form[$name]['parent']['#value']) {
        $form[$name]['#depth'] = 1;
        $dummy[$parent][$name] = $dummy[$name];
        unset($dummy[$name]);
      }
    }
  }
  return $dummy ? explode(' ', trim(drupal_render($dummy))) : array();
}

/**
 * Batching process for changing the field schema,
 * running each affected node through node_save() first, to
 * fire all hooks.
 *
 * TODO This is just a placeholder for now because batching can't be safely
 * used with API hooks. Need to come back and figure out how to incorporate
 * this and get it working properly when the fields are altered via the API.
 */
function content_alter_fields($previous_field, $new_field) {
  // See what values need to be updated in the field data.
  $mask = content_alter_db_mask($previous_field, $new_field);

  // We use batch processing to prevent timeout when updating a large number
  // of nodes. If there is no previous data to adjust, we can just go straight
  // to altering the schema, otherwise use batch processing to update
  // the database one node at a time, then update the schema.
  if (empty($mask)) {
    return content_alter_db($previous_field, $new_field);
  }
  $updates = array(
    'mask' => $mask['mask'],
    'alt_mask' => $mask['alt_mask'],
    'delta' => $mask['delta'],
    );
  $batch = array(
    'operations' => array(
      array('content_field_batch_update', array($previous_field['field_name'] => $updates)),
      array('content_alter_db', array($previous_field, $new_field))
    ),
    'finished' => '_content_alter_fields_finished',
    'title' => t('Processing'),
    'error_message' => t('The update has encountered an error.'),
    'file' => './'. drupal_get_path('module', 'content') .'/includes/content.admin.inc',
  );
  batch_set($batch);
  if (!empty($url)) {
    batch_process($url, $url);
  }
}

/**
 * Content Replace Fields 'finished' callback.
 */
function _content_alter_fields_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('The database has been altered and data has been migrated or deleted.'));
  }
  else {
    drupal_set_message(t('An error occurred and database alteration did not complete.'), 'error');
    $message = format_plural(count($results), '1 item successfully processed:', '@count items successfully processed:');
    $message .= theme('item_list', $results);
    drupal_set_message($message);
  }
}

/**
 * Create a mask for the column data that should be deleted in each field.
 *
 * This is a bit tricky. We could theoretically have some columns
 * that should be set to empty and others with valid info that should
 * not be emptied out. But if delta values > X are to be wiped out, they
 * need to wipe out even columns that still have values. And the NULL
 * values in these columns after the alteration may be enough to make
 * the item 'empty', as defined by hook_content_is_empty(), even if
 * some columns still have values, so all these things need to be tested.
 */
function content_alter_db_mask($previous_field, $new_field) {
  // Get an array of column values that will be dropped by this
  // schema change and create a mask to feed to content_batch_update.

  $dropped = content_alter_db_analyze($previous_field, $new_field);
  if (empty($dropped)) {
    return array();
  }
  $mask = array('mask' => array());
  foreach (array_keys($previous_field['columns']) as $column_name) {
    // The basic mask will empty the dropped columns.
    if (isset($dropped['columns']) && in_array($column_name, $dropped['columns'])) {
      $mask['mask'][$column_name] = NULL;
    }
    // Over the delta we'll empty all columns.
    if (isset($dropped['delta'])) {
      $mask['alt_mask'][$column_name] = NULL;
    }
  }
  if (isset($dropped['delta'])) {
    $mask['delta'] = $dropped['delta'];
  }
  return $mask;
}

/**
 * Content Field Batch Update Operation
 *
 * Find all nodes that contain a field and update their values.
 *
 * @param $updates
 *   an array like:
 *   'field_name' => array(
 *     'mask' => array()
 *       // Keyed array of column names and replacement values for use
 *       // below delta, or for all values if no delta is supplied.
 *     'alt_mask' => array()
 *       // Optional, keyed array of column names and replacement values for use
 *       // at or above delta, if a delta is supplied.
 *     'delta' => #
 *       // Optional, the number to use as the delta value where you switch from
 *       // one mask to the other.
 *     ),
 */
function content_field_batch_update($updates, &$context) {
  if (empty($field)) {
    $context['finished'] = 1;
    return;
  }
  $field_name = $updates['field_name'];
  $field = content_fields($field_name);

  if (!isset($context['sandbox']['progress'])) {
    $db_info = content_database_info($field);

    // Might run into non-existent tables when cleaning up a corrupted
    // database, like some of the old content storage changes in the
    // .install files.
    if (!db_table_exists($db_info['table'])) {
      return $context['finished'] = 1;
    }
    $nodes = array();
    $result = db_query("SELECT nid FROM {". $db_info['table'] ."}");
    while ($node = db_fetch_array($result)) {
      $nodes[] = $node['nid'];
    }
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = count($nodes);
    $context['sandbox']['nodes'] = $nodes;
  }

  // Process nodes by groups of 5.
  $count = min(5, count($context['sandbox']['nodes']));

  for ($i = 1; $i <= $count; $i++) {
    // For each nid, load the node, empty the column values
    // or the whole field, and re-save it.
    $nid = array_shift($context['sandbox']['nodes']);
    $node = content_field_replace($nid, array($updates));

    // Store result for post-processing in the finished callback.
    $context['results'][] = l($node->title, 'node/'. $node->nid);

    // Update our progress information.
    $context['sandbox']['progress']++;
    $context['message'] = t('Processing %title', array('%title' => $node->title));
  }

  // Inform the batch engine that we are not finished,
  // and provide an estimation of the completion level we reached.
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Content Field Replace
 *
 * Replace field values in a node from an array of update values.
 *
 * Supply an array of one or more fields and masks of field column values
 * to be replaced into field values, one mask for basic values and an optional
 * different mask for values in field items equal to or higher than a
 * specified delta.
 *
 * The masks should contain only the column values to be substituted in.
 * The supplied values will be merged into the existing values to replace
 * only the values in the mask, leaving all other values unchanged.
 *
 * The ability to set different masks starting at a delta allows the
 * possibility of setting values above a certain delta to NULL prior
 * to altering the database schema.
 *
 * @param $nid
 * @param $updates
 *   an array like:
 *   'field_name' => array(
 *     'mask' => array()
 *       // Keyed array of column names and replacement values for use
 *       // below delta, or for all values if no delta is supplied.
 *     'alt_mask' => array()
 *       // Optional, keyed array of column names and replacement values for use
 *       // at or above delta, if a delta is supplied.
 *     'delta' => #
 *       // Optional, the number to use as the delta value where you switch from
 *       // one mask to the other.
 *     ),
 */
function content_field_replace($nid, $updates) {
  $node = node_load($nid, NULL, TRUE);
  foreach ($updates as $field_name => $update) {
    $items = isset($node->$field_name) ? $node->$field_name : array();
    foreach ($items as $delta => $value) {
      $field_mask = (isset($update['delta']) && isset($update['alt_mask']) && $delta >= $update['delta']) ? $update['alt_mask'] : $mask['mask'];
      // Merge the mask into the field values to do the replacements.
      $items[$delta] = array_merge($items[$delta], $field_mask);
    }
    // Test if the new values will make items qualify as empty.
    $items = content_set_empty($field, $items);
    $node->$field_name = $items;
  }
  node_save($node);
  return $node;
}

/**
 * Helper form element validator : integer.
 */
function _element_validate_integer($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value)) {
    form_error($element, t('%name must be an integer.', array('%name' => $element['#title'])));
  }
}

/**
 * Helper form element validator : integer > 0.
 */
function _element_validate_integer_positive($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value || $value <= 0)) {
    form_error($element, t('%name must be a positive integer.', array('%name' => $element['#title'])));
  }
}

/**
 * Helper form element validator : number.
 */
function _element_validate_number($element, &$form_state) {
  $value = $element['#value'];
  if ($value != '' && !is_numeric($value)) {
    form_error($element, t('%name must be a number.', array('%name' => $element['#title'])));
  }
}
