<?php
// $Id: content_plugin_style_php_array_ac.inc,v 1.1.2.3 2009/11/04 15:31:28 markuspetrux Exp $

/**
 * @file
 * Handler for 'content_php_array_autocomplete' style.
 */
class content_plugin_style_php_array_ac extends views_plugin_style {
  function render() {
    $results = array();

    // Group the rows according to the grouping field, if specified.
    $sets = $this->render_grouping($this->view->result, $this->options['grouping']);

    $base_field = $this->view->base_field;
    $title_field = $this->display->display_options['content_title_field'];
    $title_field_alias = $this->view->field[$title_field]->field_alias;

    // TODO : We don't display grouping info for now.
    // Could be useful for select widget, though.
    $this->view->row_index = 0;
    foreach ($sets as $title => $records) {
      foreach ($records as $label => $row) {
        $results[$row->{$base_field}] = array(
          'title' => $row->{$title_field_alias},
          'rendered' => $this->row_plugin->render($row),
        );
        $this->view->row_index++;
      }
    }
    unset($this->view->row_index);
    return $results;
  }
}
