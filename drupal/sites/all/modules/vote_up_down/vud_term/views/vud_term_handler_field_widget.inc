<?php

/**
 * @file
 * Provide a handler for Vote Up/down widget field for terms.
 */

/**
 * A handler that provides a Vote Up/Down widget field for terms.
 */
class vud_term_handler_field_widget extends views_handler_field {
  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->additional_fields['nid'] = array('table' => 'node', 'field' => 'nid');
  }

  function query() {
    // Not calling parent method on purpose.
    $this->add_additional_fields();
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
    $nid = $values->{$this->aliases['nid']};
    $node = node_load($nid);
    _vud_term_generate_table($node);
    return $node->content['vud_term_show_table']['#value'];
  }
}
