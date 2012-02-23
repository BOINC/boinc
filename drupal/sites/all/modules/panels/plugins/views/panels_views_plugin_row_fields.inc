<?php
/**
 * @file
 * Contains the base row style plugin.
 */

/**
 * The basic 'fields' row plugin
 *
 * This displays fields one after another, giving options for inline
 * or not.
 *
 * @ingroup views_row_plugins
 */
class panels_views_plugin_row_fields extends views_plugin_row_fields {
  function option_definition() {
    $options = parent::option_definition();

    $options['layout'] = array('default' => 'twocol');
    $options['regions'] = array('default' => array());

    return $options;
  }

  /**
   * Provide a form for setting options.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    ctools_include('plugins', 'panels');
    $layouts = panels_get_layouts();
    $options = array();
    foreach ($layouts as $name => $layout) {
      if (empty($layout['builder'])) {
        $options[$name] = $layout['title'];
      }
      if ($name == $this->options['layout']) {
        $current_layout = $layout;
      }
    }

    $form['layout'] = array(
      '#prefix' => '<div class="container-inline">',
      '#type' => 'select',
      '#options' => $options,
      '#title' => t('Panel layout'),
      '#default_value' => $this->options['layout'],
    );

    $form['change'] = array(
      '#type' => 'submit',
      '#value' => t('Change'),
      '#submit' => array('panels_change_layout_button'),
      '#suffix' => '</div>',
    );

    if (!empty($current_layout)) {
      $fields = $this->display->handler->get_field_labels();
      $regions = panels_get_regions($current_layout, panels_new_display());
      foreach ($fields as $id => $title) {
        $form['regions'][$id] = array(
          '#type' => 'select',
          '#title' => $title,
          '#options' => $regions,
        );
        if (!empty($this->options['regions'][$id]) && !empty($regions[$this->options['regions'][$id]])) {
          $form['regions'][$id]['#default_value'] = $this->options['regions'][$id];
        }
      }
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit($form, &$form_state) {
    $form_state['values']['row_options']['inline'] = array_filter($form_state['values']['row_options']['inline']);
  }

  /**
   * Render a row object. This usually passes through to a theme template
   * of some form, but not always.
   */
  function render($row) {
    ctools_include('plugins', 'panels');
    $layout = panels_get_layout($this->options['layout']);
    if (!$layout) {
      // Fall back to normal behavior if the layout is somehow invalid. This
      // can happen if the layout was removed, for example.
      return theme($this->theme_functions(), $this->view, $this->options, $row, $this->field_alias);
    }

    // Store a backup copy of the array because we're going to be screwing
    // with this a lot.
    $fields = $this->view->field;
    unset($this->view->field);

    $meta = 'standard';
    // This row style gets run many times; only run this code once.
    if (empty($this->region_fields)) {
      $this->region_fields = array();
      $regions = panels_get_regions($layout, panels_new_display());

      // Ensure each region has an empty array.
      foreach ($regions as $region_id => $name) {
        if (empty($default_region)) {
          $default_region = $region_id;
        }

        $this->region_fields[$region_id] = array();
      }


      // Go through all our fields and place them in regions according to the
      // settings.
      foreach ($fields as $id => $field) {
        $region_id = ''; // ensure we don't accidentlly use the last field's region.
        if (!empty($this->options['regions'][$id]) && !empty($regions[$this->options['regions'][$id]])) {
          $region_id = $this->options['regions'][$id];
        }
        else {
          // Fallback to putting unknown fields into the first region.
          $region_id = $default_region;
        }

        // Ensure this works in PHP4 by keeping the reference.
        $this->region_fields[$region_id][$id] = &$fields[$id];
      }

      // We don't need to set 'inline' for every record, so we do it inside
      // this loop. We do need to set inline if we are in the live preview
      // so that the CSS will get transmitted via javascript:
      $meta = !empty($this->view->live_preview) ? 'inline' : 'standard';
    }

    // Now that we have distributed our fields, go through the regions and
    // render them into the content array.
    foreach ($this->region_fields as $region_id => $fields) {
      $this->view->field = $fields;
      $content[$region_id] = theme($this->theme_functions(), $this->view, $this->options, $row, $this->field_alias);
    }

    // Restore our $fields array.
    $view->field = $fields;

    // Now that we have a rendered content array, render it.
    return panels_print_layout($layout, $content, $meta);
  }
}

/**
 * Override handler for views_ui_edit_display_form
 */
function panels_change_layout_button($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_submit($form, $form_state);

  views_ui_cache_set($form_state['view']);
  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
}
