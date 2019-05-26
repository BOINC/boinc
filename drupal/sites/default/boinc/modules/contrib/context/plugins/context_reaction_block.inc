<?php

/**
 * Expose blocks as context reactions.
 */
class context_reaction_block extends context_reaction {
  /**
   * Options form.
   */
  function options_form($context) {
    // Rebuild the block info cache if necessary.
    $this->get_blocks(NULL, NULL, $this->rebuild_needed());
    $this->rebuild_needed(FALSE);

    $theme_key = variable_get('theme_default', 'garland');
    $weight_delta = $this->max_block_weight();

    $form = array(
      '#tree' => TRUE,
      '#theme' => 'context_block_form',
      'max_block_weight' => array(
        '#value' => $weight_delta,
        '#type' => 'value',
      ),
      'state' => array(
        '#type' => 'hidden',
        '#attributes' => array('class' => 'context-blockform-state'),
      ),
    );

    /**
     * Selector.
     */
    $modules = module_list();
    $form['selector'] = array(
      '#type' => 'item',
      '#tree' => TRUE,
      '#prefix' => '<div class="context-blockform-selector">',
      '#suffix' => '</div>',
    );
    foreach ($this->get_blocks() as $block) {
      if (!isset($form['selector'][$block->module])) {
        $form['selector'][$block->module] = array(
          '#type' => 'checkboxes',
          '#title' => $modules[$block->module],
          '#options' => array(),
        );
      }
      $form['selector'][$block->module]['#options'][$block->bid] = check_plain($block->info);
    }
    ksort($form['selector']);

    /**
     * Regions.
     */
    $form['blocks'] = array(
      '#tree' => TRUE,
      '#theme' => 'context_block_regions_form',
    );
    foreach (system_region_list($theme_key) as $region => $label) {
      $form['blocks'][$region] = array(
        '#type' => 'item',
        '#title' => $label,
        '#tree' => TRUE,
      );
      foreach ($this->get_blocks($region, $context) as $block) {
        if (!empty($block->context)) {
          $form['blocks'][$region][$block->bid] = array(
            '#value' => check_plain($block->info),
            '#weight' => $block->weight,
            '#type' => 'markup',
            '#tree' => TRUE,
            'weight' => array('#type' => 'weight', '#delta' => $weight_delta, '#default_value' => 0),
          );
        }
      }
    }
    return $form;
  }

  /**
   * Options form submit handler.
   */
  function options_form_submit($values) {
    $blocks = array();
    $block_info = $this->get_blocks();

    // Retrieve blocks from submitted JSON string.
    if (!empty($values['state'])) {
      $edited = $this->json_decode($values['state']);
    }
    else {
      $edited = array();
    }

    foreach ($edited as $region => $bids) {
      foreach ($bids as $position => $bid) {
        if (isset($block_info[$bid])) {
          $blocks[$bid] = array(
            'module' => $block_info[$bid]->module,
            'delta' => $block_info[$bid]->delta,
            'region' => $region,
            'weight' => $position,
          );
        }
      }
    }
    return array('blocks' => $blocks);
  }

  /**
   * Context editor form for blocks.
   */
  function editor_form($context) {
    $form = array();
    if (module_exists('jquery_ui')) {
      jquery_ui_add(array('ui.draggable', 'ui.droppable', 'ui.sortable'));
      drupal_add_js(drupal_get_path('module', 'context_ui') .'/json2.js');
      drupal_add_js(drupal_get_path('module', 'context') .'/plugins/context_reaction_block.js');
      drupal_add_css(drupal_get_path('module', 'context') .'/plugins/context_reaction_block.css');

      // We might be called multiple times so use a static to ensure this is set just once.
      static $once;
      if (!isset($once)) {
        $settings = array(
          'path' => url($_GET['q']),
          'params' => (object) array_diff_key($_GET, array('q' => '')),
          'scriptPlaceholder' => theme('context_block_script_placeholder', ''),
        );
        drupal_add_js(array('contextBlockEditor' => $settings), 'setting');
        $once = TRUE;
      }

      $form['state'] = array(
        '#type' => 'hidden',
        '#attributes' => array('class' => 'context-block-editor-state'),
      );
      $form['browser'] = array(
        '#type' => 'markup',
        '#value' => theme('context_block_browser', $this->get_blocks(NULL, NULL, $this->rebuild_needed()), $context),
      );
      $this->rebuild_needed(FALSE);
    }
    return $form;
  }

