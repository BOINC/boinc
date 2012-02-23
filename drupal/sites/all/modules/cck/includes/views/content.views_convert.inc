<?php
// $Id: content.views_convert.inc,v 1.1.2.5 2009/07/14 22:17:05 yched Exp $

/**
 * @file
 * Field conversion for fields handled by this module.
 */

/**
 * Implementation of hook_views_convert().
 *
 * Intervene to convert field values from the Views 1 format to the
 * Views 2 format. Intervene only if $view->add_item() won't produce
 * the right results, usually needed to set field options or values.
 */
function content_views_convert($display, $type, &$view, $views_field) {
  static $views_fields;

  if (empty($views_fields)) {
    $views_fields = array();
    $types = content_types();
    foreach ($types as $ctype) {
      foreach ($ctype['fields'] as $field) {
        $module = $field['module'];
        $result = (array) module_invoke($module, 'field_settings', 'views data', $field);
        drupal_alter('field_settings', $result, 'views data', $field);
        if (empty($result)) {
          // The views field name had the column name appended,
          // like field_name_value or field_username_uid.
          $column = array_shift(array_keys($field['columns']));
          $views_fields[$field['field_name'] .'_'. $column] = $field;
        }
      }
    }
  }

  // Is this a field that CCK should handle? If not, return.
  if (!in_array($views_field['field'], array_keys($views_fields))) {
    return;
  }

  // Now update values, options, etc. to those selected in the imported view.
  switch ($type) {
    case 'field':
      $view->display[$display]->display_options['fields'][$views_field['field']]['format'] = $views_field['options'];
      if ($views_field['handler'] == 'content_views_field_handler_group') {
        $view->display[$display]->display_options['fields'][$views_field['field']]['multiple']['group'] = 1;
      }
      else {
        $view->display[$display]->display_options['fields'][$views_field['field']]['multiple']['group'] = 0;
      }
      return;

    case 'filter':
      // TODO
      return;

    case 'exposed_filter':
      // TODO
      return;

    case 'argument':
      // TODO
      return;

    case 'sort':
      // TODO
      break;

  }
  return;
}