<?php
// $Id: views_or_handler_argument.inc,v 1.4 2009/08/12 19:15:27 darrenoh Exp $

/**
 * @file
 * Filter classes.
 *
 * These classes are always used together, so we keep them in the same file.
 */

/**
 * Generic views handler argument to add code to manipulate the query object.
 */
class views_or_handler_argument extends views_handler_argument {
  // Don't display empty space where the operator would be.
  var $no_operator = TRUE;

  function option_definition() {
    $options = parent::option_definition();
    $options['default_argument_fixed'] = array('default' => '');
    $options['default_action'] = array('default' => 'default');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['relationship']['#access'] = FALSE;
  }

  function validate_arg($arg) {
    return TRUE;
  }

  /**
   * Create a new array on the object to store data, and return a referrence to
   * it.
   */
  function &query_new_alternative() {
    if (empty($this->query->views_or)) {
      $this->query->views_or = array();
      $id = 0;
    }
    else {
      $id = max(array_keys($this->query->views_or)) + 1;
    }
    $this->query->views_or[$id] = array();
    return $this->query->views_or[$id];
  }

  /**
   * Return the current alternative block.
   */
  function &query_current_alternative() {
    if (empty($this->query->views_or)) {
      return $this->query_new_alternative();
    }
    return $this->query->views_or[max(array_keys($this->query->views_or))];
  }

  /**
   * Pop the current alternative block.
   */
  function query_pop_alternative() {
    unset($this->query->views_or[max(array_keys($this->query->views_or))]);
  }

  /**
   * Return the data structure from a given group.
   */
  function query_get_where_group($group) {
    if (isset($this->query->where[$group])) {
      return $this->query->where[$group];
    }
  }

  /**
   * Add a new group, with the given data structure, and return the id of the
   * group. If $as_default is true, this group becomes the default group and the
   * id returned is that of the old default group.
   */
  function query_add_where_group($group, $as_default = FALSE) {
    $new_group = $this->query->set_where_group('AND');
    if ($as_default) {
      $this->query->where[$new_group] = $this->query->where[0];
      $this->query->where[0] = $group;
    }
    else {
      $this->query->where[$new_group] = $group;
    }
    return $new_group;
  }

  /**
   * Removes a given group. If this is the default group, replace it with a new
   * empty group.
   */
  function query_remove_where_group($group) {
    if (isset($this->query->where[$group])) {
      if ($group == 0) {
        $group = $this->query->set_where_group('AND');
        $this->query->where[0] = $this->query->where[$group];
      }
      unset($this->query->where[$group]);
    }
  }

  /**
   * Compact the query such that there is only one where group. If $full is
   * TRUE, compacts such that there is only one clause in the group.
   */
  function query_compact($full = FALSE) {
    if (!empty($this->query->where)) {
      $new_group = array(
        'clauses' => array(),
        'args'    => array(),
        'type'    => $this->query->group_operator,
      );
      foreach ($this->query->where as $id => $content) {
        if (count($content['clauses'])) {
          if (count($content['clauses']) == 1) {
            $new_group['clauses'] = array_merge($new_group['clauses'], $content['clauses']);
          }
          else {
            $new_group['clauses'][] = '('. implode(') '. $content['type'] .' (', $content['clauses']) .')';
          }
          $new_group['args'] = array_merge($new_group['args'], $content['args']);
        }
      }
      if ($full && count($new_group['clauses']) > 1) {
        $new_clause = '('. implode(') '. $new_group['type'] .' (', $new_group['clauses']) .')';
        $new_group['clauses'] = array($new_clause);
      }
      $this->query->where = array(0 => $new_group);
    }
  }