  /**
   * Submit handler context editor form.
   */
  function editor_form_submit(&$context, $values) {
    $edited = !empty($values['state']) ? (array) $this->json_decode($values['state']) : array();

    $options = array();

    // Take the existing context values and remove blocks that belong affected regions.
    $affected_regions = array_keys($edited);
    if (!empty($context->reactions['block']['blocks'])) {
      $options = $context->reactions['block'];
      foreach ($options['blocks'] as $key => $block) {
        if (in_array($block['region'], $affected_regions)) {
          unset($options['blocks'][$key]);
        }
      }
    }

    // Iterate through blocks and add in the ones that belong to the context.
    foreach ($edited as $region => $blocks) {
      foreach ($blocks as $weight => $block) {
        if ($block->context === $context->name) {
          $split = explode('-', $block->bid);
          $options['blocks'][$block->bid] = array(
            'module' => array_shift($split),
            'delta' => implode('-', $split),
            'region' => $region,
            'weight' => $weight,
          );
        }
      }
    }

    return $options;
  }

  /**
   * Settings form for variables.
   */
  function settings_form() {
    $form = array();
    $form['context_reaction_block_disable_core'] = array(
      '#title' => t('Core block system'),
      '#type' => 'select',
      '#options' => array(
        FALSE => t('Enabled'),
        TRUE => t('Disabled'),
      ),
      '#default_value' => variable_get('context_reaction_block_disable_core', FALSE),
      '#description' => t('If enabled Context will include blocks enabled through the core block system when rendering regions. Disable to improve performance and hide the core block administration UI.')
    );
    $form['context_reaction_block_all_regions'] = array(
      '#title' => t('Show all regions'),
      '#type' => 'checkbox',
      '#default_value' => variable_get('context_reaction_block_all_regions', FALSE),
      '#description' => t('Show all regions including those that are empty. Enable if you are administering your site using the inline editor.')
    );
    return $form;
  }

  /**
   * Execute.
   */
  function execute($region) {
    // If the context_block querystring param is set, switch to AJAX rendering.
    // Note that we check the output buffer for any content to ensure that we
    // are not in the middle of a PHP template render.
    if (isset($_GET['context_block']) && !ob_get_contents()) {
      return $this->render_ajax($_GET['context_block']);
    }
    if ($this->is_enabled_region($region)) {
      return theme('context_block_editable_region', $region, $this->block_list($region), $this->is_editable());
    }
    return '';
  }

  /**
   * Determine whether inline editing requirements are met and that the current
   * user may edit.
   */
  protected function is_editable($reset = FALSE) {
    // Check requirements.
    // Generally speaking, it does not make sense to allow anonymous users to
    // edit a context inline. Though it may be possible to save (and restore)
    // an edited context to an anonymous user's cookie or session, it's an
    // edge case and probably not something we want to encourage anyway.
    static $editable;
    if (!isset($editable) || $reset) {
      global $user;
      if (module_exists('jquery_ui') && $user->uid) {
        $editable = TRUE;
        jquery_ui_add(array('ui.draggable', 'ui.droppable', 'ui.sortable'));
        drupal_add_js(drupal_get_path('module', 'context') .'/plugins/context_reaction_block.js');
        drupal_add_css(drupal_get_path('module', 'context') .'/plugins/context_reaction_block.css');
      }
      else {
        $editable = FALSE;
      }
    }
    return $editable;
  }

  /**
   * Return a list of enabled regions for which blocks should be built.
   * Split out into a separate method for easy overrides in extending classes.
   */
  protected function is_enabled_region($region) {
    global $theme;
    $regions = array_keys(system_region_list($theme));
    return in_array($region, $regions, TRUE);
  }

  /**
  * An alternative version of block_list() that provides any context enabled blocks.
  */
  function block_list($region) {
    static $build_queue;
    static $build_contexts;
    static $context_blocks = array();
    static $empty_blocks = array();

    // Static cache a region => block array of blocks that should be built *if*
    // the given region is requested. Note that the blocks are not themselves
    // built in this first run as not all regions may be requested even if
    // active contexts include blocks in those regions.
    $contexts = context_active_contexts();
    if (!isset($build_queue) || $build_contexts != array_keys($contexts)) {
      $info = $this->get_blocks();
      $build_queue = array();
      $build_contexts = array_keys($contexts);
      foreach ($contexts as $context) {
        $options = $this->fetch_from_context($context);
        if (!empty($options['blocks'])) {
          foreach ($options['blocks'] as $block) {
            $block = (object) $block;
            $block->context = $context->name;
            $block->bid = "{$block->module}-{$block->delta}";
            $block->cache = isset($info[$block->bid]->cache) ? $info[$block->bid]->cache : BLOCK_NO_CACHE;
            $build_queue[$block->region][] = $block;
          }
        }
      }
    }

    // Context blocks.
    if (!isset($context_blocks[$region])) {
      $context_blocks[$region] = array();
      $empty_blocks[$region] = array();
      if (!empty($build_queue[$region])) {
        foreach ($build_queue[$region] as $block) {
          $block = $this->build_block($block);
          if (!empty($block->content)) {
            $context_blocks[$region][] = $block;
          }
          else {
            $empty_blocks[$region][] = $block;
          }
        }
      }
    }

    // Get core blocks only if enabled.
    $blocks = !variable_get('context_reaction_block_disable_core', FALSE) ? block_list($region) : array();
    $blocks = array_merge($blocks, $context_blocks[$region]);

    // Only include empty blocks if all regions should be shown or there are
    // non-empty blocks in the same region.
    $all_regions = variable_get('context_reaction_block_all_regions', FALSE);
    if ($this->is_editable() && ($all_regions || !empty($blocks))) {
      $blocks = array_merge($blocks, $empty_blocks[$region]);
    }

    // Sort everything.
    uasort($blocks, array('context_reaction_block', 'block_sort'));
    return $blocks;
  }

