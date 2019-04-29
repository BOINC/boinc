<?php

/**
 * @file
 * Defines the jump style plugin.
 */

class jump_views_plugin_style extends views_plugin_style {

  function render() {
    // Build an array of field / field_alias pairs.
    $this->field_map = array();
    foreach ($this->view->field as $field_name => $object) {
      $this-> field_map[$object->field_alias] = $field_name;
    }

    // $keys contains the fields used to build up the path.
    $keys = array_filter($this->options['keys']);

    // Start building the actual $select options array to pass to our form
    // callback.
    $select = array();
    if (!empty($this->options['first_item'])) {
      $select[0] = $this->options['first_item'];
    }

    foreach ($this->view->result as $row) {
      $select_key = array();
      // For each field in the row, we first check if it's to be used
      // as part of the select key, then run views token replacement
      // on it. This allows for paths like node/n/revisions/6 to be built
      // easily.
      foreach ($row as $field => $value) {
        if (in_array($this->field_map[$field], $keys)) {
          $field_object = $this->view->field[$this->field_map[$field]];
          // Re-implementing some of the code from views_handler_field.inc
          // since we need to operate on $result, and to avoid using
          // a row style or templates.
          if (!empty($field_object->options['alter']['alter_text']) && !empty($field_object->options['alter']['text'])) {
            $tokens = $this->get_render_tokens($this->field_map[$field], $row);
            $value = $field_object->render_altered($tokens);
          }
          $select_key[] = $value;
        }
      }
      // For the value of the select, just use the output of the row, but strip
      // all markup first.
      $select_key = implode('/', $select_key);
      $raw_select_value = $this->row_plugin->render($row);
      $select_value = trim(html_entity_decode(strip_tags($raw_select_value), ENT_QUOTES));
      $select[$select_key] = $select_value;
    }

    return jump_quickly($select);
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['keys'] = array('default' => array());
    $options['first_item'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, $form_state) {
    parent::options_form($form, $form_state);
    $handlers = $this->display->handler->get_handlers('field');
    if (empty($handlers)) {
      $form['error_markup'] = array(
        '#value' => t('You need at least one field before you can configure your jump menu settings'),
        '#prefix' => '<div class="error form-item description">',
        '#suffix' => '</div>',
      );
      return;
    }
    // Create an array of fields from the data we know.
    foreach ($handlers as $field => $handler) {
      if ($label = $handler->label()) {
        $field_names[$field] = $label;
      }
      else {
        $field_names[$field] = $handler->ui_name();
      }
    }
    $form['keys'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Select keys'),
      '#default_value' => $this->options['keys'],
      '#description' => t("These fields will be used to build the path for each row, used as the 'key' in the select options, and will be automatically separated by '/'"),
      '#options' => $field_names,
    );
    $form['first_item'] = array(
      '#type' => 'textfield',
      '#title' => 'First item',
      '#default_value' => $this->options['first_item'],
      '#description' => t('This will be used as the first item in your select list, for example <em>Choose one</em>'),
    );
  }

  /**
   * Get the 'render' tokens to use for advanced rendering.
   *
   * This runs through all of the fields and arguments that
   * are available and gets their values. This will then be
   * used in one giant str_replace().
   * Slightly modified from views_handler_field.inc
   */
  function get_render_tokens($field_name, $row) {
    $tokens = array();
    if (!empty($this->view->build_info['substitutions'])) {
      $tokens = $this->view->build_info['substitutions'];
    }

    $count = 0;
    foreach ($this->view->display_handler->get_handlers('argument') as $arg => $handler) {
      $token = '%'. ++$count;
      if (!isset($tokens[$token])) {
        $tokens[$token] = '';
      }
    }

    // Now add replacements for our fields.
    $options = array();
    $map = array_flip($this->field_map);
    foreach ($this->view->display_handler->get_handlers('field') as $field => $handler) {
      $map = array_flip($this->field_map);
      if (isset($row->{$map[$field]})) {
        $tokens["[$field]"] = $row->{$map[$field]};
      }
      else {
        $tokens["[$field]"] = '';
      }
    }

    return $tokens;
  }
}
