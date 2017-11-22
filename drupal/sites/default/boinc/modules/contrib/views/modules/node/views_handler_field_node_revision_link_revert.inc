<?php
/**
 * Field handler to present a link to revert a node to a revision
 */
class views_handler_field_node_revision_link_revert extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = array('table' => 'node', 'field' => 'uid');
    $this->additional_fields['node_vid'] = array('table' => 'node', 'field' => 'vid');
    $this->additional_fields['vid'] = 'vid';
    $this->additional_fields['format'] = 'format';
  }

  function access() {
    return user_access('revert revisions') || user_access('administer nodes');
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->vid = $values->{$this->aliases['vid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->format = $values->{$this->aliases['format']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('update', $node)) {
      return;
    }

    // Current revision cannot be reverted.
    if ($node->vid == $values->{$this->aliases['node_vid']}) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('revert');
    return l($text, "node/$node->nid/revisions/$node->vid/revert", array('query' => drupal_get_destination()));
  }
}