  /**
   * Build a block's content. Largely taken from block_list().
   */
  protected function build_block($block, $reset = FALSE) {
    // Block caching is not compatible with node_access modules. We also
    // preserve the submission of forms in blocks, by fetching from cache
    // only if the request method is 'GET'.
    static $cacheable;
    if (!isset($cacheable) || $reset) {
      $cacheable = !count(module_implements('node_grants')) && $_SERVER['REQUEST_METHOD'] == 'GET';
    }

    if (!isset($block->content)) {
      $block->content = '';
      // Try fetching the block from cache.
      if ($cacheable && ($cid = _block_get_cache_id($block))) {
        if ($cache = cache_get($cid, 'cache_block')) {
          $array = $cache->data;
        }
        else {
          $array = module_invoke($block->module, 'block', 'view', $block->delta);
          cache_set($cid, $array, 'cache_block', CACHE_TEMPORARY);
        }
      }
      // Otherwise build block.
      else {
        $array = module_invoke($block->module, 'block', 'view', $block->delta);
      }
      if (isset($array) && is_array($array)) {
        foreach ($array as $k => $v) {
          $block->$k = $v;
        }
      }
    }
    if (!empty($block->content)) {
      // Only query for custom block title if block core compatibility is enabled.
      if (!variable_get('context_reaction_block_disable_core', FALSE)) {
        global $user, $theme_key;
        $block->title = db_result(db_query("SELECT title FROM {blocks} WHERE module = '%s' AND delta = '%s' AND theme = '%s'", $block->module, $block->delta, $theme_key));
      }
      // Override default block title if a custom display title is present.
      if (!empty($block->title)) {
        // Check plain here to allow module generated titles to keep any markup.
        $block->subject = $block->title == '<none>' ? '' : check_plain($block->title);
      }
      if (!isset($block->subject)) {
        $block->subject = '';
      }
    }
    return $block;
  }

  /**
   * Generate the safe weight range for a block being added to a region such that
   * there are enough potential unique weights to support all blocks.
   */
  protected function max_block_weight() {
    $blocks = $this->get_blocks();
    $block_count = 0;
    foreach ($blocks as $region => $block_list) {
      $block_count += count($block_list);
    }
    // Add 2 to make sure there's space at either end of the block list
    return round(($block_count + 2) / 2);
  }

  /**
   * Check or set whether a rebuild of the block info cache is needed.
   */
  function rebuild_needed($set = NULL) {
    if (isset($set) && $set != variable_get('context_block_rebuild_needed', FALSE)) {
      variable_set('context_block_rebuild_needed', $set);
    }
    return (bool) variable_get('context_block_rebuild_needed', FALSE);
  }

