<?php

if (module_exists('comment')) {
  /**
   * Plugins are described by creating a $plugin array which will be used
   * by the system that includes this file.
   */
  $plugin = array(
    'single' => TRUE,
    'title' => t('Comment form'),
    'icon' => 'icon_node.png',
    'description' => t('A form to add a new comment.'),
    'required context' => new ctools_context_required(t('Node'), 'node'),
    'category' => t('Node'),
    'defaults' => array('anon_links' => false),
  );
}

function ctools_node_comment_form_content_type_render($subtype, $conf, $panel_args, $context) {
  $node = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();
  $block->module = 'comments';
  $block->delta  = $node->nid;

  $block->title = t('Add comment');

  if (empty($node)) {
    $block->content = t('Comment form here.');
  }
  else {
    if (user_access('post comments') && node_comment_mode($node->nid) == COMMENT_NODE_READ_WRITE) {
      ctools_include('form');
      $form_state = array(
        'ctools comment alter' => TRUE,
        'node' => $node,
        'args' => array(array('nid' => $node->nid))
      );
      $block->content = ctools_build_form('comment_form', $form_state);
    }
    else if (!empty($conf['anon_links'])) {
      $block->content = theme('comment_post_forbidden', $node);
    }
  }

  return $block;
}

function ctools_node_comment_form_content_type_admin_title($subtype, $conf, $context) {
  return t('"@s" comment form', array('@s' => $context->identifier));
}

function ctools_node_comment_form_content_type_edit_form(&$form, &$form_state) {
  $form['anon_links'] = array(
    '#type'  => 'checkbox',
    '#title' => t('Shows links to register or login.'),
    '#description' => t('If anonymous comments are not allowed, this will display the register and login links.'),
    '#default_value' => $form_state['conf']['anon_links'],
  );
}

function ctools_node_comment_form_content_type_edit_form_submit(&$form, &$form_state) {
  // For each part of the form defined in the 'defaults' array set when you
  // defined the content type, copy the value from the form into the array
  // of items to be saved. We don't ever want to use
  // $form_state['conf'] = $form_state['values'] because values contains
  // buttons, form id and other items we don't want stored. CTools will handle
  // the actual form submission.
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Alter the comment form to get a little more control over it.
 */
function ctools_form_comment_form_alter(&$form, &$form_state) {
  if (!empty($form_state['ctools comment alter'])) {
    // Force the form to post back to wherever we are.
    $form['#action'] = url($_GET['q'], array('fragment' => 'comment-form'));
    if (empty($form['#submit'])) {
      $form['#submit'] = array('comment_form_submit');
    }
    $form['#submit'][] = 'ctools_node_comment_form_submit';
  }
}

function ctools_node_comment_form_submit(&$form, &$form_state) {
  $form_state['redirect'][0] = $_GET['q'];
}
