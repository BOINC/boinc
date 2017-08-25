<?php

/**
 * @file
 * Contains the flag Ops field handler.
 */

/**
 * Views field handler for the Flag operations links (flag/unflag).
 *
 * @ingroup views
 */
class flag_handler_field_ops extends views_handler_field {

  /**
   * Returns the flag object associated with our field.
   *
   * A field is in some relationship. This function reaches out for this
   * relationship and reads its 'flag' option, which holds the flag name.
   */
  function get_flag() {
    // When editing a view it's possible to delete the relationship (either by
    // error or to later recreate it), so we have to guard against a missing
    // one.
    if (isset($this->view->relationship[$this->options['relationship']])) {
      return $this->view->relationship[$this->options['relationship']]->get_flag();
    }
  }

  /**
   * Return the the relationship we're linked to. That is, the alias for its
   * table (which is suitbale for use with the various methods of the 'query'
   * object).
   */
  function get_parent_relationship() {
    $parent = $this->view->relationship[$this->options['relationship']]->options['relationship'];
    if (!$parent || $parent == 'none') {
      return NULL; // Base query table.
    }
    else {
      return $this->view->relationship[$parent]->alias;
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_type'] = array('default' => '');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['link_type'] = array(
      '#type' => 'radios',
      '#title' => t('Link type'),
      '#options' => array('' => t('Use flag link settings')) + _flag_link_type_options(),
      '#default_value' => $this->options['link_type'],
    );
  }

  /**
   * Override base ::query(). The purpose here is to make it possible for the
   * render() method to know two things: what's the content ID, and whether
   * it's flagged.
   */
  function query() {
    if (!($flag = $this->get_flag())) {
      return; // Error message is printed in render().
    }
    $info = $flag->get_views_info();
    $parent = $this->get_parent_relationship();

    // Find out if the content is flagged. We can't just peek at some field in
    // our loaded table because it doesn't always reflect the user browsing the
    // page. So we explicitly add the flag_content table to find this out.
    $join = new views_join();
    $join->construct('flag_content', $info['views table'], $info['join field'], 'content_id');
    $join->extra[] = array(
      'field' => 'fid',
      'value' => $flag->fid,
      'numeric' => TRUE,
    );
    if (!$flag->global) {
      $join->extra[] = array(
        'field' => 'uid',
        'value' => '***CURRENT_USER***',
        'numeric' => TRUE,
      );
      $join->extra[] = array(
        'field' => 'sid',
        'value' => '***FLAG_CURRENT_USER_SID***',
        'numeric' => TRUE,
      );
    }
    $flag_table = $this->query->add_table('flag_content', $parent, $join);
    $this->aliases['is_flagged'] = $this->query->add_field($flag_table, 'content_id');

    // Next, find out the content ID. We can't add_field() on this table
    // (flag_content), because its content_id may be NULL (in case no user has
    // flagged this content, and it's a LEFT JOIN). So we reach to the parent
    // relationship and add_field() *its* content ID column.
    $left_table = $this->view->relationship[$this->options['relationship']]->table_alias;
    $this->aliases['content_id'] = $this->query->add_field($left_table, $info['join field']);
  }

  /**
   * Find out if the flag applies to each item seen on the page. It's done in a
   * separate DB query to to avoid complexity and to make 'many to one' tests
   * (e.g. checking user roles) possible without causing duplicate rows.
   */
  function pre_render(&$values) {
    if (!($flag = $this->get_flag())) {
      return; // Error message is printed in render().
    }

    $ids = array();
    foreach ($values as $row) {
      $content_id = $row->{$this->aliases['content_id']};
      $is_flagged = $row->{$this->aliases['is_flagged']};
      if (isset($content_id)) {
        $ids[$content_id] = $is_flagged ? 'unflag' : 'flag';
      }
    }
    $this->flag_applies = $ids ? $flag->access_multiple($ids) : array();
  }

  function render($values) {
    if (!($flag = $this->get_flag())) {
      return t('Missing flag'); // get_flag() itself will print a more detailed message.
    }

    $content_id = $values->{$this->aliases['content_id']};
    $is_flagged = $values->{$this->aliases['is_flagged']};

    if (empty($this->flag_applies[$content_id])) {
      // Flag does not apply to this content.
      return;
    }

    if (!empty($this->options['link_type'])) {
      $flag->link_type = $this->options['link_type'];
    }
    return $flag->theme($is_flagged ? 'unflag' : 'flag', $content_id);
  }
}
