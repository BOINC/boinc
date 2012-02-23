<?php
// $Id: views_or_handler_field.inc,v 1.3 2009/04/28 19:01:55 darrenoh Exp $

/**
 * @file
 * Field classes.
 *
 * These classes are always used together, so we keep them in the same file.
 */

/**
 * Base class for Views Or field handlers.
 */
class views_or_handler_field extends views_handler_field {
  /**
   * Set default values for form elements.
   */
  function option_definition() {
    $options['exclude'] = array('default' => TRUE);
    return $options;
  }

  /**
   * Remove unnecessary form elements.
   */
  function options_form(&$form, &$form_state) {
    $form['relationship']['#access'] = FALSE;
  }

  /**
   * Remove advanced rendering options from form.
   */
  function allow_advanced_render() {
    return FALSE;
  }
}

/**
 * Field handler to start a group of alternatives.
 */
class views_or_handler_field_begin_alternatives extends views_or_handler_field {
  /**
   * Save existing fields and start a new field group.
   */
  function query() {
    $base = $this->query->base_field;
    $fields = $this->query->fields;
    if (isset($fields[$base])) {
      $this->query->fields = array($base => $fields[$base]);
      unset($fields[$base]);
    }
    else {
      $this->query->fields = array();
    }
    $this->query->views_or = $fields;
  }

  /**
   * Replace individual fields in the output with the COALESCE group.
   */
  function pre_render($values) {
    $coalesce = FALSE;
    foreach ($this->view->field as $id => $field) {
      if ($field->field == 'views_or_begin') {
        $coalesce = TRUE; // We are in a COALESCE group.
        $first = FALSE; // This is not the first field in the group.
      }
      if ($coalesce) {
        if (!$first) {
          // Remove all fields except the first.
          unset($this->view->field[$id]);
        }
        // The next field will be the first in the group.
        $first = $field->field == 'views_or_begin' ? TRUE : FALSE;
      }
      if ($field->field == 'views_or_end') {
        $coalesce = FALSE; // We have reached the end of the COALESCE group.
      }
    }
  }
}

/**
 * Field handler to end a group of alternatives.
 */
class views_or_handler_field_end_alternatives extends views_or_handler_field {
  /**
   * Replace individual fields in the query with the COALESCE group.
   */
  function query() {
    $base = $this->query->base_field;
    $fields = $this->query->fields;
    if (isset($fields[$base])) {
      $this->query->fields = array($base => $fields[$base]);
      unset($fields[$base]);
    }
    $this->query->fields = array_merge($this->query->fields, $this->query->views_or);
    if (!empty($fields)) {
      $coalesce = array();
      $alias = NULL;
      foreach ($fields as $field) {
        $coalesce[] = "$field[table].$field[field]";
        $alias = isset($alias) ? $alias : $field['alias'];
      }
      $this->query->add_field(NULL, 'COALESCE('. implode(', ', $coalesce) .')', $alias);
    }
  }
}
