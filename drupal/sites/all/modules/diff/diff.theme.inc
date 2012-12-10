<?php

/**
 * @file
 * Themeable function callbacks for diff.module.
 */

/**
 * Theme function to display the revisions formular with means to select
 * two revisions.
 */
function theme_diff_node_revisions($form) {
  $output = '';

  // Overview table:
  $header = array(
    t('Revision'),
    array('data' => drupal_render($form['submit']), 'colspan' => 2),
    array('data' => t('Operations'), 'colspan' => 2)
  );
  if (isset($form['info']) && is_array($form['info'])) {
    foreach (element_children($form['info']) as $key) {
      $row = array();
      if (isset($form['operations'][$key][0])) {
        // Note: even if the commands for revert and delete are not permitted,
        // the array is not empty since we set a dummy in this case.
        $row[] = drupal_render($form['info'][$key]);
        $row[] = drupal_render($form['diff']['old'][$key]);
        $row[] = drupal_render($form['diff']['new'][$key]);
        $row[] = drupal_render($form['operations'][$key][0]);
        $row[] = drupal_render($form['operations'][$key][1]);
        $rows[] = $row;
      }
      else {
        // its the current revision (no commands to revert or delete)
        $row[] = array('data' => drupal_render($form['info'][$key]), 'class' => 'revision-current');
        $row[] = array('data' => drupal_render($form['diff']['old'][$key]), 'class' => 'revision-current');
        $row[] = array('data' => drupal_render($form['diff']['new'][$key]), 'class' => 'revision-current');
        $row[] = array('data' => theme('placeholder', t('current revision')), 'class' => 'revision-current', 'colspan' => '2');
        $rows[] = array(
          'data' => $row,
          'class' => 'error',
        );
      }
    }
  }
  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}

/**
 * Theme functions
 */

/**
 * Return a themed table. This is a modified version of theme_table, adding
 * colgroup tag and col tag options.
 *
 * @param $header
 *   An array containing the table headers. Each element of the array can be
 *   either a localized string or an associative array with the following keys:
 *   - "data": The localized title of the table column.
 *   - "field": The database field represented in the table column (required if
 *     user is to be able to sort on this column).
 *   - "sort": A default sort order for this column ("asc" or "desc").
 *   - Any HTML attributes, such as "colspan", to apply to the column header cell.
 * @param $rows
 *   An array of table rows. Every row is an array of cells, or an associative
 *   array with the following keys:
 *   - "data": an array of cells
 *   - Any HTML attributes, such as "class", to apply to the table row.
 *
 *   Each cell can be either a string or an associative array with the following keys:
 *   - "data": The string to display in the table cell.
 *   - "header": Indicates this cell is a header.
 *   - Any HTML attributes, such as "colspan", to apply to the table cell.
 *
 *   Here's an example for $rows:
 *   @verbatim
 *   $rows = array(
 *     // Simple row
 *     array(
 *       'Cell 1', 'Cell 2', 'Cell 3'
 *     ),
 *     // Row with attributes on the row and some of its cells.
 *     array(
 *       'data' => array('Cell 1', array('data' => 'Cell 2', 'colspan' => 2)), 'class' => 'funky'
 *     )
 *   );
 *   @endverbatim
 *
 * @param $attributes
 *   An array of HTML attributes to apply to the table tag.
 * @param $caption
 *   A localized string to use for the <caption> tag.
 * @param $cols
 *   An array of table colum groups. Every column group is an array of columns,
 *   or an associative array with the following keys:
 *   - "data": an array of cells
 *   - Any HTML attributes, such as "class", to apply to the table column group.
 *
 *   Each column can be either an empty array or associative array with the following keys:
 *   - Any HTML attributes, such as "class", to apply to the table column group.
 *
 *   Here's an example for $cols:
 *   @verbatim
 *   $cols = array(
 *     // Simple colgroup.
 *     array(),
 *     // Simple colgroup with attributes.
 *     array(
 *       'data'  => array(), 'colspan' => 2, 'style' => 'color: green;',
 *     ),
 *     // Simple colgroup with one col.
 *     array(
 *       array(),
 *     ),
 *     // Colgroup with attributes on the colgroup and some of its cols.
 *     array(
 *       'data'  => array(array('class' => 'diff-marker'), array('colspan' => 2)), 'class' => 'funky',
 *     ),
 *   );
 *   @endverbatim
 *
 *   The HTML will look as follows:
 *   @verbatim
 *   <table>
 *     <!-- Simple colgroup. -->
 *     <colgroup />
 *
 *     <!-- Simple colgroup with attributes. -->
 *     <colgroup colspan="2" style="color: green;" />
 *
 *     <!-- Simple colgroup with one col. -->
 *     <colgroup>
 *       <col />
 *     </colgroup>
 *
 *     <!-- Colgroup with attributes on the colgroup and some of its cols. -->
 *     <colgroup class="funky">
 *       <col class="diff-marker" />
 *       <col colspan="2" />
 *     </colgroup>
 *     ...
 *   </table>
 *   @endverbatim
 *
 * @return
 *   An HTML string representing the table.
 */
