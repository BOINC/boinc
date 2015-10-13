<?php
// $Id$

/**
 * @file
 * An extended subclass for field handling that adds multiple field grouping.
 *
 * Fields that want multiple value grouping options in addition to basic
 * field and formatter handling can extend this class.
 */
class content_handler_field_multiple extends content_handler_field {
  var $defer_query;

  function init(&$view, $options) {
    $field = $this->content_field;
    parent::init($view, $options);

    $this->defer_query = !empty($options['multiple']['group']) && $field['multiple'];

    if ($this->defer_query) {
      // Grouped field: ditch the existing additional_fields (field columns + delta).
      // In the main query we'll only need:
      // - vid, which will be used to retrieve the actual values in pre_render,
      // - node type and nid, which wil be used in the pseudo-node used when
      // rendering.
      $this->additional_fields = array(
        'type' => array('table' => 'node', 'field' => 'type'),
        'nid' => array('table' => 'node', 'field' => 'nid'),
      );
      if ($view->base_table == 'node_revisions') {
        $this->additional_fields['vid'] = array('table' => 'node_revisions', 'field' => 'vid');
      }
      else {
        $this->additional_fields['vid'] = array('table' => 'node', 'field' => 'vid');
      }
    }
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['multiple'] = array(
      'contains' => array(
        'group' => array('default' => TRUE),
        'multiple_number' => array('default' => ''),
        'multiple_from' => array('default' => ''),
        'multiple_reversed' => array('default' => FALSE),
      ),
    );

    return $options;
  }

  /**
   * Provide 'group multiple values' option.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $field = $this->content_field;
    $options = $this->options;

    $form['multiple'] = array(
      '#access' => $field['multiple'],
      '#weight' => 1,
    );
    $form['multiple']['group'] = array(
      '#title' => t('Group multiple values'),
      '#type' => 'checkbox',
      '#default_value' => $options['multiple']['group'],
      '#description' => t('If unchecked, each item in the field will create a new row, which may appear to cause duplicates. This setting is not compatible with click-sorting in table displays.'),
    );
    // Make the string translatable by keeping it as a whole rather than
    // translating prefix and suffix separately.
    list($prefix, $suffix) = explode('@count', t('Show @count value(s)'));
    $form['multiple']['multiple_number'] = array(
      '#type' => 'textfield',
      '#size' => 5,
      '#field_prefix' => $prefix,
      '#field_suffix' => $suffix,
      '#default_value' => $options['multiple']['multiple_number'],
      '#prefix' => '<div class="container-inline">',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-multiple-group' => array(TRUE)),
    );
    list($prefix, $suffix) = explode('@count', t('starting from @count'));
    $form['multiple']['multiple_from'] = array(
      '#type' => 'textfield',
      '#size' => 5,
      '#field_prefix' => $prefix,
      '#field_suffix' => $suffix,
      '#default_value' => $options['multiple']['multiple_from'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-multiple-group' => array(TRUE)),
      '#description' => t('(first item is 0)'),
    );
    $form['multiple']['multiple_reversed'] = array(
      '#title' => t('Reversed'),
      '#type' => 'checkbox',
      '#default_value' => $options['multiple']['multiple_reversed'],
      '#suffix' => '</div>',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-multiple-group' => array(TRUE)),
      '#description' => t('(start from last values)'),
    );
  }

  /**
   * Determine if this field is click sortable.
   */
  function click_sortable() {
    $field = $this->content_field;
    $options = $this->options;

    // Grouped fields are not click-sortable.
    return !empty($this->definition['click sortable']) && !$this->defer_query;
  }

  function query() {
    // If this is not a grouped field, use the generic query().
    if (!$this->defer_query) {
      return parent::query();
    }

    // Grouped field: do NOT call ensure_my_table, only add additional fields.
    $this->add_additional_fields();
    $this->field_alias = $this->aliases['vid'];
  }

