<?php

/**
 * Legacy render pipeline for a panels display.
 *
 * This render pipeline mirrors the old procedural system exactly, and plugins
 * written for the legacy system will work exactly as they did before with this
 * renderer.
 *
 * Most plugins will work with the newer renderer. These are the exceptions:
 *  - Style plugins that implement panel styling no longer need to call
 *    panels_render_pane() on all contained panes; rendered pane HTML is now
 *    passed in directly.
 *  - Cache plugins are now triggered on rendered HTML, rather than on
 *    unrendered datastructures, when acting at the display level. When acting
 *    at the pane level, they still receive the unrendered datastructure.
 *
 * If your site relies on any of these plugin behaviors, you will need to use
 * this renderer instead of the new panels_renderer_standard() until those
 * plugins are updated.
 */
class panels_renderer_legacy {
  var $meta_location = 'standard';

  var $display;
  var $plugins = array();

  /**
   * Include rendered HTML prior to the layout.
   *
   * @var string
   */
  var $prefix = '';

  /**
   * Include rendered HTML after the layout.
   *
   * @var string
   */
  var $suffix = '';

  function init($plugin, &$display) {
    $this->plugin = $plugin;
    $this->plugins['layout'] = panels_get_layout($display->layout);
    if (empty($this->plugins['layout'])) {
      watchdog('panels', "Layout: @layout couldn't been found, maybe the theme is disabled.", array('@layout' => $display->layout));
    }
    $this->display = &$display;
  }

  /**
   * Add CSS information to the renderer.
   *
   * To facilitate previews over Views, CSS can now be added in a manner
   * that does not necessarily mean just using drupal_add_css. Therefore,
   * during the panel rendering process, this method can be used to add
   * css and make certain that ti gets to the proper location.
   *
   * The arguments should exactly match drupal_add_css().
   *
   * @see drupal_add_css
   */
  function add_css($filename, $type = 'module', $media = 'all', $preprocess = TRUE) {
    $path = file_create_path($filename);
    switch ($this->meta_location) {
      case 'standard':
        if ($path) {
          // Use CTools CSS add because it can handle temporary CSS in private
          // filesystem.
          ctools_include('css');
          ctools_css_add_css($filename, $type, $media, $preprocess);
        }
        else {
          drupal_add_css($filename, $type, $media, $preprocess);
        }
        break;
      case 'inline':
        if ($path) {
          $url = file_create_url($filename);
        }
        else {
          $url = base_path() . $filename;
        }

        $this->prefix .= '<link type="text/css" rel="stylesheet" media="' . $media . '" href="' . $url . '" />'."\n";
        break;
    }
  }

  /**
   * Builds inner content, then hands off to layout-specified theme function for
   * final render step.
   *
   * This is the outermost method in the Panels render pipeline. It calls the
   * inner methods, which return a content array, which is in turn passed to the
   * theme function specified in the layout plugin.
   *
   * @return string
   *  Themed & rendered HTML output.
   */
  function render() {
    if (!empty($this->plugins['layout']['css'])) {
      if (file_exists(path_to_theme() . '/' . $this->plugins['layout']['css'])) {
        drupal_add_css(path_to_theme() . '/' . $this->plugins['layout']['css']);
      }
      else {
        drupal_add_css($this->plugins['layout']['path'] . '/' . $this->plugins['layout']['css']);
      }
    }
    // This now comes after the CSS is added, because panels-within-panels must
    // have their CSS added in the right order; inner content before outer content.

    if (empty($this->display->cache['method']) || !empty($this->display->skip_cache)) {
      $content = $this->render_regions();
    }
    else {
      $cache = panels_get_cached_content($this->display, $this->display->args, $this->display->context);
      if ($cache === FALSE) {
        $cache = new panels_cache_object();
        $cache->set_content($this->render_regions());
        panels_set_cached_content($cache, $this->display, $this->display->args, $this->display->context);
      }
      $content = $cache->content;
    }

    $output = theme($this->plugins['layout']['theme'], check_plain($this->display->css_id), $content, $this->display->layout_settings, $this->display, $this->plugins['layout'], $this);

    return $this->prefix . $output . $this->suffix;
  }

