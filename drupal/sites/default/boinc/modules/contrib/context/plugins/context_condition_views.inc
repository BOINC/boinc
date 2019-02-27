<?php

class context_condition_views extends context_condition {

  /**
   * Generate a list of database and module provided views.
   */
  function condition_values() {
    $enabled_views = array();

    $views = views_get_all_views();
    ksort($views);

    foreach ($views as $view) {
      if (!isset($views[$view->name]->disabled) || !$views[$view->name]->disabled) {
        $enabled_views[$view->name] = check_plain($view->name);

        // Provide more granular options for each page display
        $displays = array();
        foreach ($view->display as $id => $display) {
          if ($display->display_plugin == 'page') {
            $displays[$view->name .":". $id] = check_plain("-- {$display->display_title}");
          }
        }
        $enabled_views += $displays;
      }
    }
    return $enabled_views;
  }

  function execute($view) {
    switch ($view->display_handler->display->display_plugin) {
      case 'page':
      case 'calendar':
        // Set contexts for this view.
        foreach ($this->get_contexts($view->name) as $context) {
          $this->condition_met($context, $view->name);
        }
        // Set any contexts associated with the current display
        if (!empty($view->current_display)) {
          foreach ($this->get_contexts("{$view->name}:{$view->current_display}") as $context) {
            $this->condition_met($context, "{$view->name}:{$view->current_display}");
          }
        }
        break;
    }
  }
}
