<?php
// $Id: context.api.php,v 1.1.2.4 2010/07/29 16:24:04 yhahn Exp $

/**
 * @file
 * Hooks provided by Context.
 */

/**
 * CTools plugin API hook for Context. Note that a proper entry in
 * hook_ctools_plugin_api() must exist for this hook to be called.
 */
function hook_context_plugins() {
  $plugins = array();
  $plugins['foo_context_condition_bar'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'foo') .'/plugins',
      'file' => 'foo_context_condition_bar.inc',
      'class' => 'foo_context_condition_bar',
      'parent' => 'context_condition',
    ),
  );
  $plugins['foo_context_reaction_baz'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'foo') .'/plugins',
      'file' => 'foo_context_reaction_baz.inc',
      'class' => 'foo_context_reaction_baz',
      'parent' => 'context_reaction',
    ),
  );
  return $plugins;
}

/**
 * Registry hook for conditions & reactions.
 *
 * Each entry associates a condition or reaction with the CTools plugin to be
 * used as its plugin class.
 */
function hook_context_registry() {
  return array(
    'conditions' => array(
      'bar' => array(
        'title' => t('Name of condition "bar"'),
        'plugin' => 'foo_context_condition_bar',
      ),
    ),
    'reactions' => array(
      'baz' => array(
        'title' => t('Name of reaction "baz"'),
        'plugin' => 'foo_context_reaction_baz',
      ),
    ),
  );
}

/**
 * Alter the registry.
 *
 * Allows modules to alter the registry. Default plugins can be replaced by
 * custom ones declared in hook_context_plugins().
 *
 * @param $registry
 *   The registry, passed by reference.
 */
function hook_context_registry_alter(&$registry) {
  if (isset($registry['reactions']['baz'])) {
    $registry['reactions']['baz']['plugin'] = 'custom_context_reaction_baz';
  }
}

/**
 * Alter/add a condition to a node-related event.
 *
 * Allows modules to add one or more context condition plugin executions to a
 * node view, form, etc.
 *
 * @param $node
 *   The node object.
 * @param $op
 *   The node-related operation: 'node', 'form', 'comment'.
 */
function hook_context_node_condition_alter(&$node, $op) {
  if ($plugin = context_get_plugin('condition', 'bar')) {
    $plugin->execute($node, $op);
  }
}

/**
 * Alter a context directly after it has been loaded. Allows modules to alter
 * a context object's reactions. While you may alter conditions, this will
 * generally have no effect as conditions are cached for performance and
 * contexts are loaded after conditions are checked, not before.
 *
 * @param &$context
 *   The context object by reference.
 */
function hook_context_load_alter(&$context) {
  if ($context->name === 'foo' && isset($context->reactions['block'])) {
    $context->reactions['block']['blocks']['locale-0'] = array(
      'module' => 'locale',
      'delta' => '0',
      'region' => 'header',
      'weight' => '2',
    );
  }
}
