<?php
// $Id: content_profile_views_handler_relationship.inc,v 1.1.2.4 2010/01/12 10:05:01 fago Exp $
// @file

/**
 * Specialized relationship handler to adding content profiles.
 */
class content_profile_views_handler_relationship extends views_handler_relationship {
  function option_definition() {
    $options = parent::option_definition();

    $options['type'] = array('default' => '');
    return $options;
  }

  /**
   * Adds a form element for choosing the right content type.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['type'] = array(
      '#type' => 'radios',
      '#title' => t('Content type'),
      '#default_value' => $this->options['type'],
      '#options' => content_profile_get_types('names'),
      '#required' => TRUE,
    );
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    // Figure out what base table this relationship brings to the party.
    $join = new views_join();
    $join->definition = array(
      'table' => 'node',
      'field' => 'uid',
      'left_table' => !empty($this->relationship) ? $this->relationship : 'users',
      'left_field' => 'uid',
      'extra' => array(
        array('field' => 'type', 'value' => $this->options['type']),
      ),
    );

    if (!empty($this->options['required'])) {
      $join->definition['type'] = 'INNER';
    }

    $join->construct();
    $this->ensure_my_table();

    $alias = $join->definition['table'] .'_'. $join->definition['left_table'];
    $this->alias = $this->query->add_relationship($alias, $join, $this->definition['base'], $this->relationship);
  }
}
