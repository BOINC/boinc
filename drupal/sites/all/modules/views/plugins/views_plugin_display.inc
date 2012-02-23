<?php
// $Id: views_plugin_display.inc,v 1.27.2.13 2010/06/17 02:46:53 merlinofchaos Exp $
/**
 * @file
 * Contains the base display plugin.
 */

/**
 * @defgroup views_display_plugins Views' display plugins
 * @{
 * Display plugins control how Views interact with the rest of Drupal.
 *
 * They can handle creating Views from a Drupal page hook; they can
 * handle creating Views from a Drupal block hook. They can also
 * handle creating Views from an external module source, such as
 * a Panels pane, or an insert view, or a CCK field type.
 *
 * @see hook_views_plugins
 */

/**
 * The default display plugin handler. Display plugins handle options and
 * basic mechanisms for different output methods.
 *
 * @ingroup views_display_plugins
 */
class views_plugin_display extends views_plugin {
  var $handlers = array();

  function init(&$view, &$display, $options = NULL) {
    $this->view = &$view;
    $this->display = &$display;

    // Make some modifications:
    if (!isset($options)) {
      $options = $display->display_options;
    }

    if ($this->is_default_display() && isset($options['defaults'])) {
      unset($options['defaults']);
    }

    $this->unpack_options($this->options, $options);
  }

  function destroy() {
    parent::destroy();

    foreach ($this->handlers as $type => $handlers) {
      foreach ($handlers as $id => $handler) {
        if (is_object($handler)) {
          $this->handlers[$type][$id]->destroy();
        }
      }
    }

    if (isset($this->default_display)) {
      unset($this->default_display);
    }
  }

  /**
   * Determine if this display is the 'default' display which contains
   * fallback settings
   */
  function is_default_display() { return FALSE; }

  /**
   * Determine if this display uses exposed filters, so the view
   * will know whether or not to build them.
   */
  function uses_exposed() {
    if (!isset($this->has_exposed)) {
      foreach (array('field', 'filter') as $type) {
        foreach ($this->view->$type as $key => $handler) {
          if ($handler->is_exposed()) {
            // one is all we need; if we find it, return true.
            $this->has_exposed = TRUE;
            return TRUE;
          }
        }
      }
      $this->has_exposed = FALSE;
    }

    return $this->has_exposed;
  }

  /**
   * Determine if this display should display the exposed
   * filters widgets, so the view will know whether or not
   * to render them.
   *
   * Regardless of what this function
   * returns, exposed filters will not be used nor
   * displayed unless uses_exposed() returns TRUE.
   */
  function displays_exposed() {
    return TRUE;
  }

  /**
   * Does the display use AJAX?
   */
  function use_ajax() {
    if (!empty($this->definition['use ajax'])) {
      return $this->get_option('use_ajax');
    }
    return FALSE;
  }

  /**
   * Does the display have a pager enabled?
   */
  function use_pager() {
    if (!empty($this->definition['use pager'])) {
      return $this->get_option('use_pager');
    }
    return FALSE;
  }

  /**
   * Does the display render the pager if it has it enabled?
   */
  function render_pager() {
    return $this->use_pager();
  }

  /**
   * Does the display have a more link enabled?
   */
  function use_more() {
    if (!empty($this->definition['use more'])) {
      return $this->get_option('use_more');
    }
    return FALSE;
  }

  /**
   * Should the enabled display more link be shown when no more items?
   */
  function use_more_always() {
    if (!empty($this->definition['use more'])) {
      return $this->get_option('use_more_always');
    }
    return FALSE;
  }

  /**
   * Does the display have custom link text?
   */
  function use_more_text() {
    if (!empty($this->definition['use more'])) {
      return $this->get_option('use_more_text');
    }
    return FALSE;
  }

  /**
   * Can this display accept attachments?
   */
  function accept_attachments() {
    return !empty($this->definition['accept attachments']);
  }

  /**
   * Allow displays to attach to other views.
   */
  function attach_to($display_id) { }

