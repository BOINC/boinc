<?php
// $Id: user.views_convert.inc,v 1.3 2009/06/30 19:14:27 merlinofchaos Exp $

/**
 * @file
 * Field conversion for fields handled by this module.
 */

/**
 * Implementation of hook_views_convert().
 */
function user_views_convert($display, $type, &$view, $field, $id = NULL) {
  switch ($type) {
    case 'field':
      switch ($field['tablename']) {
        case 'users':
          switch ($field['field']) {
            case 'uid':
              $view->set_item_option($display, 'field', $id, 'field', 'picture');
              break;
          }
          break;
      }
      break;
    case 'filter':
      if ($field['tablename'] == 'users' || !strncmp($field['tablename'], 'users_role_', 11)) {
        switch ($field['field']) {
          case 'uid':
            $operators = array('OR' => 'in', 'NOR' => 'not in');
            $view->set_item_option($display, 'filter', $id, 'operator', $operators[$field['operator']]);
            if ($rid = (integer) substr($field['tablename'], 11)) {
              $view->add_item($display, 'filter', 'users_roles', 'rid', array('value' => $rid));
            }
            break;
        }
      }
      elseif ($field['tablename'] == 'users_roles') {
        switch ($field['field']) {
          case 'rid':
            $operators = array('AND' => 'and', 'OR' => 'or', 'NOR' => 'not');
            $view->set_item_option($display, 'filter', $id, 'operator', $operators[$field['operator']]);
            break;
        }
      }
      break;
    case 'argument':
      $options = $field['argoptions'];
      switch ($field['type']) {
        case 'uid':
          $view->add_item($display, 'argument', 'users', 'uid', $options, $field['id']);
          break;
        case 'uidtouch':
          $view->add_item($display, 'argument', 'node', 'uid_touch', $options, $field['id']);
          break;
        case 'username':
          $view->add_item($display, 'argument', 'users', 'name', $options, $field['id']);
          break;
      }
      break;
  }
}
