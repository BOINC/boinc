<?php
// $Id$

/**
 * @file
 * This file provides a CTools content type for fields.
 */

/**
 * Callback function to supply a list of content types.
 */
function content_content_field_ctools_content_types() {
  return array(
    'title' => t('Content field'),
    'defaults' => array('label' => '', 'formatter' => ''),
    'content type' => 'content_content_field_content_type_content_type',
  );
}

/**
 * Return all field content types available.
 */
function content_content_field_content_type_content_types() {
  // This will hold all the individual field content types.
  $types = array();

  // Get all fields on the site.
  $field_types = _content_field_types();

  foreach (content_types() as $type_name => $type) {
    foreach ($type['fields'] as $field_name => $field) {
      if (!isset($types[$field_name])) {
        $types[$field_name] = array(
          'category' => t('Node'),
          'icon' => 'icon_cck_field.png',
          'title' => t('Field: @widget_label (@field_name) - @field_type', array(
            '@widget_label' => t($field['widget']['label']),
            '@field_name' => $field_name,
            '@field_type' => t($field_types[$field['type']]['label']),
          )),
          'description' => t('Field on the referenced node.'),
          'types' => array(),
        );
        if (isset($field_types[$field['type']]['content_icon'])) {
          $types[$field_name]['icon'] = $field_types[$field['type']]['content_icon'];
        }
      }
      $types[$field_name]['types'][$type_name] = $type['name'];
    }
  }

  // Create the required context for each field related to the content types.
  foreach ($types as $field_name => $field_content_type) {
    $types[$field_name]['required context'] = new ctools_context_required(t('Node'), 'node', array(
      'type' => array_keys($types[$field_name]['types']),
    ));
    unset($types[$field_name]['types']);
  }

  return $types;
}

/**
 * Just one subtype.
 *
 * Ordinarily this function is meant to get just one subtype. However, we are
 * using it to deal with the fact that we have changed the subtype names. This
 * lets us translate the name properly.
 */
function content_content_field_content_type_content_type($subtype) {
  // Previous versions of CCK included the content type as part of the subtype.
  // This allows those to continue to sort of work, at least during render.
  if (strpos($subtype, ':') !== FALSE) {
    list($content_type, $subtype) = explode(':', $subtype, 2);
  }

  $types = content_content_field_content_type_content_types();
  if (isset($types[$subtype])) {
    return $types[$subtype];
  }
}

/**
 * Output function for the 'field' content type.
 */
function content_content_field_content_type_render($subtype, $conf, $panel_args, $context) {
  // Previous versions of CCK included the content type as part of the subtype.
  // This allows those to continue to sort of work, at least during render.
  if (strpos($subtype, ':') !== FALSE) {
    list($content_type, $subtype) = explode(':', $subtype, 2);
  }

  if (is_array($context)) {
    $context = array_pop($context);
  }
  // If we do not have a node, then we cannot generate output.
  if (!isset($context->data)) {
    return;
  }
  $node = drupal_clone($context->data);

  // Extract the node type from the node in context, the field name from the
  // panels content type subtype, and get the content field structure.
  $field_name = $subtype;
  $field = content_fields($field_name, $node->type);

  // Get the formatter that was selected in the settings dialog.
  $formatter = $conf['formatter'];

  // Check view access to the field.
  if (!content_access('view', $field, NULL, $node)) {
    return;
  }

  // Force panel settings into the field's display settings.
  $field['display_settings']['label']['format'] = $conf['label'] == 'normal' || !empty($conf['override_title']) ? 'hidden' : $conf['label'];
  $field['display_settings']['full']['format'] = $formatter;
  $node->build_mode = NODE_BUILD_NORMAL;
  // TODO : allow panel-specific template suggestions for content-field.tpl.php ?

  $output = content_view_field($field, $node);

  $block = new stdClass();
  $block->module = 'content';
  $block->delta = $field_name;
  if ($conf['label'] == 'normal') {
    $block->title = t($field['widget']['label']);
  }
  $block->content = $output;

  return $block;
}

/**
 * Returns a settings form for the custom type.
 */
function content_content_field_content_type_edit_form(&$form, &$form_state) {
  $conf = $form_state['conf'];

  $form['label'] = array(
    '#type' => 'select',
    '#title' => t('Field label'),
    '#default_value' => isset($conf['label']) ? $conf['label'] : '',
    '#options' => array(
      'normal' => t('Block title'),
      'above' => t('Above'),
      'inline' => t('Inline'),
    ),
    '#description' => t('Configure how the label is going to be displayed. This option takes no effect when "Override title" option is enabled, the specified block title is displayed instead.'),
  );

  // Extract the field name from the panels content type subtype.
  $field_name = $form_state['subtype_name'];

  // Previous versions of CCK included the content type as part of the subtype.
  // This allows those to continue to sort of work.
  if (strpos($field_name, ':') !== FALSE) {
    list($content_type, $field_name) = explode(':', $field_name, 2);
  }

  // Get all the information about our field.
  $field = content_fields($field_name);

  // Get information about all the field types on the site.
  $field_types = _content_field_types();

  // Get the information about the type that our field is.
  $type_info = $field_types[$field['type']];

  // Put the possible formatters for our type into an array.
  $options = array();
  foreach ($type_info['formatters'] as $formatter_name => $formatter) {
    $options[$formatter_name] = $formatter['label'];
  }

  $form['formatter'] = array(
    '#type' => 'select',
    '#title' => t('Field formatter'),
    '#default_value' => isset($conf['formatter']) ? $conf['formatter'] : 'default',
    '#options' => $options,
    '#description' => t('Select a formatter.'),
    '#required' => TRUE,
  );
}

function content_content_field_content_type_edit_form_submit(&$form, &$form_state) {
  // Copy everything from our defaults.
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Admin title for field content type.
 */
function content_content_field_content_type_admin_title($subtype, $conf, $context) {
  // Previous versions of CCK included the content type as part of the subtype.
  // This allows those to continue to sort of work, at least during render.
  if (strpos($subtype, ':') !== FALSE) {
    list($content_type, $subtype) = explode(':', $subtype, 2);
  }

  // Get all fields on the site.
  $field_types = _content_field_types();

  // Get all the information about our field.
  $field = content_fields($subtype);

  return t('"@s" field: @widget_label (@field_name) - @field_type', array(
    '@s' => $context->identifier,
    '@widget_label' => t($field['widget']['label']),
    '@field_name' => $subtype,
    '@field_type' => t($field_types[$field['type']]['label']),
  ));
}
