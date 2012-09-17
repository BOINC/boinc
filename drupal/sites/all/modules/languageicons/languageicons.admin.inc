<?php
/**
 * @file
 * Admin page callbacks for the Language icons module.
 */

/**
 * Form builder; configure Language icons.
 *
 * @todo Improve (re-phrase?) $form['show']['#description'].
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function languageicons_admin_settings() {
  $form['show'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add language icons'),
    '#description' => t('Link types to add language icons.'),
  );
  $form['show']['languageicons_show_node'] = array(
    '#type' => 'checkbox',
    '#title' => t('Node links'),
    '#default_value' => variable_get('languageicons_show_node', 1),
  );
  $form['show']['languageicons_show_block'] = array(
    '#type' => 'checkbox',
    '#title' => t('Language switcher block'),
    '#default_value' => variable_get('languageicons_show_block', 1),
  );
  $form['languageicons_placement'] = array(
    '#type' => 'radios',
    '#title' => t('Icon placement'),
    '#options' => array(
      'before' => t('Before link'),
      'after' => t('After link'),
      'replace' => t('Replace link'),
    ),
    '#default_value' => variable_get('languageicons_placement', 'before'),
    '#description' => t('Where to display the icon, relative to the link title.'),
  );
  $form['languageicons_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Icons file path'),
    '#default_value' => variable_get('languageicons_path', drupal_get_path('module', 'languageicons') .'/flags/*.png'),
    '#size' => 70,
    '#maxlength' => 180,
    '#description' => t('Path for language icons, relative to Drupal installation. "*" is a placeholder for language code.'),
  );
  $form['languageicons_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Image size'),
    '#default_value' => variable_get('languageicons_size', '16x12'),
    '#size' => 7,
    '#maxlength' => 7,
    '#description' => t('Image size for language icons, in the form "width x height".'),
  );

  return system_settings_form($form);
}
