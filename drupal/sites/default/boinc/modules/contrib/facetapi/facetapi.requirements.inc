<?php

/**
 * @file
 * Requirement checking callbacks.
 */

/**
 * Checks equality of one or more realm properties.
 *
 * @param array $realm
 *   The realm definition as returned by facetapi_realm_load().
 * @param array $facet
 *   The facet definition as returned by facetapi_facet_load().
 * @param array $options
 *   An array of values keyed by properties that are being compared.
 *
 * @return
 *   TRUE if all property values are equal, FALSE otherwise.
 */
function facetapi_requirement_realm_property(array $realm, array $facet, array $options) {
  return facetapi_requirement_property($realm, $options);
}

/**
 * Checks equality of one or more facet properties.
 *
 * @param array $realm
 *   The realm definition as returned by facetapi_realm_load().
 * @param array $facet
 *   The facet definition as returned by facetapi_facet_load().
 * @param array $options
 *   An array of values keyed by properties that are being compared.
 *
 * @return
 *   TRUE if all property values are equal, FALSE otherwise.
 */
function facetapi_requirement_facet_property(array $realm, array $facet, array $options) {
  return facetapi_requirement_property($facet, $options);
}

/**
 * Checks the equality of one or more properties.
 *
 * @param array $definition
 *   The facet or realm definition.
 * @param array $options
 *   An array of values keyed by properties that are being compared.
 *
 * @return
 *   TRUE if all property values are equal, FALSE otherwise.
 */
function facetapi_requirement_property(array $definition, array $options) {
  $passed = TRUE;
  foreach ($options as $key => $requirement) {
    $condition = $definition[$key];
    if (is_array($requirement)) {
      if (array_intersect_key($condition, $requirement) != $requirement) {
        $passed = FALSE;
        break;
      }
    }
    else {
      if ($requirement != $condition) {
        $passed = FALSE;
        break;
      }
    }
  }
  return $passed;
}

/**
 * Checks whether one or more realm properties are set.
 *
 * @param array $realm
 *   The realm definition as returned by facetapi_realm_load().
 * @param array $facet
 *   The facet definition as returned by facetapi_facet_load().
 * @param array $options
 *   An array of boolean statuses keyed by properties being checked.
 *
 * @return
 *   TRUE if all properties match the passed statues, FALSE otherwise.
 */
function facetapi_requirement_realm_property_set(array $realm, array $facet, array $options) {
  return facetapi_requirement_property_set($realm, $options);
}

/**
 * Checks whether one or more facet properties are set.
 *
 * @param array $realm
 *   The realm definition as returned by facetapi_realm_load().
 * @param array $facet
 *   The facet definition as returned by facetapi_facet_load().
 * @param array $options
 *   An array of boolean statuses keyed by properties being checked.
 *
 * @return
 *   TRUE if all properties match the passed statues, FALSE otherwise.
 */
function facetapi_requirement_facet_property_set(array $realm, array $facet, array $options) {
  return facetapi_requirement_property_set($facet, $options);
}

/**
 * Checks whether one or more properties are set.
 *
 * @param array $definition
 *   The facet or realm definition.
 * @param array $options
 *   An array of boolean statuses keyed by properties being checked.
 *
 * @return
 *   TRUE if all properties match the passed statues, FALSE otherwise.
 */
function facetapi_requirement_property_set(array $definition, array $options) {
  $passed = TRUE;
  foreach ($options as $key => $requirement) {
    if (!($requirement ? !empty($definition[$key]) : empty($definition[$key]))) {
      $passed = FALSE;
      break;
    }
  }
  return $passed;
}

/**
 * Checks whether the facet is hierarchical.
 *
 * @param array $realm
 *   The realm definition as returned by facetapi_realm_load().
 * @param array $facet
 *   The facet definition as returned by facetapi_facet_load().
 * @param $option
 *   TRUE if the facet should be hierarchical, FALSE if it should be flat.
 *
 * @return
 *   TRUE if the hierarchical status matches $option, FALSE otherwise.
 */
function facetapi_requirement_facet_hierarchical(array $realm, array $facet, $option) {
  $options = array('hierarchy callback' => (bool) $option);
  return facetapi_requirement_facet_property_set($realm, $facet, $options);
}
