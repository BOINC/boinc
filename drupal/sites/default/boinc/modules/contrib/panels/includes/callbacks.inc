<?php
/**
 * @file callbacks.inc
 * Minor menu callbacks for Panels helpers.
 */

/**
 * A central administrative page for Panels.
 */
function panels_admin_page() {
  return theme('panels_dashboard');
}

function panels_dashboard_final_blocks(&$vars) {
  // Add in links for missing modules that we still want to mention:
  if (empty($vars['links']['page_manager'])) {
    $vars['links']['page_manager'] = array(
      'weight' => -100,
      'title' => t('Panel page'),
      'description' => '<em>' . t('You must activate the page manager module for this functionality.') . '</em>',
    );
  }
  if (empty($vars['links']['panels_mini'])) {
    $vars['links']['panels_mini'] = array(
      'title' => t('Mini panel'),
      'description' => '<em>' . t('You must activate the Mini panels module for this functionality.') . '</em>',
    );
  }
  if (empty($vars['links']['panels_node'])) {
    $vars['links']['panels_mini'] = array(
      'title' => t('Panel node'),
      'description' => '<em>' . t('You must activate the panel node module for this functionality.') . '</em>',
    );
  }
}

/**
 * Implementation of hook_panels_dashboard_blocks().
 *
 * Adds page information to the Panels dashboard.
 */
function panels_panels_dashboard_blocks(&$vars) {
  $vars['links']['panels_layout'] = array(
    'title' => l(t('Custom layout'), 'admin/build/panels/layouts/add'),
    'description' => t('Custom layouts can add more, site-specific layouts that you can use in your panels.'),
  );

   // Load all mini panels and their displays.
  ctools_include('export');
  $items = ctools_export_crud_load_all('panels_layout');
  $count = 0;
  $rows = array();

  foreach ($items as $item) {
    $rows[] = array(
      check_plain($item->admin_title),
      array(
        'data' => l(t('Edit'), "admin/build/panels/layouts/list/$item->name/edit"),
        'class' => 'links',
      ),
    );

    // Only show 10.
    if (++$count >= 10) {
      break;
    }
  }

  if ($rows) {
    $content = theme('table', array(), $rows, array('class' => 'panels-manage'));
  }
  else {
    $content = '<p>' . t('There are no custom layouts.') . '</p>';
  }

  $vars['blocks']['panels_layout'] = array(
    'title' => t('Manage custom layouts'),
    'link' => l(t('Go to list'), 'admin/build/panels/layouts'),
    'content' => $content,
    'class' => 'dashboard-layouts',
    'section' => 'right',
  );
}

function template_preprocess_panels_dashboard(&$vars) {
  ctools_add_css('panels-dashboard', 'panels');
  ctools_include('plugins');

  $vars['image_path'] = ctools_image_path('', 'panels');

  $vars['links'] = array();
  $vars['blocks'] = array();

  foreach (module_implements('panels_dashboard_blocks') as $module) {
    $function = $module . '_panels_dashboard_blocks';
    $function($vars);
  }

  // Add in any default links for modules that are not active
  panels_dashboard_final_blocks($vars);

  // If page manager module is enabled, add a very low eight block to
  // list the page wizards.
  if (module_exists('page_manager')) {
    $vars['blocks']['wizards'] = array(
      'weight' => -101,
      'section' => 'right',
      'title' => t('Page wizards'),
      'content' => '',
      'class' => 'dashboard-wizards',
    );

    ctools_include('page-wizard');
    $plugins = page_manager_get_page_wizards();
    uasort($plugins, 'ctools_plugin_sort');

    foreach ($plugins as $id => $plugin) {
      if (isset($plugin['type']) && $plugin['type'] == 'panels') {
        $link = array(
          'title' => l($plugin['title'], 'admin/build/pages/wizard/' . $id),
          'description' => $plugin['description'],
        );

        $vars['blocks']['wizards']['content'] .= theme('panels_dashboard_link', $link);
      }
    }

  }

  uasort($vars['links'], 'ctools_plugin_sort');

  $vars['blocks']['links'] = array(
    'weight' => -100,
    'section' => 'left',
    'title' => t('Create new') . '...',
    'content' => '',
    'class' => 'dashboard-create',
  );

  // Turn the links into a block
  foreach ($vars['links'] as $link) {
    $vars['blocks']['links']['content'] .= theme('panels_dashboard_link', $link);
  }

  uasort($vars['blocks'], 'ctools_plugin_sort');

  $vars['left'] = '';
  $vars['right'] = '';

  // Render all the blocks
  foreach ($vars['blocks'] as $block) {
    $section = !empty($block['section']) ? $block['section'] : 'left';
    $vars[$section] .= theme('panels_dashboard_block', $block);
  }
}

function panels_admin_settings_page() {
  $form = array();
  if (module_exists('page_manager')) {
    foreach (page_manager_get_tasks() as $task) {
      if ($function = ctools_plugin_get_function($task, 'admin settings')) {
        $function($form);
      }
    }
  }

  ctools_include('content');
  foreach (ctools_get_content_types() as $content) {
    if ($function = ctools_plugin_get_function($content, 'admin settings')) {
      $function($form);
    }
  }

  if (empty($form)) {
    return array('#value' => t('There are currently no settings to change, but additional plugins or modules may provide them in the future.'));
  }

  return system_settings_form($form);
}

/**
 * Settings for panel contexts created by the page manager.
 */
function panels_admin_panel_context_page() {
  ctools_include('common', 'panels');
  return drupal_get_form('panels_common_settings', 'panels_page');
}

