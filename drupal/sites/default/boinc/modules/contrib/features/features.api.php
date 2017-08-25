<?php

/**
 * Main info hook that features uses to determine what components are provided
 * by the implementing module.
 *
 * @return array
 *   An array of components, keyed by the component name. Each component can
 *   define several keys:
 *
 *   'file': Optional path to a file to include which contains the rest
 *   of the features API hooks for this module.
 *
 *   'default_hook': The defaults hook for your component that is called
 *   when the cache of default components is generated. Examples include
 *   hook_views_default_views() or hook_context_default_contexts().
 *
 *   'default_file': The file-writing behavior to use when exporting this
 *   component. May be one of 3 constant values:
 *
 *   FEATURES_DEFAULTS_INCLUDED_COMMON: write hooks/components to
 *   `.features.inc` with other components. This is the default behavior
 *   if this key is not defined.
 *
 *   FEATURES_DEFAULTS_INCLUDED: write hooks/components to a component-
 *   specific include named automatically by features.
 *
 *   FEATURES_DEFAULTS_CUSTOM: write hooks/components to a component-
 *   specific include with a custom name provided. If your module provides
 *   large amounts of code that should not be parsed often (only on specific
 *   cache clears/rebuilds, for example) you should use the 2nd or 3rd
 *   options to split your component into its own include.
 *
 *   'default_filename': The filename to use when 'default_file' is set to
 *   FEATURES_DEFAULTS_CUSTOM.
 *
 *   'features_source': Boolean value for whether this component should be
 *   offered as an option on the initial feature creation form.
 *
 *   'base': Optional. An alternative base key to use when calling features
 *   hooks for this component. Can be used for features component types that
 *   are declared "dynamically" or are part of a family of components.
 */
function hook_features_api() {
  return array(
    'mycomponent' => array(
      'default_hook' => 'mycomponent_defaults',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
      'features_source' => TRUE,
      'file' => drupal_get_path('module', 'mycomponent') .'/mycomponent.features.inc',
    ),
  );
}

/**
 * Component hook. The hook should be implemented using the name ot the
 * component, not the module, eg. [component]_features_export() rather than
 * [module]_features_export().
 *
 * Process the export array for a given component. Implementations of this hook
 * have three key tasks:
 *
 * 1. Determine module dependencies for any of the components passed to it
 *   e.g. the views implementation iterates over each views' handlers and
 *   plugins to determine which modules need to be added as dependencies.
 *
 * 2. Correctly add components to the export array. In general this is usually
 *   adding all of the items in $data to $export['features']['my_key'], but
 *   can become more complicated if components are shared between features
 *   or modules.
 *
 * 3. Delegating further detection and export tasks to related or derivative
 *   components.
 *
 * Each export processor can kickoff further export processors by returning a
 * keyed array (aka the "pipe") where the key is the next export processor hook
 * to call and the value is an array to be passed to that processor's $data
 * argument. This allows an export process to start simply at a few objects:
 *
 * [context]
 *
 * And then branch out, delegating each component to its appropriate hook:
 *
 * [context]--------+------------+
 *     |            |            |
 *   [node]      [block]      [views]
 *     |
 *   [CCK]
 *     |
 * [imagecache]
 *
 * @param array $data
 *   An array of machine names for the component in question to be exported.
 * @param array &$export
 *   By reference. An array of all components to be exported with a given
 *   feature. Component objects that should be exported should be added to
 *   this array.
 * @param string $module_name
 *   The name of the feature module to be generated.
 * @return array
 *   The pipe array of further processors that should be called.
 */
function hook_features_export($data, &$export, $module_name) {
  // The following is the simplest implementation of a straight object export
  // with no further export processors called.
  foreach ($data as $component) {
    $export['mycomponent'][$component] = $component;
  }
  return array();
}

/**
 * Component hook. The hook should be implemented using the name ot the
 * component, not the module, eg. [component]_features_export() rather than
 * [module]_features_export().
 *
 * List all objects for a component that may be exported.
 *
 * @return array
 *   A keyed array of items, suitable for use with a FormAPI select or
 *   checkboxes element.
 */
function hook_features_export_options() {
  $options = array();
  foreach (mycomponent_load() as $mycomponent) {
    $options[$mycomponent->name] = $mycomponent->title;
  }
  return $options;
}

/**
 * Component hook. The hook should be implemented using the name ot the
 * component, not the module, eg. [component]_features_export() rather than
 * [module]_features_export().
 *
 * Render one or more component objects to code.
 *
 * @param string $module_name
 *   The name of the feature module to be exported.
 * @param array $data
 *   An array of machine name identifiers for the objects to be rendered.
 * @param array $export
 *   The full export array of the current feature being exported. This is only
 *   passed when hook_features_export_render() is invoked for an actual feature
 *   update or recreate, not during state checks or other operations.
 * @return array
 *   An associative array of rendered PHP code where the key is the name of the
 *   hook that should wrap the PHP code. The hook should not include the name
 *   of the module, e.g. the key for `hook_example` should simply be `example`.
 */
function hook_features_export_render($module_name, $data, $export = NULL) {
  $code = array();
  $code[] = '$mycomponents = array();';
  foreach ($data as $name) {
    $code[] = "  \$mycomponents['{$name}'] = " . features_var_export(mycomponent_load($name)) .";";
  }
  $code[] = "return \$mycomponents;";
  $code = implode("\n", $mycomponents);
  return array('mycomponent_defaults' => $code);
}

