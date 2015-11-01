<?php

/**
 * @file
 *
 * Contains Panels display rendering functions.
 */

/**
 * Render the administrative layout of a display.
 *
 * This is used for the edit version, so that layouts can have different
 * modes, such as the flexible layout designer mode.
 */
function panels_render_layout_admin($layout, $content, $display) {
  // @todo This should be abstracted.
  if (!empty($layout['css'])) {
    if (file_exists(path_to_theme() . '/' . $layout['css'])) {
      drupal_add_css(path_to_theme() . '/' . $layout['css']);
    }
    else {
      drupal_add_css($layout['path'] . '/' . $layout['css']);
    }
  }

  if (isset($layout['admin css'])) {
    drupal_add_css($layout['path'] . '/' . $layout['admin css']);
  }

  $theme = isset($layout['admin theme']) ? $layout['admin theme'] : $layout['theme'];
  return theme($theme, isset($display->css_id) ? $display->css_id : '', $content, $display->layout_settings, $display, $layout);
}

/**
 * Render a pane using the appropriate style.
 *
 * Legacy function; this behavior has been moved onto the display renderer
 * object. The function name here is included for backwards compatibility. New
 * style plugins should NEVER call it.
 *
 * $content
 *   The already rendered content via panels_render_pane_content()
 * $pane
 *   The $pane information from the display
 * $display
 *   The display.
 */
function panels_render_pane($content, $pane, &$display) {
  if ($display->hide_title == PANELS_TITLE_PANE && !empty($display->title_pane) && $display->title_pane == $pane->pid) {

    // If the user selected to override the title with nothing, and selected
    // this as the title pane, assume the user actually wanted the original
    // title to bubble up to the top but not actually be used on the pane.
    if (empty($content->title) && !empty($content->original_title)) {
      $display->stored_pane_title = $content->original_title;
    }
    else {
      $display->stored_pane_title = !empty($content->title) ? $content->title : '';
    }
  }

  if (!empty($content->content)) {
    if (!empty($pane->style['style'])) {
      $style = panels_get_style($pane->style['style']);

      if (isset($style) && isset($style['render pane'])) {
        $output = theme($style['render pane'], $content, $pane, $display, $style);

        // This could be null if no theme function existed.
        if (isset($output)) {
          return $output;
        }
      }
    }

    // fallback
    return theme('panels_pane', $content, $pane, $display);
  }
}

/**
 * Given a display and the id of a panel, get the style in which to render
 * that panel.
 */
function panels_get_panel_style_and_settings($panel_settings, $panel) {
  if (empty($panel_settings)) {
    return array(panels_get_style('default'), array());
  }

  if (empty($panel_settings[$panel]['style']) || $panel_settings[$panel]['style'] == -1) {
    if (empty($panel_settings['style'])) {
      return array(panels_get_style('default'), array());
    }

    $style = panels_get_style($panel_settings['style']);
    $style_settings = isset($panel_settings['style_settings']['default']) ? $panel_settings['style_settings']['default'] : array();
  }
  else {
    $style = panels_get_style($panel_settings[$panel]['style']);
    $style_settings = isset($panel_settings['style_settings'][$panel]) ? $panel_settings['style_settings'][$panel] : array();
  }

  return array($style, $style_settings);
}
