<?php
// $Id: og_views.views_convert.inc,v 1.1.2.1 2009/06/10 16:13:12 weitzman Exp $

/**
 * @file
 * Field conversion for fields handled by this module.
 */

/**
 * Implementation of hook_views_convert().
 */
function og_views_views_convert($display, $type, &$view, $field, $id = NULL) {
  switch ($type) {
    case 'field':
      switch ($field['tablename']) {
        case 'og':
          switch ($field['field']) {
            case 'count':
              $view->set_item_option($display, 'field', $id, 'field', 'member_count');
              break;
            case 'post_count':
              // Drupal 6 version doesn't support counting by node type.
              break;
            case 'notification':
              // This field has no equivalent in the Drupal 6 version.
              break;
          }
          break;
        case 'og_node_data':
          switch ($field['field']) {
            case 'title':
              $view->set_item_option($display, 'field', $id, 'table', 'og_ancestry');
              $view->set_item_option($display, 'field', $id, 'field', 'group_nid');
              break;
          }
          break;
        case 'og_uid':
          switch ($field['field']) {
            case 'mail_type':
              $view->set_item_option($display, 'field', $id, 'table', 'og_notifications');
              $view->set_item_option($display, 'field', $id, 'field', 'autosubscribe');
              break;
            case 'managelink':
              $view->set_item_option($display, 'field', $id, 'field', 'managelinkmy');
              break;
            case 'created':
            case 'changed':
              $handlers = array(
                'views_handler_field_date_small' => 'small',
                'views_handler_field_date' => 'medium',
                'views_handler_field_date_large' => 'large',
                'views_handler_field_date_custom' => 'custom',
                'views_handler_field_since' => 'time ago',
              );
              $view->set_item_option($display, 'field', $id, 'date_format', $handlers[$field['handler']]);
              if (!empty($field['options'])) {
                $view->set_item_option($display, 'field', $id, 'custom_date_format', $field['options']);
              }
              break;
          }
          break;
      }
      break;
    case 'filter':
      switch ($field['tablename']) {
        case 'og':
          switch ($field['field']) {
            case 'selective':
              $operators = array('OR' => 'in', 'NOR' => 'not in');
              $view->set_item_option($display, 'filter', $id, 'operator', $operators[$field['operator']]);
              break;
            case 'language':
              $operators = array('AND' => 'not in', 'OR' => 'in', 'NOR' => 'not in');
              $view->set_item_option($display, 'filter', $id, 'operator', $operators[$field['operator']]);
              $value = $field['operator'] == 'AND' ? '***NO_LANGUAGE***' : $field['value'];
              $view->set_item_option($display, 'filter', $id, 'value', $value);
              break;
          }
          break;
        case 'og_ancestry':
          switch ($field['field']) {
            case 'gid':
              $filter = views_break_phrase($field['value']);
              $item = $view->get_item($display, 'filter', $id);
              $item['operator'] = $filter->operator;
              $item['value'] = $filter->value;
              $item['field'] = 'group_nid';
              $view->set_item($display, 'filter', $id, $item);
              break;
            case 'group_name_like':
              $item = $view->get_item($display, 'filter', $id);
              $item['relationship'] = $view->add_item($display, 'relationship', 'og_ancestry', 'group_nid', array(), 'og_ancestry_group_nid');
              $item['table'] = 'node';
              $item['field'] = 'title';
              $item['operator'] = $field['operator'];
              $item['case'] = FALSE;
              $view->set_item($display, 'filter', $id, $item);
              break;
            case 'gtype':
              $operators = array('OR' => 'in', 'NOR' => 'not in');
              $item = $view->get_item($display, 'filter', $id);
              $item['relationship'] = $view->add_item($display, 'relationship', 'og_ancestry', 'group_nid', array(), 'og_ancestry_group_nid');
              $item['operator'] = $operators[$field['operator']];
              $item['table'] = 'og';
              $item['field'] = 'type_groups';
              $view->set_item($display, 'filter', $id, $item);
              break;
          }
          break;
        case 'og_uid':
          switch ($field['field']) {
            case 'currentuidsimple':
              $operators = array('=' => TRUE, '!=' => FALSE);
              $view->set_item_option($display, 'filter', $id, 'value', $operators[$field['operator']]);
              $view->set_item_option($display, 'filter', $id, 'field', 'uid');
              break;
          }
          break;
        case 'og_uid_node':
          switch ($field['field']) {
            case 'currentuid':
              $operators = array('=' => TRUE, '!=' => FALSE);
              $item = $view->get_item($display, 'filter', $id);
              $item['relationship'] = $view->add_item($display, 'relationship', 'og_ancestry', 'group_nid', array(), 'og_ancestry_group_nid');
              $item['value'] = $operators[$field['operator']];
              $item['table'] = 'og_uid';
              $item['field'] = 'uid';
              $view->set_item($display, 'filter', $id, $item);
              break;
          }
          break;
        case 'og_views':
          switch ($field['field']) {
            case 'og_type_selector':
              $operators = array('OR' => 'in', 'NOR' => 'not in');
              $item = $view->get_item($display, 'filter', $id);
              $item['table'] = 'og';
              $item['field'] = 'type_groups';
              $item['operator'] = $operators[$field['operator']];
              $view->set_item($display, 'filter', $id, $item);
              break;
            case 'og_type':
              $view->set_item_option($display, 'filter', $id, 'table', 'og');
              $view->set_item_option($display, 'filter', $id, 'field', 'type_groups_all');
              break;
          }
          break;
      }
      break;
    case 'argument':
      $options = $field['argoptions'];
      switch ($field['type']) {
        case 'gid':
          $options['break_phrase'] = TRUE;
          $view->add_item($display, 'argument', 'og_ancestry', 'group_nid', $options, $field['id']);
          break;
        case 'name':
          $options['relationship'] = $view->add_item($display, 'relationship', 'og_ancestry', 'group_nid', array(), 'og_ancestry_group_nid');
          $view->add_item($display, 'argument', 'node', 'title', $options, $field['id']);
          break;
      }
      break;
  }
}
