<?php
// $Id: views_plugin_row_comment_view.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $
/**
 * @file
 * Contains the node RSS row style plugin.
 */

/**
 * Plugin which performs a comment_view on the resulting object.
 */
class views_plugin_row_comment_view extends views_plugin_row {
  function option_definition() {
    $options = parent::option_definition();
    $options['links'] = array('default' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display links'),
      '#default_value' => $this->options['links'],
    );
  }

  function pre_render($result) {
    $cids = array();
    $this->comments = array();

    foreach ($result as $row) {
      $cids[] = $row->cid;
    }

    if (count($cids) > 1) {
      $placeholder = " IN (" . implode(', ', array_fill(0, sizeof($cids), '%d')) . ")";
    }
    else {
      $placeholder = " = %d";
    }

    $cresult = db_query("SELECT c.cid as cid, c.pid, c.nid, c.subject, c.comment, c.format, c.timestamp, c.name, c.mail, c.homepage, u.uid, u.name AS registered_name, u.signature, u.picture, u.data, c.thread, c.status FROM {comments} c INNER JOIN {users} u ON c.uid = u.uid WHERE c.cid $placeholder", $cids);
    while ($comment = db_fetch_object($cresult)) {
      $comment = drupal_unpack($comment);
      $comment->name = $comment->uid ? $comment->registered_name : $comment->name;
      $comment->depth = count(explode('.', $comment->thread)) - 1;
      $this->comments[$comment->cid] = $comment;
    }
  }
}

