<?php
// $Id: flag_handler_filter_flagged.inc,v 1.1.2.6 2010/01/08 04:48:43 quicksketch Exp $

/**
 * @file
 * Contains the flagged content filter handler.
 */

/**
 * Handler to filter for content that has not been flagged.
 *
 * @ingroup views
 */
class flag_handler_filter_flagged extends views_handler_filter_boolean_operator {
  function option_definition() {
    $options = parent::option_definition();
    $options['value'] = array('default' => 1);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['value']['#type'] = 'radios';
    $form['value']['#title'] = t('Status');
    $form['value']['#options'] = array(1 => t('Flagged'), 0 => t('Not flagged'));
    $form['value']['#default_value'] = empty($this->options['value']) ? 0 : $this->options['value'];
    $form['value']['#description'] = '<p>' . t('This filter is only needed if the relationship used has the "Include only flagged content" option <strong>unchecked</strong>. Otherwise, this filter is useless, because all records are already limited to flagged content.') . '</p><p>' . t('By choosing <em>Not flagged</em>, it is possible to create a list of content <a href="@unflagged-url">that is specifically not flagged</a>.', array('@unflagged-url' => 'http://drupal.org/node/299335')) . '</p>';
  }

  function query() {
    $operator = $this->value ? 'IS NOT' : 'IS';
    $this->query->add_where($this->options['group'], $this->relationship .'.uid '. $operator .' NULL');
  }
}
