<?php
//$Id: flag.views_convert.inc,v 1.1.2.2 2008/11/09 08:40:01 mooffie Exp $
/**
 * @file
 * Update Views 1 views provided by both views_bookmark and flag modules.
 */

/**
 * Implementation of hook_views_convert().
 *
 * Intervene to convert field values from the Views 1 format to the
 * Views 2 format. Intervene only if $view->add_item() won't produce
 * the right results, usually needed to set field options or values.
 */
function flag_views_convert($display, $type, &$view, $field, $id) {
  static $flag_name;

  // First, replace any sign of views_bookmark with flag's Views 1 equivelant.
  $key_search = array(
    '^views_bookmark_ops_([0-9]+)' => 'flag_ops_',
    '^views_bookmark_nodes_([0-9]+)' => 'flag_content_',
    '^views_bookmark_users_([0-9]+)' => 'flag_users_',
    '^views_bookmark_node_count_([0-9]+)' => 'flag_counts_',
  );
  foreach ($field as $property => $value) {
    foreach ($key_search as $search => $replace) {
      if (!empty($value) && is_string($value) && preg_match('/'. $search .'/', $value, $matches)) {
        $flag = flag_get_flag(NULL, $matches[1]);
        $replace = $replace . $flag->name;
        $field[$property] = preg_replace('/'. $search .'/', $replace, $value);
      }
    }
  }

  // Create a table/field identifier for this field.
  foreach (array('flag_ops_', 'flag_content_', 'flag_users_', 'flag_counts_') as $table) {
    if (strpos($field['tablename'], $table) === 0) {
      $name = str_replace($table, '', $field['tablename']);
      if (!empty($name) && !isset($flag_name)) {
        $flag_name = $name;
      }
    }
  }

  // Now update values, options, etc. to those selected in the imported view.
  switch ($type) {
    case 'field':
      switch ($id) {
        case 'ops':
          $new_field = array(
            'label' => $field['label'],
            'id' => 'ops',
            'table' => 'flag_content',
            'field' => 'ops',
            'relationship' => 'flag_content_rel',
          );
          $new_rel = 'flag_content_rel';
          $flag_content_rel_user = 'current';
          break;
        case 'count':
          $new_field = array(
            'label' => $field['label'],
            'id' => 'count',
            'table' => 'flag_counts',
            'field' => 'count',
            'relationship' => 'flag_counts_rel',
          );
          $new_rel = 'flag_counts_rel';
          break;
        case 'name':
          $new_field = array(
            'label' => $field['label'],
            'link_to_user' => 1,
            'id' => 'name',
            'table' => 'users',
            'field' => 'name',
            'relationship' => 'uid',
          );
          $new_rel = 'uid';
          break;
      }
      break;

    case 'filter':
    case 'exposed_filter':
      switch ($id) {
        case 'uid':
          // The flag_content uid filter means "Include content only flagged by
          // the current user". In D6, this is no longer a filter, but instead
          // part of the relationship. Make the relationship join on the uid.
          if ($field['value'] == '***CURRENT_USER***') {
            $new_rel = 'flag_content_rel';
            $flag_content_rel_user = 'current';
          }
          // Remove the old filter.
          $view->set_item('default', $type, $id, NULL);
          break;
        case 'timestamp':
          $new_field = array(
            'operator' => $field['operator'],
            'value' => flag_views_convert_timestamp_value($field['value']),
            'group' => 0,
            'id' => 'timestamp',
            'table' => 'flag_content',
            'field' => 'timestamp',
            'relationship' => 'flag_content_rel',
            'exposed' => $type == 'exposed_filter' ? 1 : 0,
          );
          $new_rel = 'flag_content_rel';
          drupal_set_message(t('Flag is not able to convert the <em>Flagged time</em> filter. It\'s value is currently empty, but needs to be populated to work properly.'), 'warning');
          break;
        case 'count':
          $new_field = array(
            'operator' => $field['operator'],
            'value' => array('value' => $field['value']),
            'group' => 0,
            'id' => 'count',
            'table' => 'flag_counts',
            'field' => 'count',
            'relationship' => 'flag_counts_rel',
            'exposed' => $type == 'exposed_filter' ? 1 : 0,
          );
          $new_rel = 'flag_counts_rel';
          break;
      }
      break;

    case 'argument':
      // Flag in Drupal 5 only provides one argument, and in Views 1 arguments
      // weren't given and ID, so we use the field type.
      if (strpos($field['type'], 'flag_content_') === 0) {
        $new_field = array(
          'id' => 'uid',
          'table' => 'users',
          'field' => 'uid',
          'relationship' => 'uid',
        );
        $new_rel = 'uid';
      }
      break;

    case 'sort':
      switch ($id) {
        case 'count':
          $new_field = array(
            'order' => $field['sortorder'],
            'id' => 'count',
            'table' => 'flag_counts',
            'field' => 'count',
            'relationship' => 'flag_counts_rel',
          );
          $new_rel = 'flag_counts_rel';
          break;
        case 'timestamp':
          $new_field = array(
            'order' => $field['sortorder'],
            'id' => 'timestamp',
            'table' => 'flag_content',
            'field' => 'timestamp',
            'relationship' => 'flag_content_rel',
          );
          $new_rel = 'flag_content_rel';
          break;
      }
      break;
  }

  // Add any new fields.
  if (isset($new_field)) {
    // Exposed filters are now wrapped into filters.
    $type = $type == 'exposed_filter' ? 'filter' : $type;
    // Update the type in the view options.
    $view->set_item('default', $type, $id, $new_field);
  }

  // Add any new relationships (but only once per view).
  if (isset($new_rel) && !isset($view->display[$display]->display_options['relationships'][$new_rel])) {
    if ($new_rel == 'flag_content_rel') {
      $view->display[$display]->display_options['relationships'][$new_rel] = array(
        'label' => $flag_name,
        'required' => 1,
        'flag' => $flag_name,
        'user_scope' => 'any',
        'id' => 'flag_content_rel',
        'table' => 'node',
        'field' => 'flag_content_rel',
        'relationship' => 'none',
      );
    }
    elseif ($new_rel == 'flag_counts_rel') {
      $view->display[$display]->display_options['relationships'][$new_rel] = array(
        'label' => $flag_name .' counts',
        'required' => 0,
        'flag' => $flag_name,
        'id' => 'flag_counts_rel',
        'table' => 'node',
        'field' => 'flag_counts_rel',
        'relationship' => 'none',
      );
    }
    elseif ($new_rel == 'uid') {
      $view->display[$display]->display_options['relationships'][$new_rel] = array(
        'label' => isset($flag_name) ? $flag_name .' user' : 'user',
        'required' => 0,
        'id' => 'uid',
        'table' => 'flag_content',
        'field' => 'uid',
        'relationship' => 'flag_content_rel',
      );
    }
  }

  // Modify relationships if needed.
  if (isset($flag_content_rel_user) && isset($view->display[$display]->display_options['relationships']['flag_content_rel'])) {
    $view->display[$display]->display_options['relationships']['flag_content_rel']['user_scope'] = 'current';
  }
}

/**
 * In Views 1, dates were simply stored as a single string. In Views 2, the
 * value is stored as an array of options.
 */
function flag_views_convert_timestamp_value($value) {
  $type = 'date';
  if (stripos($value, 'now') !== FALSE) {
    $type = 'offset';
    $value = trim(str_ireplace('now', '', $value));
  }
  return array(
    'type' => $type,
    'value' => $value,
    'min' => '',
    'max' => '',
  );
}
