<?php
/**
 * Field handler to show if user is admin or not.
 *
 * @ingroup views_field_handlers
 */
class og_views_handler_field_boolean_og_private extends views_handler_field_boolean {

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['type']['#options']['private-public'] = t('Private/Public');
  }

  function option_definition() {
    $options = parent::option_definition();
    return $options;
  }


  function render($values) {
    $value = $values->{$this->field_alias};
    if (!empty($this->options['not'])) {
      $value = !$value;
    }

    switch ($this->options['type']) {
      case 'yes-no':
      default:
        return $value ? t('Yes') : t('No');
      case 'true-false':
        return $value ? t('True') : t('False');
      case 'on-off':
        return $value ? t('On') : t('Off');
      case 'private-public':
        return $value ? t('Private') : t('Public');
    }
  }
}

