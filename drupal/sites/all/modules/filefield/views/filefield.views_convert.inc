<?php
// $Id: filefield.views_convert.inc,v 1.1 2009/07/03 22:14:14 quicksketch Exp $

/**
 * @file
 * Field conversion for fields handled by this module.
 */

/**
 * Implementation of hook_views_convert().
 */
function filefield_views_convert($display, $type, &$view, $field, $id = NULL) {
  static $filefield_fields;
  static $filefield_filters;
  static $filefield_arguments;
  if (!isset($filefield_fields)) {
    $filefield_fields = $filefield_filters = $filefield_arguments = array();
    foreach (content_fields() as $content_field) {
      if ($content_field['module'] == 'filefield') {
        $result = module_invoke('filefield', 'field_settings', 'views data', $content_field);
        if (!empty($result)) {
          foreach ($result as $table => $fields) {
            unset($fields['table']);
            foreach ($fields as $filefield_field => $definition) {
              switch ($filefield_field) {
                case $content_field['field_name'] .'_fid':
                  $filefield_fields[$filefield_field] = array(
                    'table' => $table,
                    'field' => $filefield_field,
                  );
                  $filefield_arguments['content: '. $content_field['field_name']] = &$filefield_fields[$filefield_field];
                  break;
                case $content_field['field_name'] .'_list':
                  $filefield_filters[$content_field['field_name'] .'_fid_not null'] = array(
                    'table' => $table,
                    'field' => $filefield_field,
                  );
                  break;
              }
            }
          }
        }
      }
    }
  }
  switch ($type) {
    case 'field':
      if (isset($filefield_fields[$field['field']])) {
        $multiple = array();
        switch ($field['handler']) {
          case 'content_views_field_handler_ungroup':
            $view->set_item_option($display, 'field', $id, 'multiple', array('group' => FALSE));
            break;
          case 'content_views_field_handler_last':
            $multiple['multiple_reversed'] = TRUE;
          case 'content_views_field_handler_first':
            $multiple['multiple_number'] = 1;
            $view->set_item_option($display, 'field', $id, 'multiple', $multiple);
            break;
        }
        $view->set_item_option($display, 'field', $id, 'format', $field['options']);
      }
      break;
    case 'filter':
      if (isset($filefield_filters[$field['field']])) {
        $filter = $filefield_filters[$field['field']];
        $item = $view->get_item($display, 'filter', $id);
        $item['value'] = $field['value'];
        $item['table'] = $filter['table'];
        $item['field'] = $filter['field'];
        $view->set_item($display, 'filter', $id, $item);
      }
      break;
    case 'argument':
      if (isset($filefield_arguments[$field['type']])) {
        $argument = $filefield_arguments[$field['type']];
        $options = $field['argoptions'];
        $view->add_item($display, 'argument', $argument['table'], $argument['field'], $options, $field['id']);
      }
      break;
  }
}
