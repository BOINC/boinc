<?php
/**
 * Filter handler for taxonomy terms with depth.
 *
 * This handler is actually part of the node table and has some restrictions,
 * because it uses a subquery to find nodes with
 */
class views_handler_filter_term_node_tid_depth extends views_handler_filter_term_node_tid {
  function operator_options() {
    return array(
      'or' => t('Is one of'),
    );
  }
  
  function option_definition() {
    $options = parent::option_definition();

    $options['depth'] = array('default' => 0);

    return $options;
  }
  
  function extra_options_form(&$form, &$form_state) {
    parent::extra_options_form($form, $form_state);

    $form['depth'] = array(
      '#type' => 'weight',
      '#title' => t('Depth'),
      '#default_value' => $this->options['depth'],
      '#description' => t('The depth will match nodes tagged with terms in the hierarchy. For example, if you have the term "fruit" and a child term "apple", with a depth of 1 (or higher) then filtering for the term "fruit" will get nodes that are tagged with "apple" as well as "fruit". If negative, the reverse is true; searching for "apple" will also pick up nodes tagged with "fruit" if depth is -1 (or lower).'),
    );
  }

  function query() {
    // If no filter values are present, then do nothing.
    if (count($this->value) == 0) {
      return;
    } 
    else if (count($this->value) == 1) {
      $placeholder = " = %d";
    } 
    else {
      $placeholder = " IN (" . implode(', ', array_fill(0, sizeof($this->value), '%d')) . ")";
    }

    // The normal use of ensure_my_table() here breaks Views.
    // So instead we trick the filter into using the alias of the base table.
    // See http://drupal.org/node/271833
    // If a relationship is set, we must use the alias it provides.
    if (!empty($this->relationship)) {
      $this->table_alias = $this->relationship;
    }
    // If no relationship, then use the alias of the base table.
    else if (isset($this->query->table_queue[$this->query->base_table]['alias'])) {
      $this->table_alias = $this->query->table_queue[$this->query->base_table]['alias'];
    }
    // This should never happen, but if it does, we fail quietly.
    else {
      return;
    }

    // Now build the subqueries.
    $subquery = "\n  SELECT tn.vid FROM {term_node} tn\n";
    $where = "  WHERE tn.tid $placeholder\n";
    $args = $this->value;
    $last = "tn";

    if ($this->options['depth'] > 0) {
      $subquery .= "    LEFT JOIN {term_hierarchy} th ON th.tid = tn.tid\n";
      $last = "th";
      foreach (range(1, abs($this->options['depth'])) as $count) {
        $subquery .= "    LEFT JOIN {term_hierarchy} th$count ON $last.parent = th$count.tid\n";
        $where .= "    OR th$count.tid $placeholder\n";
        $args = array_merge($args, $this->value);
        $last = "th$count";
      }
    }
    else if ($this->options['depth'] < 0) {
      foreach (range(1, abs($this->options['depth'])) as $count) {
        $subquery .= "    LEFT JOIN {term_hierarchy} th$count ON $last.tid = th$count.parent\n";
        $where .= "    OR th$count.tid $placeholder\n";
        $args = array_merge($args, $this->value);
        $last = "th$count";
      }
    }
    
    $this->query->add_where(0, "$this->table_alias.$this->real_field IN ($subquery$where  )", $args);
  }
}
