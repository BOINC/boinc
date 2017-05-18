<?php
/**
 * @file
 * Views' relationship handlers.
 */

class views_handler_relationship_node_term_data extends views_handler_relationship  {
  function option_definition() {
    $options = parent::option_definition();
    $options['vids'] = array('default' => array());
    return $options;
  }

  /**
   * Default options form that provides the label widget that all fields
   * should have.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $vocabularies = taxonomy_get_vocabularies();
    $options = array();
    foreach ($vocabularies as $voc) {
      $options[$voc->vid] = check_plain($voc->name);
    }

    $form['vids'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Vocabularies'),
      '#options' => $options,
      '#default_value' => $this->options['vids'],
      '#description' => t('Choose which vocabularies you wish to relate. Remember that every term found will create a new record, so this relationship is best used on just one vocabulary that has only one term per node.'),
    );
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    $this->ensure_my_table();

    $def = $this->definition;
    $def['table'] = 'term_data';

    if (!empty($this->options['required']) || !array_filter($this->options['vids'])) {
      $term_node = $this->query->add_table('term_node', $this->relationship);
      $def['left_table'] = $term_node;
      $def['left_field'] = 'tid';
      $def['field'] = 'tid';
      if (!empty($this->options['required'])) {
        $def['type'] = 'INNER';
      }
    }
    else {
      // If the join is optional, join a subselect that will emulate term_data table instead
      $def['left_table'] = $this->table_alias;
      $def['left_field'] = 'vid';
      $def['field'] = 'revision';
      // fapi ensures vids are safe here.
      $vids = implode(', ', array_filter($this->options['vids']));
      $def['table formula'] = "(SELECT td.*, tn.vid AS revision FROM {term_data} td INNER JOIN {term_node} tn ON tn.tid = td.tid WHERE td.vid IN ($vids))";
    }

    $join = new views_join();

    $join->definition = $def;
    $join->construct();
    $join->adjusted = TRUE;

    // use a short alias for this:
    $alias = $def['table'] . '_' . $this->table;

    $this->alias = $this->query->add_relationship($alias, $join, 'term_data', $this->relationship);
  }
}
