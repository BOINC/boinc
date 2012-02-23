<?php
// $Id: views_handler_filter_node_comment.inc,v 1.1 2008/09/03 19:21:29 merlinofchaos Exp $

/**
 * Filter based on comment node status
 */
class views_handler_filter_node_comment extends views_handler_filter_in_operator {
  function get_value_options() {
    $this->value_options = array(
      COMMENT_NODE_DISABLED => t('Disabled'),
      COMMENT_NODE_READ_ONLY => t('Read only'),
      COMMENT_NODE_READ_WRITE => t('Read/Write'),
    );
  }
}
