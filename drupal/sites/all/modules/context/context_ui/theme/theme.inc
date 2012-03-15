<?php

/**
 * Preprocessor for theme('context_ui_editor').
 */
function template_preprocess_context_ui_editor(&$vars) {
  drupal_add_css(drupal_get_path('module', 'context_ui') . '/context_ui.css');
  drupal_add_js(drupal_get_path('module', 'context_ui') . '/context_ui.js');
  drupal_add_js(drupal_get_path('module', 'context_ui') . '/jquery.pageEditor.js');

  $vars['contexts'] = $vars['form']['contexts'];
  unset($vars['form']['contexts']);

  $vars['buttons'] = $vars['form']['buttons'];
  unset($vars['form']['buttons']);
}

/**
 * Preprocessor for theme('context_ui_plugins').
 */
function template_preprocess_context_ui_plugins(&$vars) {
  drupal_add_css(drupal_get_path("module", "context_ui") ."/context_ui.css");
  drupal_add_js(drupal_get_path("module", "context_ui") ."/context_ui.js");
  drupal_add_js(drupal_get_path('module', 'context_ui') . '/jquery.pageEditor.js');

  // Provide title & desc.
  $vars['title'] = check_plain($vars['form']['#title']);
  $vars['description'] = check_plain($vars['form']['#description']);

  // Build list of plugins.
  $plugins = array();
  $vars['plugins'] = array();
  foreach (element_children($vars['form']['plugins']) as $plugin) {
    $link = array(
      'title' => $vars['form']['plugins'][$plugin]['#plugin']->title . "<span class='remove'>". t('Remove') ."</span>",
      'href' => $_GET['q'],
      'html' => TRUE,
      'fragment' => "context-plugin-form-{$plugin}",
    );
    $class = $vars['form']['plugins'][$plugin]['#context_enabled'] ? "context-plugin-{$plugin}" : "context-plugin-{$plugin} disabled";
    $vars['plugins'][$class] = $link;
  }
}

/**
 * Preprocessor for theme('context_ui_form').
 */
function template_preprocess_context_ui_form(&$vars) {
  drupal_add_css(drupal_get_path("module", "context_ui") ."/context_ui.css");
  drupal_add_js(drupal_get_path("module", "context_ui") ."/context_ui.js");
  drupal_add_js(drupal_get_path('module', 'context_ui') . '/jquery.pageEditor.js');
  $vars['buttons'] = $vars['form']['buttons'];
  unset($vars['form']['buttons']);
}
