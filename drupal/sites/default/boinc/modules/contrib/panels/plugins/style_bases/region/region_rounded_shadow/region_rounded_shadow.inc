<?php

/**
 * @file
 * Definition of the style base for the rounded shadow box.
 *
 * This box is colorable, has rounded corners and a drop shadow.
 */

$plugin = array(
  'category' => t('Basic styles'),
  'title' => t('Rounded shadow box'),
  'module' => 'panels',
  'type' => 'region',
  'css' => 'region-rounded-shadow.css',
  'icon' => 'icon.png',
  'defaults' => array('font' => array(), 'padding' => array()),
  'palette' => array(
    'background' => array(
      'label' => t('Background'),
      'default_value' => '#FFFFFF',
    ),
    'text' => array(
      'label' => t('Text color'),
      'default_value' => '#000000',
    ),
  ),
  'actions' => array(
    array('load', 'box', 'box-color.png'),
    array('colorize', 'background'),
    array('load', 'shadow', 'box-shadow.png'),
    array('merge_from', 'box'),
    array('slice', 'rounded-shadow-top-left-corner.png', 17, 49, 19, 19),
    array('slice', 'rounded-shadow-top-right-corner.png', 473, 49, 19, 19),
    array('slice', 'rounded-shadow-bottom-left-corner.png', 17, 442, 19, 19),
    array('slice', 'rounded-shadow-bottom-right-corner.png', 473, 442, 19, 19),
    array('slice', 'rounded-shadow-left-edge.png', 17, 60, 10, 10),
    array('slice', 'rounded-shadow-right-edge.png', 474, 60, 18, 10),
    array('slice', 'rounded-shadow-top-edge.png', 28, 49, 10, 10),
    array('slice', 'rounded-shadow-bottom-edge.png', 28, 443, 10, 18),
    array('slice', 'rounded-shadow-background.png', 150, 150, 1, 1),
  ),

  'theme' => 'region_rounded_shadow',

  'build' => 'panels_rounded_shadow_style_base_build',
//  'preview' => 'panels_stylizer_region_preview',

  'settings form' => 'panels_region_rounded_shadow_style_settings',
  'settings form submit' => 'panels_region_rounded_shadow_style_settings_submit',
  'build' => 'panels_region_rounded_shadow_style_base_build',
);

function panels_region_rounded_shadow_style_settings(&$form, &$form_state) {
  $form['font'] = array();
  ctools_stylizer_font_selector_form($form['font'], $form_state, t('Font'), $form_state['settings']['font']);

  $form['padding'] = array();
  ctools_stylizer_padding_selector_form($form['padding'], $form_state, t('Padding'), $form_state['settings']['padding']);
}

function panels_region_rounded_shadow_style_settings_submit(&$form, &$form_state) {
  ctools_stylizer_font_selector_form_submit($form['font'], $form_state, $form_state['values']['font'], $form_state['settings']['font']);
  ctools_stylizer_padding_selector_form_submit($form['padding'], $form_state, $form_state['values']['padding'], $form_state['settings']['padding']);
}

function panels_region_rounded_shadow_style_base_build($plugin, $settings, &$css, $replacements) {
  ctools_stylizer_font_apply_style($css, '%style', $settings['font']);
  ctools_stylizer_padding_apply_style($css, '%style', $settings['padding']);
}

