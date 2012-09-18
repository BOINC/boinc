<?php

/**
 * @file
 * Plugin to handle the 'node' content type which allows individual nodes
 * to be embedded into a panel.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t('Node'),
  'single' => TRUE,
  'defaults' => array(
    'nid' => '',
    'links' => TRUE,
    'leave_node_title' => FALSE,
    'identifier' => '',
    'build_mode' => 'teaser',
  ),
  'title' => t('Existing node'),
  'icon' => 'icon_node.png',
  'description' => t('Add a node from your site as content.'),
  'category' => t('Custom'),
  'top level' => TRUE,
  'js' => array('misc/autocomplete.js'),
);

/**
 * Output function for the 'node' content type.
 *
 * Outputs a node based on the module and delta supplied in the configuration.
 */
function ctools_node_content_type_render($subtype, $conf, $panel_args) {
  $nid = $conf['nid'];
  $block = new stdClass();

  foreach (explode('/', $_GET['q']) as $id => $arg) {
    $nid = str_replace("%$id", $arg, $nid);
  }

  foreach ($panel_args as $id => $arg) {
    if (is_string($arg)) {
      $nid = str_replace("@$id", $arg, $nid);
    }
  }

  // Support node translation
  if (module_exists('translation')) {
    if ($translations = module_invoke('translation', 'node_get_translations', $nid)) {
  	  if ($translations[$GLOBALS['language']->language])  {
        $nid = $translations[$GLOBALS['language']->language]->nid;
      }
    }
  }

  if (!is_numeric($nid)) {
    return;
  }

  $node = node_load($nid);
  if (!node_access('view', $node)) {
    return;
  }

  if (node_access('update', $node)) {
    $block->admin_links['update'] = array(
      'title' => t('Edit node'),
      'alt' => t("Edit this node"),
      'href' => "node/$node->nid/edit",
      'query' => drupal_get_destination(),
    );
  }

  $block->module = 'node';
  $block->delta = $node->nid;

  // Set block->title to the plain node title, then additionally set block->title_link to
  // the node url if required. The actual link is rendered in ctools_content_render().
  $block->title = check_plain($node->title);
  if (!empty($conf['link_node_title'])) {
    $block->title_link = 'node/' . $node->nid;
  }

  if (empty($conf['leave_node_title'])) {
    $node->title = NULL;
  }

  if (!empty($conf['identifier'])) {
    $node->panel_identifier = $conf['identifier'];
  }

  // Handle existing configurations with the deprecated 'teaser' option.
  if (isset($conf['teaser'])) {
    $conf['build_mode'] = $conf['teaser'] ? 'teaser' : 'full';
  }

  if (!isset($node->build_mode)) {
    $node->build_mode = ($conf['build_mode'] == 'teaser' || $conf['build_mode'] == 'full') ? NODE_BUILD_NORMAL : $conf['build_mode'];
  }

  $block->content = node_view($node, $conf['build_mode'] == 'teaser', FALSE, $conf['links']);
  return $block;
}

/**
 * The form to add or edit a node as content.
 */
