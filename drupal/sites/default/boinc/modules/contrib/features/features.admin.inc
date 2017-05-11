<?php

/**
 * Form callback for features export form. Acts as a router based on the form_state.
 */
function features_export_form($form_state, $feature = NULL) {
  module_load_include('inc', 'features', 'features.export');
  features_include();

  $form = array(
    '#attributes' => array('class' => 'features-export-form'),
    '#feature' => isset($feature) ? $feature : NULL,
  );
  $form['info'] = array(
    '#type' => 'fieldset',
    '#tree' => FALSE,
  );
  $form['info']['name'] = array(
    '#title' => t('Name'),
    '#description' => t('Example: Image gallery'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => !empty($feature->info['name']) ? $feature->info['name'] : '',
    '#attributes' => array('class' => 'feature-name'),
  );
  $form['info']['module_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine-readable name'),
    '#description' => t('Example: image_gallery'). '<br/>' .t('May only contain lowercase letters, numbers and underscores. <strong>Try to avoid conflicts with the names of existing Drupal projects.</strong>'),
    '#required' => TRUE,
    '#default_value' => !empty($feature->name) ? $feature->name : '',
    '#attributes' => array('class' => 'feature-module-name'),
    '#element_validate' => array('features_export_form_validate_field'),
  );
  // If recreating this feature, disable machine name field and blank out
  // js-attachment classes to ensure the machine name cannot be changed.
  if (isset($feature)) {
    $form['info']['module_name']['#value'] = $feature->name;
    $form['info']['module_name']['#disabled'] = TRUE;
    $form['info']['name']['#attributes'] = array();
  }
  $form['info']['description'] = array(
    '#title' => t('Description'),
    '#description' => t('Provide a short description of what users should expect when they enable your feature.'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#default_value' => !empty($feature->info['description']) ? $feature->info['description'] : '',
  );
  $form['info']['version'] = array(
    '#title' => t('Version'),
    '#description' => t('Examples: 6.x-1.0, 6.x-1.0-beta1'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#default_value' => !empty($feature->info['version']) ? $feature->info['version'] : '',
    '#size' => 30,
    '#element_validate' => array('features_export_form_validate_field'),
  );
  $form['info']['project_status_url'] = array(
    '#title' => t('URL of update XML'),
    '#description' => t('Example: http://mywebsite.com/fserver'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#default_value' => !empty($feature->info['project status url']) ? $feature->info['project status url'] : '',
    '#size' => 30,
    '#element_validate' => array('features_export_form_validate_field'),
  );

  // User-selected feature source components.
  $components = features_get_components();
  uasort($components, 'features_compare_component_name');

  $form['export'] = array(
    '#type' => 'fieldset',
    '#tree' => FALSE,
    '#theme' => 'features_form_export',
  );
  $form['export']['components'] = array(
    '#title' => t('Edit components'),
    '#type' => 'select',
    '#options' => array('------'),
    '#attributes' => array('class' => 'features-select-components'),
  );
  $form['export']['sources'] = array(
    '#tree' => TRUE,
    '#theme' => 'features_form_components',
  );
  foreach ($components as $component => $component_info) {
    $options = features_invoke($component, 'features_export_options');
    if ($component === 'dependencies') {
      $default_value = !empty($feature->info['dependencies']) ? $feature->info['dependencies'] : array();
    }
    else {
      $default_value = !empty($feature->info['features'][$component]) ? $feature->info['features'][$component] : array();
    }
    if ($options) {
      // Find all default components that are not provided by this feature and
      // strip them out of the possible options.
      if ($map = features_get_default_map($component)) {
        foreach ($map as $k => $v) {
          if (isset($options[$k]) && (!isset($feature->name) || $v !== $feature->name)) {
            unset($options[$k]);
          }
        }
      }
      // Ensure all options are stripped of potentially bad values.
      foreach ($options as $k => $v) {
        $options[$k] = check_plain($v);
      }
      $label = (isset($component_info['name']) ? $component_info['name'] . ": " . $component : $component);
      $form['export']['components']['#options'][$component] = $label;
      if (!empty($options)) {
        $form['export']['sources'][$component] = array(
          '#type' => 'checkboxes',
          '#options' => features_dom_encode_options($options),
          '#title' => $component,
          '#default_value' => features_dom_encode_options($default_value, FALSE),
          '#ahah' => array(
            'path' => 'admin/build/features/export/populate',
            'wrapper' => 'features-export-populated',
          ),
        );
      }
      else {
        $form['export']['sources'][$component] = array(
          '#type' => 'item',
          '#title' => $component,
          '#value' => t('All components of this type are exported by other features or modules.'),
        );
      }
    }
  }
  $form['export']['features'] = array(
    '#tree' => TRUE,
    '#type' => 'markup',
    '#prefix' => "<div id='features-export-populated'>",
    '#suffix' => "</div>",
    '#value' => !empty($feature->info) ? theme('features_components', $feature->info, $feature->info['features']) : "<div class='placeholder'></div>",
  );

  $form['buttons'] = array('#theme' => 'features_form_buttons', '#tree' => FALSE);
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Download feature'),
    '#weight' => 10,
    '#submit' => array('features_export_build_form_submit'),
  );
  return $form;
}

/**
 * Validation for project field.
 */
function features_export_form_validate_field($element, &$form_state) {
  switch ($element['#name']) {
    case 'module_name':
      if (!preg_match('!^[a-z0-9_]+$!', $element['#value'])) {
        form_error($element, t('The machine-readable name must contain only lowercase letters, numbers, and underscores.'));
      }
      // If user is filling out the feature name for the first time and uses
      // the name of an existing module throw an error.
      else if (empty($element['#default_value']) && features_get_info('module', $element['#value'])) {
        form_error($element, t('A module by the name @name already exists on your site. Please choose a different name.', array('@name' => $element['#value'])));
      }
      break;
    case 'project_status_url':
      if (!empty($element['#value']) && !valid_url($element['#value'])) {
        form_error($element, t('The URL %url is invalid. Please enter a fully-qualified URL, such as http://www.example.com/feed.xml.', array('%url' => $element['#value'])));
      }
      break;
    case 'version':
      preg_match('/^(?P<core>\d+\.x)-(?P<major>\d+)\.(?P<patch>\d+)-?(?P<extra>\w+)?$/', $element['#value'], $matches);
      if (!empty($element['#value']) && !isset($matches['core'], $matches['major'])) {
        form_error($element, t('Please enter a valid version with core and major version number. Example: !example', array('!example' => '6.x-1.0')));
      };
      break;
  }
}

/**
 * Submit handler for features_export_form_build().
 */
function features_export_build_form_submit($form, &$form_state) {
  module_load_include('inc', 'features', 'features.export');
  features_include();

  // Assemble the combined component list
  $stub = array();
  $components = array_keys(features_get_components());
  foreach ($components as $component) {
    // User-selected components take precedence.
    if (!empty($form_state['values']['sources'][$component])) {
      $stub[$component] = features_dom_decode_options(array_filter($form_state['values']['sources'][$component]));
    }
    // Only fallback to an existing feature's values if there are no export options for the component.
    else if (!empty($form['#feature']->info['features'][$component])) {
      $stub[$component] = $form['#feature']->info['features'][$component];
    }
  }

  // Generate populated feature
  $module_name = $form_state['values']['module_name'];
  $export = features_populate($stub, $form_state['values']['sources']['dependencies'], $module_name);

  // Directly copy the following attributes
  $attr = array('name', 'description');
  foreach ($attr as $key) {
    $export[$key] = isset($form_state['values'][$key]) ? $form_state['values'][$key] : NULL;
  }
  // If either update status-related keys are provided, add a project key
  // corresponding to the module name.
  if (!empty($form_state['values']['version']) || !empty($form_state['values']['project_status_url'])) {
    $export['project'] = $form_state['values']['module_name'];
  }
  if (!empty($form_state['values']['version'])) {
    $export['version'] = $form_state['values']['version'];
  }
  if (!empty($form_state['values']['project_status_url'])) {
    $export['project status url'] = $form_state['values']['project_status_url'];
  }

  // Generate download
  if ($files = features_export_render($export, $module_name, TRUE)) {
    $filename = (!empty($export['version']) ? "{$module_name}-{$export['version']}" : $module_name) . '.tar';

    // Clear out output buffer to remove any garbage from tar output.
    if (ob_get_level()) {
      ob_end_clean();
    }

    drupal_set_header('Content-type: application/x-tar');
    drupal_set_header('Content-Disposition: attachment; filename="'. $filename .'"');

    $tar = array();
    $filenames = array();
    foreach ($files as $extension => $file_contents) {
      if (!in_array($extension, array('module', 'info'))) {
        $extension .= '.inc';
      }
      $filenames[] = "{$module_name}.$extension";
      print features_tar_create("{$module_name}/{$module_name}.$extension", $file_contents);
    }
    if (features_get_modules($module_name, TRUE)) {
      $module_path = drupal_get_path('module', $module_name);
      // file_scan_directory() can throw warnings when using PHP 5.3, messing
      // up the output of our file stream. Suppress errors in this one case in
      // order to produce valid output.
      foreach (@file_scan_directory($module_path, '.*') as $file) {
        $filename = substr($file->filename, strlen($module_path) + 1);
        if (!in_array($filename, $filenames)) {
          // Add this file.
          $contents = file_get_contents($file->filename);
          print features_tar_create("{$module_name}/{$filename}", $contents);
          unset($contents);
        }
      }
    }
    print pack("a1024","");
    exit;
  }
}

/**
 * AHAH handler for features_export_form_build().
 */
function features_export_build_form_populate() {
  module_load_include('inc', 'features', 'features.export');
  features_include();

  $form_state = array();
  $submitted = $_POST;

  if ($form = form_get_cache($submitted['form_build_id'], $form_state)) {
    $stub = array();

    // Assemble the combined component list
    $components = array_keys(features_get_components());
    foreach ($components as $component) {
      // User-selected components take precedence.
      if (!empty($submitted['sources'][$component])) {
        // Validate and set the default value for each selected option. This
        foreach ($submitted['sources'][$component] as $key => $value) {
          if (isset($form['export']['sources'][$component]['#options'][$key])) {
            $form['export']['sources'][$component]['#default_value'][$key] = $value;
          }
        }
        $stub[$component] = features_dom_decode_options(array_filter($submitted['sources'][$component]));
      }
      // Only fallback to an existing feature's values if there are no export options for the component.
      else if (!isset($form['export']['sources'][$component]) && !empty($form['#feature']->info['features'][$component])) {
        $stub[$component] = $form['#feature']->info['features'][$component];
      }
    }

    // Assemble dependencies
    $dependencies = isset($submitted['sources']['dependencies']) ? $submitted['sources']['dependencies'] : array();

    // Generate populated feature
    $module_name = isset($form['#feature'], $form['#feature']->name) ? $form['#feature']->name : '';
    $export = features_populate($stub, $dependencies, $module_name);

    // Render component display
    $components_rendered = theme('features_components', $export, $stub);
    $form['export']['features']['#value'] = $components_rendered;

    // Re-cache form. This ensures that if the form fails to validate, selected
    // values are preserved for the user.
    form_set_cache($submitted['form_build_id'], $form, $form_state);

    drupal_json(array('status' => TRUE, 'data' => $components_rendered . theme('status_messages')));
    exit;
  }
  drupal_json(array('status' => FALSE, 'data' => ''));
  exit;
}

/**
 * admin/build/features page callback.
 */
function features_admin_form($form_state) {
  // Load export functions to use in comparison.
  module_load_include('inc', 'features', 'features.export');

  // Clear & rebuild key caches
  module_rebuild_cache();
  features_rebuild();

  features_get_info(NULL, NULL, TRUE);
  $modules = features_get_modules();
  $features = features_get_features();
  $conflicts = features_get_conflicts();

  foreach ($modules as $key => $module) {
    if ($module->status && !empty($module->info['dependencies'])) {
      foreach ($module->info['dependencies'] as $dependent) {
        if (isset($features[$dependent])) {
          $features[$dependent]->dependents[$key] = $module->info['name'];
        }
      }
    }
  }

  $form = array(
    '#features' => $features,
    '#theme' => 'features_form',
  );

  // Generate features form.
  foreach ($features as $name => $module) {
    $package_title = !empty($module->info['package']) ? $module->info['package'] : t('Other');
    $package = strtolower(preg_replace('/[^a-zA-Z0-9-]+/', '-', $package_title));

    // Set up package elements
    if (!isset($form[$package])) {
      $form[$package] = array(
        '#tree' => FALSE,
        '#title' => t($package_title),
        '#theme' => 'features_form_package'
      );
      $form[$package]['links'] =
      $form[$package]['version'] =
      $form[$package]['weight'] =
      $form[$package]['status'] =
      $form[$package]['action'] = array('#tree' => TRUE);
    }

    $disabled = FALSE;
    $description = isset($module->info['description']) ? $module->info['description'] : '';

    // Detect unmet dependencies
    if (!empty($module->info['dependencies'])) {
      $unmet_dependencies = array();
      $dependencies = _features_export_maximize_dependencies($module->info['dependencies']);
      foreach ($dependencies as $dependency) {
        if (empty($modules[$dependency])) {
          $unmet_dependencies[] = theme('features_module_status', FEATURES_MODULE_MISSING, $dependency);
        }
      }
      if (!empty($unmet_dependencies)) {
        $description .= "<div class='dependencies'>". t('Unmet dependencies: !dependencies', array('!dependencies' => implode(', ', $unmet_dependencies))) ."</div>";
        $disabled = TRUE;
      }
    }

    if (!empty($module->dependents)) {
      $disabled = TRUE;
      $description .= "<div class='requirements'>". t('Required by: !dependents', array('!dependents' => implode(', ', $module->dependents))) ."</div>";
    }

    // Detect potential conflicts
    if (!empty($conflicts[$name])) {
      $module_conflicts = array();
      foreach (array_keys($conflicts[$name]) as $conflict) {
        $module_conflicts[] = theme('features_module_status', FEATURES_MODULE_MISSING, $conflict);
        // Only disable modules with conflicts if they are not already enabled.
        // If they are already enabled, somehow the user got themselves into a
        // bad situation and they need to be able to disable a conflicted module.
        if (module_exists($conflict) && !module_exists($name)) {
          $disabled = TRUE;
        }
      }
      $description .= "<div class='conflicts'>". t('Conflicts with: !conflicts', array('!conflicts' => implode(', ', $module_conflicts))) ."</div>";
    }

    $form[$package]['status'][$name] = array(
      '#type' => 'checkbox',
      '#title' => $module->info['name'],
      '#description' => $description,
      '#default_value' => $module->status,
      '#disabled' => $disabled,
    );

    if (!empty($module->info['project status url'])) {
      $uri = l(truncate_utf8($module->info['project status url'], 35, TRUE, TRUE), $module->info['project status url']);
    }
    else if (isset($module->info['project'], $module->info['version'], $module->info['datestamp'])) {
      $uri = l('http://drupal.org', 'http://drupal.org/project/'. $module->info['project']);
    }
    else {
      $uri = t('Unavailable');
    }
    $version = !empty($module->info['version']) ? $module->info['version'] : '';
    $version = !empty($version) ? "<div class='description'>$version</div>" : '';
    $form[$package]['sign'][$name] = array('#type' => 'markup', '#value' => "{$uri} {$version}");

    if (user_access('administer features')) {
      // Add status link
      $href = "admin/build/features/{$name}";
      if ($module->status) {
        $state = '<span class="admin-loading features-storage">' . t('Checking...') . '</span>';
        $state .= l(t('Check'), "admin/build/features/{$name}/status", array('attributes' => array('class' => 'admin-check')));
        $state .= theme('features_storage_link', FEATURES_REBUILDING, NULL, $href);
        $state .= theme('features_storage_link', FEATURES_NEEDS_REVIEW, NULL, $href);
        $state .= theme('features_storage_link', FEATURES_OVERRIDDEN, NULL, $href);
        $state .= theme('features_storage_link', FEATURES_DEFAULT, NULL, $href);
      }
      elseif (!empty($conflicts[$name])) {
        $state = theme('features_storage_link', FEATURES_CONFLICT, NULL, $href);
      }
      else {
        $state = theme('features_storage_link', FEATURES_DISABLED, NULL, $href);
      }
      $form[$package]['state'][$name] = array(
        '#type' => 'markup',
        '#value' => !empty($state) ? $state : '',
      );

      // Add in recreate link
      $form[$package]['actions'][$name] = array(
        '#type' => 'markup',
        '#value' => l(t('Recreate'), "admin/build/features/{$name}/recreate", array('attributes' => array('class' => 'admin-update'))),
      );
    }
  }

  $form['buttons'] = array(
    '#theme' => 'features_form_buttons',
  );
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
    '#submit' => array('features_form_submit'),
    '#validate' => array('features_form_validate'),
  );
  return $form;
}

/**
 * Display the components of a feature.
 */
function features_admin_components($form_state, $feature) {
  module_load_include('inc', 'features', 'features.export');
  $form = array();

  // Store feature info for theme layer.
  $form['module'] = array('#type' => 'value', '#value' => $feature->name);
  $form['#info'] = $feature->info;
  $form['#dependencies'] = array();
  if (!empty($feature->info['dependencies'])) {
    foreach ($feature->info['dependencies'] as $dependency) {
      $status = features_get_module_status($dependency);
      $form['#dependencies'][$dependency] = $status;
    }
  }

  $review = $revert = FALSE;

  // Iterate over components and retrieve status for display
  $states = features_get_component_states(array($feature->name), FALSE);
  $form['revert']['#tree'] = TRUE;
  foreach ($feature->info['features'] as $component => $items) {
    if (user_access('administer features') && in_array($states[$feature->name][$component], array(FEATURES_OVERRIDDEN, FEATURES_NEEDS_REVIEW))) {
      switch ($states[$feature->name][$component]) {
        case FEATURES_OVERRIDDEN:
          $revert = TRUE;
          break;
        case FEATURES_NEEDS_REVIEW:
          $review = TRUE;
          break;
      }
      $form['revert'][$component] = array(
        '#type' => 'checkbox',
        '#default_value' => FALSE,
      );
    }
    if (module_exists('diff')) {
      $item = menu_get_item("admin/build/features/{$feature->name}/diff/{$component}");
      $path = ($item && $item['access']) ? $item['href'] : NULL;
    }
    else {
      $path = NULL;
    }
    $form['components'][$component] = array(
      '#type' => 'markup',
      '#value' => theme('features_storage_link', $states[$feature->name][$component], NULL, $path),
    );
  }

  if ($review || $revert) {
    $form['buttons'] = array('#theme' => 'features_form_buttons', '#tree' => TRUE);
    if ($revert || $review) {
      $form['buttons']['revert'] = array(
        '#type' => 'submit',
        '#value' => t('Revert components'),
        '#submit' => array('features_admin_components_revert'),
      );
    }
    if ($review) {
      $form['buttons']['review'] = array(
        '#type' => 'submit',
        '#value' => t('Mark as reviewed'),
        '#submit' => array('features_admin_components_review'),
      );
    }
  }
  return $form;
}

/**
 * Submit handler for revert form.
 */
function features_admin_components_revert(&$form, &$form_state) {
  module_load_include('inc', 'features', 'features.export');
  features_include();
  $module = $form_state['values']['module'];
  $revert = array();
  foreach (array_filter($form_state['values']['revert']) as $component => $status) {
    $revert[$module][] = $component;
    drupal_set_message(t('Reverted all <strong>!component</strong> components for <strong>!module</strong>.', array('!component' => $component, '!module' => $module)));
  }
  features_revert($revert);
  $form_state['redirect'] = 'admin/build/features/'. $module;
}

/**
 * Submit handler for revert form.
 */
function features_admin_components_review(&$form, &$form_state) {
  module_load_include('inc', 'features', 'features.export');
  features_include();
  $module = $form_state['values']['module'];
  $revert = array();
  foreach (array_filter($form_state['values']['revert']) as $component => $status) {
    features_set_signature($module, $component);
    drupal_set_message(t('All <strong>!component</strong> components for <strong>!module</strong> reviewed.', array('!component' => $component, '!module' => $module)));
  }
  $form_state['redirect'] = 'admin/build/features/'. $module;
}

/**
 * Validate handler for the 'manage features' form.
 */
function features_form_validate(&$form, &$form_state) {
  include_once './includes/install.inc';
  $conflicts = features_get_conflicts();
  foreach ($form_state['values']['status'] as $module => $status) {
    if ($status) {
      if (!empty($conflicts[$module])) {
        foreach (array_keys($conflicts[$module]) as $conflict) {
          if (!empty($form_state['values']['status'][$conflict])) {
            form_set_error('status', t('The feature !module cannot be enabled because it conflicts with !conflict.', array('!module' => $module, '!conflict' => $conflict)));
          }
        }
      }
      if (!drupal_check_module($module)) {
        form_set_error('status', t('The feature !module cannot be enabled because it has unmet requirements.', array('!module' => $module, '!conflict' => $conflict)));
      }
    }
  }
}

/**
 * Submit handler for the 'manage features' form
 */
function features_form_submit(&$form, &$form_state) {
  // Clear drupal caches after enabling a feature. We do this in a separate
  // page callback rather than as part of the submit handler as some modules
  // have includes/other directives of importance in hooks that have already
  // been called in this page load.
  $form_state['redirect'] = array('admin/build/features/cleanup/clear', array('token' => drupal_get_token()));

  $features = $form['#features'];
  if (!empty($features)) {
    $status = $form_state['values']['status'];
    $install = array_keys(array_filter($status));
    $disable = array_diff(array_keys($status), $install);

    // Disable first. If there are any features that are disabled that are
    // dependencies of features that have been queued for install, they will
    // be re-enabled.
    module_disable($disable);
    features_install_modules($install);
  }
}

/**
 * Form for disabling orphaned dependencies.
 */
function features_cleanup_form($form_state, $cache_clear = FALSE) {
  $form = array();

  // Clear caches if we're getting a post-submit redirect that requests it.
  if ($cache_clear && !empty($_GET['token']) && drupal_valid_token($_GET['token'])) {
    drupal_flush_all_caches();

    // The following functions need to be run because drupal_flush_all_caches()
    // runs rebuilds in the wrong order. The node type cache is rebuilt *after*
    // the menu is rebuilt, meaning that the menu tree is stale in certain
    // circumstances after drupal_flush_all_caches(). We rebuild again.
    menu_rebuild();
  }

  // Retrieve orphaned modules and provide them as optional modules to be disabled.
  // Exclude any modules that have been added to the 'ignored' list.
  $options = array();
  $orphans = features_get_orphans();
  $ignored = variable_get('features_ignored_orphans', array());
  if (!empty($orphans)) {
    foreach ($orphans as $module) {
      if (!in_array($module->name, $ignored, TRUE)) {
        $options[$module->name] = check_plain($module->info['name']);
      }
    }
  }

  if (!empty($options)) {
    $form['orphans'] = array(
      '#title' => t('Orphaned dependencies'),
      '#description' => t('These modules are dependencies of features that have been disabled. They may be disabled without affecting other components of your website.'),
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => array_keys($options),
    );
    $form['buttons'] = array('#tree' => TRUE, '#theme' => 'features_form_buttons');
    $form['buttons']['disable'] = array(
      '#type' => 'submit',
      '#value' => t('Disable selected modules'),
      '#submit' => array('features_cleanup_form_disable'),
    );
    $form['buttons']['ignore'] = array(
      '#type' => 'submit',
      '#value' => t('Leave enabled'),
      '#submit' => array('features_cleanup_form_ignore'),
    );
  }
  else {
    drupal_goto('admin/build/features');
  }
  return $form;
}

/**
 * Submit handler for disable action on features_cleanup_form().
 */
function features_cleanup_form_disable(&$form, &$form_state) {
  if (!empty($form_state['values']['orphans'])) {
    $disable = array_keys(array_filter($form_state['values']['orphans']));
    $ignored = array_diff(array_keys($form_state['values']['orphans']), $disable);

    // Disable any orphans that have been selected.
    module_disable($disable);
    drupal_flush_all_caches();

    // Add enabled modules to ignored orphans list.
    $ignored_orphans = variable_get('features_ignored_orphans', array());
    foreach ($ignored as $module) {
      $ignored_orphans[$module] = $module;
    }
    variable_set('features_ignored_orphans', $ignored_orphans);
  }
  $form_state['redirect'] = 'admin/build/features/cleanup';
}

/**
 * Submit handler for ignore action on features_cleanup_form().
 */
function features_cleanup_form_ignore(&$form, &$form_state) {
  if (!empty($form_state['values']['orphans'])) {
    $ignored = array_keys($form_state['values']['orphans']);
    $ignored_orphans = variable_get('features_ignored_orphans', array());
    foreach ($ignored as $module) {
      $ignored_orphans[$module] = $module;
    }
    variable_set('features_ignored_orphans', $ignored_orphans);
  }
  $form_state['redirect'] = 'admin/build/features/cleanup';
}

/**
 * Page callback to display the differences between what's in code and
 * what is in the db.
 *
 * @param $feature
 *   A loaded feature object to display differences for.
 * @param $component
 *   Optional: specific component to display differences for. If excluded, all components are used.
 *
 * @return Themed display of what is different.
 */
function features_feature_diff($feature, $component = NULL) {
  drupal_add_css(drupal_get_path('module', 'features') .'/features.css');
  module_load_include('inc', 'features', 'features.export');

  $overrides = features_detect_overrides($feature);

  $output = '';
  if (!empty($overrides)) {
    // Filter overrides down to specified component.
    if (isset($component) && isset($overrides[$component])) {
      $overrides = array($component => $overrides[$component]);
    }

    module_load_include('php', 'diff', 'DiffEngine');
    $formatter = new DrupalDiffFormatter();
    $rows = array();
    foreach ($overrides as $component => $items) {
      $diff = new Diff(explode("\n", $items['default']), explode("\n", $items['normal']));
      $rows[] = array(array('data' => $component, 'colspan' => 4, 'header' => TRUE));
      $rows = array_merge($rows, $formatter->format($diff));
    }
    $header = array(
      array('data' => t('Default'), 'colspan' => 2),
      array('data' => t('Overrides'), 'colspan' => 2),
    );
    $output .= theme('diff_table', $header, $rows, array('class' => 'diff features-diff'));
  }
  else {
    $output = "<div class='features-empty'>". t('No changes have been made to this feature.') ."</div>";
  }
  $output = "<div class='features-comparison'>{$output}</div>";
  return $output;
}

/**
 * Compare the component names. Used to sort alphabetically.
 */
function features_compare_component_name($a, $b) {
  return strcasecmp($a['name'], $b['name']);
}

/**
 * Javascript call back that returns the status of a feature.
 */
function features_feature_status($feature) {
  module_load_include('inc', 'features', 'features.export');
  return drupal_json(array('storage' => features_get_storage($feature->name)));
}

/**
 * Make a Drupal options array safe for usage with jQuery DOM selectors.
 * Encodes known bad characters into __[ordinal]__ so that they may be
 * safely referenced by JS behaviors.
 */
function features_dom_encode_options($options = array(), $keys_only = TRUE) {
  $replacements = array(
    ':' => '__'. ord(':') .'__',
    '/' => '__'. ord('/') .'__',
    ',' => '__'. ord(',') .'__',
    '.' => '__'. ord(',') .'__',
    '<' => '__'. ord('<') .'__',
    '>' => '__'. ord('>') .'__',
  );
  $encoded = array();
  foreach ($options as $key => $value) {
    $encoded[strtr($key, $replacements)] = $keys_only ? $value : strtr($value, $replacements);
  }
  return $encoded;
}

/**
 * Decode an array of option values that have been encoded by
 * features_dom_encode_options().
 */
function features_dom_decode_options($options, $keys_only = FALSE) {
  $replacements = array_flip(array(
    ':' => '__'. ord(':') .'__',
    '/' => '__'. ord('/') .'__',
    ',' => '__'. ord(',') .'__',
    '.' => '__'. ord(',') .'__',
    '<' => '__'. ord('<') .'__',
    '>' => '__'. ord('>') .'__',
  ));
  $encoded = array();
  foreach ($options as $key => $value) {
    $encoded[strtr($key, $replacements)] = $keys_only ? $value : strtr($value, $replacements);
  }
  return $encoded;
}
