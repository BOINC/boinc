<?php

/**
 * @file
 * Definition of the 'stylizer' panel style.
 */

// Plugin definition
$plugin = array(
  'title' => t('Custom style'),
  'weight' => -10,
  'description' => t('Allows choice of a stylizer style'),

  'render pane' => 'panels_stylizer_stylizer_style_render_pane',
  'pane settings form' => 'panels_stylizer_stylizer_style_settings_form',

  'render region' => 'panels_stylizer_stylizer_style_render_region',
  'settings form' => 'panels_stylizer_stylizer_style_settings_form',

  // We offer substyles so provide callbacks to do so.
  'get child' => 'panels_stylizer_get_substyle',
  'get children' => 'panels_stylizer_get_substyles',

  // Set up an AJAX callback for the style
  'ajax' => array(
    'custom' => 'panels_stylizer_pane_add_style',
  ),
//  'settings validate' => 'panels_stylizer_stylizer_style_settings_validate',
);

/**
 * Merge the main stylizer plugin with a style to create a sub plugin.
 *
 * This is used for both panels_stylizer_get_substyle and
 * panels_stylizer_get_substyles.
 */
function panels_stylizer_merge_plugin($plugin, $style) {
  $plugin['name'] = 'stylizer:' . $style->name;
  $plugin['title'] = check_plain($style->admin_title);
  $plugin['description'] = check_plain($style->admin_description);
  $plugin['style'] = $style;
  $plugin['weight'] = 0;

  ctools_include('stylizer');
  $base = ctools_get_style_base($style->settings['style_base']);
  if ($base['type'] == 'pane') {
    unset($plugin['render region']);
  }
  else {
    unset($plugin['render pane']);
  }

  unset($plugin['settings form']);
  unset($plugin['pane settings form']);
  return $plugin;
}

/**
 * Callback to provide a single stored stylizer style.
 */
function panels_stylizer_get_substyle($plugin, $style_name, $substyle_name) {
  // Do not worry about caching; Panels is handling that for us.
  ctools_include('export');
  $item = ctools_export_crud_load('stylizer', $substyle_name);
  if ($item) {
    return panels_stylizer_merge_plugin($plugin, $item);
  }
}

/**
 * Callback to provide all stored stylizer styles.
 */
function panels_stylizer_get_substyles($plugin, $style_name) {
  $styles[$style_name] = $plugin;
  ctools_include('export');
  ctools_include('stylizer');
  $items = ctools_export_crud_load_all('stylizer');
  foreach ($items as $name => $item) {
    $base = ctools_get_style_base($item->settings['style_base']);
    if ($base && $base['module'] == 'panels') {
      $styles['stylizer:' . $name] = panels_stylizer_merge_plugin($plugin, $item);
    }
  }

  return $styles;
}

function _panels_stylizer_get_style($plugin, $style_settings) {
  if (!empty($plugin['style'])) {
    return $plugin['style']->settings;
  }

  if (empty($style_settings)) {
    return array();
  }

  if ($style_settings['style'] == '$') {
    return $style_settings['settings'];
  }

  ctools_include('export');
  $style = ctools_export_crud_load('stylizer', $style_settings['style']);
  if ($style) {
    return $style->settings;
  }
}

/**
 * Region render theme.
 */
function theme_panels_stylizer_stylizer_style_render_region($display, $owner_id, $panes, $style_settings, $region_id, $plugin) {
  $output = '';

  foreach ($panes as $pane_id => $pane_output) {
    $output .= $pane_output;
  }

  $settings = _panels_stylizer_get_style($plugin, $style_settings);

  if (!empty($settings)) {
    ctools_include('stylizer');
    $plugin = ctools_get_style_base($settings['style_base']);
    ctools_stylizer_add_css($plugin, $settings);

    return theme($plugin['theme'], $settings, ctools_stylizer_get_css_class($plugin, $settings), $output);
  }
  else {
    // if the style is gone, just display the output.
    return $output;
  }
}

/**
 * Pane render theme
 */
function theme_panels_stylizer_stylizer_style_render_pane($content, $pane, $display, $plugin) {
  $settings = _panels_stylizer_get_style($plugin, $pane->style['settings']);

  if ($settings) {
    ctools_include('stylizer');
    $plugin = ctools_get_style_base($settings['style_base']);

    if (empty($content->css_class)) {
      $content->css_class = ctools_stylizer_get_css_class($plugin, $settings);
    }
    else {
      $content->css_class .= ' ' . ctools_stylizer_get_css_class($plugin, $settings);
    }

    ctools_stylizer_add_css($plugin, $settings);

    if (isset($plugin['theme'])) {
      return theme($plugin['theme'], $settings, $content, $pane, $display);
    }
  }

  // if the style is gone or has no theme of its own, just display the output.
  return theme('panels_pane', $content, $pane, $display);
}

/**
 * Settings form callback.
 */
function panels_stylizer_stylizer_style_settings_form($style_settings, $display, $pid, $type, $form_state) {
  // Just redirect this to the custom style settings ajax.
  panels_stylizer_pane_add_style($form_state['renderer'], array(), $style_settings, $type, $pid);
  ctools_ajax_render($form_state['renderer']->commands);
}


/**
 * Allow on-the-fly creation of styles in panes.
 */
