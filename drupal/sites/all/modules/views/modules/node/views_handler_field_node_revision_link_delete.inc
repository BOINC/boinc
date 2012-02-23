<?php
// $Id: views_handler_field_node_revision_link_delete.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * Field handler to present delete a node revision.
 */
class views_handler_field_node_revision_link_delete extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = array('table' => 'node', 'field' => 'uid');
    $this->additional_fields['node_vid'] = array('table' => 'node', 'field' => 'vid');
    $this->additional_fields['vid'] = 'vid';
    $this->additional_fields['format'] = 'format';
  }

  function access() {
    return user_access('delete revisions') || user_access('administer nodes');
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->vid = $values->{$this->aliases['vid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->format = $values->{$this->aliases['format']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('delete', $node)) {
      return;
    }

    // Current revision cannot be deleted.
    if ($node->vid == $values->{$this->aliases['node_vid']}) {
      return;
    }
    $text = !empty($this->options['text']) ? $this->options['text'] : t('delete');
    return l($text, "node/$node->nid/revisions/$node->vid/delete", array('query' => drupal_get_destination()));
  }
}
