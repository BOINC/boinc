<?php
// $Id: filefield_meta_handler_field_duration.inc,v 1.3 2010/04/24 06:28:03 quicksketch Exp $

/**
 * @file
 * A special handler that properly formats duration fields as minutes:seconds.
 */

/**
 * Render a field as a readable value in hours, minutes, and seconds.
 *
 * @ingroup views_field_handlers
 */
class filefield_meta_handler_field_duration extends views_handler_field_numeric {
  function option_definition() {
    $options = parent::option_definition();

    $options['format'] = array('default' => 'default', 'translatable' => TRUE);

    // Remove the separator options since we don't need them.
    unset($options['separator']);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Remove the separator options since we don't need them.
    unset($form['separator']);

    $form['prefix']['#weight'] = 10;
    $form['suffix']['#weight'] = 10;
    $form['format'] = array(
      '#type' => 'select',
      '#title' => t('Time format'),
      '#default_value' => $this->options['format'],
      '#options' => array(
        'default' => t('Default (usually mm:ss)'),
        'hours' => t('Hours: h'),
        'minutes' => t('Minutes: mm'),
        'seconds' => t('Seconds: ss'),
        'total' => t('Total seconds'),
      ),
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};

    switch ($this->options['format']) {
      case 'hours':
        $value = date('g', (int) $value);
        break;
      case 'minutes':
        $value = date('i', (int) $value);
        break;
      case 'seconds':
        $value = date('s', (int) $value);
        break;
      case 'total':
        $value = $value;
        break;
      default:
        $output = theme('filefield_meta_duration', $value);
    }

    // Check to see if hiding should happen before adding prefix and suffix.
    if ($this->options['hide_empty'] && empty($value) && ($value !== 0 || $this->options['empty_zero'])) {
      return '';
    }

    return check_plain($this->options['prefix']) . $output . check_plain($this->options['suffix']);
  }
}
