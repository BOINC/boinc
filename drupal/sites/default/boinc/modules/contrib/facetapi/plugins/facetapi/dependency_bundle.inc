<?php

/**
 * @file
 * Performs a dependency check against the passed bundle.
 */

/**
 * Adds a dependency on bundle.
 */
class FacetapiDependencyBundle extends FacetapiDependency {

  /**
   * Executes the dependency check.
   */
  public function execute() {
    switch ($this->settings['bundle']) {
      case 'referenced':
        // @todo Do we need defensive coding here?
        $field_info = field_info_field($this->facet['field api name']);

        // Iterate over field info, collect bundles this field is attached to.
        $field_bundles = array();
        foreach ($field_info['bundles'] as $entity => $bundles) {
          $field_bundles[$entity] = array_flip($bundles);
        }

        // Check match between the field's bundles and active bundle items.
        foreach ($this->getEnabledBundleFacets()  as $facet) {
          foreach ($facet['field api bundles'] as $entity) {
            $bundles = (isset($field_bundles[$entity])) ? $field_bundles[$entity] : array();
            if (array_intersect_key($this->activeItems[$facet['name']], $bundles)) {
              return NULL;
            }
          }
        }

        // There was no match.
        return FALSE;

      case 'selected':
        // Check match between selected bundles and active bundle items.
        $selected = array_filter($this->settings['bundle_selected']);
        foreach ($this->getEnabledBundleFacets() as $facet) {
          if (array_intersect_key($this->activeItems[$facet['name']], $selected)) {
            return NULL;
          }
        }

        // There was no match.
        return FALSE;
    }
  }

  /**
   * Returns an array of enabled facets that filter by bundle.
   *
   * @return array
   *   An array of enabled bundle facets.
   */
  public function getEnabledBundleFacets() {
    $enabled = array_filter($this->adapter->getEnabledFacets(), array($this, 'filterBundleFacets'));
    $active = array_filter($this->activeItems);
    return array_intersect_key($enabled, $active);
  }

  /**
   * Returns TRUE if the facet contains bundle information.
   *
   * @param array $facet
   *   The facet definition beinf filtered.
   *
   * @return
   *   A boolean flagging whether the item should remain in the array.
   */
  public function filterBundleFacets($facet) {
    return !empty($facet['field api bundles']);
  }

  /**
   * Adds dependency settings to the form.
   */
  public function settingsForm(&$form, &$form_state) {

    // Builds array of options.
    $options = array();
    $options['none'] = t('No dependencies.');
    if ($this->facet['field api name']) {
      $options['referenced'] = t('A bundle this field is attached to must be active.');
    }
    $options['selected'] = t('At least one of the selected bundles must be active.');

    $form[$this->id]['bundle'] = array(
      '#title' => t('Dependency settings'),
      '#type' => 'radios',
      '#options' => $options,
      '#default_value' => $this->settings['bundle'],
    );

    $form[$this->id]['bundle_selected'] = array(
      '#title' => t('Required bundles'),
      '#type' => 'checkboxes',
      '#options' => $this->getBundleOptions($form['#facetapi']['adapter']->getTypes()),
      '#default_value' => $this->settings['bundle_selected'],
      '#states' => array(
        'visible' => array(
          'input[name="bundle"]' => array('value' => 'selected'),
        ),
      ),
      '#description' => t('At least one of the selected bundles must be active for this facet to be rendered.')
    );
  }

  /**
   * Gets bundles.
   *
   * @param array $entity_types
   *   An array containing the machine readable name of the entities.
   *
   * @return
   *   An array of bundles associates with the entities.
   */
  public function getBundleOptions(array $entity_types) {
    $options = array();
    // No entities in D6 - Only nodes
    $bundle_types = node_get_types();
    foreach ($bundle_types as $bundle => $bundle_info) {
      $options[$bundle] = check_plain($bundle_info->name);
    }
    return $options;
  }

  /**
   * Returns defaults for settings.
   */
  public function getDefaultSettings() {
    return array(
      'bundle' => 'none',
      'bundle_selected' => array(),
    );
  }
}