  /**
   * Render all panes in the attached display into their panel regions, then
   * render those regions.
   *
   * @return array $content
   *    An array of rendered panel regions, keyed on the region name.
   */
  function render_regions() {
    ctools_include('content');

    // First, render all the panes into little boxes. We do this here because
    // some panes request to be rendered after other panes (primarily so they
    // can do the leftovers of forms).
    $panes = $first = $normal = $last = array();

    foreach ($this->display->content as $pid => $pane) {
      $pane->shown = !empty($pane->shown); // guarantee this field exists.
      // If the user can't see this pane, do not render it.
      if (!$pane->shown || !panels_pane_access($pane, $this->display)) {
        continue;
      }

      $content_type = ctools_get_content_type($pane->type);

      // If this pane wants to render last, add it to the $last array. We allow
      // this because some panes need to be rendered after other panes,
      // primarily so they can do things like the leftovers of forms.
      if (!empty($content_type['render last'])) {
        $last[$pid] = $pane;
      }
      // If it wants to render first, add it to the $first array. This is used
      // by panes that need to do some processing before other panes are
      // rendered.
      else if (!empty($content_type['render first'])) {
        $first[$pid] = $pane;
      }
      // Otherwise, render it in the normal order.
      else {
        $normal[$pid] = $pane;
      }
    }

    foreach (($first + $normal + $last) as $pid => $pane) {
      $panes[$pid] = $this->render_pane($pane);
    }

    // Loop through all panels, put all panes that belong to the current panel
    // in an array, then render the panel. Primarily this ensures that the
    // panes are in the proper order.
    $content = array();
    foreach ($this->display->panels as $panel_name => $pids) {
      $panel_panes = array();
      foreach ($pids as $pid) {
        if (!empty($panes[$pid])) {
          $panel_panes[$pid] = $panes[$pid];
        }
      }
      $content[$panel_name] = $this->render_region($panel_name, $panel_panes);
    }

    // Prevent notices by making sure that all panels at least have an entry:
    $panels = panels_get_regions($this->plugins['layout'], $this->display);
    foreach ($panels as $id => $panel) {
      if (!isset($content[$id])) {
        $content[$id] = NULL;
      }
    }

    return $content;
  }

  /**
   * Render the contents of a single pane.
   *
   * This method retrieves pane content and produces a ready-to-render content
   * object. It also manages pane-specific caching.
   *
   * @param stdClass $pane
   *    A Panels pane object, as loaded from the database.
   */
  function render_pane(&$pane) {
    ctools_include('context');
    if (!is_array($this->display->context)) {
      $this->display->context = array();
    }

    $content = FALSE;
    $caching = !empty($pane->cache['method']) && empty($this->display->skip_cache);
    if ($caching && ($cache = panels_get_cached_content($this->display, $this->display->args, $this->display->context, $pane))) {
      $content = $cache->content;
    }
    else {
      $content = ctools_content_render($pane->type, $pane->subtype, $pane->configuration, array(), $this->display->args, $this->display->context);
      foreach (module_implements('panels_pane_content_alter') as $module) {
        $function = $module . '_panels_pane_content_alter';
        $function($content, $pane, $this->display->args, $this->display->context);
      }
      if ($caching) {
        $cache = new panels_cache_object();
        $cache->set_content($content);
        panels_set_cached_content($cache, $this->display, $this->display->args, $this->display->context, $pane);
        $content = $cache->content;
      }
    }

    // Pass long the css_id that is usually available.
    if (!empty($pane->css['css_id']) && is_object($content)) {
      $content->css_id = $pane->css['css_id'];
    }

    // Pass long the css_class that is usually available.
    if (!empty($pane->css['css_class']) && is_object($content)) {
      $content->css_class = $pane->css['css_class'];
    }

    return $content;
  }

  /**
   * Render a single panel region.
   *
   * Primarily just a passthrough to the panel region rendering callback
   * specified by the style plugin that is attached to the current panel region.
   *
   * @param $region_name
   *   The ID of the panel region being rendered
   * @param $panes
   *   An array of panes that are assigned to the panel that's being rendered.
   *
   * @return
   *   The rendered HTML for the passed-in panel region.
   */
  function render_region($region_name, $panes) {
    list($style, $style_settings) = panels_get_panel_style_and_settings($this->display->panel_settings, $region_name);
    $callback = 'render panel';

    // Retrieve the pid (can be a panel page id, a mini panel id, etc.), this
    // might be used (or even necessary) for some panel display styles.
    $owner_id = 0;
    if (isset($this->display->owner) && is_object($this->display->owner) && isset($this->display->owner->id)) {
      $owner_id = $this->display->owner->id;
    }

    // Check to see if we're actually running a current style plugin even though
    // we're in the legacy renderer
    if (version_compare($style['version'], 2.0, '>=')) {
      // We are, so pre-render the content as the current version expects
      foreach($panes as $pane_id => $pane) {
        $content = panels_render_pane($pane, $this->display->content[$pane_id], $this->display);
        if ($content) {
          $panes[$pane_id] = $content;
        }
        else {
          unset($panes[$pane_id]);
        }
      }
      // And set the callback to the new key
      $callback = 'render region';

    }

    return theme($style[$callback], $this->display, $owner_id, $panes, $style_settings, $region_name, $style);
  }
}
