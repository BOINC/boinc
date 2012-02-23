<?php
// $Id: ctools_custom_content_ui.class.php,v 1.1.2.1 2010/07/14 01:57:42 merlinofchaos Exp $

class ctools_custom_content_ui extends ctools_export_ui {

  function edit_form(&$form, &$form_state) {
    parent::edit_form($form, $form_state);

    $form['category'] = array(
      '#type' => 'textfield',
      '#title' => t('Category'),
      '#description' => t('What category this content should appear in. If left blank the category will be "Miscellaneous".'),
      '#default_value' => $form_state['item']->category,
    );

    $form['title'] = array(
      '#type' => 'textfield',
      '#default_value' => $form_state['item']->settings['title'],
      '#title' => t('Title'),
    );

    $form['body_field']['body'] = array(
      '#title' => t('Body'),
      '#type' => 'textarea',
      '#default_value' => $form_state['item']->settings['body'],
    );
    $parents[] = 'format';
    $form['body_field']['format'] = filter_form($form_state['item']->settings['format'], 1, $parents);

    $form['substitute'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use context keywords'),
      '#description' => t('If checked, context keywords will be substituted in this content.'),
      '#default_value' => !empty($form_state['item']->settings['substitute']),
    );
  }

  function edit_form_submit(&$form, &$form_state) {
    parent::edit_form_submit($form, $form_state);

    // Since items in our settings are not in the schema, we have to do these manually:
    $form_state['item']->settings['title'] = $form_state['values']['title'];
    $form_state['item']->settings['body'] = $form_state['values']['body'];
    $form_state['item']->settings['format'] = $form_state['values']['format'];
    $form_state['item']->settings['substitute'] = $form_state['values']['substitute'];
  }

  function list_form(&$form, &$form_state) {
    parent::list_form($form, $form_state);

    $options = array('all' => t('- All -'));
    foreach ($this->items as $item) {
      $options[$item->category] = $item->category;
    }

    $form['top row']['category'] = array(
      '#type' => 'select',
      '#title' => t('Category'),
      '#options' => $options,
      '#default_value' => 'all',
      '#weight' => -10,
    );
  }

  function list_filter($form_state, $item) {
    if ($form_state['values']['category'] != 'all' && $form_state['values']['category'] != $item->category) {
      return TRUE;
    }

    return parent::list_filter($form_state, $item);
  }

  function list_sort_options() {
    return array(
      'disabled' => t('Enabled, title'),
      'title' => t('Title'),
      'name' => t('Name'),
      'category' => t('Category'),
      'storage' => t('Storage'),
    );
  }

  function list_build_row($item, &$form_state, $operations) {
    // Set up sorting
    switch ($form_state['values']['order']) {
      case 'disabled':
        $this->sorts[$item->name] = empty($item->disabled) . $item->admin_title;
        break;
      case 'title':
        $this->sorts[$item->name] = $item->admin_title;
        break;
      case 'name':
        $this->sorts[$item->name] = $item->name;
        break;
      case 'category':
        $this->sorts[$item->name] = $item->category;
        break;
      case 'storage':
        $this->sorts[$item->name] = $item->type . $item->admin_title;
        break;
    }

    $this->rows[$item->name] = array(
      'data' => array(
        array('data' => check_plain($item->name), 'class' => 'ctools-export-ui-name'),
        array('data' => check_plain($item->admin_title), 'class' => 'ctools-export-ui-title'),
        array('data' => check_plain($item->category), 'class' => 'ctools-export-ui-category'),
        array('data' => theme('links', $operations), 'class' => 'ctools-export-ui-operations'),
      ),
      'title' => check_plain($item->admin_description),
      'class' => !empty($item->disabled) ? 'ctools-export-ui-disabled' : 'ctools-export-ui-enabled',
    );
  }

  function list_table_header() {
    return array(
      array('data' => t('Name'), 'class' => 'ctools-export-ui-name'),
      array('data' => t('Title'), 'class' => 'ctools-export-ui-title'),
      array('data' => t('Category'), 'class' => 'ctools-export-ui-category'),
      array('data' => t('Operations'), 'class' => 'ctools-export-ui-operations'),
    );
  }

}