function panels_stylizer_pane_add_style(&$renderer, $plugin, &$conf, $type, $pid, $step = NULL) {
  if (!user_access('administer panels styles')) {
    return;
  }

  ctools_include('stylizer');
  $js = FALSE;

  $path = $renderer->get_url('style', 'custom', $type, $pid, '%step');

  $info = array(
    'module' => 'panels',
    'type' => $type,
    'path' => $path,
    'modal' => t('Create custom style'),
    'owner form' => 'panels_stylizer_edit_pane_style_form',
    'owner form validate' => 'panels_stylizer_edit_pane_style_form_validate',
    'owner form submit' => 'panels_stylizer_edit_pane_style_form_submit',
    'owner settings' => array('preconfigured' => FALSE, 'name' => '', 'admin_title' => '', 'admin_description' => ''),
    'cache' => &$renderer->cache,
    'conf' => &$conf,
    'pid' => $pid,
  );

  if (!empty($conf['settings'])) {
    $info['settings'] = $conf['settings'];
  }

  $output = ctools_stylizer_edit_style($info, TRUE, $step);
  if (!empty($info['complete'])) {
    if (!empty($info['owner settings']['preconfigured'])) {
      ctools_include('export');
      $style = ctools_export_crud_new('stylizer');
      $style->name = $info['settings']['name'];
      $style->admin_title = $info['owner settings']['admin_title'];
      $style->admin_description = $info['owner settings']['admin_description'];
      $style->settings = $info['settings'];
      ctools_export_crud_save('stylizer', $style);
      $conf['style'] = $info['settings']['name'];
      if (isset($conf['settings'])) {
        unset($conf['settings']);
      }
    }
    else {
      $conf['style'] = '$';
      $conf['settings'] = $info['settings'];
    }

    // Be sure to unset the temporary if the style was just changed.
    if (isset($renderer->cache->style)) {
      unset($renderer->cache->style);
    }
    // $conf was a reference so it should just modify.
    panels_edit_cache_set($renderer->cache);

    $renderer->commands[] = ctools_modal_command_dismiss();

    if ($type == 'pane') {
      $renderer->command_update_pane($pid);
    }
    else if ($type == 'region') {
      $renderer->command_update_region_links($pid);
    }
    else {
      $renderer->command_update_display_links();
    }
  }
  else {
    $renderer->commands = $output;
  }
}


/**
 * The form for determining if a pane should create a local style or a
 * preconfigured style.
 */
function panels_stylizer_edit_pane_style_form(&$form, &$form_state) {
  if (!user_access('administer panels styles') || !module_exists('stylizer')) {
    return;
  }
  ctools_include('dependent');

  $settings = $form_state['owner info']['owner settings'];
  $form['panels']['admin_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Administrative title'),
    '#description' => t('The name of this style. This will appear in the administrative interface to easily identify it.'),
    '#default_value' => $settings['admin_title'],
    '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-preconfigured' => array(1)),
  );

  $form['panels']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine name'),
    '#description' => t('The machine readable name of this page. It must be unique, and it must contain only alphanumeric characters and underscores. Once created, you will not be able to change this value!'),
    '#default_value' => $settings['name'],
    '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-preconfigured' => array(1)),
  );

  $form['panels']['admin_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Administrative description'),
    '#description' => t('A description of what this style is, does or is for, for administrative use.'),
    '#default_value' => $settings['admin_description'],
    '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-preconfigured' => array(1)),
  );

  // Add the checkbox, set the weight early
  $form['panels']['preconfigured'] = array(
    '#type' => 'checkbox',
    '#title' => t('Make this style available to other regions or panes'),
    '#default_value' => $settings['name'],
    '#weight' => -1,
  );

}

/**
 * Validate to see if we need to check the preconfigured values.
 */
function panels_stylizer_edit_pane_style_form_validate(&$form, &$form_state) {
  if (!user_access('administer panels styles')) {
    return;
  }

  // Only validate if preconfigured is checked.
  if ($form_state['values']['preconfigured'] && !empty($form_state['clicked_button']['#wizard type'])) {
    if (empty($form_state['values']['admin_title'])) {
      form_error($form['panels']['admin_title'], t('You must choose an administrative title.'));
    }

    // If this is new, make sure the name is unique:
    if ($form_state['op'] == 'add') {
      if (empty($form_state['values']['name'])) {
        form_error($form['panels']['name'], t('You must choose a machine name.'));
      }

      ctools_include('export');
      $test = ctools_export_crud_load('stylizer', $form_state['values']['name']);
      if ($test) {
        form_error($form['panels']['name'], t('That name is used by another style: @page', array('@page' => $test->admin_title)));
      }

      // Ensure name fits the rules:
      if (preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['name'])) {
        form_error($form['panels']['name'], t('Name must be alphanumeric or underscores only.'));
      }
    }
  }
}

/**
 * Store the preconfigured values.
 */
function panels_stylizer_edit_pane_style_form_submit(&$form, &$form_state) {
  if (!user_access('administer panels styles')) {
    return;
  }

  // Only validate if preconfigured is checked.
  if ($form_state['values']['preconfigured'] && !empty($form_state['clicked_button']['#wizard type'])) {
    $form_state['owner info']['owner settings']['admin_title'] = $form_state['values']['admin_title'];
    $form_state['owner info']['owner settings']['admin_description'] = $form_state['values']['admin_description'];

    // Clean up preview files before we set the name
    ctools_stylizer_cleanup_style($form_state['plugin'], $form_state['settings']);

    $form_state['settings']['name'] = $form_state['values']['name'];
    $form_state['name'] = $form_state['values']['name'];
    $form_state['owner info']['owner settings']['preconfigured'] = $form_state['values']['preconfigured'];
  }
}