/**
 * Component hook. The hook should be implemented using the name ot the
 * component, not the module, eg. [component]_features_export() rather than
 * [module]_features_export().
 *
 * Revert all component objects for a given feature module.
 *
 * @param string $module_name
 *   The name of the feature module whose components should be reverted.
 * @return boolean
 *   TRUE or FALSE for whether the components were successfully reverted.
 */
function hook_features_revert($module_name) {
  $mycomponents = module_invoke_all($module_name, 'mycomponent_defaults');
  if (!empty($$mycomponents)) {
    foreach ($mycomponents as $mycomponent) {
      mycomponent_delete($mycomponent);
    }
  }
}

/**
 * Component hook. The hook should be implemented using the name ot the
 * component, not the module, eg. [component]_features_export() rather than
 * [module]_features_export().
 *
 * Rebuild all component objects for a given feature module. Should only be
 * implemented for 'faux-exportable' components.
 *
 * This hook is called at points where Features determines that it is safe
 * (ie. the feature is in state `FEATURES_REBUILDABLE`) for your module to
 * replace objects in the database with defaults that you collect from your
 * own defaults hook. See API.txt for how Features determines whether a
 * rebuild of components is possible.
 *
 * @param string $module_name
 *   The name of the feature module whose components should be rebuilt.
 */
function hook_features_export_rebuild($module_name) {
  $mycomponents = module_invoke_all($module_name, 'mycomponent_defaults');
  if (!empty($$mycomponents)) {
    foreach ($mycomponents as $mycomponent) {
      mycomponent_save($mycomponent);
    }
  }
}

/**
 * Alter the final export array just prior to the rendering of defaults. Allows
 * modules a final say in altering what component objects are exported.
 *
 * @param array &$export
 *   By reference. An array of all components to be exported with a given
 *   feature.
 * @param array $module_name
 *   The name of the feature module to be generated.
 */
function hook_features_export_alter(&$export, $module_name) {
  // Example: do not allow the page content type to be exported, ever.
  if (!empty($export['features']['node']['page'])) {
    unset($export['features']['node']['page']);
  }
}

/**
 * Alter the pipe array for a given component. This hook should be implemented
 * with the name of the component type in place of `component` in the function
 * name, e.g. `features_pipe_views_alter()` will alter the pipe for the Views
 * component.
 *
 * @param array &$pipe
 *   By reference. The pipe array of further processors that should be called.
 * @param array $data
 *   An array of machine names for the component in question to be exported.
 * @param array &$export
 *   By reference. An array of all components to be exported with a given
 *   feature.
 */
function hook_features_pipe_component_alter(&$pipe, $data, $export) {
}

/**
 * @defgroup features_component_alter_hooks Feature's component alter hooks
 * @{
 * Hooks to modify components defined by other features. These come in the form
 * hook_COMPONENT_alter where COMPONENT is the default_hook declared by any of
 * components within features.
 *
 * CTools also has a variety of hook_FOO_alters.
 *
 * Note: While views is a component of features, it declares it's own alter 
 * function which takes a similar form:
 * hook_views_default_views_alter(&$views)
 */

/**
 * Alter the default cck fields right before they are cached into the database.
 *
 * @param &$fields
 *   By reference. The fields that have been declared by another feature.
 */
function hook_content_default_fields_alter(&$fields) {
}

/**
 * Alter the default fieldgroup groups right before they are cached into the 
 * database.
 *
 * @param &$groups
 *   By reference. The fieldgroup groups that have been declared by another 
 *   feature.
 */
function hook_fieldgroup_default_groups_alter(&$groups) {
}

/**
 * Alter the default filter formats right before they are cached into the 
 * database.
 *
 * @param &$formats
 *   By reference. The formats that have been declared by another feature.
 */
function hook_filter_default_formats_alter(&$formats) {
}

/**
 * Alter the default menus right before they are cached into the database.
 *
 * @param &$menus
 *   By reference. The menus that have been declared by another feature.
 */
function hook_menu_default_menu_custom_alter(&$menus) {
}

/**
 * Alter the default menu links right before they are cached into the database.
 *
 * @param &$links
 *   By reference. The menu links that have been declared by another feature.
 */
function hook_menu_default_menu_links_alter(&$links) {
}

/**
 * Alter the default menu items right before they are cached into the database.
 *
 * @param &$items
 *   By reference. The menu items that have been declared by another feature.
 */
function hook_menu_default_items_alter(&$items) {
}

/**
 * Alter the default vocabularies right before they are cached into the
 * database.
 *
 * @param &$vocabularies
 *   By reference. The vocabularies that have been declared by another feature.
 */
function hook_taxonomy_default_vocabularies_alter(&$vocabularies) {
}

/**
 * Alter the default permissions right before they are cached into the
 * database.
 *
 * @param &$permissions
 *   By reference. The permissions that have been declared by another feature.
 */
function hook_user_default_permissions_alter(&$permissions) {
}

/**
 * Alter the default roles right before they are cached into the database.
 *
 * @param &$roles
 *   By reference. The roles that have been declared by another feature.
 */
function hook_user_default_roles_alter(&$roles) {
}

/**
 * @}
 */
