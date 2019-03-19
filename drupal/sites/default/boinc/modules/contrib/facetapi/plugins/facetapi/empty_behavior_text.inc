<?php

/**
 * @file
 * Empty behavior that returns text.
 */

/**
 * Returns text.
 */
class FacetapiEmptyBehaviorText extends FacetapiEmptyBehavior {

  /**
   * A boolean flagging whether the input format is set, FALSE means it is
   * being pulled from FacetapiEmptyBehavior::getDefaultSettings().
   */
  protected $formatSet = FALSE;

  /**
   * Checks if a format was selected, calls parent's constructor.
   */
  public function __construct(stdClass $settings) {
    if (isset($settings->settings['empty_text_format'])) {
      $this->formatSet = TRUE;
    }
    parent::__construct($settings);
  }

  /**
   * Returns an empty array.
   */
  public function execute() {
    $format_id = $this->settings['empty_text_format'];
    $text = $this->translate('empty_text', $this->settings['empty_text']);
    return check_markup($text, $format_id);
  }

  /**
   * Adds setting for the empty behavior.
   */
  public function settingsForm(&$form, &$form_state) {
    global $user;

    $format_id = filter_resolve_format(($this->formatSet) ? $this->settings['empty_text_format'] : FILTER_FORMAT_DEFAULT);
    $format = filter_format_load($format_id);
    // NOTE: There is a core bug with form #states and the text_format #type.
    // @see http://drupal.org/node/997826
    $form['widget']['empty']['empty_text'] = array(
      '#type' => 'textarea',
      '#access' => $format && filter_access($format, $user),
      '#title' => t('Empty text'),
      '#default_value' => $this->settings['empty_text'],
      /*
      '#states' => array(
        'visible' => array(
          'select[name="empty_behavior"]' => array('value' => 'text'),
        ),
      ),
      */
    );
    $form['widget']['empty']['empty_text_format'] = filter_form($format_id);
  }

  /**
   * Returns an array of default settings.
   */
  public function getDefaultSettings() {
    return array(
      'empty_text' => '',
      'empty_text_format' => FILTER_FORMAT_DEFAULT,
    );
  }
}
