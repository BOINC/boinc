<?php

class context_layouts_reaction_block extends context_reaction_block {
  /**
   * Override of is_enabled_region().
   * Check that there is an active layout and it supports the given region.
   */
  protected function is_enabled_region($region) {
    $layout = $this->get_active_layout();
    if ($layout && isset($layout['regions']) && is_array($layout['regions'])) {
      return in_array($region, $layout['regions'], TRUE) && parent::is_enabled_region($region);
    }
    return parent::is_enabled_region($region);
  }

  /**
   * Retrieve the first layout specified found by any active contexts.
   */
  function get_active_layout($info = TRUE) {
    $contexts = $this->get_contexts();
    $layouts = context_layouts_get_layouts();
    if (!empty($contexts) && !empty($layouts)) {
      foreach ($contexts as $context) {
        $values = $this->fetch_from_context($context);
        if (isset($values['layout']) && isset($layouts[$values['layout']])) {
          return $info ? $layouts[$values['layout']] : $values['layout'];
        }
      }
    }
    // Fallback to default layout if provided.
    if (isset($layouts['default'])) {
      return $info ? $layouts['default'] : 'default';
    }
    return FALSE;
  }

  /**
   * Add the layout template to page vars.
   */
  function add_layout_template(&$vars) {
    if ($layout = $this->get_active_layout()) {
      if (!empty($layout['template'])) {
        $vars['template_files'][] = $layout['template'];
      }
    }
  }

  /**
   * Add the layout stylesheet to the CSS.
   */
  function add_layout_stylesheet() {
    if ($layout = $this->get_active_layout()) {
      if (!empty($layout['stylesheet'])) {
        drupal_add_css(drupal_get_path('theme', $layout['theme']) .'/'. $layout['stylesheet']);
      }
    }
  }

  /**
   * Override of editor form.
   */
  function editor_form($context) {
    drupal_add_css(drupal_get_path('module', 'context_layouts') .'/plugins/context_layouts_reaction_block.css');

    $form = parent::editor_form($context);

    if ($layouts = $this->get_layout_options()) {
      $options = $this->fetch_from_context($context);
      $form['layout'] = array(
        // #tree *must* be true for our values to be nested correctly.
        '#tree' => TRUE,
        '#prefix' => '<div class="context-editor-block-layouts">',
        '#suffix' => '</div>',
        '#weight' => -100,
        'layout' => array(
          '#title' => t('Layout'),
          '#options' => $layouts,
          '#type' => 'select',
          '#weight' => -100,
          '#default_value' => isset($options['layout']) ? $options['layout'] : NULL,
        ),
        'update' => array(
          '#value' => t('Change layout'),
          '#type' => 'submit',
        ),
      );
    }
    return $form;
  }

  /**
   * Override of editor form submit.
   */
  function editor_form_submit(&$context, $values) {
    // Someone has changed the layout, assume that the block values are not actually usable here.
    if (isset($context->reactions['block']['layout']) && $context->reactions['block']['layout'] != $values['layout']['layout']) {
      $options = $context->reactions['block'];
    }
    else {
      $options = parent::editor_form_submit($context, $values);
    }

    if (!empty($values['layout']['layout'])) {
      $options['layout'] = $values['layout']['layout'];
    }
    else {
      unset($options['layout']);
    }
    return $options;
  }

  /**
   * Override of options form.
   */
  function options_form($context) {
    $form = parent::options_form($context);
    $options = $this->fetch_from_context($context);

    // Only alter the options form if the theme provides layouts.
    $theme_key = variable_get('theme_default', 'garland');
    $layouts = $this->get_layout_options();
    if (!empty($layouts)) {
      $form['layout'] = array(
        '#title' => t('Layout'),
        '#description' => t('Choose one of the layouts provided by the default theme.'),
        '#options' => $layouts,
        '#type' => 'select',
        '#weight' => -100,
        '#default_value' => $options['layout'] ? $options['layout'] : NULL,
        '#attributes' => array('class' => 'context-blockform-layout'),
      );

      // Add js.
      // @TODO: Move this to a theme function or somewhere that will get called even
      // if the form is using a cached version of itself (e.g. when validate fails).
      drupal_add_js(drupal_get_path('module', 'context_layouts') .'/plugins/context_layouts_reaction_block.js');
      drupal_add_js(array('contextLayouts' => array('layouts' => $this->get_layout_regions())), 'setting');
    }
    return $form;
  }

  /**
   * Override of submit handler.
   */
  function options_form_submit($values) {
    $options = parent::options_form_submit($values);

    // Only alter the options form if the theme provides layouts.
    $theme_key = variable_get('theme_default', 'garland');
    $layouts = context_layouts_get_layouts($theme_key);

    // Check that this is a valid layout.
    if (!empty($values['layout']) && isset($layouts[$values['layout']])) {
      $layout = $values['layout'];
      $options['layout'] = $layout;

      // Remove blocks that don't belong to regions in this layout.
      if (isset($layouts[$layout]['regions'])) {
        foreach ($options['blocks'] as $bid => $block) {
          if (!in_array($block['region'], $layouts[$layout]['regions'])) {
            unset($options['blocks'][$bid]);
          }
        }
      }
    }
    return $options;
  }

  /**
   * Get layout options for the given theme.
   */
  protected function get_layout_options($theme_key = NULL) {
    $theme_key = !isset($theme_key) ? variable_get('theme_default', 'garland') : $theme_key;
    $layouts = context_layouts_get_layouts($theme_key);
    $layout_options = array();
    if (!empty($layouts)) {
      $layout_options[0] = '<'. t('Site default') .'>';
      foreach ($layouts as $layout => $info) {
        $layout_options[$layout] = isset($info['name']) ? $info['name'] : $layout_options;
      }
    }
    return $layout_options;
  }

  /**
   * Get a layout to region map for the given theme.
   */
  protected function get_layout_regions($theme_key = NULL) {
    $theme_key = !isset($theme_key) ? variable_get('theme_default', 'garland') : $theme_key;
    $layouts = context_layouts_get_layouts($theme_key);
    if (!empty($layouts)) {
      $layout_regions = array();
      foreach ($layouts as $layout => $info) {
        $layout_regions[$layout] = is_array($info['regions']) ? $info['regions'] : array();
      }
    }
    return $layout_regions;
  }
}
