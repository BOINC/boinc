<?php
// $Id: views_plugin_row_node_view.inc,v 1.3 2009/04/08 20:54:58 merlinofchaos Exp $
/**
 * @file
 * Contains the node view row style plugin.
 */

/**
 * Plugin which performs a node_view on the resulting object.
 *
 * Most of the code on this object is in the theme function.
 */
class views_plugin_row_node_view extends views_plugin_row {
  // Basic properties that let the row style follow relationships.
  var $base_table = 'node';
  var $base_field = 'nid';

  function init(&$view, &$display, $options = NULL) {
    parent::init($view, $display, $options);
    // Handle existing views with the deprecated 'teaser' option.
    if (isset($this->options['teaser'])) {
      $this->options['build_mode'] = $this->options['teaser'] ? 'teaser' : 'full';
    }
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['build_mode'] = array('default' => 'teaser');
    $options['links'] = array('default' => TRUE);
    $options['comments'] = array('default' => FALSE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // CCK holds the registry of available build modes, but can hardly
    // push them as options for the node row style, so we break the normal
    // rule of not directly relying on non-core modules.
    if ($modes = module_invoke('content', 'build_modes')) {
      $options = array();
      foreach ($modes as $key => $value) {
        if (isset($value['views style']) && $value['views style']) {
          $options[$key] = $value['title'];
        }
      }
    }
    else {
      $options = array(
        'teaser' => t('Teaser'),
        'full' => t('Full node')
      );
    }
    $form['build_mode'] = array(
      '#type' => 'select',
      '#options' => $options,
      '#title' => t('Build mode'),
      '#default_value' => $this->options['build_mode'],
     );
    $form['links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display links'),
      '#default_value' => $this->options['links'],
    );
    $form['comments'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display node comments'),
      '#default_value' => $this->options['comments'],
    );
  }
}
