<?php
// $Id: vud_comment_handler_field_widget.inc,v 1.1.2.2 2010/08/02 05:14:51 marvil07 Exp $

/**
 * @file
 * Provide a handler for Vote Up/down widget field for comments.
 */

/**
 * A handler that provides a Vote Up/Down widget field for nodes.
 */
class vud_comment_handler_field_widget extends views_handler_field {
  function query() {
    // We have to override the parent here, because we don't want
    // to do anything.
  }

  function option_definition() {
    $options = parent::option_definition();
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    // It doesn't make sense to have the ability to alter the output.
    $form['alter']['#access'] = FALSE;
  }

  function render($values) {
    $widget = '';
    $query = 'SELECT n.type FROM {comments} c LEFT JOIN {node} n ON c.nid = n.nid WHERE c.cid = %d';
    $type = db_result(db_query($query, $values->cid));
    $comment_allow = in_array($type, variable_get('vud_comment_node_types', array()), TRUE);
    if ($comment_allow && user_access('use vote up/down on comments')) {
      $tag = variable_get('vud_tag', 'vote');
      $widget_type = variable_get('vud_comment_widget', 'plain');
      $widget = theme('vud_widget', $values->cid, 'comment', $tag, $widget_type);
    }

    return $widget;
  }
}
