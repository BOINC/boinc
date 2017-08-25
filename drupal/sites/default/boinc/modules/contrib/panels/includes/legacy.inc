<?php

/**
 * Legacy state manager for Panels.
 *
 * Checks all possible ways (using discovery of patterned method names) in which
 * Panels may need to operate in legacy mode,
 * sets variables as appropriate, and returns an informational
 *
 */
class PanelsLegacyState {
  var $legacy = NULL;

  function t() {
    $func = get_t();
    $args = func_get_args();
    return call_user_func_array($func, $args);
  }

  function getStatus() {
    if (!isset($this->legacy)) {
      $this->determineStatus();
    }
    return $this->legacy;
  }

  /**
   * Run all compatibility checks.
   */
  function determineStatus() {
    $this->legacy = array();
    foreach(get_class_methods($this) as $method) {
      if (strtolower(substr($method, 0, 5)) == 'check') {
        $this->legacy[$method] = $this->$method();
      }
    }
    $this->legacy = array_filter($this->legacy);
  }

  /**
   * Compatibility checker that ensures modules that implement Panels styles
   * list their api as being at least 2.0; this corresponds to the change with
   * the initial IPE commit that made region styles take a fully rendered pane
   * HTML string instead of a pane object that still needed rendering.
   */
  function checkStylesIPE1() {
    $legacy_info = array(
      'explanation' => $this->t('Panels 3.6 made changes to the rendering order in a way that affects certain style plugins. The above modules implement style plugins, but have not indicated their compatibility with this new system. See !link for information on how to update style plugins to the new system.', array('!link' => url('http://drupal.org/node/865840', array('external' => TRUE)))),
      'modules' => array(),
    );


    $naughties = &$legacy_info['modules'];
    $legacy = FALSE;

    ctools_include('plugins', 'panels');
    // TODO given that the plugin cache is also clearing at this time, should
    // check this to ensure this isn't causing some kind of weird race condition
    $styles = panels_get_styles();

    foreach ($styles as $style) {
      if (version_compare($style['version'], 2.0, '<') && empty($naughties[$style['module']])) {
        $legacy = TRUE;
        $naughties[$style['module']] = $this->t('Style plugins');
      }
    }
    variable_set('panels_legacy_rendering_mode', $legacy);
    return $legacy ? $legacy_info : array();
  }
}
