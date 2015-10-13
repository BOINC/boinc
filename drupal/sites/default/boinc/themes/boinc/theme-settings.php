<?php
// $Id: theme-settings.php,v 1.7 2008/09/11 09:36:50 johnalbin Exp $

// Include the definition of zen_settings() and zen_theme_get_default_settings().
include_once './' . drupal_get_path('theme', 'zen') . '/theme-settings.php';


/**
 * Implementation of THEMEHOOK_settings() function.
 *
 * @param $saved_settings
 *   An array of saved settings for this theme.
 * @return
 *   A form array.
 */
function boinc_settings($saved_settings) {

  // Get the default values from the .info file.
  $defaults = zen_theme_get_default_settings('boinc');

  // Merge the saved variables and their default values.
  $settings = array_merge($defaults, $saved_settings);

  /*
   * Create the form using Forms API: http://api.drupal.org/api/6
   */
  $form = array();
  $form['boinc_stats_charts'] = array(
    '#type' => 'fieldset',
    '#title' => t('Chart settings'),
    '#attributes' => array(
      'class' => 'zen-settings',
    ),
  );
  $form['boinc_stats_charts']['boinc_stats_chart_color'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Chart color'),
    '#default_value' => $settings['boinc_stats_chart_color'],
    '#description'   => t('The primary color of the stats chart in hex format (e.g. #FAA341).'),
  );
  $form['boinc_stats_charts']['boinc_stats_chart_bcolor'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Chart background color'),
    '#default_value' => $settings['boinc_stats_chart_bcolor'],
    '#description'   => t('The background color of the stats chart in hex format (e.g. #FFFFFF).'),
  );

  // Add the base theme's settings.
  $form += zen_settings($saved_settings, $defaults);

  // Remove some of the base theme's settings.
  unset($form['themedev']['zen_layout']); // We don't need to select the base stylesheet.

  // Return the form
  return $form;
}
