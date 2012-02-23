<?php

/**
 * Base class for most {og_uid} field handlers.
 */
class og_views_handler_field_og_uid extends views_handler_field {
  function query() {
    if ($this->view->base_table !== 'users' || ($group = og_get_group_context())) {
      parent::query();
    }
  }
}
