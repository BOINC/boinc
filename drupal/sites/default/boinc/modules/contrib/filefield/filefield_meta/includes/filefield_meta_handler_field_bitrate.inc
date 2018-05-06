<?php

/**
 * @file
 * A special handler that properly formats bit rate fields as Kbps.
 */

/**
 * Render a field as a readable value in hours, minutes, and seconds.
 *
 * @ingroup views_field_handlers
 */
class filefield_meta_handler_field_bitrate extends views_handler_field_numeric {
  function option_definition() {
    $options = parent::option_definition();

    $options['format'] = array('default' => 'default', 'translatable' => TRUE);

    // Remove the separator options since we don't need them.
    unset($options['separator']);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Remove the separator and alter options since we don't need them.
    unset($form['separator']);

    $form['prefix']['#weight'] = 10;
    $form['suffix']['#weight'] = 10;
    $form['format'] = array(
      '#type' => 'select',
      '#title' => t('Format'),
      '#default_value' => $this->options['format'],
      '#options' => array(
        'default' => t('Default (Mbps or Kbps)'),
        'raw' => t('Raw numberic value'),
      ),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};

    // Check to see if hiding should happen before adding prefix and suffix.
    if ($this->options['hide_empty'] && empty($value) && ($value !== 0 || $this->options['empty_zero'])) {
      return '';
    }

    switch ($this->options['format']) {
      case 'raw':
        $output = $value;
        break;
      default:
        $output = theme('filefield_meta_bitrate', $value);
    }

    return check_plain($this->options['prefix']) . $output . check_plain($this->options['suffix']);
  }
}
