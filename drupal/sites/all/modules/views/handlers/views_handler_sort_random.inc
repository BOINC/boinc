<?php

/**
 * Handle a random sort.
 */
class views_handler_sort_random extends views_handler_sort {
  function query() {
    global $db_type;
    switch ($db_type) {
      case 'mysql':
      case 'mysqli':
        $formula = 'RAND()';
        break;
      case 'pgsql':
        $formula = 'RANDOM()';
        break;
    }
    if (!empty($formula)) {
      $this->query->add_orderby(NULL, $formula, $this->options['order'], '_' . $this->field);
    }
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['order']['#access'] = FALSE;
  }
}
