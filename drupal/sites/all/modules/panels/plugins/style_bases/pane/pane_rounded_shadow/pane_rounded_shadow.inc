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
  'type' => 'pane',
  'css' => 'pane-rounded-shadow.css',
  'icon' => 'icon.png',
  'defaults' => array('header_font' => array(), 'text_font' => array(), 'padding' => array(), 'header_border' => array()),
  'palette' => array(
    'background' => array(
      'label' => t('Background'),
      'default_value' => '#FFFFFF',
    ),
    'text' => array(
      'label' => t('Text'),
      'default_value' => '#000000',
    ),
    'header-text' => array(
      'label' => t('Header text'),
      'default_value' => '#000000',
    ),
    'header-border' => array(
      'label' => t('Header border'),
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

  'theme' => 'pane_rounded_shadow',
  'build' => 'panels_rounded_shadow_style_base_build',
//  'preview' => 'panels_stylizer_pane_preview',

  'settings form' => 'panels_pane_rounded_shadow_style_settings',
  'settings form submit' => 'panels_pane_rounded_shadow_style_settings_submit',
  'build' => 'panels_pane_rounded_shadow_style_base_build',
);

function template_preprocess_pane_rounded_shadow(&$vars) {
  $vars['classes'] = $vars['content']->css_class;
  $vars['content']->css_class = '';

  $vars['output'] = theme('panels_pane', $vars['content'], $vars['pane'], $vars['display']);
}

function panels_pane_rounded_shadow_style_settings(&$form, &$form_state) {
  $form['header_font'] = array();
  ctools_stylizer_font_selector_form($form['header_font'], $form_state, t('Header font'), $form_state['settings']['header_font']);

  $form['header_border'] = array();
  ctools_stylizer_border_selector_form($form['header_border'], $form_state, t('Header border'), $form_state['settings']['header_border']);

  $form['text_font'] = array();
  ctools_stylizer_font_selector_form($form['text_font'], $form_state, t('Text font'), $form_state['settings']['text_font']);

  $form['padding'] = array();
  ctools_stylizer_padding_selector_form($form['padding'], $form_state, t('Padding'), $form_state['settings']['padding']);
}

function panels_pane_rounded_shadow_style_settings_submit(&$form, &$form_state) {
  ctools_stylizer_font_selector_form_submit($form['header_font'], $form_state, $form_state['values']['header_font'], $form_state['settings']['header_font']);

  ctools_stylizer_font_selector_form_submit($form['text_font'], $form_state, $form_state['values']['text_font'], $form_state['settings']['text_font']);

  ctools_stylizer_padding_selector_form_submit($form['padding'], $form_state, $form_state['values']['padding'], $form_state['settings']['padding']);

  ctools_stylizer_border_selector_form_submit($form['header_border'], $form_state, $form_state['values']['header_border'], $form_state['settings']['header_border']);

}

function panels_pane_rounded_shadow_style_base_build($plugin, $settings, &$css, $replacements) {
  ctools_stylizer_font_apply_style($css, '%style .pane-title', $settings['header_font']);
  ctools_stylizer_border_apply_style($css, '%style .pane-title', $settings['header_border'], '%header-border', 'bottom');
  ctools_stylizer_font_apply_style($css, '%style .pane-content', $settings['text_font']);
  ctools_stylizer_padding_apply_style($css, '%style', $settings['padding']);
}

