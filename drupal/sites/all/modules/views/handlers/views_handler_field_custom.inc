<?php
// $Id: views_handler_field_custom.inc,v 1.1 2009/01/30 00:01:41 merlinofchaos Exp $

/**
 * A handler to provide a field that is completely custom by the administrator.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_custom extends views_handler_field {
  function query() {
    // do nothing -- to override the parent query.
  }

  function option_definition() {
    $options = parent::option_definition();

    // Override the alter text option to always alter the text.
    $options['alter']['contains']['alter_text'] = array('default' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Remove the checkbox
    unset($form['alter']['alter_text']);
    unset($form['alter']['text']['#dependency']);
    unset($form['alter']['text']['#process']);
  }

  function render($values) {
    // Nothing to render.
    return '';
  }
}
