<?php

/**
 * @file
 * Plugin that adds custom text to the current search block.
 */

/**
 * Extension of CurrentSearchItem that displays all active items.
 */
class CurrentSearchItemText extends CurrentSearchItem {

  /**
   * Implements CurrentSearchItem::execute().
   */
  public function execute(FacetapiAdapter $adapter) {
    $data = array('facetapi_adapter' => $adapter);

    // Determines plurality of string.
    if ($this->settings['plural']) {
      $condition = '[' . $this->settings['plural_condition'] . ']';
      $count = (int) token_replace($condition, $data);
      if ($count != 1) {
        $raw_text = $this->settings['text_plural'];
        $translation_key = 'text_plural';
      }
      else {
        $raw_text = $this->settings['text'];
        $translation_key = 'text';
      }
    }
    else {
      $raw_text = $this->settings['text'];
      $translation_key = 'text';
    }

    // Translates text, returns themed output.
    $translated_text = $this->translate($translation_key, $raw_text);
    $variables = array(
      'text' => filter_xss_admin(token_replace($translated_text, $data)),
      'wrapper' => $this->settings['wrapper'],
      'element' => $this->settings['element'],
      'css' => $this->settings['css'],
      'class' => current_search_get_classes($this->settings['classes'], $data),
      'options' => array('html' => TRUE),
    );
    return array('#value' => theme('current_search_text', $variables));
  }

  /**
   * Implements CurrentSearchItem::settingsForm().
   */
  public function settingsForm(&$form, &$form_state) {

    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text'),
      '#default_value' => $this->settings['text'],
      '#maxlength' => 255,
      '#description' => t('Custom text displayed in the text box. Token replacement patterns are allowed.'),
    );

    $form['plural'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add plural text'),
      '#default_value' => $this->settings['plural'],
    );

    $form['text_plural'] = array(
      '#type' => 'textfield',
      '#title' => t('Plural text'),
      '#default_value' => $this->settings['text_plural'],
      '#maxlength' => 255,
      '#description' => t('Plural equivalent of the custom text displayed in the text box. Token replacement patterns are allowed.'),
      '#states' => array(
        'visible' => array(
          ':input[name="plugin_settings[' . $this->name . '][plural]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['plural_condition'] = array(
      '#type' => 'select',
      '#title' => t('Plural condition'),
      '#options' => array(
        'facetapi_results:result-count' => t('Total number of results'),
        'facetapi_results:page-number' => t('Page number'),
        'facetapi_results:page-limit' => t('Results per page'),
      ),
      '#default_value' => $this->settings['plural_condition'],
      '#description' => t('The condition that determines whether the singular or plural string is used.'),
      '#states' => array(
        'visible' => array(
          ':input[name="plugin_settings[' . $this->name . '][plural]"]' => array('checked' => TRUE),
        ),
      ),
    );

    // Adds HTML wrapper elements.
    $this->wrapperForm($form, $form_state);

    // Adds token tree.
    $form['tokens'] = $this->getTokenTree(array('facetapi_results'));
  }

  /**
   * Implements CurrentSearchItem::getDefaultSettings().
   */
  public function getDefaultSettings() {
    $defaults = array(
      'plural' => 0,
      'plural_condition' => 'facetapi_results:result-count',
      'text' => '',
      'text_plural' => '',
    );
    return $defaults + $this->wrapperDefaults();
  }
}
