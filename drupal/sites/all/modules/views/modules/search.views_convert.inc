<?php

/**
 * @file
 * Field conversion for fields handled by this module.
 */

function search_views_convert($display, $type, &$view, $field, $id = NULL) {
  switch ($type) {
    case 'filter':
      switch ($field['tablename']) {
        case 'temp_search_results':
          switch ($field['field']) {
            case 'word':
              $view->set_item_option($display, 'filter', $id, 'table', 'search_index');
              $view->set_item_option($display, 'filter', $id, 'field', 'keys');
              break;
          }
          break;
      }
      break;
  }
}