function ctools_node_content_type_edit_form(&$form, &$form_state) {
  $conf = $form_state['conf'];

  $form['leave_node_title'] = array(
    '#type' => 'checkbox',
    '#default_value' => !empty($conf['leave_node_title']),
    '#title' => t('Leave node title'),
    '#description' => t('Advanced: if checked, do not touch the node title; this can cause the node title to appear twice unless your theme is aware of this.'),
  );

  $form['link_node_title'] = array(
    '#type' => 'checkbox',
    '#default_value' => !empty($conf['link_node_title']),
    '#title' => t('Link the node title to the node'),
    '#description' => t('Check this box if you would like your pane title to link to the node.'),
  );


  if ($form_state['op'] == 'add') {
    $form['nid'] = array(
      '#prefix' => '<div class="no-float">',
      '#title' => t('Enter the title or NID of a node'),
      '#description' => t('To use a NID from the URL, you may use %0, %1, ..., %N to get URL arguments. Or use @0, @1, @2, ..., @N to use arguments passed into the panel.'),
      '#type' => 'textfield',
      '#maxlength' => 512,
      '#autocomplete_path' => 'ctools/autocomplete/node',
      '#weight' => -10,
      '#suffix' => '</div>',
    );
  }
  else {
    $form['nid'] = array(
      '#type' => 'value',
      '#value' => $conf['nid'],
    );
  }

  $form['links'] = array(
    '#type' => 'checkbox',
    '#default_value' => $conf['links'],
    '#title' => t('Include node links for "add comment", "read more" etc.'),
  );

  $form['identifier'] = array(
    '#type' => 'textfield',
    '#default_value' => !empty($conf['identifier']) ? $conf['identifier'] : '',
    '#title' => t('Template identifier'),
    '#description' => t('This identifier will be added as a template suggestion to display this node: node-panel-IDENTIFIER.tpl.php. Please see the Drupal theming guide for information about template suggestions.'),
  );

  // CCK holds the registry of available build modes, but can hardly
  // push them as options for the build mode options, so we break the normal
  // rule of not directly relying on non-core modules.
  if ($modes = module_invoke('content', 'build_modes')) {
    $build_mode_options = array();
    foreach ($modes as $key => $value) {
      if (isset($value['views style']) && $value['views style']) {
        $build_mode_options[$key] = $value['title'];
      }
    }
  }
  else {
    $build_mode_options = array(
      'teaser' => t('Teaser'),
      'full' => t('Full node')
    );
  }

  // Handle existing configurations with the deprecated 'teaser' option.
  // Also remove the teaser key from the form_state.
  if (isset($conf['teaser']) || !isset($conf['build_mode'])) {
    unset($form_state['conf']['teaser']);
    $conf['build_mode'] = $conf['teaser'] ? 'teaser' : 'full';
  }
  $form['build_mode'] = array(
    '#title' => t('Build mode'),
    '#type' => 'select',
    '#description' => t('Select a build mode for this node.'),
    '#options' => $build_mode_options,
    '#default_value' => $conf['build_mode'],
  );

}

/**
 * Validate the node selection.
 */
function  ctools_node_content_type_edit_form_validate(&$form, &$form_state) {
  if ($form_state['op'] != 'add') {
    return;
  }

  $nid = $form_state['values']['nid'];
  $preg_matches = array();
  $match = preg_match('/\[nid: (\d+)\]/', $nid, $preg_matches);
  if (!$match) {
    $match = preg_match('/^nid: (\d+)/', $nid, $preg_matches);
  }

  if ($match) {
    $nid = $preg_matches[1];
  }
  if (is_numeric($nid)) {
    $node = db_fetch_object(db_query(db_rewrite_sql("SELECT n.nid, n.status FROM {node} n WHERE n.nid = %d"), $nid));
  }
  else {
    $node = db_fetch_object(db_query(db_rewrite_sql("SELECT n.nid, n.status FROM {node} n WHERE LOWER(n.title) = LOWER('%s')"), $nid));
  }
  if ($node) {
    $form_state['values']['nid'] = $node->nid;
  }

  if (!($node || preg_match('/^[@%]\d+$/', $nid)) ||
      // Do not allow unpublished nodes to be selected by unprivileged users
      (empty($node->status) && !user_access('administer nodes'))) {
    form_error($form['nid'], t('Invalid node'));
  }
}

/**
 * Validate the node selection.
 */
function ctools_node_content_type_edit_form_submit(&$form, &$form_state) {
  foreach (array('nid', 'links', 'leave_node_title', 'link_node_title', 'identifier', 'build_mode') as $key) {
    $form_state['conf'][$key] = $form_state['values'][$key];
  }
}

/**
 * Returns the administrative title for a node.
 */
function ctools_node_content_type_admin_title($subtype, $conf) {
  if (!is_numeric($conf['nid'])) {
    return t('Node loaded from @var', array('@var' => $conf['nid']));
  }

  $node = node_load($conf['nid']);
  if ($node) {
    if (!empty($data->status) || user_access('administer nodes')) {
      return check_plain($node->title);
    }
    else {
      return t('Unpublished node @nid', array('@nid' => $conf['nid']));
    }
  }
  else {
    return t('Deleted/missing node @nid', array('@nid' => $conf['nid']));
  }
}

/**
 * Display the administrative information for a node pane.
 */
function ctools_node_content_type_admin_info($subtype, $conf) {
  // Just render the node.
  return ctools_node_content_type_render($subtype, $conf, array());
}
