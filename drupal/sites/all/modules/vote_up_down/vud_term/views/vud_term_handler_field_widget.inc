<?php
// $Id: vud_term_handler_field_widget.inc,v 1.1.2.2 2010/08/02 05:14:51 marvil07 Exp $

/**
 * @file
 * Provide a handler for Vote Up/down widget field for terms.
 */

/**
 * A handler that provides a Vote Up/Down widget field for terms.
 */
class vud_term_handler_field_widget extends views_handler_field {
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
    $node = node_load($values->nid);
    _vud_term_generate_table($node);
    return $node->content['vud_term_show_table']['#value'];
  }
}
