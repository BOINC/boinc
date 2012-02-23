<?php
// $Id: vud_node_handler_field_widget.inc,v 1.1.2.2 2010/08/02 05:14:51 marvil07 Exp $

/**
 * @file
 * Provide a handler for Vote Up/down widget field for nodes.
 */

/**
 * A handler that provides a Vote Up/Down widget field for nodes.
 */
class vud_node_handler_field_widget extends views_handler_field {
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
    if (($can_edit = user_access('use vote up/down on nodes')) || user_access('view vote up/down count on nodes')) {
      $nid = $values->nid;
      $type = db_result(db_query("SELECT type FROM {node} WHERE nid = '%d'", $nid));
      $valid_type = in_array($type, variable_get('vud_node_types', array()), TRUE);

      if ($valid_type) {
        $tag = variable_get('vud_tag', 'vote');
        $widget_type = variable_get('vud_node_widget', 'plain');
        $widget_message_code = (!$can_edit) ? VUD_WIDGET_MESSAGE_DENIED : VUD_WIDGET_MESSAGE_ERROR;

        $widget = theme('vud_widget', $nid, 'node', $tag, $widget_type, !$can_edit, $widget_message_code);
      }
    }

    return $widget;
  }
}
