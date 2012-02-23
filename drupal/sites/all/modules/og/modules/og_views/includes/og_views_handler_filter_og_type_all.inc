<?php
class og_views_handler_filter_og_type_all extends views_handler_filter {
  function query() {
    if ($group_types = og_get_types('group')) {
      $placeholders = db_placeholders($group_types, 'varchar');
      $table = $this->query->ensure_table('node');
      $this->query->add_where($this->options['group'], "$table.type IN ($placeholders)", $group_types);
    }
    else {
      $this->query->add_where($this->options['group'], "FALSE");
      drupal_set_message(t('You have no node types which are acting as groups. See the notes section of the !readme_file and the content types fieldset at top of <a href="!settings">OG settings</a>.', array('!readme_file' => og_readme(), '!settings' => url('admin/og/og'))), 'error');
    }
  }
}