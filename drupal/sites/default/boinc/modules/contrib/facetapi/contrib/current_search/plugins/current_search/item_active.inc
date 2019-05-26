<?php

/**
 * @file
 * Displays all active items.
 */

/**
 * Extension of CurrentSearchItem that displays all active items.
 */
class CurrentSearchItemActive extends CurrentSearchItem {

  /**
   * Implements CurrentSearchItem::execute().
   */
  public function execute(FacetapiAdapter $adapter) {
    $items = array();

    // Makes sure facet builds are initialized.
    $adapter->processFacets();

    // Adds the keywords if any were passed.
    $keys = $adapter->getSearchKeys();
    if ($this->settings['keys'] && $keys) {
      $items[] = theme('current_search_keys', array('keys' => $keys, 'adapter' => $adapter));
    }

    // Initializes links attributes, adds rel="nofollow" if configured.
    $attributes = ($this->settings['nofollow']) ? array('rel' => 'nofollow') : array();
    $attributes += array('class' => array());

    // Gets the translated pattern with token replacements in tact.
    $pattern = $this->translate('pattern', $this->settings['pattern']);

    // Adds active facets to the current search block.
    foreach ($adapter->getAllActiveItems() as $item) {
      // Adds adapter to the active item for token replacement.
      $item['adapter'] = $adapter;

      // Builds variables to pass to theme function.
      $data = array('facetapi_active_item' => $item);
      $variables = array(
        'text' => token_replace($pattern, $data),
        'path' => $this->getFacetPath($item, $adapter),
        'options' => array(
          'attributes' => $attributes,
          'html' => TRUE,
          'query' => $this->getQueryString($item, $adapter),
        ),
      );

      // Renders the active link.
      $items[] = theme('facetapi_link_active', $variables);
    }

    // If there are items, return the render array.
    if ($items) {
      $classes = ($this->settings['css']) ? current_search_get_classes($this->settings['classes']) : array();
      return array(
        '#theme' => 'item_list',
        '#items' => $items,
        '#attributes' => array('class' => $classes),
      );
    }
  }

  /**
   * Implements CurrentSearchItem::settingsForm().
   */
  public function settingsForm(&$form, &$form_state) {

    $form['pattern'] = array(
      '#type' => 'textfield',
      '#title' => t('Pattern'),
      '#default_value' => $this->settings['pattern'],
      '#description' => t('The pattern used to render active items in the list. Token replacement patterns are allowed.'),
      '#maxlength' => 255,
    );

    $form['keys'] = array(
      '#type' => 'checkbox',
      '#title' => t('Append the keywords passed by the user to the list'),
      '#default_value' => $this->settings['keys'],
    );

    $form['css'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add CSS classes to wrapper element'),
      '#default_value' => $this->settings['css'],
    );

    $form['classes'] = array(
      '#type' => 'textfield',
      '#title' => t('CSS classes'),
      '#default_value' => $this->settings['classes'],
      '#description' => t('A comma separated list of CSS classes.'),
      '#maxlength' => 128,
      '#states' => array(
        'visible' => array(
          ':input[name="plugin_settings[' . $this->name . '][css]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['nofollow'] = array(
      '#type' => 'checkbox',
      '#title' => t('Prevent crawlers from following active item links'),
      '#default_value' => $this->settings['nofollow'],
      '#description' => t('Add the <code>rel="nofollow"</code> attribute to active item links to maximize SEO by preventing crawlers from indexing duplicate content and getting stuck in loops.'),
    );

    // Adds token tree.
    $form['tokens'] = $this->getTokenTree(array('facetapi_active'));
  }

  /**
   * Implements CurrentSearchItem::getDefaultSettings().
   */
  public function getDefaultSettings() {
    return array(
      'pattern' => '[facetapi_active:active-value]',
      'keys' => FALSE,
      'css' => FALSE,
      'classes' => '',
      'nofollow' => 1,
    );
  }
}
