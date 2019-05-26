<?php


/**
 * @file
 * Functions used by more than one panels client module.
 */

/**
 * Class definition for the allowed layouts governing structure.
 *
 * @ingroup mainapi
 *
 * This class is designed to handle panels allowed layouts data from start to finish, and sees
 * action at two times:\n
 *    - When a client module wants to generate a form allowing an admin to create or edit a set
 *      of allowed layouts. In this case, either a new panels_allowed_layouts object is created
 *      or one is retrieved from storage and panels_allowed_layouts::set_allowed() is called to
 *      generate the allowed layouts form. \n
 *    - When a client module is calling panels_edit_layout(), a saved instantiation of this object
 *      can be called up and passed in to the fourth parameter, and only the allowed layouts saved
 *      in that object will be displayed on the form. \n
 * Because the panels API does not impose a data structure on the allowed_layouts data, client
 * modules can create as many of these objects as they want, and organize them around any concept:
 * node types, date published, author roles...anything.
 *
 * To call the settings form, instantiate this class - or, if your client module's needs are
 * heavy-duty, extend this class and instantiate your subclass - assign values to any relevant
 * desired members, and call panels_allowed_layouts::set_allowed(). See the documentation on
 * that method for a sample implementation.
 *
 * Note that when unserializing saved tokens of this class, you must
 * run panels_load_include('common') before unserializing in order to ensure
 * that the object is properly loaded.
 *
 * Client modules extending this class should implement a save() method and use it for
 * their custom data storage routine. You'll need to rewrite other class methods if
 * you choose to go another route.
 *
 * @see panels_edit_layout()
 * @see _panels_edit_layout()
 *
 */
class panels_allowed_layouts {

  /**
   *  Specifies whether newly-added layouts (as in, new .inc files) should be automatically
   *  allowed (TRUE) or disallowed (FALSE) for $this. Defaults to TRUE, which is more
   *  permissive but less of an administrative hassle if/when you add new layouts. Note
   *  that this parameter will be derived from $allowed_layouts if a value is passed in.
   */
  var $allow_new = TRUE;

  /**
   *  Optional member. If provided, the Panels API will generate a drupal variable using
   *  variable_set($module_name . 'allowed_layouts', serialize($this)), thereby handling the
   *  storage of this object entirely within the Panels API. This object will be
   *  called and rebuilt by panels_edit_layout() if the same $module_name string is passed in
   *  for the $allowed_types parameter. \n
   *  This is primarily intended for convenience - client modules doing heavy-duty implementations
   *  of the Panels API will probably want to create their own storage method.
   * @see panels_edit_layout()
   */
  var $module_name = NULL;

  /**
   *  An associative array of all available layouts, keyed by layout name (as defined
   *  in the corresponding layout plugin definition), with value = 1 if the layout is
   *  allowed, and value = 0 if the layout is not allowed.
   *  Calling array_filter(panels_allowed_layouts::$allowed_layout_settings) will return an associative array
   *  containing only the allowed layouts, and wrapping that in array_keys() will
   *  return an indexed version of that array.
   */
  var $allowed_layout_settings = array();

  /**
   * Hack-imitation of D6's $form_state. Used by the panels_common_set_allowed_types()
   * form to indicate whether the returned value is in its 'render', 'failed-validate',
   * or 'submit' stage.
   */
  var $form_state;

  /**
   * Constructor function; loads the $allowed_layout_settings array with initial values according
   * to $start_allowed
   *
   * @param bool $start_allowed
   *  $start_allowed determines whether all available layouts will be marked
   *  as allowed or not allowed on the initial call to panels_allowed_layouts::set_allowed()
   *
   */
  function __construct($start_allowed = TRUE) {
    // TODO would be nice if there was a way to just fetch the names easily
    foreach ($this->list_layouts() as $layout_name) {
      $this->allowed_layout_settings[$layout_name] = $start_allowed ? 1 : 0;
    }
  }

  function panels_allowed_layouts($start_allowed = TRUE) {
    self::__construct($start_allowed);
  }

