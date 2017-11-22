<?php

/**
 * Determine context major branch version.
 */
function context_features_get_version() {
  if (function_exists('context_context_plugins')) {
    return 3;
  }
  return 2;
}

/**
 * Implementation of hook_features_api().
 */
function context_features_api() {
  $api = array('context' => array());
  // 3.x
  if (context_features_get_version() === 3) {
    $api['context'] = array(
      'api' => 'context',
      'current_version' => 3,
      'module' => 'context',
      'default_file' => FEATURES_DEFAULTS_CUSTOM,
      'default_filename' => 'context',
    );
  }
  // 2.x
  $api['context'] += array(
    'name' => 'Contexts',
    'feature_source' => TRUE,
    'default_hook' => 'context_default_contexts'
  );
  return $api;
}

/**
 * Implementation of hook_features_export_options();
 */
function context_features_export_options() {
  // 3.x
  if (context_features_get_version() === 3) {
    return ctools_component_features_export_options('context');
  }
  // 2.x
  $contexts = context_enabled_contexts();
  $options = array();
  foreach ($contexts as $identifier => $context) {
    $options[$identifier] = "{$context->namespace} > {$context->attribute} > {$context->value}";
  }
  return $options;
}

/**
 * Implementation of hook_features_export().
 */
function context_features_export($data, &$export, $module_name = '') {
  // 3.x
  if (context_features_get_version() === 3) {
    $pipe = ctools_component_features_export('context', $data, $export, $module_name);

    $contexts = context_load();
    foreach ($data as $identifier) {
      if (isset($contexts[$identifier])) {
        $context = $contexts[$identifier];
        // Conditions.
        // Currently only node and views conditions are supported.
        // @TODO: Should this be delegated to a method on the plugin?
        foreach (array('node', 'views') as $key) {
          if (!empty($context->conditions{$key}['values'])) {
            foreach ($context->conditions{$key}['values'] as $item) {
              // Special pipe for views
              if ($key === 'views') {
                $split = explode(':', $item);
                $view_name = array_shift($split);
                $pipe[$key][$view_name] = $view_name;
              }
              else {
                $pipe[$key][$item] = $item;
              }
            }
          }
        }
        // Reactions.
        if (!empty($context->reactions['block']['blocks'])) {
          foreach ($context->reactions['block']['blocks'] as $block) {
            $block = (array) $block;
            $bid = "{$block['module']}-{$block['delta']}";
            $pipe['block'][$bid] = $bid;
          }
        }
      }
    }
    return $pipe;
  }

  // 2.x
  $export['dependencies']['context'] = 'context';

  // Collect a context to module map
  $map = features_get_default_map('context', NULL, 'context_features_identifier_2');
  $pipe = array();
  $contexts = context_enabled_contexts();
  foreach ($data as $identifier) {
    // If this context is already provided by another module, add it
    // as a dependency and prevent it from becoming a duplicate export.
    if (isset($map[$identifier]) && $map[$identifier] != $module_name) {
      if (isset($export['features']['context'][$identifier])) {
        unset($export['features']['context'][$identifier]);
      }
      $module = $map[$identifier];
      $export['dependencies'][$module] = $module;
    }
    // Otherwise, export it.
    else if (!empty($contexts[$identifier])) {
      $export['features']['context'][$identifier] = $identifier;

      $context = $contexts[$identifier];

      foreach (array('node', 'menu') as $key) {
        if (!empty($context->{$key})) {
          if (is_array($context->{$key})) {
            foreach ($context->{$key} as $item) {
              $pipe[$key][$item] = $item;
            }
          }
          else {
            $item = $context->{$key};
            $pipe[$key][$item] = $item;
          }
        }
      }

      // Special pipe for views
      if (!empty($context->views) && is_array($context->views)) {
        foreach ($context->views as $view_name) {
          $split = explode(':', $view_name);
          $view_name = array_shift($split);
          $pipe['views'][$view_name] = $view_name;
        }
      }

      // Special pipe for blocks
      if (!empty($context->block)) {
        foreach ($context->block as $block) {
          $block = (array) $block;
          $bid = "{$block['module']}-{$block['delta']}";
          $pipe['block'][$bid] = $bid;
        }
      }
    }
  }
  return $pipe;
}

/**
 * Implementation of hook_features_export_render().
 */
function context_features_export_render($module, $data) {
  // 3.x
  if (context_features_get_version() === 3) {
    return ctools_component_features_export_render('context', $module, $data);
  }
  // 2.x
  $code = array();
  $code[] = '  $items = array();';
  $code[] = '';

  foreach ($data as $identifier) {
    $contexts = context_enabled_contexts();
    $context = $contexts[$identifier];

    // prune system specific information and cast for Drupal's AOP (array oriented programming)
    $prune = array('cid', 'status', 'system', 'type');
    foreach ($prune as $key) {
      if (isset($context->{$key})) {
        unset($context->{$key});
      }
    }
    $context = (array) $context;

    // clean up blocks
    if (!empty($context['block'])) {
      foreach ($context['block'] as $bid => $block) {
        unset($block->bid);
        $context['block'][$bid] = (array) $block;
      }
    }

    $context_identifier = context_var_export($identifier);
    $context_export = context_var_export($context, '  ');
    $code[] = "  \$items[{$context_identifier}] = {$context_export};";
  }

  $code[] = '  return $items;';
  $code = implode("\n", $code);
  return array('context_default_contexts' => $code);
}

/**
 * Implementation of hook_features_revert().
 *
 * @param $module
 * name of module to revert content for
 */
function context_features_revert($module = NULL) {
  // 3.x
  if (context_features_get_version() === 3) {
    $return = ctools_component_features_revert('context', $module);
    context_invalidate_cache();
    return $return;
  }
  // 2.x
  if ($default_contexts = features_get_default('context', $module)) {
    foreach($default_contexts as $default_context) {
      $current_context = new StdClass();
      $current_context->namespace = $default_context['namespace'];
      $current_context->attribute = $default_context['attribute'];  
      $current_context->value = $default_context['value'];
      $context_to_delete = context_load_context($current_context);
      context_delete_context($context_to_delete);
    }
  }
}

/**
 * 2.x: Callback for generating the context exportable identifier.
 */
function context_features_identifier_2($object) {
  return isset($object->namespace, $object->attribute, $object->value) ? "{$object->namespace}-{$object->attribute}-{$object->value}" : FALSE;
}
