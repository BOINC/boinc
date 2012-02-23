<?php
// $Id: views_plugin_style_summary_jump_menu.inc,v 1.1.2.2 2009/10/28 02:22:47 merlinofchaos Exp $
/**
 * @file
 * Contains the default summary style plugin, which displays items in an HTML list.
 */

/**
 * The default style plugin for summaries.
 *
 * @ingroup views_style_plugins
 */
class views_plugin_style_summary_jump_menu extends views_plugin_style {
  function option_definition() {
    $options = parent::option_definition();

    $options['count'] = array('default' => TRUE);
    $options['hide'] = array('default' => FALSE);
    $options['text'] = array('default' => t('Go'), 'translatable' => TRUE);

    return $options;
  }

  function query() {
    $this->view->set_items_per_page(0);
    $this->view->set_use_pager(0);
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['count'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['count']),
      '#title' => t('Display record count with link'),
    );

    $form['hide'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide the "Go" button.'),
      '#default_value' => !empty($this->options['hide']),
      '#description' => t('If hidden, this button will only be hidden for users with javascript and the page will automatically jump when the select is changed.'),
    );

    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Button text'),
      '#default_value' => $this->options['text'],
    );
  }

  function render() {
    $argument = $this->view->argument[$this->view->build_info['summary_level']];

    $url_options = array();

    if (!empty($this->view->exposed_raw_input)) {
      $url_options['query'] = $this->view->exposed_raw_input;
    }

    $options = array();
    foreach ($this->view->result as $id => $row) {
      $args = $this->view->args;
      $args[$argument->position] = $argument->summary_argument($row);
      $path = url($this->view->get_url($args), $url_options);

      $options[$path] = strip_tags($argument->summary_name($row));
      if (!empty($this->options['count'])) {
        $options[$path] .= ' (' . intval($row->{$argument->count_alias}) . ')';
      }
    }

    ctools_include('jump-menu');
    $settings = array(
      'hide' => $this->options['hide'],
      'button' => $this->options['text'],
    );

    return drupal_get_form('ctools_jump_menu', $options, $settings);
  }
}