  /**
   * Manage panels_common_set_allowed_layouts(), the FAPI code for selecting allowed layouts.
   *
   * MAKE SURE to set panels_allowed_layouts::allow_new before calling this method. If you want the panels API
   * to handle saving these allowed layout settings, panels_allowed_layouts::module_name must also be set.
   *
   * Below is a sample implementation; refer to the rest of the class documentation to understand all the
   * specific pieces. Values that are intended to be replaced are wrapped with <>.
   *
   * \n @code
   *  function docdemo_allowed_layouts() {
   *    ctools_include('common', 'panels');
   *    if (!is_a($allowed_layouts = unserialize(variable_get('panels_common_allowed_layouts', serialize(''))), 'panels_allowed_layouts')) {
   *     $allowed_layouts = new panels_allowed_layouts();
   *      $allowed_layouts->allow_new = TRUE;
   *      $allowed_layouts->module_name = '<client_module_name>';
   *    }
   *    $result = $allowed_layouts->set_allowed('<Desired client module form title>');
   *    if (in_array($allowed_layouts->form_state, array('failed-validate', 'render'))) {
   *     return $result;
   *    }
   *    elseif ($allowed_layouts->form_state == 'submit') {
   *      drupal_goto('</path/to/desired/redirect>');
   *    }
   *  }
   * @endcode \n
   *
   * If $allowed_layouts->form_state == 'failed-validate' || 'render', then you'll need to return
   * $result as it contains the structured form HTML generated by drupal_render_form() and is ready
   * to be passed through index.php's call to theme('page', ...).
   *
   * However, if $allowed_layouts->form_state == 'submit', then the form has been submitted and we should
   * react. It's really up to your client module how you handle the rest; panels_allowed_layouts::save() (or
   * panels_allowed_layouts::api_save(), if that's the route you're going) will have already been called,
   * so if those methods handle your save routine, then all there is left to do is handle redirects, if you
   * want. The current implementation of the allowed layouts form currently never redirects, so it's up to
   * you to control where the user ends up next.
   *
   * @param string $title
   *  Used to set the title of the allowed layouts form. If no value is given, defaults to
   *  'Panels: Allowed Layouts'.
   *
   * @return mixed $result
   *  - On the first passthrough when the form is being rendered, $result is the form's structured
   *    HTML, ready to be pushed to the screen with a call to theme('page', ...).
   *  - A successful second passthrough indicates a successful submit, and
   *    $result === panels_allowed_layouts::allowed_layout_settings. Returning it is simply for convenience.
   */
  function set_allowed($title = 'Panels: Allowed Layouts') {
    $this->sync_with_available();
    $form_id = 'panels_common_set_allowed_layouts';
    // TODO switch to drupal_build_form(); need to pass by ref
    $form = drupal_retrieve_form($form_id, $this, $title);

    if ($result = drupal_process_form($form_id, $form)) {
      // successful submit
      $this->form_state = 'submit';
      return $result;
    }
    $this->form_state = isset($_POST['op']) ? 'failed-validate' : 'render';
    $result = drupal_render_form($form_id, $form);
    return $result;
  }

  /**
   * Checks for newly-added layouts and deleted layouts. If any are found, updates panels_allowed_layouts::allowed_layout_settings;
   * new additions are made according to panels_allowed_layouts::allow_new, while deletions are unset().
   *
   * Note that any changes made by this function are not saved in any permanent location.
   */
  function sync_with_available() {
    $layouts = $this->list_layouts();
    foreach (array_diff($layouts, array_keys($this->allowed_layout_settings)) as $new_layout) {
      $this->allowed_layout_settings[$new_layout] = $this->allow_new ? 1 : 0;
    }
    foreach (array_diff(array_keys($this->allowed_layout_settings), $layouts) as $deleted_layout) {
      unset($this->allowed_layout_settings[$deleted_layout]);
    }
  }

  /**
   * Use panels_allowed_layouts::module_name to generate a variable for variable_set(), in which
   * a serialized version of $this will be stored.
   *
   * Does nothing if panels_allowed_layouts::module_name is not set.
   *
   * IMPORTANT NOTE: if you use variable_get() in a custom client module save() method, you MUST
   * wrap $this in serialize(), then unserialize() what you get from variable_get(). Failure to
   * do so will result in an incomplete object. The following code will work:
   * @code
   *  $allowed_layouts = unserialize(variable_get('your_variable_name', serialize(''));
   * @endcode
   *
   * If you don't serialize the second parameter of variable_get() and the variable name you provide
   * can't be found, an E_STRICT warning will be generated for trying to unserialize an entity
   * that has not been serialized.
   *
   */
  function save() {
    if (!is_null($this->module_name)) {
      variable_set($this->module_name . "_allowed_layouts", serialize($this));
    }
  }

