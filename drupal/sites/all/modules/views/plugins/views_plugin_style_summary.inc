<?php
// $Id: views_plugin_style_summary.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $
/**
 * @file
 * Contains the default summary style plugin, which displays items in an HTML list.
 */

/**
 * The default style plugin for summaries.
 *
 * @ingroup views_style_plugins
 */
class views_plugin_style_summary extends views_plugin_style {
  function option_definition() {
    $options = parent::option_definition();

    $options['count'] = array('default' => TRUE);
    $options['override'] = array('default' => FALSE);
    $options['items_per_page'] = array('default' => 25);

    return $options;
  }

  function query() {
    if (!empty($this->options['override'])) {
      $this->view->set_items_per_page(intval($this->options['items_per_page']));
    }
  }

  function options_form(&$form, &$form_state) {
    $form['count'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['count']),
      '#title' => t('Display record count with link'),
    );
    $form['override'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['override']),
      '#title' => t('Override number of items to display'),
    );
    $form['items_per_page'] = array(
      '#type' => 'textfield',
      '#title' => t('Items to display'),
      '#default_value' => $this->options['items_per_page'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-style-options-override' => array(TRUE)),
    );
  }

  function render() {
    $rows = array();
    foreach ($this->view->result as $row) {
      // @todo: Include separator as an option.
      $rows[] = $row;
    }
    return theme($this->theme_functions(), $this->view, $this->options, $rows);
  }
}

