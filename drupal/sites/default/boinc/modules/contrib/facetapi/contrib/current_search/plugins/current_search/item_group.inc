<?php

/**
 * @file
 * Displays field groups.
 */

/**
 * Extension of CurrentSearchItem that displays field groups.
 */
class CurrentSearchGroup extends CurrentSearchItem {

  /**
   * Implements CurrentSearchItem::execute().
   */
  public function execute(FacetapiAdapter $adapter) {
    $groups = array();

    // Makes sure facet builds are initialized.
    $adapter->processFacets();

    // Adds other current search module's CSS.
    $path = drupal_get_path('module', 'current_search');
    drupal_add_css($path . '/current_search.css');

    // Adds active facets to the current search block.
    $searcher = $adapter->getSearcher();
    foreach ($adapter->getAllActiveItems() as $item) {
      $facet_name = $item['facets'][0];
      $facet_value = $item['value'];
      $groups[$facet_name][$facet_value] = $item;
    }

    // Initializes links attributes, adds rel="nofollow" if configured.
    $attributes = ($this->settings['nofollow']) ? array('rel' => 'nofollow') : array();
    $attributes += array('class' => array());

    // Gets the translated pattern with token replacements in tact.
    $field_pattern = $this->translate('field_pattern', $this->settings['field_pattern']);

    // Iterates over groups, builds list.
    $build = array();
    foreach ($groups as $facet_name => $group) {
      $items = array();

      // Builds list items.
      foreach ($group as $item) {
        $markup = $adapter->getMappedValue($item['facets'][0], $item['value']);
        $text = ($markup['#html']) ? $markup['#value'] : check_plain($markup['#value']);
        $variables = array(
          'text' => $text,
          'path' => $this->getFacetPath($item, $adapter),
          'options' => array(
            'attributes' => $attributes,
            'html' => TRUE,
            'query' => $this->getQueryString($item, $adapter),
          ),
        );
        $items[] = theme('current_search_link_active', $variables);
      }

      // If there are items, add the render array.
      if ($items) {
        $build[$facet_name]['#theme_wrappers'] = array('current_search_group_wrapper');

        // Performs token replacemenets and themes the group title.
        $data = array('facetapi_facet' => facetapi_facet_load($facet_name, $searcher));
        $title = filter_xss(token_replace($field_pattern, $data));
        $build[$facet_name]['title']['#value'] = theme('current_search_group_title', array('title' => $title));

        // Builds the list.
        $build[$facet_name]['list'] = array(
          '#theme' => 'item_list',
          '#items' => $items,
          '#attributes' => array('class' => array('inline')),
        );
      }
    }

    return $build;
  }

  /**
   * Implements CurrentSearchItem::settingsForm().
   */
  public function settingsForm(&$form, &$form_state) {

    $form['field_pattern'] = array(
      '#type' => 'textfield',
      '#title' => t('Field pattern'),
      '#default_value' => $this->settings['field_pattern'],
      '#maxlength' => 255,
      '#description' => t('The pattern of the field label preceeding the links. Token replacement patterns are allowed.'),
    );

    $form['nofollow'] = array(
      '#type' => 'checkbox',
      '#title' => t('Prevent crawlers from following active item links'),
      '#default_value' => $this->settings['nofollow'],
      '#description' => t('Add the <code>rel="nofollow"</code> attribute to active item links to maximize SEO by preventing crawlers from indexing duplicate content and getting stuck in loops.'),
    );

    // Adds token tree.
    $form['tokens'] = $this->getTokenTree(array('facetapi_facet'));
  }

  /**
   * Implements CurrentSearchItem::getDefaultSettings().
   */
  public function getDefaultSettings() {
    return array(
      'field_pattern' => '[facetapi_facet:facet-label]:',
      'nofollow' => 1,
    );
  }
}
