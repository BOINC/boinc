<?php
// $Id$

/**
 * @file
 * The subclass adds basic field and formatter info,
 * for field-specific subclasses to use if they need to.
 *
 * Fields could extend this class if they want field and formatter handling
 * but don't want the multiple value grouping options created by
 * content_handler_field_multiple.
 */
class content_handler_field extends views_handler_field_node {
  var $content_field;

  function construct() {
    parent::construct();
    $this->content_field = content_fields($this->definition['content_field_name']);
  }

  function init(&$view, $options) {
    $field = $this->content_field;
    parent::init($view, $options);
    if ($field['multiple']) {
      $this->additional_fields['delta'] = 'delta';
    }
    // Make sure we grab enough information to build a pseudo-node with enough
    // credentials at render-time.
    $this->additional_fields['type'] = array('table' => 'node', 'field' => 'type');
    $this->additional_fields['nid'] = array('table' => 'node', 'field' => 'nid');
    $this->additional_fields['vid'] = array('table' => 'node', 'field' => 'vid');
  }

  function option_definition() {
    $options = parent::option_definition();
    $field = $this->content_field;

    // Override views_handler_field_node's default label
    $options['label'] = array('default' => '', 'translatable' => TRUE);
    $options['label_type'] = array('default' => 'widget');
    $options['format'] = array('default' => 'default');

    return $options;
  }

  /**
   * Provide formatter option.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    // TODO: do we want the 'link to node' checkbox ?
    // That's usually formatters business...

    $field = $this->content_field;
    $options = $this->options;

    $form['label_type'] = array(
      '#title' => t('Label'),
      '#type' => 'radios',
      '#options' => array(
        'none' => t('None'),
        'widget' => t('Widget label (@label)', array('@label' => $field['widget']['label'])),
        'custom' => t('Custom'),
      ),
      '#default_value' => $options['label_type'],
      '#weight' => 2,
    );
    $form['label'] = array(
      '#title' => t('Custom label'),
      '#type' => 'textfield',
      '#default_value' => $options['label'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('radio:options[label_type]' => array('custom')),
      '#weight' => 3,
     );

    $field_types = _content_field_types();
    $formatters = array();
    if (is_array($field_types[$field['type']]['formatters'])) {
      foreach ($field_types[$field['type']]['formatters'] as $name => $info) {
        $formatters[$name] = $info['label'];
      }
    }
    $form['format'] = array(
      '#title' => t('Format'),
      '#type' => 'select',
      '#options' => $formatters,
      '#required' => TRUE,
      '#default_value' => $options['format'],
      '#weight' => 4,
    );
  }


  /**
   * Make sure some value is stored as a label.
   *
   * Don't use t(), since Views' views_handler_field already has
   * $this->options['label'] marked as a translatable field.
   *
   * @see http://drupal.org/node/285470
   */
  function options_submit($form, &$form_state) {
    switch ($form_state['values']['options']['label_type']) {
      case 'none':
        $form_state['values']['options']['label'] = '';
        break;
      case 'widget':
        $form_state['values']['options']['label'] = $this->content_field['widget']['label'];
        break;
    }
  }

  /**
   * @TODO
   * Now that we save the label in the submit process above we could
   * get rid of this function. Leave it here for now to be sure the
   * label works for fields that haven't been updated since this
   * change was made, since $this->options['label'] will be missing a
   * value until it is updated in the view.
   *
   * Don't use t(), since Views' views_handler_field already has
   * $this->options['label'] marked as a translatable field.
   */
  function label() {
    $field = $this->content_field;
    switch ($this->options['label_type']) {
      case 'none':
        return '';
      case 'widget':
        return $field['widget']['label'];
      default:
        return $this->options['label'];
    }
  }

  /**
   * Return DIV or SPAN based upon the field's element type.
   */
  function element_type($none_supported = FALSE, $default_empty = FALSE) {
    // The 'element_type' property denotes Views 3.x ('semantic views'
    // functionnality). If the property is set, and not set to '' ("default"),
    // let the generic method handle the output.
    if (isset($this->options['element_type']) && $this->options['element_type'] !== '') {
      return parent::element_type($none_supported, $default_empty);
    }

    if ($default_empty) {
      return '';
    }

    if (isset($this->definition['element type'])) {
      return $this->definition['element type'];
    }

    // TODO Figure out exactly when to return a div or a <span>. Any field
    // that ever needs to be shown inline in Views UI. It needs to return
    // a div for textareas to prevent wrapping a <span> around a <p>.
    // Earl says we need to be sure that other fields we don't know
    // about won't end up wrapping a span around a block-level element.
    if ($this->content_field['widget']['type'] == 'text_textarea') {
      return 'div';
    }
    else {
      return 'span';
    }
  }

  function options_validate($form, &$form_state) { }

  /**
   * Provide text for the administrative summary
   */
  function admin_summary() {
    // Display the formatter name.
    $field = $this->content_field;
    $field_types = _content_field_types();
    if (isset($field_types[$field['type']]['formatters'][$this->options['format']])) {
      return t($field_types[$field['type']]['formatters'][$this->options['format']]['label']);
    }
  }

  function render($values) {
    // We're down to a single node here, so we can retrieve the actual field
    // definition for the node type being considered.
    $field = content_fields($this->content_field['field_name'], $values->{$this->aliases['type']});

    // If the field does not appear in the node type, then we have no value
    // to display, and can just return.
    if (empty($field)) {
      return '';
    }

    $options = $this->options;
    $db_info = content_database_info($field);

    // Build a pseudo-node from the retrieved values.
    $node = drupal_clone($values);
    $node->type = $values->{$this->aliases['type']};
    $node->nid = $values->{$this->aliases['nid']};
    $node->vid = $values->{$this->aliases['vid']};
    // Some formatters need to behave differently depending on the build_mode
    // (for instance: preview), so we provide one.
    $node->build_mode = NODE_BUILD_NORMAL;

    $item = array();
    foreach ($db_info['columns'] as $column => $attributes) {
      $item[$column] = $values->{$this->aliases[$attributes['column']]};
    }

    $item['#delta'] = $field['multiple'] ?  $values->{$this->aliases['delta']} : 0;

    // Render items.
    $formatter_name = $options['format'];
    if ($formatter = _content_get_formatter($formatter_name, $field['type'])) {
      if (content_handle('formatter', 'multiple values', $formatter) == CONTENT_HANDLE_CORE) {
        // Single-value formatter.
        $output = content_format($field, $item, $formatter_name, $node);
      }
      else {
        // Multiple values formatter - we actually have only one value to display.
        $output = content_format($field, array($item), $formatter_name, $node);
      }
      return $this->render_link($output, $values);
    }
    return '';
  }

}