  function pre_render(&$values) {
    // If there are no values to render (displaying a summary, or query returned no results),
    // or if this is not a grouped field, do nothing specific.
    if (isset($this->view->build_info['summary']) || empty($values) || !$this->defer_query) {
      return parent::pre_render($values);
    }

    $field = $this->content_field;
    $db_info = content_database_info($field);
    $options = $this->options;

    // Build the list of vids to retrieve.
    // TODO: try fetching from cache_content first ??
    $vids = array();
    $this->field_values = array();
    foreach ($values as $result) {
      if (isset($result->{$this->field_alias})) {
        $vids[] = $result->{$this->field_alias};
      }
    }

    // It may happend that the multiple values field is related to a non
    // required relation for which no node data related to the field being
    // processed here is available.
    if (empty($vids)) {
      return parent::pre_render($values);
    }

    // List columns to retrieve.
    $alias = content_views_tablename($field);
    // Prefix aliases with '_' to avoid clashing with field columns names.
    $query_columns = array(
      'vid AS _vid',
      "delta as _delta",
      // nid is needed to generate the links for 'link to node' option.
      'nid AS _nid',
    );
    // The actual field columns.
    foreach ($db_info['columns'] as $column => $attributes) {
      $query_columns[] = "$attributes[column] AS $column";
    }
    $query = 'SELECT '. implode(', ', $query_columns) .
             ' FROM {'. $db_info['table'] ."}".
             " WHERE vid IN (". implode(',', $vids) .')'.
             " ORDER BY _nid ASC, _delta ". ($options['multiple']['multiple_reversed'] ? 'DESC' : 'ASC');
    $result = db_query($query);

    while ($item = db_fetch_array($result)) {
      // Clean up the $item from vid and delta. We keep nid for now.
      $vid = $item['_vid'];
      unset($item['_vid']);
      $delta = !empty($item['_delta']) ? $item['_delta'] : 0;
      $item['#delta'] = $item['_delta'];
      unset($item['_delta']);
      $this->field_values[$vid][$delta] = $item;
    }
  }

  /**
   * Return DIV or SPAN based upon the field's element type.
   *
   * Fields rendered with the 'group multiple' option use <div> markers,
   * and thus shouldn't be wrapped in a <span>.
   */
  function element_type($none_supported = FALSE, $default_empty = FALSE) {
    // If this is not a grouped field, use the parent method.
    if (!$this->defer_query) {
      return parent::element_type($none_supported, $default_empty);
    }

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

    return 'div';
  }

  function render($values) {
    // If this is not a grouped field, use content_handler_field::render().
    if (!$this->defer_query) {
      return parent::render($values);
    }

    // We're down to a single node here, so we can retrieve the actual field
    // definition for the node type being considered.
    $field = content_fields($this->content_field['field_name'], $values->{$this->aliases['type']});

    // If the field does not appear in the node type, then we have no value
    // to display, and can just return.
    if (empty($field)) {
      return '';
    }

    $options = $this->options;

    $vid = $values->{$this->field_alias};
    if (isset($this->field_values[$vid])) {
      // Gather items, respecting the 'Display n values starting from m' settings.
      $count_skipped = 0;
      $items = array();
      foreach ($this->field_values[$vid] as $item) {
        if (empty($options['multiple']['multiple_from']) || ($count_skipped >= $options['multiple']['multiple_from'])) {
          if (empty($options['multiple']['multiple_number']) || (count($items) < $options['multiple']['multiple_number'])) {
            // Grab the nid - needed for render_link().
            $nid = $item['_nid'];
            unset($item['_nid']);
            $items[] = $item;
          }
          else {
            break;
          }
        }
        $count_skipped++;
      }

      // Build a pseudo-node from the retrieved values.
      $node = drupal_clone($values);
      // content_format and formatters will need a 'type'.
      $node->type = $values->{$this->aliases['type']};
      $node->nid = $values->{$this->aliases['nid']};
      $node->vid = $values->{$this->aliases['vid']};

      // Some formatters need to behave differently depending on the build_mode
      // (for instance: preview), so we provide one.
      $node->build_mode = NODE_BUILD_NORMAL;

      // Render items.
      $formatter_name = $options['format'];
      if ($items && ($formatter = _content_get_formatter($formatter_name, $field['type']))) {
        $rendered = array();
        if (content_handle('formatter', 'multiple values', $formatter) == CONTENT_HANDLE_CORE) {
          // Single-value formatter.
          foreach ($items as $item) {
            $output = content_format($field, $item, $formatter_name, $node);
            if (!empty($output)) {
              $rendered[] = $this->render_link($output, (object) array('nid' => $nid));
            }
          }
        }
        else {
          // Multiple values formatter.
          $output = content_format($field, $items, $formatter_name, $values);
          if (!empty($output)) {
            $rendered[] = $this->render_link($output, (object) array('nid' => $nid));
          }
        }

        if (count($rendered) > 1) {
          // TODO: could we use generic field display ?
          return theme('content_view_multiple_field', $rendered, $field, $values);
        }
        elseif ($rendered) {
          return $rendered[0];
        }
      }
    }

    return '';
  }

  function render_link($data, $values) {
    if (!$this->defer_query) {
      return parent::render_link($data, $values);
    }

    if (!empty($this->options['link_to_node']) && $data !== NULL && $data !== '') {
      if (method_exists('render_as_link', 'views_handler_field')) {
        // Views 2.3+
        $this->options['alter']['make_link'] = TRUE;
        $this->options['alter']['path'] = "node/" . $values->{$this->aliases['nid']};
      }
      else {
        // Views up to 2.2
        return l($data, "node/" . $values->nid, array('html' => TRUE));
      }
    }
    else {
      return $data;
    }
  }

}
