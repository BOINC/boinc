<?php
// $Id: node_attachments.inc,v 1.1.2.1 2010/01/29 20:18:25 merlinofchaos Exp $

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'single' => TRUE,
  'title' => t('Attached files'),
  'icon' => 'icon_node.png',
  'description' => t('A list of files attached to the node.'),
  'required context' => new ctools_context_required(t('Node'), 'node'),
  'category' => t('Node'),
);

function ctools_node_attachments_content_type_render($subtype, $conf, $panel_args, $context) {
  $node = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'attachments';

  $block->title = t('Attached files');
  if ($node) {
    if (!empty($node->files)) {
      $block->content = theme('upload_attachments', $node->files);
    }
    $block->delta = $node->nid;
  }
  else {
    $block->content = t('Attached files go here.');
    $block->delta = 'unknown';
  }

  return $block;
}

function ctools_node_attachments_content_type_admin_title($subtype, $conf, $context) {
  return t('"@s" attachments', array('@s' => $context->identifier));
}

function ctools_node_attachments_content_type_edit_form(&$form, &$form_state) {
  // provide a blank form so we have a place to have context setting.
}

