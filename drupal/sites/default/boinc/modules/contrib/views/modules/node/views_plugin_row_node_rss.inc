<?php
/**
 * @file
 * Contains the node RSS row style plugin.
 */

/**
 * Plugin which performs a node_view on the resulting object
 * and formats it as an RSS item.
 */
class views_plugin_row_node_rss extends views_plugin_row {
  // Basic properties that let the row style follow relationships.
  var $base_table = 'node';
  var $base_field = 'nid';

  function option_definition() {
    $options = parent::option_definition();

    $options['item_length'] = array('default' => 'default');

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['item_length'] = array(
      '#type' => 'select',
      '#title' => t('Display type'),
      '#options' => array(
        'fulltext' => t('Full text'),
        'teaser' => t('Title plus teaser'),
        'title' => t('Title only'),
        'default' => t('Use default RSS settings'),
      ),
      '#default_value' => $this->options['item_length'],
    );
  }

  function render($row) {
    // For the most part, this code is taken from node_feed() in node.module
    global $base_url;

    $nid = $row->{$this->field_alias};
    if (!is_numeric($nid)) {
      return;
    }

    $item_length = $this->options['item_length'];
    if ($item_length == 'default') {
      $item_length = variable_get('feed_item_length', 'teaser');
    }

    // Load the specified node:
    $node = node_load($nid);
    if (empty($node)) {
      return;
    }

    $node->build_mode = NODE_BUILD_RSS;

    if ($item_length != 'title') {
      $teaser = ($item_length == 'teaser') ? TRUE : FALSE;

      // Filter and prepare node teaser
      if (node_hook($node, 'view')) {
        $node = node_invoke($node, 'view', $teaser, FALSE);
      }
      else {
        $node = node_prepare($node, $teaser);
      }

      // Allow modules to change $node->teaser before viewing.
      node_invoke_nodeapi($node, 'view', $teaser, FALSE);
    }

    // Set the proper node part, then unset unused $node part so that a bad
    // theme can not open a security hole.
    $content = drupal_render($node->content);
    if ($teaser) {
      $node->teaser = $content;
      unset($node->body);
    }
    else {
      $node->body = $content;
      unset($node->teaser);
    }

    // Allow modules to modify the fully-built node.
    node_invoke_nodeapi($node, 'alter', $teaser, FALSE);

    $item = new stdClass();
    $item->title = $node->title;
    $item->link = url("node/$row->nid", array('absolute' => TRUE));
    $item->nid = $node->nid;
    $item->readmore = $node->readmore;

    // Allow modules to add additional item fields and/or modify $item
    $extra = node_invoke_nodeapi($node, 'rss item');
    $item->elements = array_merge($extra,
      array(
        array('key' => 'pubDate', 'value' => gmdate('r', $node->created)),
        array(
          'key' => 'dc:creator',
          'value' => $node->name,
          'namespace' => array('xmlns:dc' => 'http://purl.org/dc/elements/1.1/'),
        ),
        array(
          'key' => 'guid',
          'value' => $node->nid . ' at ' . $base_url,
          'attributes' => array('isPermaLink' => 'false')
        ),
      )
    );

    foreach ($item->elements as $element) {
      if (isset($element['namespace'])) {
        $this->view->style_plugin->namespaces = array_merge($this->view->style_plugin->namespaces, $element['namespace']);
      }
    }

    // Prepare the item description
    switch ($item_length) {
      case 'fulltext':
        $item->description = $node->body;
        break;
      case 'teaser':
        $item->description = $node->teaser;
        if (!empty($item->readmore)) {
          $item->description .= '<p>' . l(t('read more'), 'node/' . $item->nid, array('absolute' => TRUE, 'attributes' => array('target' => '_blank'))) . '</p>';
        }
        break;
      case 'title':
        $item->description = '';
        break;
    }

    return theme($this->theme_functions(), $this->view, $this->options, $item);
  }
}

