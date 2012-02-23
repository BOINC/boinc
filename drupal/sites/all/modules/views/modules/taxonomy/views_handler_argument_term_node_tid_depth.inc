<?php
// $Id: views_handler_argument_term_node_tid_depth.inc,v 1.3.2.1 2010/03/16 23:38:23 merlinofchaos Exp $
/**
 * Argument handler for taxonomy terms with depth.
 *
 * This handler is actually part of the node table and has some restrictions,
 * because it uses a subquery to find nodes with
 */
class views_handler_argument_term_node_tid_depth extends views_handler_argument {
  function option_definition() {
    $options = parent::option_definition();

    $options['depth'] = array('default' => 0);
    $options['break_phrase'] = array('default' => FALSE);
    $options['set_breadcrumb'] = array('default' => FALSE);
    $options['use_taxonomy_term_path'] = array('default' => FALSE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['depth'] = array(
      '#type' => 'weight',
      '#title' => t('Depth'),
      '#default_value' => $this->options['depth'],
      '#description' => t('The depth will match nodes tagged with terms in the hierarchy. For example, if you have the term "fruit" and a child term "apple", with a depth of 1 (or higher) then filtering for the term "fruit" will get nodes that are tagged with "apple" as well as "fruit". If negative, the reverse is true; searching for "apple" will also pick up nodes tagged with "fruit" if depth is -1 (or lower).'),
    );

    $form['break_phrase'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow multiple terms per argument'),
      '#description' => t('If selected, users can enter multiple arguments in the form of 1+2+3. Due to the number of JOINs it would require, AND will be treated as OR with this argument.'),
      '#default_value' => !empty($this->options['break_phrase']),
    );

    $form['set_breadcrumb'] = array(
      '#type' => 'checkbox',
      '#title' => t("Set the breadcrumb for the term parents"),
      '#description' => t('If selected, the breadcrumb trail will include all parent terms, each one linking to this view. Note that this only works if just one term was received.'),
      '#default_value' => !empty($this->options['set_breadcrumb']),
    );

    $form['use_taxonomy_term_path'] = array(
      '#type' => 'checkbox',
      '#title' => t("Use Drupal's taxonomy term path to create breadcrumb links"),
      '#description' => t('If selected, the links in the breadcrumb trail will be created using the standard drupal method instead of the custom views method. This is useful if you are using modules like taxonomy redirect to modify your taxonomy term links.'),
      '#default_value' => !empty($this->options['use_taxonomy_term_path']),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-set-breadcrumb' => array(TRUE)),
    );
  }

  function set_breadcrumb(&$breadcrumb) {
    if (empty($this->options['set_breadcrumb']) || !is_numeric($this->argument)) {
      return;
    }

    return views_taxonomy_set_breadcrumb($breadcrumb, $this);
  }

  /**
   * Override default_actions() to remove summary actions.
   */
  function default_actions($which = NULL) {
    if ($which) {
      if (in_array($which, array('ignore', 'not found', 'empty', 'default'))) {
        return parent::default_actions($which);
      }
      return;
    }
    $actions = parent::default_actions();
    unset($actions['summary asc']);
    unset($actions['summary desc']);
    return $actions;
  }

  function query() {
    $this->ensure_my_table();

    if (!empty($this->options['break_phrase'])) {
      $tids = new stdClass();
      $tids->value = $this->argument;
      $tids = views_break_phrase($this->argument, $tids);
      if ($tids->value == -1) {
        return FALSE;
      }

      if (count($tids->value) > 1) {
        $placeholder = " IN (" . implode(', ', array_fill(0, sizeof($tids->value), '%d')) . ")";
      }
      else {
        $placeholder = " = %d";
      }

      $tids = $tids->value;
    }
    else {
      $placeholder = "= %d";
      $tids = array($this->argument);
    }
    $subquery = "\n  SELECT tn.vid FROM {term_node} tn\n";
    $where = "  WHERE tn.tid $placeholder\n";
    $args = $tids;
    $last = "tn";

    if ($this->options['depth'] > 0) {
      $subquery .= "    LEFT JOIN {term_hierarchy} th ON th.tid = tn.tid\n";
      $last = "th";
      foreach (range(1, abs($this->options['depth'])) as $count) {
        $subquery .= "    LEFT JOIN {term_hierarchy} th$count ON $last.parent = th$count.tid\n";
        $where .= "    OR th$count.tid $placeholder\n";
        $args = array_merge($args, $tids);
        $last = "th$count";
      }
    }
    else if ($this->options['depth'] < 0) {
      foreach (range(1, abs($this->options['depth'])) as $count) {
        $subquery .= "    LEFT JOIN {term_hierarchy} th$count ON $last.tid = th$count.parent\n";
        $where .= "    OR th$count.tid $placeholder\n";
        $args = array_merge($args, $tids);
        $last = "th$count";
      }
    }

    $this->query->add_where(0, "$this->table_alias.$this->real_field IN ($subquery$where  )", $args);
  }

  function title() {
    $term = taxonomy_get_term($this->argument);
    if (!empty($term)) {
      return check_plain($term->name);
    }
    // TODO review text
    return t('No name');
  }
}
