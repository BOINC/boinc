<?php

/**
 * @file
 * Base class for empty behaviors.
 */

/**
 * Abstract class implemented by all empty behavior plugins.
 */
abstract class FacetapiEmptyBehavior {

  /**
   * The machine readable name of facet configuration.
   *
   * @var string
   */
  protected $configName;

  /**
   * An array of facet settings.
   *
   * @var array
   */
  protected $settings;

  /**
   * Initializes settings.
   */
  public function __construct(stdClass $settings) {
    $this->configName = $settings->name;
    $this->settings = $settings->settings;
    $this->settings += $this->getDefaultSettings();
  }

  /**
   * Executes the abstract class behavior.
   *
   * @return
   *   The element's render array.
   */
  abstract public function execute();

  /**
   * Allows for backend specific overrides to the settings form.
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do...
  }

  /**
   * Returns an array of default settings.
   */
  public function getDefaultSettings() {
    return array();
  }

  /**
   * Helper function for translating strings.
   *
   * @param string $key
   *   The array key of the element being translated.
   * @param string $string
   *   The string being translated.
   *
   * @return
   *   The translated string.
   *
   * @see facetapi_translate_string()
   */
  public function translate($key, $string) {
    $config_name = preg_replace('@[^a-zA-Z0-9]@', '_', $this->configName);
    $name = 'facetapi:' . $config_name . ':empty_text:' . $key;
    return facetapi_translate_string($name, $string);
  }
}

/**
 * Plugin that returns an empty array, meaning
 */
class FacetapiEmptyBehaviorNone extends FacetapiEmptyBehavior {

  /**
   * Returns an empty array.
   */
  public function execute() {
    return array();
  }
}