  /* Finish an alternative. This is called to process the data whenever an
   * alternative statement finishes (ie. either from the "next alternative" or
   * "end alternatives" arguments).
   */
  function finish_alternative() {
    $alt = &$this->query_current_alternative();
    // Compact the query
    $this->query_compact(TRUE);
    // Look for INNER joins in the new tables, and make them LEFT joins + where condition instead
    $clauses = array();
    foreach (array_diff(array_keys($this->query->table_queue), $alt['tables']) as $table) {
      $alt['tables'][] = $table;
      $def = $this->query->table_queue[$table];
      if (!empty($def['join']) && $def['join']->type == 'INNER') {
        $this->query->table_queue[$table]['join']->type = 'LEFT';
        $field = $def['alias'] .'.'. $def['join']->field;
        $clauses[] = $field .' IS NOT NULL';
      }
    }
    if (count($clauses)) {
      // Add a new group with our clauses
      $grp = $this->query->set_where_group('AND');
      foreach ($clauses as $c) {
        $this->query->add_where($grp, $c);
      }
      // Make sure this will be ANDed, and compact again. The query
      // has already been compacted, so we know this will work.
      $old_group_operator = $this->query->group_operator;
      $this->query->group_operator = 'AND';
      $this->query_compact(TRUE);
      $this->query->group_operator = $old_group_operator;
    }
    // Now store it for later
    $alt['alternatives'][] = $this->query_get_where_group(0);
    $this->query_remove_where_group(0);
  }
}

/**
 * Filter handler to start a block of alternatives.
 */
class views_or_handler_argument_begin_alternatives extends views_or_handler_argument {
  function query() {
    $this->query_compact();
    $alt = &$this->query_new_alternative();
    $alt['tables'] = array_keys($this->query->table_queue);
    $alt['pre'] = $this->query_get_where_group(0);
    $alt['alternatives'] = array();
    $alt['position'] = $this->position;
    $this->query_remove_where_group(0);
    if ($this->view->args[$this->position] != '') {
      array_splice($this->view->args, $this->position, 0, '');
    }
  }
}

/**
 * Argument handler to indicate the next block of alternatives
 */
class views_or_handler_argument_next_alternative extends views_or_handler_argument {
  function option_definition() {
    $options = parent::option_definition();
    $options['share_arguments'] = array('default' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['share_arguments'] = array(
      '#type' => 'checkbox',
      '#title' => t('Share arguments'),
      '#default_value' => $this->options['share_arguments'],
      '#description' => t('Leave this checked to make this set of alternative arguments use the same arguments as the last set. This eliminates the need to repeat the same arguments in the URL for each set of alternatives.'),
    );
  }

  function query() {
    $this->finish_alternative();
    $alt = &$this->query_current_alternative();
    if ($this->view->args[$this->position] != '') {
      if ($this->options['share_arguments']) {
        array_splice($this->view->args, $this->position, 0, array_slice($this->view->args, $alt['position'], $this->position - $alt['position']));
        $alt['position'] = $this->position;
      }
      else {
        array_splice($this->view->args, $this->position, 0, '');
      }
    }
  }
}

/**
 * Argument handler to indicate the end of a block of alternatives.
 */
class views_or_handler_argument_end_alternatives extends views_or_handler_argument {
  function query() {
    $this->finish_alternative();
    $alt = &$this->query_current_alternative();
    $new_group = array(
      'clauses' => array(),
      'args'    => array(),
      'type'    => 'OR',
    );
    foreach ($alt['alternatives'] as $alternative) {
      if (empty($alternative['clauses'])) continue;
      $new_group['clauses'][] = $alternative['clauses'][0];
      $new_group['args'] = array_merge($new_group['args'], $alternative['args']);
    }
    $old = $this->query_add_where_group($alt['pre'], TRUE);
    $this->query_remove_where_group($old);
    if (count($new_group['clauses'])) {
      $this->query_add_where_group($new_group);
    }
    if (!isset($alt['pre'])) {
      unset($this->query->where[0]);
    }
    $this->query_pop_alternative();
    if ($this->view->args[$this->position] != '') {
      array_splice($this->view->args, $this->position, 0, '');
    }
  }
}