function theme_diff_table($header, $rows, $attributes = array(), $caption = NULL, $cols = array()) {
  $output = '<table'. drupal_attributes($attributes) .">\n";

  if (isset($caption)) {
    $output .= '<caption>'. $caption ."</caption>\n";
  }

  // Format the table columns:
  if (count($cols)) {
    foreach ($cols as $number => $col) {
      $attributes = array();

      // Check if we're dealing with a simple or complex column
      if (isset($col['data'])) {
        foreach ($col as $key => $value) {
          if ($key == 'data') {
            $cells = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $cells = $col;
      }

      // Build colgroup
      if (is_array($cells) && count($cells)) {
        $output .= ' <colgroup'. drupal_attributes($attributes) .'>';
        $i = 0;
        foreach ($cells as $cell) {
          $output .= ' <col'. drupal_attributes($cell) .' />';
        }
        $output .= " </colgroup>\n";
      }
      else {
        $output .= ' <colgroup'. drupal_attributes($attributes) ." />\n";
      }
    }
  }

  // Format the table header:
  if (count($header)) {
    $ts = tablesort_init($header);
    $output .= ' <thead><tr>';
    foreach ($header as $cell) {
      $cell = tablesort_header($cell, $header, $ts);
      $output .= _theme_table_cell($cell, TRUE);
    }
    $output .= " </tr></thead>\n";
  }

  // Format the table rows:
  $output .= "<tbody>\n";
  if (count($rows)) {
    $flip = array('even' => 'odd', 'odd' => 'even');
    $class = 'even';
    foreach ($rows as $number => $row) {
      $attributes = array();

      // Check if we're dealing with a simple or complex row
      if (isset($row['data'])) {
        foreach ($row as $key => $value) {
          if ($key == 'data') {
            $cells = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $cells = $row;
      }

      // Add odd/even class
      $class = $flip[$class];
      if (isset($attributes['class'])) {
        $attributes['class'] .= ' '. $class;
      }
      else {
        $attributes['class'] = $class;
      }

      // Build row
      $output .= ' <tr'. drupal_attributes($attributes) .'>';
      $i = 0;
      foreach ($cells as $cell) {
        $cell = tablesort_cell($cell, $header, $ts, $i++);
        $output .= _theme_table_cell($cell);
      }
      $output .= " </tr>\n";
    }
  }

  $output .= "</tbody></table>\n";
  return $output;
}

/**
 * Theme function for a header line in the diff.
 */
function theme_diff_header_line($lineno) {
  return '<strong>'. t('Line %lineno', array('%lineno' => $lineno)) .'</strong>';
}

/**
 * Theme function for a content line in the diff.
 */
function theme_diff_content_line($line) {
  return '<div>'. $line .'</div>';
}

/**
 * Theme function for an empty line in the diff.
 */
function theme_diff_empty_line($line) {
  return $line;
}

/**
 * Theme function for inline diff form.
 */
function theme_diff_inline_form($form) {
  drupal_add_css(drupal_get_path('module', 'diff') .'/diff.css');
  return drupal_render($form);
}

/**
 * Display inline diff metadata.
 */
function theme_diff_inline_metadata($node) {
  drupal_add_css(drupal_get_path('module', 'diff') .'/diff.css');
  $output = "<div class='diff-inline-metadata clear-block'>";
  $output .= "<div class='diff-inline-byline'>";
  $output .= t('Updated by !name on @date', array(
    '!name' => theme('username', $node),
    '@date' => format_date($node->revision_timestamp, 'small'),
  ));
  $output .= "</div>";
  $output .= "<div class='diff-inline-legend clear-block'>";
  $output .= "<label>". t('Legend') ."</label>";
  $output .= theme('diff_inline_chunk', t('Added'), 'add');
  $output .= theme('diff_inline_chunk', t('Changed'), 'change');
  $output .= theme('diff_inline_chunk', t('Deleted'), 'delete');
  $output .= "</div>";
  $output .= "</div>";
  return $output;
}

/**
 * Theme a span of changed text in an inline diff display.
 */
function theme_diff_inline_chunk($text, $type = NULL) {
  switch ($type) {
    case 'add':
      return "<span class='diff-added'>{$text}</span>";
    case 'change':
      return "<span class='diff-changed'>{$text}</span>";
    case 'delete':
      return "<span class='diff-deleted'>{$text}</span>";
    default:
      return $text;
  }
}
