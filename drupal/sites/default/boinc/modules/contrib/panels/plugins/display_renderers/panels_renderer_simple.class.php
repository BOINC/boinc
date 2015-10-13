<?php
/**
 * @file
 * Contains the simple display renderer.
 */

/**
 * The simple display renderer renders a display normally, except each pane
 * is already rendered content, rather than a pane containing CTools content
 * to be rendered. Styles are not supported.
 */
class panels_renderer_simple extends panels_renderer_standard {
  function render_regions() {
    $this->rendered['regions'] = array();
    foreach ($this->display->content as $region_id => $content) {
      if (is_array($content)) {
        $content = implode('', $content);
      }

      $this->rendered['regions'][$region_id] = $content;
    }
    return $this->rendered['regions'];
  }

  function render_panes() {
    // NOP
  }

  function prepare() {
    $this->prep_run = TRUE;
  }
}
