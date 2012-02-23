<?php
// $Id: content_plugin_display_simple.inc,v 1.1.2.5 2010/06/09 04:10:25 karens Exp $

/**
 * @file
 * Handler for 'content_simple' display.
 */
class content_plugin_display_simple extends views_plugin_display {
  function execute() {
    return $this->view->render($this->display->id);
  }

  function render() {
    return !empty($this->view->result) || !empty($this->view->style_plugin->definition['even empty']) ? $this->view->style_plugin->render($this->view->result) : '';
  }

  function uses_exposed() {
    return FALSE;
  }
}

class content_plugin_display_references extends content_plugin_display_simple {
  function query() {
    $options = $this->get_option('content_options');

    if ($options['string'] !== '') {
      $like = $GLOBALS["db_type"] == 'pgsql' ? "ILIKE" : "LIKE";
      $match_clauses = array(
        'contains' => "$like '%%%s%%'",
        'equals' => "= '%s'",
        'starts_with' => "$like '%s%%'",
      );
      $clause = isset($match_clauses[$options['match']]) ? $match_clauses[$options['match']] : $match_clauses['contains'];
      $alias = $this->view->query->ensure_table($options['table']);
      $this->view->query->add_where(NULL, "$alias.$options[field_string] $clause", $options['string']);
    }
    elseif ($options['ids']) {
      $alias = $this->view->query->ensure_table($options['table']);
      $this->view->query->add_where(NULL, "$alias.$options[field_id] IN (" . db_placeholders($options['ids']) . ')', $options['ids']);
    }
  }
}