  /**
   * Helper function to generate a list of blocks from a specified region. If provided a context object,
   * will generate a full list of blocks for that region distinguishing between system blocks and
   * context-provided blocks.
   *
   * @param $region
   *   The string identifier for a theme region. e.g. "left"
   * @param $context
   *   A context object.
   *
   * @return
   *   A keyed (by "module_delta" convention) array of blocks.
   */
  function get_blocks($region = NULL, $context = NULL, $reset = FALSE) {
    static $block_info;

    if (!isset($block_info) || $reset) {
      $block_info = array();
      if (!$reset) {
        $block_info = context_cache_get('block_info');
      }
      if (empty($block_info)) {
        $block_info = array();
        foreach (module_implements('block') as $module) {
          $module_blocks = module_invoke($module, 'block', 'list');
          if (!empty($module_blocks)) {
            foreach ($module_blocks as $delta => $block) {
              $block = (object) $block;
              $block->module = $module;
              $block->delta = $delta;
              $block->bid = "{$block->module}-{$block->delta}";
              $block_info[$block->bid] = $block;
            }
          }
        }
        context_cache_set('block_info', $block_info);
      }
      // Allow other modules that may declare blocks dynamically to alter
      // this list.
      drupal_alter('context_block_info', $block_info);

      // Gather only region info from the database.
      $theme_key = variable_get('theme_default', 'garland');
      $result = db_query("SELECT module,weight,delta,region,status FROM {blocks} WHERE theme = '%s' ORDER BY weight ASC", $theme_key);
      while ($row = db_fetch_object($result)) {
        if ($row->status && isset($block_info["{$row->module}-{$row->delta}"])) {
          $block_info["{$row->module}-{$row->delta}"]->weight = $row->weight;
          $block_info["{$row->module}-{$row->delta}"]->region = $row->region;
        }
      }
    }

    $blocks = array();

    // No region specified, provide all blocks.
    if (!isset($region)) {
      $blocks = $block_info;
    }
    // Region specified.
    else {
      foreach ($block_info as $bid => $block) {
        if (isset($block->region) && $block->region == $region) {
          $blocks[$bid] = $block;
        }
      }
    }

    // Add context blocks if provided.
    if (is_object($context) && $options = $this->fetch_from_context($context)) {
      if (!empty($options['blocks'])) {
        foreach ($options['blocks'] as $block) {
          if (
            isset($block_info["{$block['module']}-{$block['delta']}"]) && // Block is valid.
            (!isset($region) || (!empty($region) && $block['region'] == $region)) // No region specified or regions match.
          ) {
            $context_block = $block_info["{$block['module']}-{$block['delta']}"];
            $context_block->weight = $block['weight'];
            $context_block->region = $block['region'];
            $context_block->context = $context->name;
            $blocks[$context_block->bid] = $context_block;
          }
        }
      }
      uasort($blocks, array('context_reaction_block', 'block_sort'));
    }
    return $blocks;
  }

  /**
   * Sort callback.
   */
  static function block_sort($a, $b) {
    return ($a->weight - $b->weight);
  }

  /**
   * Compatibility wrapper around json_decode().
   */
  protected function json_decode($json, $assoc = FALSE) {
    // Requires PHP 5.2.
    if (function_exists('json_decode')) {
      return json_decode($json, $assoc);
    }
    else {
      watchdog('context', 'Please upgrade your PHP version to one that supports json_decode.');
    }
  }

  /**
   * Block renderer for AJAX requests. Triggered when $_GET['context_block']
   * is set. See ->execute() for how this is called.
   */
  function render_ajax($param) {
    // Besure the page isn't a 404 or 403.
    $headers = drupal_set_header();

    // Support for Pressflow drupal_set_header.
    if (!is_array($headers)) {
      $headers = explode("\n", $headers);
    }

    foreach ($headers as $header) {
      if (strpos($header, "404 Not Found") !== FALSE || strpos($header, "403 Forbidden") !== FALSE) {
        return;
      }
    }
    // Set the header right away. This will inform any players in the stack
    // that we are in the middle of responding to an AJAX request.
    drupal_set_header('Content-Type: text/javascript; charset=utf-8');

    if (strpos($param, ',') !== FALSE) {
      list($bid, $context) = explode(',', $param);
      list($module, $delta) = explode('-', $bid, 2);
      // Check token to make sure user has access to block.
      if (!(user_access('context ajax block access') || $this->context_block_ajax_rendering_allowed($bid))) {
        echo drupal_to_js(array('status' => 0));
        exit;
      }

      // Ensure $bid is valid.
      $info = $this->get_blocks();
      if (isset($info[$bid])) {
        $block = $info[$bid];
        $block->context = $context;
        $block->region = '';
        $block = $this->build_block($block);

        if (empty($block->content)) {
          $block->content = "<div class='context-block-empty'>". t('This block appears empty when displayed on this page.') ."</div>";
        }

        $css = array();

        // On behalf of panels we try to load some css, but we don't support
        // panels inside panels currently.
        if ($block->module == 'panels_mini') {
          $panel = panels_mini_load($block->delta);
          $layout = panels_get_layout($panel->display->layout);
          if (!empty($layout['css'])) {
            if (file_exists(path_to_theme() . '/' . $layout['css'])) {
              $css[] = path_to_theme() . '/' . $layout['css'];
            }
            else {
              $css[] = $layout['path'] . '/' . $layout['css'];
            }
          }
        }
        echo drupal_to_js(array(
          'status' => 1,
          'block' => theme('context_block_editable_block', $block),
          'css' => $css,
        ));
        exit;
      }
    }
    echo drupal_to_js(array('status' => 0));
    exit;
  }

  /**
   * Allow modules to selectively allow ajax rendering of a specific block
   */
  private function context_block_ajax_rendering_allowed($bid) {
    $allowed = FALSE;
    foreach (module_invoke_all('context_allow_ajax_block_access', $bid) as $module_allow) {
      $allowed = $allow || $module_allow;
      if ($allowed) {
        break;
      }
    }
    return $allowed;
  }
}
