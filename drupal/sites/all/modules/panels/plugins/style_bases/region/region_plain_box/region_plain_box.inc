<?php

/**
 * @file
 * Definition of the style base for the rounded shadow box.
 *
 * This box is colorable, has rounded corners and a drop shadow.
 */

$plugin = array(
  'category' => t('Basic styles'),
  'title' => t('Plain'),
  'description' => t('A plain box with an optional border. You may set the color of the text and the border.'),
  'module' => 'panels',
  'type' => 'region',

  'css' => 'region-plain-box.css',
  'icon' => 'icon.png',

  'defaults' => array('font' => array(), 'border' => array(), 'padding' => array()),
  'palette' => array(
    'background' => array(
      'label' => t('Background'),
      'default_value' => '#FFFFFF',
    ),
    'text' => array(
      'label' => t('Text'),
      'default_value' => '#000000',
    ),
    'border' => array(
      'label' => t('Border'),
      'default_value' => '#000000',
    ),
  ),

  'theme' => 'region_plain_box',
//  'preview' => 'panels_stylizer_region_preview',

  'settings form' => 'panels_region_plain_box_style_settings',
  'settings form submit' => 'panels_region_plain_box_style_settings_submit',
  'build' => 'panels_region_plain_box_style_base_build',
);

function panels_region_plain_box_style_settings(&$form, &$form_state) {
  $form['font'] = array();
  ctools_stylizer_font_selector_form($form['font'], $form_state, t('Font'), $form_state['settings']['font']);

  $form['border'] = array();
  ctools_stylizer_border_selector_form($form['border'], $form_state, t('Border'), $form_state['settings']['border']);

  $form['padding'] = array();
  ctools_stylizer_padding_selector_form($form['padding'], $form_state, t('Padding'), $form_state['settings']['padding']);
}

function panels_region_plain_box_style_settings_submit(&$form, &$form_state) {
  ctools_stylizer_font_selector_form_submit($form['font'], $form_state, $form_state['values']['font'], $form_state['settings']['font']);
  ctools_stylizer_border_selector_form_submit($form['border'], $form_state, $form_state['values']['border'], $form_state['settings']['border']);
  ctools_stylizer_padding_selector_form_submit($form['padding'], $form_state, $form_state['values']['padding'], $form_state['settings']['padding']);
}

function panels_region_plain_box_style_base_build($plugin, $settings, &$css, $replacements) {
  ctools_stylizer_font_apply_style($css, '%style', $settings['font']);
  ctools_stylizer_border_apply_style($css, '%style', $settings['border'], '%border');
  ctools_stylizer_padding_apply_style($css, '%style', $settings['padding']);
}
