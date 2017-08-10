<?php

/**
 * @file
 * Definition of the 'rounded_corners' panel style.
 */

// Plugin definition
$plugin = array(
  'title' => t('Rounded corners'),
  'description' => t('Presents the panes or panel regions with a rounded corner box around them'),
  'render region' => 'panels_rounded_corners_style_render_region',
  'render pane' => 'panels_rounded_corners_style_render_pane',
  'settings form' => 'panels_rounded_corners_style_settings_form',
  'hook theme' => array(
    'panels_rounded_corners_box' => array(
      'arguments' => array('content' => NULL),
      'path' => panels_get_path('plugins/styles/corners'),
      'template' => 'panels-rounded-corners-box',
    ),
  ),
);

/**
 * Render callback.
 *
 * @ingroup themeable
 */
function theme_panels_rounded_corners_style_render_region($display, $region_id, $panes, $settings) {
  $output = '';

  // Determine where to put the box. If empty or 'pane' around each pane. If
  // 'panel' then just around the whole panel.
  $where = empty($settings['corner_location']) ? 'pane' : $settings['corner_location'];

  $print_separator = FALSE;
  foreach ($panes as $pane_id => $pane_output) {
    if ($pane_output) {
      // Add the separator if we've already displayed a pane.
      if ($print_separator) {
        $output .= '<div class="panel-region-separator">&nbsp;</div>';
      }

      if ($where == 'pane') {
        $output .= theme('panels_rounded_corners_box', $pane_output);
      }
      else {
        $output .= $pane_output;
        $print_separator = TRUE;
      }
    }
  }

  if ($where == 'panel') {
    $output = theme('panels_rounded_corners_box', $output);
  }

  panels_add_rounded_corners_css($display, $where);

  return $output;
}

function panels_add_rounded_corners_css($display, $where) {
  static $displays_used = array();
  if (empty($displays_used[$display->css_id])) {
    panels_rounded_corners_css($display, $where);
    $displays_used[$display->css_id] = TRUE;
  }
}

/**
 * Render callback for a single pane.
 */
function theme_panels_rounded_corners_style_render_pane($content, $pane, $display) {
  if (empty($content->content)) {
    return;
  }

  $output = theme('panels_pane', $content, $pane, $display);

  // Just stick a box around the standard theme_panels_pane.
  $output = theme('panels_rounded_corners_box', $output);
  panels_add_rounded_corners_css($display, 'pane');
  return $output;
}

/**
 * Settings form callback.
 */
function panels_rounded_corners_style_settings_form($style_settings) {
  $form['corner_location'] = array(
    '#type' => 'select',
    '#title' => t('Box around'),
    '#options' => array(
      'pane' => t('Each pane'),
      'panel' => t('Each region'),
    ),
    '#default_value' => (isset($style_settings['corner_location'])) ? $style_settings['corner_location'] : 'ul',
    '#description' => t('Choose whether to include the box around each pane (piece of content) or region (each column or region)'),
  );

  return $form;
}

/**
 * Generates the dynamic CSS.
 *
 * @param $display
 *   A Panels display object.
 */
function panels_rounded_corners_css($display) {
  $idstr = empty($display->css_id) ? '.rounded-corner' : "#$display->css_id .rounded-corner";
  $css_id = 'rounded-corner:' . $idstr;

  ctools_include('css');
  $filename = ctools_css_retrieve($css_id);
  if (!$filename) {
    $filename = ctools_css_store($css_id, _panels_rounded_corners_css($idstr), FALSE);
  }

  ctools_css_add_css($filename, 'module', 'all', FALSE);
}

/**
 * Generates the dynamic CSS.
 */
function _panels_rounded_corners_css($idstr) {
  $url = panels_get_path('plugins/styles/corners', TRUE);

  $css = <<<EOF

.t-edge, .b-edge, .l-edge, .r-edge, .wrap-corner {
  position: relative;
  /* hasLayout -1 ? For IE only */
  zoom: 1;
}
$idstr .t-edge {
  background: url('$url/shadow-t.png') repeat-x 0 top;
  font-size: 1px;
}
$idstr .b-edge {
  background: url('$url/shadow-b.png') repeat-x 0 bottom;
  font-size: 1px;
}
$idstr .l-edge {
  background: url('$url/shadow-l.png') repeat-y 0 0;
}
$idstr .r-edge {
  background: url('$url/shadow-r.png') repeat-y right 0;
}
$idstr .wrap-corner {
  background: #fff !important;
}
$idstr .wrap-corner .t-edge, $idstr .wrap-corner .b-edge {
  height: 11px;
}
$idstr .wrap-corner .l, $idstr .wrap-corner .r {
  position: absolute;
  top: 0;
  height: 11px;
  width: 11px;
  background-image: url('$url/corner-bits.png');
}
$idstr .wrap-corner .l {
  left: 0;
}
$idstr .wrap-corner .r {
  right: 0;
  background-position: -11px 0;
}
$idstr .wrap-corner .b-edge .l {
  background-position: 0 -11px;
}
$idstr .wrap-corner .b-edge .r {
  background-position: -11px -11px;
}
$idstr .wrap-corner .r-edge {
  padding: 5px 24px;
}
$idstr div.admin-links {
  margin-top: -14px;
  margin-left: -12px;
}

$idstr .panel-region-separator {
  background: url('$url/shadow-b.png') repeat-x 0 center;
  font-size: 1px;
  height: 30px;
}

$idstr .rounded-corner {
  margin-bottom: 1em;
}

EOF;

  return $css;
}
