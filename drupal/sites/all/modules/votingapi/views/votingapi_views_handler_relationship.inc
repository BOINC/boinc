<?php
// $Id: votingapi_views_handler_relationship.inc,v 1.1.2.8 2009/08/15 17:58:06 eaton Exp $

/**
 * @file
 * Provide views handler for votingapi joins.
 */

/**
 * A custom join handler that connects arbitrary base tables to VotingAPI's data.
 *
 * The base relationship handler can only handle a single join. Some relationships
 * are more complex and might require chains of joins; for those, you must
 * utilize a custom relationship handler.
 *
 * Definition items:
 * - base: The new base table this relationship will be adding. This does not
 *   have to be a declared base table, but if there are no tables that
 *   utilize this base table, it won't be very effective.
 * - relationship table: The actual table this relationship operates against.
 *   This is analogous to using a 'table' override.
 * - relationship field: The actual field this relationsihp operates against.
 *   This is analogous to using a 'real field' override.
 * - label: The default label to provide for this relationship, which is
 *   shown in parentheses next to any field/sort/filter/argument that uses
 *   the relationship.
 */
class votingapi_views_handler_relationship extends views_handler_relationship {
  function option_definition() {
    $options = parent::option_definition();

    $label = !empty($this->definition['label']) ? $this->definition['label'] : $this->definition['title'];
    $options['label'] = array('default' => $label, 'translatable' => TRUE);

    $options['votingapi']['value_type'] = array('default' => NULL);
    $options['votingapi']['tag'] = array('default' => NULL);
    if ($this->definition['base'] == 'votingapi_cache') {
      $options['votingapi']['function'] = array('default' => NULL);
    }
    elseif ($this->definition['base'] == 'votingapi_vote') {
      $options['current_user'] = array('default' => FALSE);
    }

    return $options;
  }

  /**
   * Default options form that provides the label widget that all fields
   * should have.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $options['value_types'][''] = t('No filtering');
    $options['tags'][''] = t('No filtering');
    $options['functions'][''] = t('No filtering');
    foreach(votingapi_metadata() as $bin => $bin_data) {
      foreach ($bin_data as $key => $data) {
        $options[$bin][$key] = $data['name'];
      }
    }

    $form['votingapi'] = array(
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#title' => t('Data filters'),
      '#description' => t('For each piece of content, many pieces of voting data may be saved. Use these options to specify exactly which types should be available via this relationship. <strong>Warning!</strong> Leaving any of these filters empty may result in multiple copies of each piece of content being displayed in listings.'),
      '#tree' => TRUE,
    );
    
    $default = $this->options['votingapi']['value_type'];
    if (!isset($options['value_types'][$default])) {
      $default = '**OTHER**';
    }
    $form['votingapi']['value_type'] = array(
      '#title' => t('Value type'),
      '#type' => 'select',
      '#options' => $options['value_types'],
      '#default_value' => $default,
    );
    $form['votingapi']['value_type']['#options']['**OTHER**'] = t(' -Other- ');
    $form['votingapi']['value_type_other'] = array(
      '#type' => 'textfield',
      '#default_value' => $this->options['votingapi']['value_type'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-votingapi-value-type' => array('**OTHER**')),
    );
    
    $default = $this->options['votingapi']['tag'];
    if (!isset($options['tags'][$default])) {
      $default = '**OTHER**';
    }
    $form['votingapi']['tag'] = array(
      '#title' => t('Vote tag'),
      '#type' => 'select',
      '#options' => $options['tags'],
      '#default_value' => $default,
    );
    $form['votingapi']['tag']['#options']['**OTHER**'] = t(' -Other- ');
    $form['votingapi']['tag_other'] = array(
      '#type' => 'textfield',
      '#default_value' => $this->options['votingapi']['tag'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-votingapi-tag' => array('**OTHER**')),
    );

    if ($this->definition['base'] == 'votingapi_cache') {
      $default = $this->options['votingapi']['function'];
      if (!isset($options['functions'][$default])) {
        $default = '**OTHER**';
      }
      $form['votingapi']['function'] = array(
        '#title' => t('Aggregation function'),
        '#type' => 'select',
        '#options' => $options['functions'],
        '#default_value' => $default,
      );
      $form['votingapi']['function']['#options']['**OTHER**'] = t(' -Other- ');
      $form['votingapi']['function_other'] = array(
        '#type' => 'textfield',
        '#default_value' => $this->options['votingapi']['function'],
        '#process' => array('views_process_dependency'),
        '#dependency' => array('edit-options-votingapi-function' => array('**OTHER**')),
      );
    }
    else {
      $form['current_user'] = array(
        '#title' => t('Restrict to current user'),
        '#type' => 'checkbox',
        '#return_value' => TRUE,
        '#default_value' => $this->options['current_user'],
      );
    }
  }
  
  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    $v = $form_state['values']['options']['votingapi'];
    if ($v['value_type'] == '**OTHER**') {
      $form_state['values']['options']['votingapi']['value_type'] = $v['value_type_other'];
    }

    if ($v['tag'] == '**OTHER**') {
      $form_state['values']['options']['votingapi']['tag'] = $v['tag_other'];
    }

    if ($v['function'] == '**OTHER**') {
      $form_state['values']['options']['votingapi']['function'] = $v['function_other'];
    }
    
    foreach (array('function_other', 'value_type_other', 'tag_other') as $key) {
      unset($form_state['values']['options']['votingapi'][$key]);
    }
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    // Figure out what base table this relationship brings to the party.
    $table_data = views_fetch_data($this->definition['base']);

    $def = $this->definition;
    $def['table'] = $this->definition['base'];
    $def['field'] = 'content_id';
    $def['left_table'] = $this->relationship ? $this->relationship : $this->table;
    $def['left_field'] = $this->field;
    if (!empty($this->options['required'])) {
      $def['type'] = 'INNER';
    }

    if (!empty($def['join_handler']) && class_exists($def['join_handler'])) {
      $join = new $def['join_handler'];
    }
    else {
      $join = new views_join();
    }

    // use a short alias for this:
    $alias = $def['table'] . '_' . $def['left_table'];

    if (!empty($this->options['votingapi'])) {
      foreach ($this->options['votingapi'] as $field => $value) {
        if (!empty($value)) {
          $def['extra'][] = array(
            'field' => $field,
            'value' => $value,
            'numeric' => FALSE
          );
          $alias .= '_'. $value;
        }
      }
    }

    if (!empty($this->options['current_user'])) {
      $def['extra'][] = array(
        'field' => 'uid',
        'value' => '***CURRENT_USER***',
        'numeric' => FALSE
      );
      $alias .= '_curuser';
    }

    $join->definition = $def;
    $join->construct();

    $this->ensure_my_table();

    $this->alias = $this->query->add_relationship($alias, $join, $this->definition['base'], $this->relationship);
  }
}