  /**
   * Static member function to list which sections are defaultable
   * and what items each section contains.
   */
  function defaultable_sections($section = NULL) {
    $sections = array(
      'access' => array('access'),
      'cache' => array('cache'),
      'title' => array('title'),
      'css_class' => array('css_class'),
      'header' => array('header', 'header_format', 'header_empty'),
      'footer' => array('footer', 'footer_format', 'footer_empty'),
      'empty' => array('empty', 'empty_format'),
      'use_ajax' => array('use_ajax'),
      'items_per_page' => array('items_per_page', 'offset', 'use_pager', 'pager_element'),
      'use_pager' => array('items_per_page', 'offset', 'use_pager', 'pager_element'),
      'use_more' => array('use_more', 'use_more_always', 'use_more_text'),
      'link_display' => array('link_display'),
      'distinct' => array('distinct'),
      'exposed_block' => array('exposed_block'),

      // Force these to cascade properly.
      'style_plugin' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'style_options' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'row_plugin' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),
      'row_options' => array('style_plugin', 'style_options', 'row_plugin', 'row_options'),

      // These guys are special
      'relationships' => array('relationships'),
      'fields' => array('fields'),
      'sorts' => array('sorts'),
      'arguments' => array('arguments'),
      'filters' => array('filters'),
    );
    if ($section) {
      if (!empty($sections[$section])) {
        return $sections[$section];
      }
    }
    else {
      return $sections;
    }
  }

  /**
   * Set default options.
   *
   * Displays put their options in a different place than everything else; also
   * displays spread their options out. We don't want to set defaults for
   * items that are normally defaulted elsewhere.
   */
  function _set_option_defaults(&$storage, $options, $level = 0) {
    foreach ($options as $option => $definition) {
      // If defaulted to elsewhere and we're not the default display, skip.
      if ($level == 0 && !$this->is_default_display() && !empty($options['defaults']['default'][$option])) {
        continue;
      }

      if (isset($definition['contains']) && is_array($definition['contains'])) {
        $storage[$option] = array();
        $this->_set_option_defaults($storage[$option], $definition['contains'], $level++);
      }
      else {
        $storage[$option] = isset($definition['default']) ? $definition['default'] : NULL;
      }
    }
  }

  function option_definition() {
    $options = array(
      'defaults' => array(
        'default' => array(
          'access' => TRUE,
          'cache' => TRUE,
          'title' => TRUE,
          'css_class' => TRUE,
          'header' => TRUE,
          'header_format' => TRUE,
          'header_empty' => TRUE,
          'footer' => TRUE,
          'footer_format' => TRUE,
          'footer_empty' => TRUE,
          'empty' => TRUE,
          'empty_format' => TRUE,

          'use_ajax' => TRUE,
          'items_per_page' => TRUE,
          'offset' => TRUE,
          'use_pager' => TRUE,
          'pager_element'  => TRUE,
          'use_more' => TRUE,
          'use_more_always' => TRUE,
          'use_more_text' => TRUE,
          'distinct' => TRUE,
          'exposed_block' => TRUE,

          'link_display' => TRUE,

          'style_plugin' => TRUE,
          'style_options' => TRUE,
          'row_plugin' => TRUE,
          'row_options' => TRUE,

          'relationships' => TRUE,
          'fields' => TRUE,
          'sorts' => TRUE,
          'arguments' => TRUE,
          'filters' => TRUE,
        ),
      ),
      'relationships' => array(
        'default' => array(),
        'export' => 'export_item',
      ),
      'fields' => array(
        'default' => array(),
        'export' => 'export_item',
      ),
      'sorts' => array(
        'default' => array(),
        'export' => 'export_item',
      ),
      'arguments' => array(
        'default' => array(),
        'export' => 'export_item',
      ),
      'filters' => array(
        'default' => array(),
        'export' => 'export_item',
      ),
      'access' => array(
        'contains' => array(
          'type' => array('default' => 'none'),
         ),
      ),
      'cache' => array(
        'contains' => array(
          'type' => array('default' => 'none'),
         ),
      ),
      'title' => array(
        'default' => '',
        'translatable' => TRUE,
      ),
      'css_class' => array(
        'default' => '',
        'translatable' => FALSE,
      ),
      'header' => array(
        'default' => '',
        'translatable' => TRUE,
      ),
      'header_format' => array(
        'default' => FILTER_FORMAT_DEFAULT,
      ),
      'header_empty' => array(
        'default' => FALSE,
      ),
      'footer' => array(
        'default' => '',
        'translatable' => TRUE,
      ),
      'footer_format' => array(
        'default' => FILTER_FORMAT_DEFAULT,
      ),
      'footer_empty' => array(
        'default' => FALSE,
      ),
      'empty' => array(
        'default' => '',
        'translatable' => TRUE,
      ),
      'empty_format' => array(
        'default' => FILTER_FORMAT_DEFAULT,
      ),
      'use_ajax' => array(
        'default' => FALSE,
      ),
      'items_per_page' => array(
        'default' => 10,
      ),
      'offset' => array(
        'default' => 0,
      ),
      'use_pager' => array(
        'default' => FALSE,
      ),
      'pager_element' => array(
        'default' => 0,
      ),
      'use_more' => array(
        'default' => FALSE,
      ),
      'use_more_always' => array(
        'default' => FALSE,
      ),
      'use_more_text' => array(
        'default' => 'more',
        'translatable' => TRUE,
      ),
      'link_display' => array(
        'default' => '',
      ),
      'distinct' => array(
        'default' => FALSE,
      ),

      'style_plugin' => array(
        'default' => 'default',
      ),
      'style_options' => array(
        'default' => array(),
      ),
      'row_plugin' => array(
        'default' => 'fields',
      ),
      'row_options' => array(
        'default' => array(),
      ),

      'exposed_block' => array(
        'default' => FALSE,
      ),
    );

    if ($this->is_default_display()) {
      unset($options['defaults']);
    }
    return $options;
  }

  /**
   * Check to see if the display has a 'path' field.
   *
   * This is a pure function and not just a setting on the definition
   * because some displays (such as a panel pane) may have a path based
   * upon configuration.
   *
   * By default, displays do not have a path.
   */
  function has_path() { return FALSE; }

  /**
   * Check to see if the display has some need to link to another display.
   *
   * For the most part, displays without a path will use a link display. However,
   * sometimes displays that have a path might also need to link to another display.
   * This is true for feeds.
   */
  function uses_link_display() { return !$this->has_path(); }

  /**
   * Check to see which display to use when creating links within
   * a view using this display.
   */
  function get_link_display() {
    $display_id = $this->get_option('link_display');
    // If unknown, pick the first one.
    if (empty($display_id) || empty($this->view->display[$display_id])) {
      foreach ($this->view->display as $display_id => $display) {
        if (!empty($display->handler) && $display->handler->has_path()) {
          return $display_id;
        }
      }
    }
    else {
      return $display_id;
    }
    // fall-through returns NULL
  }

  /**
   * Return the base path to use for this display.
   *
   * This can be overridden for displays that do strange things
   * with the path.
   */
  function get_path() {
    if ($this->has_path()) {
      return $this->get_option('path');
    }

    $display_id = $this->get_link_display();
    if ($display_id && !empty($this->view->display[$display_id]) && is_object($this->view->display[$display_id]->handler)) {
      return $this->view->display[$display_id]->handler->get_path();
    }
  }

  /**
   * Check to see if the display needs a breadcrumb
   *
   * By default, displays do not need breadcrumbs
   */
  function uses_breadcrumb() { return FALSE; }

  /**
   * Determine if a given option is set to use the default display or the
   * current display
   *
   * @return
   *   TRUE for the default display
   */
  function is_defaulted($option) {
    return !$this->is_default_display() && !empty($this->default_display) && !empty($this->options['defaults'][$option]);
  }

  /**
   * Intelligently get an option either from this display or from the
   * default display, if directed to do so.
   */
  function get_option($option) {
    if ($this->is_defaulted($option)) {
      return $this->default_display->get_option($option);
    }

    if (array_key_exists($option, $this->options)) {
      return $this->options[$option];
    }
  }

  /**
   * Determine if the display's style uses fields.
   */
  function uses_fields() {
    $plugin = $this->get_plugin();
    if ($plugin) {
      return $plugin->uses_fields();
    }
  }

  /**
   * Get the display or row plugin, if it exists.
   */
  function get_plugin($type = 'style', $name = NULL) {
    if (!$name) {
      $name = $this->get_option($type . '_plugin');
    }

    $plugin = views_get_plugin($type, $name);
    if ($plugin) {
      $options = $this->get_option($type . '_options');
      $plugin->init($this->view, $this->display, $options);
      return $plugin;
    }
  }

  /**
   * Get the access plugin
   */
  function get_access_plugin($name = NULL) {
    if (!$name) {
      $access = $this->get_option('access');
      $name = $access['type'];
    }

    $plugin = views_get_plugin('access', $name);
    if ($plugin) {
      $plugin->init($this->view, $this->display);
      return $plugin;
    }
  }

  /**
   * Get the cache plugin
   */
  function get_cache_plugin($name = NULL) {
    if (!$name) {
      $cache = $this->get_option('cache');
      $name = $cache['type'];
    }

    $plugin = views_get_plugin('cache', $name);
    if ($plugin) {
      $plugin->init($this->view, $this->display);
      return $plugin;
    }
  }

  /**
   * Get the handler object for a single handler.
   */
  function &get_handler($type, $id) {
    if (!isset($this->handlers[$type])) {
      $this->get_handlers($type);
    }

    if (isset($this->handlers[$type][$id])) {
      return $this->handlers[$type][$id];
    }

    // So we can return a reference.
    $null = NULL;
    return $null;
  }

  /**
   * Get a full array of handlers for $type. This caches them.
   */
  function get_handlers($type) {
    if (!isset($this->handlers[$type])) {
      $this->handlers[$type] = array();
      $types = views_object_types();
      $plural = $types[$type]['plural'];
      foreach ($this->get_option($plural) as $id => $info) {
        if ($info['id'] != $id) {
          $info['id'] = $id;
        }

        $handler = views_get_handler($info['table'], $info['field'], $type);
        if ($handler) {
          $handler->init($this->view, $info);
          $this->handlers[$type][$id] = &$handler;
        }

        // Prevent reference problems.
        unset($handler);
      }
    }

    return $this->handlers[$type];
  }

  /**
   * Retrieve a list of fields for the current display with the
   *  relationship associated if it exists.
   */
  function get_field_labels() {
    $options = array();
    foreach ($this->get_handlers('relationship') as $relationship => $handler) {
      if ($label = $handler->label()) {
        $relationships[$relationship] = $label;
      }
      else {
        $relationships[$relationship] = $handler->ui_name();
      }
    }

    foreach ($this->get_handlers('field') as $id => $handler) {
      if ($label = $handler->label()) {
        $options[$id] = $label;
      }
      else {
        $options[$id] = $handler->ui_name();
      }
      if (!empty($handler->options['relationship']) && !empty($relationships[$handler->options['relationship']])) {
        $options[$id] = '(' . $relationships[$handler->options['relationship']] . ') ' . $options[$id];
      }
    }
    return $options;
  }

  /**
   * Intelligently set an option either from this display or from the
   * default display, if directed to do so.
   */
  function set_option($option, $value) {
    if ($this->is_defaulted($option)) {
      return $this->default_display->set_option($option, $value);
    }

    // Set this in two places: On the handler where we'll notice it
    // but also on the display object so it gets saved. This should
    // only be a temporary fix.
    $this->display->display_options[$option] = $value;
    return $this->options[$option] = $value;
  }

  /**
   * Set an option and force it to be an override.
   */
  function override_option($option, $value) {
    $this->set_override($option, FALSE);
    $this->set_option($option, $value);
  }

  /**
   * Because forms may be split up into sections, this provides
   * an easy URL to exactly the right section. Don't override this.
   */
  function option_link($text, $section, $class = '', $title = '') {
    if (!empty($class)) {
      $text = '<span>' . $text . '</span>';
    }

    if (!trim($text)) {
      $text = t('Broken field');
    }

    if (empty($title)) {
      $title = $text;
    }

    return l($text, 'admin/build/views/nojs/display/' . $this->view->name . '/' . $this->display->id . '/' . $section, array('attributes' => array('class' => 'views-ajax-link ' . $class, 'title' => $title), 'html' => TRUE));
  }

  /**
   * Provide the default summary for options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    $categories['basic'] = array(
      'title' => t('Basic settings'),
    );

    $options['display_title'] = array(
      'category' => 'basic',
      'title' => t('Name'),
      'value' => check_plain($this->display->display_title),
      'desc' => t('Change the name of this display.'),
    );

    $title = strip_tags($this->get_option('title'));
    if (!$title) {
      $title = t('None');
    }

    $options['title'] = array(
      'category' => 'basic',
      'title' => t('Title'),
      'value' => $title,
      'desc' => t('Change the title that this display will use.'),
    );

    $style_plugin = views_fetch_plugin_data('style', $this->get_option('style_plugin'));
    $style_title = empty($style_plugin['title']) ? t('Missing style plugin') : $style_plugin['title'];

    $style = '';

    $options['style_plugin'] = array(
      'category' => 'basic',
      'title' => t('Style'),
      'value' => $style_title,
      'desc' => t('Change the style plugin.'),
    );

    // This adds a 'Settings' link to the style_options setting if the style has options.
    if (!empty($style_plugin['uses options'])) {
      $options['style_plugin']['links']['style_options'] = t('Change settings for this style');
    }

    if (!empty($style_plugin['uses row plugin'])) {
      $row_plugin = views_fetch_plugin_data('row', $this->get_option('row_plugin'));
      $row_title = empty($row_plugin['title']) ? t('Missing style plugin') : $row_plugin['title'];

      $options['row_plugin'] = array(
        'category' => 'basic',
        'title' => t('Row style'),
        'value' => $row_title,
        'desc' => t('Change the row plugin.'),
      );
      // This adds a 'Settings' link to the row_options setting if the row style has options.
      if (!empty($row_plugin['uses options'])) {
        $options['row_plugin']['links']['row_options'] = t('Change settings for this style');
      }
    }
    if (!empty($this->definition['use ajax'])) {
      $options['use_ajax'] = array(
        'category' => 'basic',
        'title' => t('Use AJAX'),
        'value' => $this->get_option('use_ajax') ? t('Yes') : t('No'),
        'desc' => t('Change whether or not this display will use AJAX.'),
      );
    }

    if (!empty($this->definition['use pager'])) {
      $options['use_pager'] = array(
        'category' => 'basic',
        'title' => t('Use pager'),
        'value' => $this->get_option('use_pager') ? ($this->get_option('use_pager') === 'mini' ? t('Mini') : t('Yes')) : t('No'),
        'desc' => t("Change this display's pager setting."),
      );
    }

    $items = intval($this->get_option('items_per_page'));
    $options['items_per_page'] = array(
      'category' => 'basic',
      'title' => $this->use_pager() ? t('Items per page') : t('Items to display'),
      'value' => $items ? $items : t('Unlimited'),
      'desc' => t('Change how many items to display.'),
    );

    if (!empty($this->definition['use more'])) {
      $options['use_more'] = array(
        'category' => 'basic',
        'title' => t('More link'),
        'value' => $this->get_option('use_more') ? t('Yes') : t('No'),
        'desc' => t('Specify whether this display will provide a "more" link.'),
      );
    }

    $options['distinct'] = array(
      'category' => 'basic',
      'title' => t('Distinct'),
      'value' => $this->get_option('distinct') ? t('Yes') : t('No'),
      'desc' => t('Display only distinct items, without duplicates.'),
    );

    $access_plugin = $this->get_access_plugin();
    if (!$access_plugin) {
      // default to the no access control plugin.
      $access_plugin = views_get_plugin('access', 'none');
    }

    $access_str = $access_plugin->summary_title();

    $options['access'] = array(
      'category' => 'basic',
      'title' => t('Access'),
      'value' => $access_str,
      'desc' => t('Specify access control type for this display.'),
    );

    if (!empty($access_plugin->definition['uses options'])) {
      $options['access']['links']['access_options'] = t('Change settings for this access type.');
    }

    $cache_plugin = $this->get_cache_plugin();
    if (!$cache_plugin) {
      // default to the no cache control plugin.
      $cache_plugin = views_get_plugin('cache', 'none');
    }

    $cache_str = $cache_plugin->summary_title();

    $options['cache'] = array(
      'category' => 'basic',
      'title' => t('Caching'),
      'value' => $cache_str,
      'desc' => t('Specify caching type for this display.'),
    );

    if (!empty($cache_plugin->definition['uses options'])) {
      $options['cache']['links']['cache_options'] = t('Change settings for this caching type.');
    }

    if ($this->uses_link_display()) {
      // Only show the 'link display' if there is more than one option.
      $count = 0;
      foreach ($this->view->display as $display_id => $display) {
        if (is_object($display->handler) && $display->handler->has_path()) {
          $count++;
        }
        if ($count > 1) {
          break;
        }
      }

      if ($count > 1) {
        $display_id = $this->get_link_display();
        $link_display = empty($this->view->display[$display_id]) ? t('None') : check_plain($this->view->display[$display_id]->display_title);
        $options['link_display'] = array(
          'category' => 'basic',
          'title' => t('Link display'),
          'value' => $link_display,
          'desc' => t('Specify which display this display will link to.'),
        );
      }
    }

    $options['exposed_block'] = array(
      'category' => 'basic',
      'title' => t('Exposed form in block'),
      'value' => $this->get_option('exposed_block') ? t('Yes') : t('No'),
      'desc' => t('Allow the exposed form to appear in a block instead of the view.'),
    );

    foreach (array('header' => t('Header'), 'footer' => t('Footer'), 'empty' => t('Empty text')) as $type => $name) {
      if (!$this->get_option($type)) {
        $field = t('None');
      }
      else {
        // A lot of code to get the name of the filter format.
        $fmt_string = $this->get_option($type . '_format');
        if (empty($fmt_string)) {
          $fmt_string = FILTER_FORMAT_DEFAULT;
        }
        $format_val = filter_resolve_format($fmt_string);
        $format = filter_formats($format_val);
        if ($format) {
          $field = check_plain($format->name);
        }
        else {
          $field = t('Unknown/missing format');
        }
      }

      $options[$type] = array(
        'category' => 'basic',
        'title' => $name,
        'value' => $field,
        'desc' => t("Change this display's !name.", array('!name' => strtolower($name))),
      );
    }

    $css_class = check_plain(trim($this->get_option('css_class')));
    if (!$css_class) {
      $css_class = t('None');
    }

    $options['css_class'] = array(
      'category' => 'basic',
      'title' => t('CSS class'),
      'value' => $css_class,
      'desc' => t('Change the CSS class name(s) that will be added to this display.'),
    );

    $options['analyze-theme'] = array(
      'category' => 'basic',
      'title' => t('Theme'),
      'value' => t('Information'),
      'desc' => t('Get information on how to theme this display'),
    );
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    if ($this->defaultable_sections($form_state['section'])) {
      $this->add_override_button($form, $form_state, $form_state['section']);
    }
    $form['#title'] = check_plain($this->display->display_title) . ': ';

    // Set the 'section' to hilite on the form.
    // If it's the item we're looking at is pulling from the default display,
    // reflect that. Don't use is_defaulted since we want it to show up even
    // on the default display.
    if (!empty($this->options['defaults'][$form_state['section']])) {
      $form['#section'] = 'default-' . $form_state['section'];
    }
    else {
      $form['#section'] = $this->display->id . '-' . $form_state['section'];
    }

    switch ($form_state['section']) {
      case 'display_title':
        $form['#title'] .= t('The name of this display');
        $form['display_title'] = array(
          '#type' => 'textfield',
          '#description' => t('This title will appear only in the administrative interface for the View.'),
          '#default_value' => $this->display->display_title,
        );
        break;
      case 'title':
        $form['#title'] .= t('The title of this view');
        $form['title'] = array(
          '#type' => 'textfield',
          '#description' => t('This title will be displayed with the view, wherever titles are normally displayed; i.e, as the page title, block title, etc.'),
          '#default_value' => $this->get_option('title'),
        );
        break;
      case 'css_class':
        $form['#title'] .= t('CSS class');
        $form['css_class'] = array(
          '#type' => 'textfield',
          '#description' => t('The CSS class names will be added to the view. This enables you to use specific CSS code for each view. You may define multiples classes separated by spaces.'),
          '#default_value' => $this->get_option('css_class'),
        );
        break;
      case 'use_ajax':
        $form['#title'] .= t('Use AJAX when available to load this view');
        $form['description'] = array(
          '#prefix' => '<div class="description form-item">',
          '#suffix' => '</div>',
          '#value' => t('If set, this view will use an AJAX mechanism for paging, table sorting and exposed filters. This means the entire page will not refresh. It is not recommended that you use this if this view is the main content of the page as it will prevent deep linking to specific pages, but it is very useful for side content.'),
        );
        $form['use_ajax'] = array(
          '#type' => 'radios',
          '#options' => array(1 => t('Yes'), 0 => t('No')),
          '#default_value' => $this->get_option('use_ajax') ? 1 : 0,
        );
        break;
      case 'use_pager':
        $form['#title'] .= t('Use a pager for this view');
        $form['use_pager'] = array(
          '#type' => 'radios',
          '#options' => array(TRUE => t('Full pager'), 'mini' => t('Mini pager'), 0 => t('No')),
          '#default_value' => $this->get_option('use_pager'),
        );
        $form['pager_element'] = array(
          '#type' => 'textfield',
          '#title' => t('Pager element'),
          '#description' => t("Unless you're experiencing problems with pagers related to this view, you should leave this at 0. If using multiple pagers on one page you may need to set this number to a higher value so as not to conflict within the ?page= array. Large values will add a lot of commas to your URLs, so avoid if possible."),
          '#default_value' => intval($this->get_option('pager_element')),
        );
        break;
      case 'items_per_page':
        $form['#title'] .= $this->use_pager() ? t('Items per page') : t('Items to display');

        $form['items_per_page'] = array(
          '#type' => 'textfield',
          '#description' => t('The number of items to display per page. Enter 0 for no limit.'),
          '#default_value' => intval($this->get_option('items_per_page')),
        );
        $form['offset'] = array(
          '#type' => 'textfield',
          '#title' => t('Offset'),
          '#description' => t('The number of items to skip. For example, if this field is 3, the first 3 items will be skipped and not displayed. Offset can not be used if items to display is 0; instead use a very large number there.'),
          '#default_value' => intval($this->get_option('offset')),
        );
        break;
      case 'use_more':
        $form['#title'] .= t('Add a more link to the bottom of the display.');
        $form['use_more'] = array(
          '#type' => 'checkbox',
          '#title' => t('Create more link'),
          '#description' => t("This will add a more link to the bottom of this view, which will link to the page view. If you have more than one page view, the link will point to the display specified in 'Link display' above."),
          '#default_value' => $this->get_option('use_more'),
        );
        $form['use_more_always'] = array(
          '#type' => 'checkbox',
          '#title' => t('Always display more link'),
          '#description' => t("This will display the more link even if there are no more items to display."),
          '#default_value' => $this->get_option('use_more_always'),
        );
        $form['#title'] .= t('Text to use for the more link.');
        $form['use_more_text'] = array(
          '#type' => 'textfield',
          '#title' => t('More link text'),
          '#description' => t("The text to display for the more link."),
          '#default_value' => $this->get_option('use_more_text'),
        );
        break;
      case 'distinct':
        $form['#title'] .= t('Display only distinct items, without duplicates.');
        $form['distinct'] = array(
          '#type' => 'checkbox',
          '#title' => t('Distinct'),
          '#description' => t('This will make the view display only distinct items. If there are multiple identical items, each will be displayed only once. You can use this to try and remove duplicates from a view, though it does not always work. Note that this can slow queries down, so use it with caution.'),
          '#default_value' => $this->get_option('distinct'),
        );
        break;
      case 'access':
        $form['#title'] .= t('Access restrictions');
        $form['access'] = array(
          '#prefix' => '<div class="clear-block">',
          '#suffix' => '</div>',
          '#tree' => TRUE,
        );

        $access = $this->get_option('access');
        $form['access']['type'] =  array(
          '#type' => 'radios',
          '#options' => views_fetch_plugin_names('access'),
          '#default_value' => $access['type'],
        );

        $access_plugin = views_fetch_plugin_data('access', $access['type']);
        if (!empty($access_plugin['uses options'])) {
          $form['markup'] = array(
            '#prefix' => '<div class="form-item description">',
            '#suffix' => '</div>',
            '#value' => t('You may also adjust the !settings for the currently selected access restriction by clicking on the icon.', array('!settings' => $this->option_link(t('settings'), 'access_options'))),
          );
        }

        break;
      case 'access_options':
        $access = $this->get_option('access');
        $plugin = $this->get_access_plugin();
        $form['#title'] .= t('Access options');
        if ($plugin) {
          $form['#help_topic'] = $plugin->definition['help topic'];
          $form['#help_module'] = $plugin->definition['module'];

          $form['access_options'] = array(
            '#tree' => TRUE,
          );
          $form['access_options']['type'] = array(
            '#type' => 'value',
            '#value' => $access['type'],
          );
          $plugin->options_form($form['access_options'], $form_state);
        }
        break;
      case 'cache':
        $form['#title'] .= t('Caching');
        $form['cache'] = array(
          '#prefix' => '<div class="clear-block">',
          '#suffix' => '</div>',
          '#tree' => TRUE,
        );

        $cache = $this->get_option('cache');
        $form['cache']['type'] =  array(
          '#type' => 'radios',
          '#options' => views_fetch_plugin_names('cache'),
          '#default_value' => $cache['type'],
        );

        $cache_plugin = views_fetch_plugin_data('cache', $cache['type']);
        if (!empty($cache_plugin['uses options'])) {
          $form['markup'] = array(
            '#prefix' => '<div class="form-item description">',
            '#suffix' => '</div>',
            '#value' => t('You may also adjust the !settings for the currently selected cache mechanism by clicking on the icon.', array('!settings' => $this->option_link(t('settings'), 'cache_options'))),
          );
        }
        break;
      case 'cache_options':
        $cache = $this->get_option('cache');
        $plugin = $this->get_cache_plugin();
        $form['#title'] .= t('Caching options');
        if ($plugin) {
          $form['#help_topic'] = $plugin->definition['help topic'];
          $form['#help_module'] = $plugin->definition['module'];

          $form['cache_options'] = array(
            '#tree' => TRUE,
          );
          $form['cache_options']['type'] = array(
            '#type' => 'value',
            '#value' => $cache['type'],
          );
          $plugin->options_form($form['cache_options'], $form_state);
        }
        break;
      case 'header':
        $form['#title'] .= t('Header');
        $form['header_empty'] = array(
          '#type' => 'checkbox',
          '#title' => t('Display even if view has no result'),
          '#default_value' => $this->get_option('header_empty'),
        );
        $form['header'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('header'),
          '#rows' => 6,
          '#description' => t('Text to display at the top of the view. May contain an explanation or links or whatever you like. Optional.'),
        );

        $form['header_format'] = filter_form($this->get_option('header_format'), NULL, array('header_format'));
        break;
      case 'footer':
        $form['#title'] .= t('Footer');
        $form['footer_empty'] = array(
          '#type' => 'checkbox',
          '#title' => t('Display even if view has no result'),
          '#default_value' => $this->get_option('footer_empty'),
        );
        $form['footer'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('footer'),
          '#rows' => 6,
          '#description' => t('Text to display beneath the view. May contain an explanation or links or whatever you like. Optional.'),
        );

        $form['footer_format'] = filter_form($this->get_option('footer_format'), NULL, array('footer_format'));
        break;
      case 'empty':
        $form['#title'] .= t('Empty text');
        $form['empty'] = array(
          '#type' => 'textarea',
          '#default_value' => $this->get_option('empty'),
          '#rows' => 6,
          '#description' => t('Text to display if the view has no results. Optional.'),
        );

        $form['empty_format'] = filter_form($this->get_option('empty_format'), NULL, array('empty_format'));
        break;
      case 'style_plugin':
        $form['#title'] .= t('How should this view be styled');
        $form['#help_topic'] = 'style';
        $form['style_plugin'] =  array(
          '#type' => 'radios',
          '#options' => views_fetch_plugin_names('style', $this->get_style_type(), array($this->view->base_table)),
          '#default_value' => $this->get_option('style_plugin'),
          '#description' => t('If the style you choose has settings, be sure to click the settings button that will appear next to it in the View summary.'),
        );

        $style_plugin = views_fetch_plugin_data('style', $this->get_option('style_plugin'));
        if (!empty($style_plugin['uses options'])) {
          $form['markup'] = array(
            '#prefix' => '<div class="form-item description">',
            '#suffix' => '</div>',
            '#value' => t('You may also adjust the !settings for the currently selected style by clicking on the icon.', array('!settings' => $this->option_link(t('settings'), 'style_options'))),
          );
        }

        break;
      case 'style_options':
        $form['#title'] .= t('Style options');
        $style = TRUE;
        $type = 'style_plugin';
        $name = $this->get_option('style_plugin');

      case 'row_options':
        if (!isset($name)) {
          $name = $this->get_option('row_plugin');
        }
        // if row, $style will be empty.
        if (empty($style)) {
          $form['#title'] .= t('Row style options');
          $type = 'row_plugin';
        }
        $plugin = $this->get_plugin(empty($style) ? 'row' : 'style');
        if ($plugin) {
          if (isset($plugin->definition['help topic'])) {
            $form['#help_topic'] = $plugin->definition['help topic'];
            $form['#help_module'] = $plugin->definition['module'];
          }
          $form[$form_state['section']] = array(
            '#tree' => TRUE,
          );
          $plugin->options_form($form[$form_state['section']], $form_state);
        }
        break;
      case 'row_plugin':
        $form['#title'] .= t('How should each row in this view be styled');
        $form['#help_topic'] = 'style-row';
        $form['row_plugin'] =  array(
          '#type' => 'radios',
          '#options' => views_fetch_plugin_names('row', $this->get_style_type(), array($this->view->base_table)),
          '#default_value' => $this->get_option('row_plugin'),
        );

        $row_plugin = views_fetch_plugin_data('row', $this->get_option('row_plugin'));
        if (!empty($row_plugin['uses options'])) {
          $form['markup'] = array(
            '#prefix' => '<div class="form-item description">',
            '#suffix' => '</div>',
            '#value' => t('You may also adjust the !settings for the currently selected row style by clicking on the icon.', array('!settings' => $this->option_link(t('settings'), 'row_options'))),
          );
        }

        break;
      case 'link_display':
        $form['#title'] .= t('Which display to use for path');
        foreach ($this->view->display as $display_id => $display) {
          if ($display->handler->has_path()) {
            $options[$display_id] = $display->display_title;
          }
        }
        $form['link_display'] = array(
          '#type' => 'radios',
          '#options' => $options,
          '#description' => t("Which display to use to get this display's path for things like summary links, rss feed links, more links, etc."),
          '#default_value' => $this->get_link_display(),
        );
        break;
      case 'analyze-theme':
        $form['#title'] .= t('Theming information');
        $form['#help_topic'] = 'analyze-theme';

        if (isset($_POST['theme'])) {
          $this->view->theme = $_POST['theme'];
        }
        else if (empty($this->view->theme)) {
          $this->view->theme = variable_get('theme_default', 'garland');
        }

        global $custom_theme;
        $custom_theme = $this->view->theme;
        init_theme();

        $funcs = array();
        // Get theme functions for the display. Note that some displays may
        // not have themes. The 'feed' display, for example, completely
        // delegates to the style.
        if (!empty($this->definition['theme'])) {
          $funcs[] = $this->option_link(t('Display output'), 'analyze-theme-display') . ': '  . $this->format_themes($this->theme_functions());
          $themes = $this->additional_theme_functions();
          if ($themes) {
            foreach ($themes as $theme) {
              $funcs[] = $this->option_link(t('Alternative display output'), 'analyze-theme-display') . ': '  . $this->format_themes($theme);
            }
          }
        }

        $plugin = $this->get_plugin();
        if ($plugin) {
          $funcs[] = $this->option_link(t('Style output'), 'analyze-theme-style') . ': ' . $this->format_themes($plugin->theme_functions(), $plugin->additional_theme_functions());
          $themes = $plugin->additional_theme_functions();
          if ($themes) {
            foreach ($themes as $theme) {
              $funcs[] = $this->option_link(t('Alternative style'), 'analyze-theme-style') . ': '  . $this->format_themes($theme);
            }
          }

          if ($plugin->uses_row_plugin()) {
            $row_plugin = $this->get_plugin('row');
            if ($row_plugin) {
              $funcs[] = $this->option_link(t('Row style output'), 'analyze-theme-row') . ': ' . $this->format_themes($row_plugin->theme_functions());
              $themes = $row_plugin->additional_theme_functions();
              if ($themes) {
                foreach ($themes as $theme) {
                  $funcs[] = $this->option_link(t('Alternative row style'), 'analyze-theme-row') . ': '  . $this->format_themes($theme);
                }
              }
            }
          }

          if ($plugin->uses_fields()) {
            foreach ($this->get_handlers('field') as $id => $handler) {
              $funcs[] = $this->option_link(t('Field @field (ID: @id)', array('@field' => $handler->ui_name(), '@id' => $id)), 'analyze-theme-field') . ': ' . $this->format_themes($handler->theme_functions());
            }
          }
        }

        $form['important'] = array(
          '#prefix' => '<div class="form-item description">',
          '#suffix' => '</div>',
          '#value' => '<p>' . t('This section lists all possible templates for the display plugin and for the style plugins, ordered roughly from the least specific to the most specific. The active template for each plugin -- which is the most specific template found on the system -- is highlighted in bold.') . '</p>',
        );

        foreach (list_themes() as $key => $theme) {
          $options[$key] = $theme->info['name'];
        }

        $form['box'] = array(
          '#prefix' => '<div class="container-inline">',
          '#suffix' => '</div>',
        );
        $form['box']['theme'] = array(
          '#type' => 'select',
          '#options' => $options,
          '#default_value' => $this->view->theme,
        );

        $form['box']['change'] = array(
          '#type' => 'submit',
          '#value' => t('Change theme'),
          '#submit' => array('views_ui_edit_display_form_change_theme'),
        );

        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => theme('item_list', $funcs),
        );

        $form['rescan_button'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
        );
        $form['rescan_button']['button'] = array(
          '#type' => 'submit',
          '#value' => t('Rescan template files'),
          '#submit' => array('views_ui_config_item_form_rescan'),
        );
        $form['rescan_button']['markup'] = array(
          '#prefix' => '<div class="description">',
          '#suffix' => '</div>',
          '#value' => t("<strong>Important!</strong> When adding, removing, or renaming template files, it is necessary to make Drupal aware of the changes by making it rescan the files on your system. By clicking this button you clear Drupal's theme registry and thereby trigger this rescanning process. The highlighted templates above will then reflect the new state of your system."),
        );

        $form_state['ok_button'] = TRUE;
        break;
      case 'analyze-theme-display':
        $form['#title'] .= t('Theming information (display)');
        $output = '<p>' . t('Back to !info.', array('!info' => $this->option_link(t('theming information'), 'analyze-theme'))) . '</p>';

        if (empty($this->definition['theme'])) {
          $output .= t('This display has no theming information');
        }
        else {
          $output .= '<p>' . t('This is the default theme template used for this display.') . '</p>';
          $output .= '<pre>' . check_plain(file_get_contents('./' . $this->definition['theme path'] . '/' . strtr($this->definition['theme'], '_', '-') . '.tpl.php')) . '</pre>';
        }

        if (!empty($this->definition['additional themes'])) {
          foreach ($this->definition['additional themes'] as $theme => $type) {
            $output .= '<p>' . t('This is an alternative template for this display.') . '</p>';
            $output .= '<pre>' . check_plain(file_get_contents('./' . $this->definition['theme path'] . '/' . strtr($theme, '_', '-') . '.tpl.php')) . '</pre>';
          }
        }

        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => $output,
        );

        $form_state['ok_button'] = TRUE;
        break;
      case 'analyze-theme-style':
        $form['#title'] .= t('Theming information (style)');
        $output = '<p>' . t('Back to !info.', array('!info' => $this->option_link(t('theming information'), 'analyze-theme'))) . '</p>';

        $plugin = $this->get_plugin();

        if (empty($plugin->definition['theme'])) {
          $output .= t('This display has no style theming information');
        }
        else {
          $output .= '<p>' . t('This is the default theme template used for this style.') . '</p>';
          $output .= '<pre>' . check_plain(file_get_contents('./' . $plugin->definition['theme path'] . '/' . strtr($plugin->definition['theme'], '_', '-') . '.tpl.php')) . '</pre>';
        }

        if (!empty($plugin->definition['additional themes'])) {
          foreach ($plugin->definition['additional themes'] as $theme => $type) {
            $output .= '<p>' . t('This is an alternative template for this style.') . '</p>';
            $output .= '<pre>' . check_plain(file_get_contents('./' . $plugin->definition['theme path'] . '/' . strtr($theme, '_', '-') . '.tpl.php')) . '</pre>';
          }
        }

        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => $output,
        );

        $form_state['ok_button'] = TRUE;
        break;
      case 'analyze-theme-row':
        $form['#title'] .= t('Theming information (row style)');
        $output = '<p>' . t('Back to !info.', array('!info' => $this->option_link(t('theming information'), 'analyze-theme'))) . '</p>';

        $plugin = $this->get_plugin('row');

        if (empty($plugin->definition['theme'])) {
          $output .= t('This display has no row style theming information');
        }
        else {
          $output .= '<p>' . t('This is the default theme template used for this row style.') . '</p>';
          $output .= '<pre>' . check_plain(file_get_contents('./' . $plugin->definition['theme path'] . '/' . strtr($plugin->definition['theme'], '_', '-') . '.tpl.php')) . '</pre>';
        }

        if (!empty($plugin->definition['additional themes'])) {
          foreach ($plugin->definition['additional themes'] as $theme => $type) {
            $output .= '<p>' . t('This is an alternative template for this row style.') . '</p>';
            $output .= '<pre>' . check_plain(file_get_contents('./' . $plugin->definition['theme path'] . '/' . strtr($theme, '_', '-') . '.tpl.php')) . '</pre>';
          }
        }

        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => $output,
        );

        $form_state['ok_button'] = TRUE;
        break;
      case 'analyze-theme-field':
        $form['#title'] .= t('Theming information (row style)');
        $output = '<p>' . t('Back to !info.', array('!info' => $this->option_link(t('theming information'), 'analyze-theme'))) . '</p>';

        $output .= '<p>' . t('This is the default theme template used for this row style.') . '</p>';

        // Field templates aren't registered the normal way...and they're always
        // this one, anyhow.
        $output .= '<pre>' . check_plain(file_get_contents(drupal_get_path('module', 'views') . '/theme/views-view-field.tpl.php')) . '</pre>';

        $form['analysis'] = array(
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
          '#value' => $output,
        );
        $form_state['ok_button'] = TRUE;
        break;

      case 'exposed_block':
        $form['#title'] .= t('Put the exposed form in a block');
        $form['description'] = array(
          '#prefix' => '<div class="description form-item">',
          '#suffix' => '</div>',
          '#value' => t('If set, any exposed widgets will not appear with this view. Instead, a block will be made available to the Drupal block administration system, and the exposed form will appear there. Note that this block must be enabled manually, Views will not enable it for you.'),
        );
        $form['exposed_block'] = array(
          '#type' => 'radios',
          '#options' => array(1 => t('Yes'), 0 => t('No')),
          '#default_value' => $this->get_option('exposed_block') ? 1 : 0,
        );
        break;
    }
  }

  /**
   * Format a list of theme templates for output by the theme info helper.
   */
  function format_themes($themes) {
    $registry = theme_get_registry();

    // Run through the theme engine variables, if necessary
    global $theme_engine;
    $extension = '.tpl.php';
    if (isset($theme_engine)) {
      $extension_function = $theme_engine . '_extension';
      if (function_exists($extension_function)) {
        $extension = $extension_function();
      }
    }

    $output = '';
    $picked = FALSE;
    foreach ($themes as $theme) {
      $template = strtr($theme, '_', '-') . $extension;
      if (!$picked && !empty($registry[$theme])) {
        $template_path = isset($registry[$theme]['path']) ? $registry[$theme]['path'] . '/' : './';
        if (file_exists($template_path . $template)) {
          $hint = t('File found in folder @template-path', array('@template-path' => $template_path));
          $template = '<strong title="'. $hint .'">' . $template . '</strong>';
        }
        else {
          $template = '<strong class="error">' . $template . ' ' . t('(File not found, in folder @template-path)', array('@template-path' => $template_path)) . '</strong>';
        }
        $picked = TRUE;
      }
      $fixed[] = $template;
    }

    return implode(', ', array_reverse($fixed));
  }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) {
    switch ($form_state['section']) {
      case 'css_class':
        $css_class = $form_state['values']['css_class'];
        if (preg_match('/[^a-zA-Z0-9- ]/', $css_class)) {
          form_error($form['css_class'], t('CSS classes must be alphanumeric or dashes only.'));
        }
      break;
      case 'style_options':
        $style = TRUE;
      case 'row_options':
        // if row, $style will be empty.
        $plugin = $this->get_plugin(empty($style) ? 'row' : 'style');
        if ($plugin) {
          $plugin->options_validate($form[$form_state['section']], $form_state);
        }
        break;
      case 'access_options':
        $plugin = $this->get_access_plugin();
        if ($plugin) {
          $plugin->options_validate($form['access_options'], $form_state);
        }
        break;
      case 'cache_options':
        $plugin = $this->get_cache_plugin();
        if ($plugin) {
          $plugin->options_validate($form['cache_options'], $form_state);
        }
        break;
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit(&$form, &$form_state) {
    // Not sure I like this being here, but it seems (?) like a logical place.
    $cache_plugin = $this->get_cache_plugin();
    if ($cache_plugin) {
      $cache_plugin->cache_flush();
    }

    $section = $form_state['section'];
    switch ($section) {
      case 'display_title':
        $this->display->display_title = $form_state['values']['display_title'];
        break;
      case 'access':
        $access = $this->get_option('access');
        if ($access['type'] != $form_state['values']['access']['type']) {
          $plugin = views_get_plugin('access', $form_state['values']['access']['type']);
          if ($plugin) {
            $access = array('type' => $form_state['values']['access']['type']);
            $plugin->option_defaults($access);
            $this->set_option('access', $access);
            if (!empty($plugin->definition['uses options'])) {
              views_ui_add_form_to_stack('display', $this->view, $this->display->id, array('access_options'));
            }
          }
        }

        break;
      case 'access_options':
        $plugin = views_get_plugin('access', $form_state['values'][$section]['type']);
        if ($plugin) {
          $plugin->options_submit($form['access_options'], $form_state);
          $this->set_option('access', $form_state['values'][$section]);
        }
        break;
      case 'cache':
        $cache = $this->get_option('cache');
        if ($cache['type'] != $form_state['values']['cache']['type']) {
          $plugin = views_get_plugin('cache', $form_state['values']['cache']['type']);
          if ($plugin) {
            $cache = array('type' => $form_state['values']['cache']['type']);
            $plugin->option_defaults($cache);
            $this->set_option('cache', $cache);
            if (!empty($plugin->definition['uses options'])) {
              views_ui_add_form_to_stack('display', $this->view, $this->display->id, array('cache_options'));
            }
          }
        }

        break;
      case 'cache_options':
        $plugin = views_get_plugin('cache', $form_state['values'][$section]['type']);
        if ($plugin) {
          $plugin->options_submit($form['cache_options'], $form_state);
          $this->set_option('cache', $form_state['values'][$section]);
        }
        break;
      case 'title':
      case 'css_class':
      case 'link_display':
        $this->set_option($section, $form_state['values'][$section]);
        break;
      case 'use_ajax':
        $this->set_option($section, (bool)$form_state['values'][$section]);
        break;
      case 'use_pager':
        $this->set_option($section, $form_state['values'][$section]);
        $this->set_option('pager_element', intval($form_state['values']['pager_element']));
        break;
      case 'items_per_page':
        $this->set_option($section, intval($form_state['values'][$section]));
        $this->set_option('offset', intval($form_state['values']['offset']));
        break;
      case 'use_more':
        $this->set_option($section, intval($form_state['values'][$section]));
        $this->set_option('use_more_always', intval($form_state['values']['use_more_always']));
        $this->set_option('use_more_text', $form_state['values']['use_more_text']);
      case 'distinct':
        $this->set_option($section, $form_state['values'][$section]);
        break;
      case 'row_plugin':
        // This if prevents resetting options to default if they don't change
        // the plugin.
        if ($this->get_option($section) != $form_state['values'][$section]) {
          $plugin = views_get_plugin('row', $form_state['values'][$section]);
          if ($plugin) {
            $this->set_option($section, $form_state['values'][$section]);
            $this->set_option('row_options', array());

            // send ajax form to options page if we use it.
            if (!empty($plugin->definition['uses options'])) {
              views_ui_add_form_to_stack('display', $this->view, $this->display->id, array('row_options'));
            }
          }
        }
        break;
      case 'style_plugin':
        // This if prevents resetting options to default if they don't change
        // the plugin.
        if ($this->get_option($section) != $form_state['values'][$section]) {
          $plugin = views_get_plugin('style', $form_state['values'][$section]);
          if ($plugin) {
            $this->set_option($section, $form_state['values'][$section]);
            $this->set_option('style_options', array());
            // send ajax form to options page if we use it.
            if (!empty($plugin->definition['uses options'])) {
              views_ui_add_form_to_stack('display', $this->view, $this->display->id, array('style_options'));
            }
          }
        }
        break;
      case 'style_options':
        $style = TRUE;
      case 'row_options':
        // if row, $style will be empty.
        $plugin = $this->get_plugin(empty($style) ? 'row' : 'style');
        if ($plugin) {
          $plugin->options_submit($form[$section], $form_state);
        }
        $this->set_option($section, $form_state['values'][$section]);
        break;
      case 'header':
      case 'footer':
      case 'empty':
        $this->set_option($section, $form_state['values'][$section]);
        $this->set_option($section . '_format', $form_state['values'][$section . '_format']);
        if ($section != 'empty') {
          $this->set_option($section . '_empty', $form_state['values'][$section . '_empty']);
        }
        break;
      case 'exposed_block':
        $this->set_option($section, (bool) $form_state['values'][$section]);
        break;
    }
  }

  /**
   * Add an override button for a given section, allowing the user to
   * change whether this info is stored on the default display or on
   * the current display.
   */
  function add_override_button(&$form, &$form_state, $section) {
    if ($this->is_default_display()) {
      return;
    }

    $form['override'] = array(
      '#prefix' => '<div class="views-override clear-block">',
      '#suffix' => '</div>',
    );
    if ($this->is_defaulted($section)) {
      $form['override']['button'] = array(
        '#type' => 'submit',
        '#value' => t('Override'),
        '#submit' => array('views_ui_edit_display_form_override'),
      );
      $form['override']['markup'] = array(
        '#prefix' => '<div class="description">',
        '#value' => theme('advanced_help_topic', 'views', 'overrides') . t('Status: using default values.'),
        '#suffix' => '</div>',
      );

      $form_state['update_name'] = t('Update default display');
    }
    else {
      $form['override']['button'] = array(
        '#type' => 'submit',
        '#value' => t('Use default'),
        '#submit' => array('views_ui_edit_display_form_override'),
      );
      $form['override']['markup'] = array(
        '#prefix' => '<div class="description">',
        '#value' => theme('advanced_help_topic', 'views', 'overrides') . t('Status: using overridden values.'),
        '#suffix' => '</div>',
      );

      $form_state['update_name'] = NULL;
    }
  }

  /**
   * If override/revert was clicked, perform the proper toggle.
   */
  function options_override($form, &$form_state) {
    $this->set_override($form_state['section']);
  }

  /**
   * Flip the override setting for the given section.
   */
  function set_override($section, $new_state = NULL) {
    $options = $this->defaultable_sections($section);
    if (!$options) {
      return;
    }

    if (!isset($new_state)) {
      $new_state = empty($this->options['defaults'][$section]);
    }

    // For each option that is part of this group, fix our settings.
    foreach ($options as $option) {
      if ($new_state) {
        // Revert to defaults.
        unset($this->options[$option]);
        unset($this->display->display_options[$option]);
      }
      else {
        // copy existing values into our display.
        $this->options[$option] = $this->get_option($option);
        $this->display->display_options[$option] = $this->options[$option];
      }
      $this->options['defaults'][$option] = $new_state;
      $this->display->display_options['defaults'][$option] = $new_state;
    }
  }

  /**
   * Inject anything into the query that the display handler needs.
   */
  function query() {
    // Make the query distinct if the option was set.
    if ($this->get_option('distinct')) {
      $this->view->query->set_distinct();
    }
  }

  /**
   * Not all display plugins will support filtering
   */
  function render_filters() { }

  /**
   * Render the 'more' link
   */
  function render_more_link() {
    if ($this->use_more() && ($this->use_more_always() || !isset($this->view->total_rows) || $this->view->total_rows > $this->view->pager['items_per_page'])) {
      $path = $this->get_path();
      if ($path) {
        $path = $this->view->get_url(NULL, $path);
        $url_options = array();
        if (!empty($this->view->exposed_raw_input)) {
          $url_options['query'] = $this->view->exposed_raw_input;
        }
        $theme = views_theme_functions('views_more', $this->view, $this->display);
        $path = check_url(url($path, $url_options));
        return theme($theme, $path, check_plain($this->use_more_text()));
      }
    }
  }

  /**
   * Render a text area, using the proper format.
   */
  function render_textarea($area) {
    static $formats = array();

    $value = $this->get_option($area);
    // Check to make sure the filter format exists; if not, we don't
    // display anything.
    $format = filter_resolve_format($this->get_option($area . '_format'));

    if (!array_key_exists($format, $formats)) {
      $formats[$format] = db_result(db_query("SELECT name FROM {filter_formats} WHERE format = %d", $format));
    }

    if (!$formats[$format]) {
      return;
    }

    if ($value) {
      return check_markup($value, $format, FALSE);
    }
  }

  /**
   * Render the header of the view.
   */
  function render_header() {
    if (!empty($this->view->result) || $this->get_option('header_empty')) {
      return $this->render_textarea('header');
    }
  }

  /**
   * Render the footer of the view.
   */
  function render_footer() {
    if (!empty($this->view->result) || $this->get_option('footer_empty')) {
      return $this->render_textarea('footer');
    }
  }

  /**
   * Render the empty text of the view.
   */
  function render_empty() { return $this->render_textarea('empty'); }
  /**
   * If this display creates a block, implement one of these.
   */
  function hook_block($op = 'list', $delta = 0, $edit = array()) { return array(); }

  /**
   * If this display creates a page with a menu item, implement it here.
   */
  function hook_menu() { return array(); }

  /**
   * Render this display.
   */
  function render() {
    return theme($this->theme_functions(), $this->view);
  }

  /**
   * Determine if the user has access to this display of the view.
   */
  function access($account = NULL) {
    if (!isset($account)) {
      global $user;
      $account = $user;
    }

    // Full override.
    if (user_access('access all views', $account)) {
      return TRUE;
    }

    $plugin = $this->get_access_plugin();
    if ($plugin) {
      return $plugin->access($account);
    }

    // fallback to all access if no plugin.
    return TRUE;
  }

  /**
   * Set up any variables on the view prior to execution. These are separated
   * from execute because they are extremely common and unlikely to be
   * overridden on an individual display.
   */
  function pre_execute() {
    $this->view->set_use_ajax($this->use_ajax());
    // Copy pager information from the display.
    $this->view->set_use_pager($this->use_pager());
    $this->view->set_pager_element($this->get_option('pager_element'));
    $this->view->set_items_per_page($this->get_option('items_per_page'));
    $this->view->set_offset($this->get_option('offset'));
    if ($this->use_more()) {
      $this->view->get_total_rows = TRUE;
    }
  }

  /**
   * When used externally, this is how a view gets run and returns
   * data in the format required.
   *
   * The base class cannot be executed.
   */
  function execute() { }

  /**
   * Fully render the display for the purposes of a live preview or
   * some other AJAXy reason.
   */
  function preview() { return $this->view->render(); }

  /**
   * Displays can require a certain type of style plugin. By default, they will
   * be 'normal'.
   */
  function get_style_type() { return 'normal'; }

  /**
   * Make sure the display and all associated handlers are valid.
   *
   * @return
   *   Empty array if the display is valid; an array of error strings if it is not.
   */
  function validate() {
    $errors = array();
    // Make sure displays that use fields HAVE fields.
    if ($this->uses_fields()) {
      $fields = FALSE;
      foreach ($this->get_handlers('field') as $field) {
        if (empty($field->options['exclude'])) {
          $fields = TRUE;
        }
      }

      if (!$fields) {
        $errors[] = t('Display "@display" uses fields but there are none defined for it or all are excluded.', array('@display' => $this->display->display_title));
      }
    }

    if ($this->has_path() && !$this->get_option('path')) {
      $errors[] = t('Display "@display" uses a path but the path is undefined.', array('@display' => $this->display->display_title));
    }

    // Validate style plugin
    $style = $this->get_plugin();
    if (empty($style)) {
      $errors[] = t('Display "@display" has an invalid style plugin.', array('@display' => $this->display->display_title));
    }
    else {
      $result = $style->validate();
      if (!empty($result) && is_array($result)) {
        $errors = array_merge($errors, $result);
      }
    }

    // Validate handlers
    foreach (views_object_types() as $type => $info) {
      foreach ($this->get_handlers($type) as $handler) {
        $result = $handler->validate();
        if (!empty($result) && is_array($result)) {
          $errors = array_merge($errors, $result);
        }
      }
    }

    return $errors;
  }

  /**
   * Check if the provided identifier is unique.
   */
  function is_identifier_unique($id, $identifier) {
    foreach (views_object_types() as $type => $info) {
      foreach ($this->get_handlers($type) as $key => $handler) {
        if ($handler->can_expose() && $handler->is_exposed()) {
          if ($id != $key && $identifier == $handler->options['expose']['identifier']) {
            return FALSE;
          }
        }
      }
    }
    return TRUE;
  }

  /**
   * Provide the block system with any exposed widget blocks for this display.
   */
  function get_special_blocks() {
    $delta = '-exp-' . $this->view->name . '-' . $this->display->id;
    $desc = t('Exposed form: @view-@display_id', array('@view' => $this->view->name, '@display_id' => $this->display->id));

    return array(
      $delta => array(
        'info' => $desc,
      )
    );
  }

  /**
   * Render any special blocks provided for this display.
   */
  function view_special_blocks($type) {
    if ($type == '-exp') {
      // avoid interfering with the admin forms.
      if (arg(0) == 'admin' && arg(1) == 'build' && arg(2) == 'views') {
        return;
      }
      $this->view->init_handlers();
      return array(
        'content' => $this->view->render_exposed_form(TRUE),
      );
    }
  }

}


/**
 * @}
 */

