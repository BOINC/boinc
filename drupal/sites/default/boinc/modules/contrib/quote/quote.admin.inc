<?php
// $Id: quote.admin.inc,v 1.1.2.2 2008/05/13 06:20:53 karthik Exp $

/**
 * @file
 * Admin page callbacks for the quote module.
 */

/**
 * Implementation of hook_help().
 */
function quote_help($path, $arg) {
  switch ($path) {
    case 'admin/settings/quote':
      return t('<p>The quote filter allows users to quote other posts in their
comments. Besides the following settings, the quote filter will need to be
enabled for each <a href="!input-format">input format</a> (as required). Please
make sure that the quote filter is arranged <em>after</em> any HTML filters and
<em>before</em> the line break filter. For more information, please visit the
<a href="!project-page">project page</a>.</p>', array('!input-format' => url('admin/settings/filters'), '!project-page' => url('http://drupal.org/project/quote', array('absolute' => TRUE))));
  }
}

/**
 * Menu callback: quote module settings form.
 */
function quote_settings_form() {
  $form = array();
  $form['quote'] = array(
    '#type' => 'fieldset',
    '#title' => t('Quote module settings'),
    '#tree' => TRUE
  );
  $form['quote']['node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Node associations'),
    '#description' => t('Select the node types to associate with the quote filter.'),
    '#options' => _quote_get_node_types(),
    '#default_value' => _quote_variable_get('node_types')
  );
  $form['quote']['node_link_display'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display the quote link for nodes'),
    '#description' => t('Leave this option disabled if you use a PHP or similar filter in your nodes. The quote link is always displayed for comments.'),
    '#default_value' => _quote_variable_get('node_link_display')
  );
  $form['quote']['subject_required'] = array(
    '#type' => 'checkbox',
    '#title' => t('Make the comment subject field a required field'),
    '#description' => t('Making the comment subject field a required field will ensure that unsightly [quote] tags are not displayed.'),
    '#default_value' => _quote_variable_get('subject_required')
  );

  return system_settings_form($form);
}

/**
 * Validate quote settings form submission.
 */
function quote_settings_form_validate($form, &$form_state) {
  // Run the node type checkboxes through array_filter to strip unselected
  // items.
  form_set_value($form['quote']['node_types'], array_filter($form_state['values']['quote']['node_types']), $form_state);
}