  /**
   * Snag a list of the current layouts for internal use.
   *
   * Data is not saved in a class member in order to ensure that it's
   * fresh.
   *
   * @return array $layouts
   *  An indexed array of the system names for all currently available layouts.
   */
  function list_layouts() {
    static $layouts = array();
    if (empty($layouts)) {
      ctools_include('plugins', 'panels');
      $layouts = array_keys(panels_get_layouts());
    }
    return $layouts;
  }
}

/**
 * A common settings page for Panels modules, because this code is relevant to
 * any modules that don't already have special requirements.
 */
function panels_common_settings(&$form_state, $module_name = 'panels_common') {
  ctools_include('plugins', 'panels');
  ctools_include('content');
  $content_types = ctools_get_content_types();
  $skip = FALSE;

  $default_types = variable_get($module_name . '_default', NULL);
  if (!isset($default_types)) {
    $default_types = array('other' => TRUE);
    $skip = TRUE;
  }

  foreach ($content_types as $id => $info) {
    if (empty($info['single'])) {
      $default_options[$id] = t('New @s', array('@s' => $info['title']));
      if ($skip) {
        $default_types[$id] = TRUE;
      }
    }
  }

  $default_options['other'] = t('New content of other types');
  $form['panels_common_default'] = array(
    '#type' => 'checkboxes',
    '#title' => t('New content behavior'),
    '#description' => t('Select the default behavior of new content added to the system. If checked, new content will automatically be immediately available to be added to Panels pages. If not checked, new content will not be available until specifically allowed here.'),
    '#options' => $default_options,
    '#default_value' => array_keys(array_filter($default_types)),
  );

  $form_state['skip'] = $skip;
  if ($skip) {
    $form['markup'] = array('#value' => t('<p>Click Submit to be presented with a complete list of available content types set to the defaults you selected.</p>'));
  }
  else {
    // Rebuild the entire list, setting appropriately from defaults. Give
    // each type its own checkboxes set unless it's 'single' in which
    // case it can go into our fake other set.
    $available_content_types = ctools_content_get_all_types();
    $allowed_content_types = variable_get($module_name . '_allowed_types', array());

    foreach ($available_content_types as $id => $types) {
      foreach ($types as $type => $info) {
        $key = $id . '-' . $type;
        $checkboxes = empty($content_types[$id]['single']) ? $id : 'other';
        $options[$checkboxes][$key] = $info['title'];
        if (!isset($allowed_content_types[$key])) {
          $allowed[$checkboxes][$key] = isset($default_types[$id]) ? $default_types[$id] : $default_types['other'];
        }
        else {
          $allowed[$checkboxes][$key] = $allowed_content_types[$key];
        }
      }
    }

    $form['content_types'] = array(
      '#tree' => TRUE,
      '#prefix' => '<div class="clear-block">',
      '#suffix' => '</div>',
    );
    // cheat a bit
    $content_types['other'] = array('title' => t('Other'), 'weight' => 10);
    foreach ($content_types as $id => $info) {
      if (isset($allowed[$id])) {
        $form['content_types'][$id] = array(
          '#prefix' => '<div class="panels-page-type-container">',
          '#suffix' => '</div>',
          '#type' => 'checkboxes',
          '#title' => t('Allowed @s content', array('@s' => $info['title'])),
          '#options' => $options[$id],
          '#default_value' => array_keys(array_filter($allowed[$id])),
          '#checkall' => TRUE,
        );
      }
    }
  }

  panels_common_allowed_layouts_form($form, $form_state, $module_name);

  $form['module_name'] = array(
    '#type' => 'value',
    '#value' => $module_name,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  ctools_add_css('panels_page', 'panels');
  return $form;
}

/**
 * Submit hook for panels_common_settings
 */
function panels_common_settings_validate($form, &$form_state) {
  panels_common_allowed_layouts_form_validate($form, $form_state);
}

/**
 * Submit hook for panels_common_settings
 */
function panels_common_settings_submit($form, &$form_state) {
  panels_common_allowed_layouts_form_submit($form, $form_state);
  $module_name = $form_state['values']['module_name'];
  variable_set($module_name . '_default', $form_state['values']['panels_common_default']);
  if (!$form_state['skip']) {
    // merge the broken apart array neatly back together
    variable_set($module_name . '_allowed_types', call_user_func_array('array_merge', $form_state['values']['content_types']));
  }
  drupal_set_message(t('Your changes have been saved.'));
}

/**
 * Based upon the settings, get the allowed types for this node.
 */
function panels_common_get_allowed_types($module, $contexts = array(), $has_content = FALSE, $default_defaults = array(), $default_allowed_types = array()) {
  // Get a list of all types that are available

  $default_types = variable_get($module . '_default', $default_defaults);
  $allowed_types = variable_get($module . '_allowed_types', $default_allowed_types);

  // By default, if they haven't gone and done the initial setup here,
  // let all 'other' types (which will be all types) be available.
  if (!isset($default_types['other'])) {
    $default_types['other'] = TRUE;
  }

  ctools_include('content');
  $content_types = ctools_content_get_available_types($contexts, $has_content, $allowed_types, $default_types);

  return $content_types;
}

/**
 * The FAPI code for generating an 'allowed layouts' selection form.
 *
 * NOTE: Because the Panels API does not guarantee a particular method of storing the data on allowed layouts,
 * it is not_possible for the Panels API to implement any checks that determine whether reductions in
 * the set of allowed layouts conflict with pre-existing layout selections. $displays in that category
 * will continue to function with their current layout as normal until the user/owner/admin attempts
 * to change layouts on that display, at which point they will have to select from the new set of
 * allowed layouts. If this is not the desired behavior for your client module, it's up to you to
 * write a validation routine that determines what should be done with conflicting layouts.
 *
 * Remember that changing layouts where panes have already been created can result in data loss;
 * consult panels_change_layout() to see how the Panels API handles that process. Running
 * drupal_execute('panels_change_layout', ...) is one possible starting point.
 *
 * @ingroup forms
 *
 * @param array $allowed_layouts
 *  The set of allowed layouts that should be used as the default values
 *  for this form. If none is provided, then by default no layouts will be restricted.
 */
function panels_common_allowed_layouts_form(&$form, &$form_state, $module_name) {
  // Fetch our allowed layouts from variables.
  $allowed_layouts = panels_common_get_allowed_layout_object($module_name);

  $layouts = panels_get_layouts();
  foreach ($layouts as $id => $layout) {
    $options[$id] = panels_print_layout_icon($id, $layout, check_plain($layout['title']));
  }

  $form_state['allowed_layouts'] = &$allowed_layouts;

  ctools_add_js('layout', 'panels');
  $form['layouts'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select allowed layouts'),
    '#options' => $options,
    '#description' => t('Check the boxes for all layouts you want to allow users choose from when picking a layout. You must allow at least one layout.'),
    '#default_value' => array_keys(array_filter($allowed_layouts->allowed_layout_settings)),
    '#prefix' => '<div class="clear-block panels-layouts-checkboxes">',
    '#suffix' => '</div>',
    '#checkall' => TRUE,
  );

  return $form;
}

function panels_common_allowed_layouts_form_validate($form, &$form_state) {
  $selected = array_filter($form_state['values']['layouts']);
  if (empty($selected)) {
    form_set_error('layouts', 'You must choose at least one layout to allow.');
  }
}

function panels_common_allowed_layouts_form_submit($form, &$form_state) {
  foreach ($form_state['values']['layouts'] as $layout => $setting) {
    $form_state['allowed_layouts']->allowed_layout_settings[$layout] = (bool) $setting;
  }
  $form_state['allowed_layouts']->save();
}

/**
 * Get the allowed layout object for the given module.
 */
function panels_common_get_allowed_layout_object($module_name) {
  $allowed_layouts = unserialize(variable_get($module_name . "_allowed_layouts", serialize('')));

  // if no parameter was provided, or the variable_get failed
  if (!$allowed_layouts) {
    // still no dice. simply creates a dummy version where all layouts
    // are allowed.
    $allowed_layouts = new panels_allowed_layouts();
    $allowed_layouts->allow_new = TRUE;
    $allowed_layouts->module_name = $module_name;
  }

  // sanitize allowed layout listing; this is redundant if the
  // $allowed_layouts param was null, but the data is cached anyway
  $allowed_layouts->sync_with_available();

  return $allowed_layouts;
}

/**
 * Get the allowed layouts for the given module.
 */
function panels_common_get_allowed_layouts($module_name) {
  $available_layouts = panels_get_layouts();
  if (empty($module_name)) {
    return $available_layouts;
  }
  else if (is_object($module_name)) {
    $allowed_layouts = $module_name;
  }
  else {
    $allowed_layouts = panels_common_get_allowed_layout_object($module_name);
  }

  $allowed = array_filter($allowed_layouts->allowed_layout_settings);
  $order = array();
  foreach ($available_layouts as $name => $plugin) {
    if (!empty($allowed[$name])) {
      $order[$name] = $plugin['category'] . ':' . $plugin['title'];
    }
  }

  // Sort
  $layouts = array();

  asort($order);
  foreach ($order as $name => $junk) {
    $layouts[$name] = $available_layouts[$name];
  }

  return $layouts;
}

/**
 * Create a visible list of content in a display.
 * Note that the contexts must be pre-loaded.
 */
function theme_panels_common_content_list($display) {
  $layout = panels_get_layout($display->layout);
  $content = '<dl class="content-list">';
  foreach (panels_get_regions($layout, $display) as $panel_id => $title) {
    $content .= "<dt>$title</dt><dd>";
    if (!empty($display->panels[$panel_id])) {
      $content .= '<ol>';
      foreach ($display->panels[$panel_id] as $pid) {
        $content .= '<li>' . panels_get_pane_title($display->content[$pid], $display->context) . '</li>';
      }
      $content .= '</ol>';
    }
    else {
      $content .= t('Empty');
    }
    $content .= '</dd>';
  }
  $content .= '</dl>';
  return $content;
}

/**
 * Print a selector of layouts, each linked to the next step.
 *
 * Most operations use radio buttons for selecting layouts, but some will
 * give each layout as a link that goes to the next step. This function
 * makes it easy to simply provide a list of allowed layouts and the base
 * path.
 *
 * One limitation is that it will only append the layout name to the end, so
 * if the actual layout name is needed in the middle, that can't happen.
 *
 * @return
 *   The rendered output.
 */
function panels_common_print_layout_links($layouts, $base_path, $link_options = array()) {
  $output = '';

  $categories = array();
  ctools_include('cleanstring');
  foreach ($layouts as $id => $layout) {
    $category = ctools_cleanstring($layout['category']);

    $categories[$category] = $layout['category'];
    $options[$category][$id] = panels_print_layout_link($id, $layout, $base_path . '/' . $id, $link_options);
  }

  $form = array();
  $form['categories'] = array(
    '#title' => t('Category'),
    '#type' => 'select',
    '#options' => $categories,
    '#name' => 'categories',
    '#id' => 'edit-categories',
    '#value' => '',
    '#parents' => array('categories'),
  );

  $output .= drupal_render($form);

  $output .= '<div class="panels-choose-layout panels-layouts-checkboxes clear-block">';

  // We're doing these dependencies completely manualy, which is unusual, but
  // the process code only supports doing them in a form.
  // @todo modify dependent.inc to make this easier.

  $dependencies = array();
  foreach ($options as $category => $links) {
    $dependencies['panels-layout-category-' . $category] = array(
      'values' => array('edit-categories' => array($category)),
      'num' => 1,
      'type' => 'hide',
    );

    $output .= '<div id="panels-layout-category-' . $category . '-wrapper">';
    $output .= '<div id="panels-layout-category-' . $category . '" class="form-checkboxes clear-block">';
    $output .= '<div class="panels-layouts-category">' . $categories[$category] . '</div>';

    foreach ($links as $key => $link) {
      $output .= $link;
    }
    $output .= '</div></div>';
  }

  $output .= '</div>';

  ctools_add_js('dependent');
  $js['CTools']['dependent'] = $dependencies;
  drupal_add_js($js, 'setting');

  return $output;
}
