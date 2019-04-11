<?php

/**
 * @file
 * Widgets for facets rendered as links.
 */

/**
 * Widget that renders facets as a list of clickable links.
 */
class FacetapiWidgetLinks extends FacetapiWidget {

  /**
   * Overrides constructor to reset the key.
   */
  public function __construct($id, array $realm, FacetapiFacet $facet, stdClass $settings) {
    parent::__construct($id, $realm, $facet, $settings);
    $this->jsSettings['limit'] = $this->settings->settings['soft_limit'];
    $this->key = $facet['name'];
  }

  /**
   * Renders the links.
   */
  public function execute() {
    $element = &$this->build[$this->facet['field alias']];

    // Sets each item's theme hook, builds item list.
    $this->setThemeHooks($element);
    $element = theme('item_list', $this->buildListItems($element), NULL, 'ul', $this->build['#attributes']);
  }

  /**
   * Recursive function that sets each item's theme hook.
   *
   * @param array &$build
   *   A render array containing the facet items.
   */
  protected function setThemeHooks(array &$build) {
    foreach ($build as $value => &$item) {
      $item['#theme'] = ($item['#active']) ? 'facetapi_link_active' : 'facetapi_link_inactive';
      if (!empty($item['#item_children'])) {
        $this->setThemeHooks($item['#item_children']);
      }
    }
    return $this;
  }

  /**
   * Gets the base class array for a facet item.
   *
   * @return array
   *   An array of classes.
   */
  function getItemClasses() {
    return array();
  }

  /**
   * Recursive function that converts the render array into an array that can be
   * passed to theme_item_list().
   *
   * @param array $build
   *   The facet's render array.
   *
   * @return array
   *   The "items" parameter for theme_item_list().
   */
  function buildListItems($build) {
    $settings = $this->settings->settings;

    // Initializes links attributes, adds rel="nofollow" if configured.
    $attributes = ($settings['nofollow']) ? array('rel' => 'nofollow') : array();
    $attributes += array('class' => $this->getItemClasses());

    // Builds rows.
    $items = array();
    foreach ($build as $value => $item) {
      $row = array('class' => array());

      // Initializes variables passed to theme hook.
      $variables = array(
        'text' => $item['#value'],
        'path' => $item['#path'],
        'count' => $item['#count'],
        'options' => array(
          'attributes' => $attributes,
          'html' => $item['#html'],
          'query' => $item['#query'],
        ),
      );

      // Adds the facetapi-zero-results class to items that have no results.
      if (!$item['#count']) {
        $variables['options']['attributes']['class'][] = 'facetapi-zero-results';
      }

      // If the item has no children, it is a leaf.
      if (empty($item['#item_children'])) {
        $row['class'][] = 'leaf';
      }
      else {
        // If the item is active or the "show_expanded" setting is selected,
        // show this item as expanded so we see its children.
        if ($item['#active'] || !empty($settings['show_expanded'])) {
          $row['class'][] = 'expanded';
          $row['children'] = $this->buildListItems($item['#item_children']);
        }
        else {
          $row['class'][] = 'collapsed';
        }
      }

      // Gets theme hook, adds last minute classes.
      $class = ($item['#active']) ? 'facetapi-active' : 'facetapi-inactive';
      $variables['options']['attributes']['class'][] = $class;

      // Drupal 6 does not allow classes to be in an array
      $variables['options']['attributes']['class'] = implode(' ', $variables['options']['attributes']['class']);

      // Themes the link, adds row to items.
      $row['data'] = theme($item['#theme'], $variables);

      // Drupal 6 does not allow classes to be in an array
      $row['class'] = implode(' ', $row['class']);
      $items[] = $row;
    }

    return $items;
  }

  /**
   * Adds the soft limit setting.
   */
  function settingsForm(&$form, &$form_state) {
    // @see http://drupal.org/node/735528 for supporting multiple values in the
    // FAPI #states. The following hack adds multiple form elements and uses CSS
    // and JavaScript to ensure only one is displayed at a time. Only the last
    // form element actually passes the value.
    $form['widget']['widget_settings']['links'][$this->id]['soft_limit'] = array(
      '#type' => 'select',
      '#title' => t('Soft limit'),
      '#default_value' => $this->settings->settings['soft_limit'],
      '#options' => array(0 => t('No limit')) + drupal_map_assoc(array(50, 40, 30, 20, 15, 10, 5, 3)),
      '#description' => t('Limits the number of displayed facets via JavaScript.'),
      '#states' => array(
        'visible' => array(
          'select[name="widget"]' => array('value' => $this->id),
        ),
      ),
    );

    // @see http://drupal.org/node/1370342
    $form['widget']['widget_settings']['links'][$this->id]['nofollow'] = array(
      '#type' => 'checkbox',
      '#title' => t('Prevent crawlers from following facet links'),
      '#default_value' => !empty($this->settings->settings['nofollow']),
      '#description' => t('Add the <code>rel="nofollow"</code> attribute to facet links to maximize SEO by preventing crawlers from indexing duplicate content and getting stuck in loops.'),
      '#states' => array(
        'visible' => array(
          'select[name="widget"]' => array('value' => $this->id),
        ),
      ),
    );

    // @see http://drupal.org/node/735528
    if ($this->facet['hierarchy callback']) {
      $form['widget']['widget_settings']['links'][$this->id]['show_expanded'] = array(
        '#type' => 'checkbox',
        '#title' => t('Expand hierarchy'),
        '#default_value' => !empty($this->settings->settings['show_expanded']),
        '#description' => t('Show the entire tree regardless of whether the parent items are active.'),
        '#states' => array(
          'visible' => array(
            'select[name="widget"]' => array('value' => $this->id),
          ),
        ),
      );
    }

    // Hides all but the last element. The #states system will override this,
    // however it is necessary if JavaScript is disabled so multiple elements
    // aren't displayed to the user.
    $last = end($form['widget']['widget_settings']['links']);
    foreach ($form['widget']['widget_settings']['links'] as $id => $element) {
      if ($last != $element) {
        $form['widget']['widget_settings']['links'][$id]['#attributes']['style'] = 'display: none;';
      }
    }
  }

  /**
   * Returns defaults for the settings this widget provides.
   */
  function getDefaultSettings() {
    return array(
      'soft_limit' => 20,
      'nofollow' => 1,
      'show_expanded' => 0,
    );
  }
}

/**
 * Widget that renders facets as a list of clickable checkboxes.
 */
class FacetapiWidgetCheckboxLinks extends FacetapiWidgetLinks {

  /**
   * Adds JavaScript settings.
   */
  public function init() {
    parent::init();
    $this->jsSettings['makeCheckboxes'] = 1;
  }

  /**
   * Sets the base class for checkbox facet items.
   *
   * @return array
   *   An array of classes.
   */
  public function getItemClasses() {
    return array('facetapi-checkbox');
  }
}
