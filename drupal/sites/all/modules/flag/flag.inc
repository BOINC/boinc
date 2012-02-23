<?php
// $Id: flag.inc,v 1.1.2.32 2010/01/08 05:00:45 quicksketch Exp $

/**
 * @file
 * Implements various flags. Uses object oriented style inspired by that
 * of Views 2.
 */

/**
 * Implementation of hook_flag_definitions().
 *
 * Defines the flags this module implements.
 *
 * @return
 *   An "array of arrays", keyed by content-types. The 'handler' slot
 *   should point to the PHP class implementing this flag.
 */
function flag_flag_definitions() {
  return array(
    'node' => array(
      'title' => t('Nodes'),
      'description' => t("Nodes are a Drupal site's primary content."),
      'handler' => 'flag_node',
    ),
    'comment' => array(
      'title' => t('Comments'),
      'description' => t('Comments are responses to node content.'),
      'handler' => 'flag_comment',
    ),
    'user' => array(
      'title' => t('Users'),
      'description' => t('Users who have created accounts on your site.'),
      'handler' => 'flag_user',
    ),
  );
}

/**
 * Returns a flag definition.
 */
function flag_fetch_definition($content_type = NULL) {
  static $cache;
  if (!isset($cache)) {
    $cache = module_invoke_all('flag_definitions');
    if (!isset($cache['node'])) {
      // We want our API to be available in hook_install, but our module is not
      // enabled by then, so let's load our implementation directly:
      $cache += flag_flag_definitions();
    }
  }

  if (isset($content_type)) {
    if (isset($cache[$content_type])) {
      return $cache[$content_type];
    }
  }
  else {
    return $cache;
  }
}

/**
 * Returns all flag types defined on the system.
 */
function flag_get_types() {
  static $types;
  if (!isset($types)) {
    $types = array_keys(flag_fetch_definition());
  }
  return $types;
}

/**
 * Instantiates a new flag handler. A flag handler is more commonly know as "a
 * flag". A factory method usually populates this empty flag with settings
 * loaded from the database.
 */
function flag_create_handler($content_type) {
  $definition = flag_fetch_definition($content_type);
  if (isset($definition) && class_exists($definition['handler'])) {
    $handler = new $definition['handler'];
  }
  else  {
    $handler = new flag_broken;
  }
  $handler->content_type = $content_type;
  $handler->construct();
  return $handler;
}

/**
 * This abstract class represents a flag, or, in Views 2 terminology, "a handler".
 *
 * This is the base class for all flag implementations. Notable derived
 * classes are flag_node and flag_comment.
 */
class flag_flag {

  // The database ID. Null for flags that haven't been saved to the database yet.
  var $fid = NULL;

  // The content-type this flag works with.
  var $content_type = NULL;

  // The flag's "machine readable" name.
  var $name = '';

  // Various non-serialized properties of the flag, corresponding directly to
  // database columns.
  var $title = '';
  var $roles = array(DRUPAL_AUTHENTICATED_RID);
  var $global = FALSE;
  // The sub-types, e.g. node types, this flag applies to.
  var $types = array();

  /**
   * Creates a flag from a database row. Returns it.
   *
   * This is static method.
   *
   * The reason this isn't a non-static instance method --like Views's init()--
   * is because the class to instantiate changes according to the 'content_type'
   * database column. This design pattern is known as the "Single Table
   * Inheritance".
   *
   * @static
   */
  function factory_by_row($row) {
    $flag = flag_create_handler($row->content_type);

    // Lump all data unto the object...
    foreach ($row as $field => $value) {
      $flag->$field = $value;
    }
    // ...but skip the following two.
    unset($flag->options, $flag->type);

    $options = (array)unserialize($row->options);
    // Make the unserialized options accessible as normal properties.
    foreach ($options as $option => $value) {
      $flag->$option = $value;
    }

    if (!empty($row->type)) {
      // The loop loading from the database should further populate this property.
      $flag->types[] = $row->type;
    }

    $flag->roles = empty($row->roles) ? array() : explode(',', $row->roles);

    return $flag;
  }

  /**
   * Create a complete flag (except an FID) from an array definition.
   */
  function factory_by_array($config) {
    $flag = flag_create_handler($config['content_type']);

    foreach ($config as $option => $value) {
      $flag->$option = $value;
    }

    if (is_array($config['locked'])) {
      $flag->locked = drupal_map_assoc($config['locked']);
    }

    return $flag;
  }

  /**
   * Another factory method. Returns a new, "empty" flag; e.g., one suitable for
   * the "Add new flag" page.
   *
   * @static
   */
  function factory_by_content_type($content_type) {
    return flag_create_handler($content_type);
  }

  /**
   * Declares the options this flag supports, and their default values.
   *
   * Derived classes should want to override this.
   */
  function default_options() {
    return array(
      'flag_short' => '',
      'flag_long' => '',
      'flag_message' => '',
      'flag_confirmation' => '',
      'unflag_short' => '',
      'unflag_long' => '',
      'unflag_message' => '',
      'unflag_confirmation' => '',
      'link_type' => 'toggle',
    );
  }

  /**
   * Provides a form for setting options.
   *
   * Derived classes should want to override this.
   */
  function options_form(&$form) {
  }

  /**
   * Default constructor. Loads the default options.
   */
  function construct() {
    $options = $this->default_options();
    foreach ($options as $option => $value) {
      $this->$option = $value;
    }
  }

  /**
   * Update the flag with settings entered in a form.
   */
  function form_input($form_values) {
    // Load the form fields indiscriminately unto the flag (we don't care about
    // stray FormAPI fields because we aren't touching unknown properties anyway.
    foreach ($form_values as $field => $value) {
      $this->$field = $value;
    }
    // But checkboxes need some massaging:
    $this->roles = array_values(array_filter($this->roles));
    $this->types = array_values(array_filter($this->types));
    // Clear internal titles cache:
    $this->get_title(NULL, TRUE);
  }

  /**
   * Validates a flag settings
   */
  function validate() {
    $this->validate_name();
  }

  function validate_name() {
    // Ensure a safe machine name.
    if (!preg_match('/^[a-z_][a-z0-9_]*$/', $this->name)) {
      form_set_error('name', t('The flag name may only contain lowercase letters, underscores, and numbers.'));
    }
    // Ensure the machine name is unique.
    if (!isset($this->fid)) {
      $flag = flag_get_flag($this->name);
      if (!empty($flag)) {
        form_set_error('name', t('Flag names must be unique. This flag name is already in use.'));
      }
    }
  }

  /**
   * Fetches, possibly from some cache, a content object this flag works with.
   */
  function fetch_content($content_id, $object_to_remember = NULL) {
    static $cache = array();
    if (isset($object_to_remember)) {
      $cache[$content_id] = $object_to_remember;
    }
    if (!array_key_exists($content_id, $cache)) {
      $content = $this->_load_content($content_id);
      $cache[$content_id] = $content ? $content : NULL;
    }
    return $cache[$content_id];
  }

  /**
   * Loads a content object this flag works with.
   * Derived classes must implement this.
   *
   * @abstract
   * @private
   * @static
   */
  function _load_content($content_id) {
    return NULL;
  }

  /**
   * Stores some object in fetch_content()'s cache, so subsequenet calls to
   * fetch_content() return it.
   *
   * This is needed because otherwise fetch_object() loads the object from the
   * database (by calling _load_content()), whereas sometimes we want to fetch
   * an object that hasn't yet been saved to the database. See flag_nodeapi().
   */
  function remember_content($content_id, $object) {
    $this->fetch_content($content_id, $object);
  }

  /**
   * Returns TRUE if the flag applies to the given content.
   * Derived classes must implement this.
   *
   * @abstract
   */
  function applies_to_content_object($content) {
    return FALSE;
  }

  /**
   * Returns TRUE if the flag applies to the content with the given ID.
   *
   * This is a convenience method that simply loads the object and calls
   * applies_to_content_object(). If you already have the object, don't call
   * this function: call applies_to_content_object() directly.
   */
  function applies_to_content_id($content_id) {
    return $this->applies_to_content_object($this->fetch_content($content_id));
  }

  /**
   * Given a content object, returns its ID.
   * Derived classes must implement this.
   *
   * @abstract
   */
  function get_content_id($content) {
    return NULL;
  }

  /**
   * Returns TRUE if the flag is configured to show the flag-link using hook_link.
   * Derived classes are likely to implement this.
   */
  function uses_hook_link($teaser) {
    return FALSE;
  }

  /**
   * Returns TRUE if user has access to use this flag.
   *
   * @param $account
   *   Optional. The user object. If none given, the current user will be used.
   *
   * @return
   *   Boolean TRUE if the user is allowed to flag/unflag. FALSE otherwise.
   */
  function user_access($account = NULL) {
    if (!isset($account)) {
      $account = $GLOBALS['user'];
    }
    $matched_roles = array_intersect($this->roles, array_keys($account->roles));
    return !empty($matched_roles) || empty($this->roles) || $account->uid == 1;
  }

  /**
   * Flags, on unflags, an item.
   *
   * @param $action
   *   Either 'flag' or 'unflag'.
   * @param $content_id
   *   The ID of the item to flag or unflag.
   * @param $account
   *   The user on whose behalf to flag. Leave empty for the current user.
   * @param $skip_permission_check
   *   Flag the item even if the $account user don't have permission to do so.
   * @return
   *   FALSE if some error occured (e.g., user has no permission, flag isn't
   *   applicable to the item, etc.), TRUE otherwise.
   */
  function flag($action, $content_id, $account = NULL, $skip_permission_check = FALSE) {
    if (!isset($account)) {
      $account = $GLOBALS['user'];
    }
    if (!$account) {
      return FALSE;
    }

    if (!$account->uid) {
      // Anonymous users can't flag with this system. For now.
      //
      // @todo This is legacy code. $flag->user_access() should handle this.
      // This will also make it posible to have flags that do support anonymous
      // users.
      return FALSE;
    }

    if (!$skip_permission_check && !$this->user_access($account)) {
      // User has no permission to use this flag.
      return FALSE;
    }
    if (!$this->applies_to_content_id($content_id)) {
      // Flag does not apply to this content.
      return FALSE;
    }

    // Clear various caches; We don't want code running after us to report
    // wrong counts or false flaggings.
    flag_get_counts(NULL, NULL, TRUE);
    flag_get_user_flags(NULL, NULL, NULL, TRUE);

    // Perform the flagging or unflagging of this flag.
    $uid = $this->global ? 0 : $account->uid;
    $flagged = $this->_is_flagged($content_id, $uid);
    if ($action == 'unflag' && $flagged) {
      $this->_unflag($content_id, $uid);
      // Let other modules perform actions.
      module_invoke_all('flag', 'unflag', $this, $content_id, $account);
    }
    elseif ($action == 'flag' && !$flagged) {
      $this->_flag($content_id, $uid);
      // Let other modules perform actions.
      module_invoke_all('flag', 'flag', $this, $content_id, $account);
    }

    return TRUE;
  }

  /**
   * Returns TRUE if a certain user has flagged this content.
   *
   * Thanks to using a cache, inquiring several different flags about the same
   * item results in only one SQL query.
   *
   * @param $uid
   *   Optional. The user ID whose flags we're checking. If none given, the
   *   current user will be used.
   */
  function is_flagged($content_id, $uid = NULL) {
    $uid = !isset($uid) ? $GLOBALS['user']->uid : $uid;
    // flag_get_user_flags() does caching.
    $user_flags = flag_get_user_flags($this->content_type, $content_id, $uid);
    return isset($user_flags[$this->name]);
  }

  /**
   * Returns TRUE if a certain user has flagged this content.
   *
   * You probably shouldn't call this raw private method: call the
   * is_flagged() method instead.
   *
   * This method is similar to is_flagged() except that it does direct SQL and
   * doesn't do caching. Use it when you want to not affect the cache, or to
   * bypass it.
   *
   * @private
   */
  function _is_flagged($content_id, $uid) {
    return db_result(db_query("SELECT fid FROM {flag_content} WHERE fid = %d AND uid = %d AND content_id = %d", $this->fid, $uid, $content_id));
  }

  /**
   * A low-level method to flag content.
   *
   * You probably shouldn't call this raw private method: call the flag()
   * function instead.
   *
   * @private
   */
  function _flag($content_id, $uid) {
    db_query("INSERT INTO {flag_content} (fid, content_type, content_id, uid, timestamp) VALUES (%d, '%s', %d, %d, %d)", $this->fid, $this->content_type, $content_id, $uid, time());
    $this->_update_count($content_id);
  }

  /**
   * A low-level method to unflag content.
   *
   * You probably shouldn't call this raw private method: call the flag()
   * function instead.
   *
   * @private
   */
  function _unflag($content_id, $uid) {
    db_query("DELETE FROM {flag_content} WHERE fid = %d AND uid = %d AND content_id = %d", $this->fid, $uid, $content_id);
    $this->_update_count($content_id);
  }

  /**
   * Updates the flag count for this content
   *
   * @private
   */
  function _update_count($content_id) {
    $count = db_result(db_query("SELECT COUNT(*) FROM {flag_content} WHERE fid = %d AND content_id = %d", $this->fid, $content_id));
    if ($count == 0) {
      db_query("DELETE FROM {flag_counts} WHERE fid = %d AND content_id = %d", $this->fid, $content_id);
    }
    else {
      db_query("UPDATE {flag_counts} SET count = %d WHERE fid = %d AND content_id = %d", $count, $this->fid, $content_id);
      if (!db_affected_rows()) {
        db_query("INSERT INTO {flag_counts} (fid, content_type, content_id, count) VALUES (%d, '%s', %d, %d)", $this->fid, $this->content_type, $content_id, $count);
      }
    }
  }

  /**
   * Returns the number of times an item is flagged.
   *
   * Thanks to using a cache, inquiring several different flags about the same
   * item results in only one SQL query.
   */
  function get_count($content_id) {
    $counts = flag_get_counts($this->content_type, $content_id);
    return isset($counts[$this->name]) ? $counts[$this->name] : 0;
  }

  /**
   * Returns the number of items a user has flagged.
   *
   * For global flags, pass '0' as the user ID.
   */
  function get_user_count($uid) {
    return db_result(db_query('SELECT COUNT(*) FROM {flag_content} WHERE fid = %d AND uid = %d', $this->fid, $uid));
  }

  /**
   * Processes a flag label for display. This means language translation and
   * token replacements.
   *
   * You should always call this function and not get at the label directly.
   * E.g., do `print $flag->get_label('title')` instead of `print
   * $flag->title`.
   *
   * @param $label
   *   The label to get, e.g. 'title', 'flag_short', 'unflag_short', etc.
   * @param $content_id
   *   The ID in whose context to interpret tokens. If not given, only global
   *   tokens will be substituted.
   * @return
   *   The processed label.
   */
  function get_label($label, $content_id = NULL) {
    if (!isset($this->$label)) {
      return;
    }
    $label = t($this->$label);
    if (strpos($label, '[') !== FALSE && module_exists('token')) {
      $label = $this->replace_tokens($label, array('global' => NULL), $content_id);
    }
    return filter_xss_admin($label);
  }

  /**
   * Replaces tokens in a label. Only the 'global' token context is regognized
   * by default, so derived classes should override this method to add all
   * token contexts they understand.
   */
  function replace_tokens($label, $contexts, $content_id) {
    return token_replace_multiple($label, $contexts);
  }

  /**
   * Returns the token types this flag understands in labels. These are used
   * for narrowing down the token list shown in the help box to only the
   * relevant ones.
   *
   * Derived classes should override this.
   */
  function get_labels_token_types() {
    return array('global');
  }

  /**
   * A convenience method for getting the flag title.
   *
   * `$flag->get_title()` is shorthand for `$flag->get_label('title')`.
   */
  function get_title($content_id = NULL, $reset = FALSE) {
    static $titles = array();
    if ($reset) {
      $titles = array();
    }
    $slot = intval($content_id); // Convert NULL to 0.
    if (!isset($titles[$this->fid][$slot])) {
      $titles[$this->fid][$slot] = $this->get_label('title', $content_id);
    }
    return $titles[$this->fid][$slot];
  }

  /**
   * Returns a 'flag action' object. It exists only for the sake of its
   * informative tokens. Currently, it's utilized only for the 'mail' action.
   *
   * Derived classes should populate the 'content_title' and 'content_url'
   * slots.
   */
  function get_flag_action($content_id) {
    $flag_action = new stdClass();
    $flag_action->flag = $this->name;
    $flag_action->content_type = $this->content_type;
    $flag_action->content_id = $content_id;
    return $flag_action;
  }

  /**
   * @addtogroup actions
   * @{
   * Methods that can be overridden to support Actions.
   */

  /**
   * Returns an array of all actions that are executable with this flag.
   */
  function get_valid_actions() {
    $actions = module_invoke_all('action_info');
    foreach ($actions as $callback => $action) {
      if ($action['type'] != $this->content_type && !isset($action['hooks'][$this->content_type])) {
        unset($actions[$callback]);
      }
    }
    return $actions;
  }

  /**
   * Returns objects the action may possibly need. This method should return at
   * least the 'primary' object the action operates on.
   *
   * This method is needed because get_valid_actions() returns actions that
   * don't necessarily operate on an object of a type this flag manages. For
   * example, flagging a comment may trigger an 'Unpublish post' action on a
   * node; So the comment flag needs to tell the action about some node.
   *
   * Derived classes must implement this.
   *
   * @abstract
   */
  function get_relevant_action_objects($content_id) {
    return array();
  }

  /**
   * @} End of "addtogroup actions".
   */

  /**
   * Methods that can be overridden to support the Rules module.
   *
   * @addtogroup rules
   * @{
   */

  /**
   * Defines the Rules arguments involved in a flag event.
   */
  function rules_get_event_arguments_definition() {
    return array();
  }
  
  /**
   * Defines the Rules argument for flag actions or conditions
   */
  function rules_get_element_argument_definition() {
    return array();
  }
  
  /**
   * @} End of "addtogroup rules".
   */

  /**
   * @addtogroup views
   * @{
   * Methods that can be overridden to support the Views module.
   */

  /**
   * Returns information needed for Views integration. E.g., the Views table
   * holding the flagged content, its primary key, and various labels. See
   * derived classes for examples.
   *
   * @static
   */
  function get_views_info() {
    return array();
  }

  /**
   * Similar to applies_to_content_id() but works on a bunch of IDs. It is
   * called in the pre_render() stage of the 'Flag links' field to find out where
   * that link applies. The reason we do a separate DB query, and not lump this
   * test in the Views query, is to make 'many to one' tests possible without
   * interfering with the rows, and also to reduce the complexity of the code.
   */
  function applies_to_content_id_array($content_ids) {
    return array();
  }

  /**
   * @} End of "addtogroup views".
   */

  /**
   * Saves a flag to the database. It is a wrapper around update() and insert().
   */
  function save() {
    if (isset($this->fid)) {
      $this->update();
    }
    else {
      $this->insert();
    }
  }

  /**
   * Saves an existing flag to the database. Better use save().
   */
  function update() {
    db_query("UPDATE {flags} SET name = '%s', title = '%s', roles = '%s', global = %d, options = '%s' WHERE fid = %d", $this->name, $this->title, implode(',', $this->roles), $this->global, $this->get_serialized_options(), $this->fid);
    db_query("DELETE FROM {flag_types} WHERE fid = %d", $this->fid);
    foreach ($this->types as $type) {
      db_query("INSERT INTO {flag_types} (fid, type) VALUES (%d, '%s')", $this->fid, $type);
    }
  }

  /**
   * Saves a new flag to the database. Better use save().
   */
  function insert() {
    if (function_exists('db_last_insert_id')) {
      // Drupal 6. We have a 'serial' primary key.
      db_query("INSERT INTO {flags} (content_type, name, title, roles, global, options) VALUES ('%s', '%s', '%s', '%s', %d, '%s')", $this->content_type, $this->name, $this->title, implode(',', $this->roles), $this->global, $this->get_serialized_options());
      $this->fid = db_last_insert_id('flags', 'fid');
    }
    else {
      // Drupal 5. We have an 'integer' primary key.
      $this->fid = db_next_id('{flags}_fid');
      db_query("INSERT INTO {flags} (fid, content_type, name, title, roles, global, options) VALUES (%d, '%s', '%s', '%s', '%s', %d, '%s')", $this->fid, $this->content_type, $this->name, $this->title, implode(',', $this->roles), $this->global, $this->get_serialized_options());
    }
    foreach ($this->types as $type) {
      db_query("INSERT INTO {flag_types} (fid, type) VALUES (%d, '%s')", $this->fid, $type);
    }
  }

  /**
   * Options are stored serialized in the database.
   */
  function get_serialized_options() {
    $option_names = array_keys($this->default_options());
    $options = array();
    foreach ($option_names as $option) {
      $options[$option] = $this->$option;
    }
    return serialize($options);
  }

  /**
   * Deletes a flag from the database.
   */
  function delete() {
    db_query('DELETE FROM {flags} WHERE fid = %d', $this->fid);
    db_query('DELETE FROM {flag_content} WHERE fid = %d', $this->fid);
    db_query('DELETE FROM {flag_types} WHERE fid = %d', $this->fid);
    db_query('DELETE FROM {flag_counts} WHERE fid = %d', $this->fid);
  }

  /**
   * Disable a flag provided by a module.
   */
  function disable() {
    if (isset($this->module)) {
      $flag_status = variable_get('flag_default_flag_status', array());
      $flag_status[$this->name] = FALSE;
      variable_set('flag_default_flag_status', $flag_status);
    }
  }

  /**
   * Enable a flag provided by a module.
   */
  function enable() {
    if (isset($this->module)) {
      $flag_status = variable_get('flag_default_flag_status', array());
      $flag_status[$this->name] = TRUE;
      variable_set('flag_default_flag_status', $flag_status);
    }
  }

  /**
   * Renders a flag/unflag link. This is a wrapper around theme('flag') that,
   * in Drupal 6, easily channels the call to the right template file.
   *
   * For parameters docmentation, see theme_flag().
   */
  function theme($action, $content_id, $after_flagging = FALSE) {
    if (!_flag_is_drupal_5()) {
      // We're running Drupal 6.
      return theme($this->theme_suggestions(), $this, $action, $content_id, $after_flagging);
    }
    else {
      // We're running Drupal 5. Noting to do: The theme_suggestions[] are
      // handed to phptemplate in phptemplate_flag(), if the user bothered to
      // copy that function into her 'template.php'.
      return theme('flag', $this, $action, $content_id, $after_flagging);
    }
  }

  /**
   * Provides an array of possible themes to try for a given flag.
   */
  function theme_suggestions() {
    $suggestions = array();
    $suggestions[] = 'flag__' . $this->name;
    $suggestions[] = 'flag';
    return $suggestions;
  }
}

/**
 * Implements a node flag.
 */
class flag_node extends flag_flag {
  function default_options() {
    $options = parent::default_options();
    $options += array(
      'show_on_page' => TRUE,
      'show_on_teaser' => TRUE,
      'show_on_form' => FALSE,
      'i18n' => 0,
    );
    return $options;
  }

  function options_form(&$form) {
    parent::options_form($form);
    // Note there isn't a translation helpers module in Drupal 5,
    // this feature is essentially Drupal 6 only.
    $form['i18n'] = array(
      '#type' => 'radios',
      '#title' => t('Internationalization'),
      '#options' => array(
        '1' => t('Flag translations of content as a group'),
        '0' => t('Flag each translation of content separately'),
      ),
      '#default_value' => $this->i18n,
      '#description' => t('Flagging translations as a group effectively allows users to flag the original piece of content regardless of the translation they are viewing. Changing this setting will <strong>not</strong> update content that has been flagged already.'),
      '#access' => module_exists('translation_helpers'),
      '#weight' => 5,
    );
    $form['display']['show_on_teaser'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display link on node teaser'),
      '#default_value' => $this->show_on_teaser,
      '#access' => empty($this->locked['show_on_teaser']),
    );
    $form['display']['show_on_page'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display link on node page'),
      '#default_value' => $this->show_on_page,
      '#access' => empty($this->locked['show_on_page']),
    );
    $form['display']['show_on_form'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display checkbox on node edit form'),
      '#default_value' => $this->show_on_form,
      '#description' => t('If you elect to have a checkbox on the node edit form, you may specify its initial state in the settings form <a href="@content-types-url">for each content type</a>.', array('@content-types-url' => url('admin/content/types'))),
      '#access' => empty($this->locked['show_on_form']),
    );
  }

  function _load_content($content_id) {
    return is_numeric($content_id) ? node_load($content_id) : NULL;
  }

  function applies_to_content_object($node) {
    if ($node && in_array($node->type, $this->types)) {
      return TRUE;
    }
    return FALSE;
  }

  function get_content_id($node) {
    return $node->nid;
  }

  /**
   * Adjust the Content ID to find the translation parent if i18n-enabled.
   *
   * @param $content_id
   *   The nid for the content.
   * @return
   *   The tnid if available, the nid otherwise.
   */
  function get_translation_id($content_id) {
    if ($this->i18n) {
      $node = $this->fetch_content($content_id);
      if (!empty($node->tnid)) {
        $content_id = $node->tnid;
      }
    }
    return $content_id;
  }

  function uses_hook_link($teaser) {
    if ($teaser && $this->show_on_teaser || !$teaser && $this->show_on_page) {
      return TRUE;
    }
    return FALSE;
  }

  function flag($action, $content_id, $account = NULL, $skip_permission_check = FALSE) {
    $content_id = $this->get_translation_id($content_id);
    return parent::flag($action, $content_id, $account, $skip_permission_check);
  }


  function is_flagged($content_id, $uid = NULL) {
    $content_id = $this->get_translation_id($content_id);
    return parent::is_flagged($content_id, $uid);
  }

  function get_labels_token_types() {
    return array('node');
  }

  function replace_tokens($label, $contexts, $content_id) {
    if ($content_id && ($node = $this->fetch_content($content_id))) {
      $contexts['node'] = $node;
    }
    return parent::replace_tokens($label, $contexts, $content_id);
  }

  function get_flag_action($content_id) {
    $flag_action = parent::get_flag_action($content_id);
    $node = $this->fetch_content($content_id);
    $flag_action->content_title = $node->title;
    $flag_action->content_url = _flag_url('node/' . $node->nid);
    return $flag_action;
  }

  function get_valid_actions() {
    $actions = module_invoke_all('action_info');
    foreach ($actions as $callback => $action) {
      if ($action['type'] != 'node' && !isset($action['hooks']['nodeapi'])) {
        unset($actions[$callback]);
      }
    }
    return $actions;
  }

  function get_relevant_action_objects($content_id) {
    return array(
      'node' => $this->fetch_content($content_id),
    );
  }

  function rules_get_event_arguments_definition() {
    return array(
      'node' => array(
        'type' => 'node',
        'label' => t('flagged content'),
        'handler' => 'flag_rules_get_event_argument',
      ),
    );
  }
  
  function rules_get_element_argument_definition() {
    return array('type' => 'node', 'label' => t('Flagged content'));
  }

  function get_views_info() {
    return array(
      'views table' => 'node',
      'join field' => 'nid',
      'title field' => 'title',
      'title' => t('Node flag'),
      'help' => t('Limit results to only those nodes flagged by a certain flag; Or display information about the flag set on a node.'),
      'counter title' => t('Node flag counter'),
      'counter help' => t('Include this to gain access to the flag counter field.'),
    );
  }

  function applies_to_content_id_array($content_ids) {
    $passed = array();
    $content_ids  = implode(',', array_map('intval', $content_ids));
    $placeholders = implode(',', array_fill(0, sizeof($this->types), "'%s'"));
    $result = db_query("SELECT nid FROM {node} WHERE nid IN ($content_ids) AND type in ($placeholders)", $this->types);
    while ($row = db_fetch_object($result)) {
      $passed[$row->nid] = TRUE;
    }
    return $passed;
  }

}

/**
 * Implements a comment flag.
 */
class flag_comment extends flag_flag  {
  function default_options() {
    $options = parent::default_options();
    $options += array(
      'show_on_comment' => TRUE,
    );
    return $options;
  }

  function options_form(&$form) {
    parent::options_form($form);
    $form['display']['show_on_comment'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display link under comment'),
      '#default_value' => $this->show_on_comment,
      '#access' => empty($this->locked['show_on_comment']),
    );
  }

  function _load_content($content_id) {
    return _comment_load($content_id);
  }

  function applies_to_content_object($comment) {
    if ($comment && ($node = node_load($comment->nid)) && in_array($node->type, $this->types)) {
      return TRUE;
    }
    return FALSE;
  }

  function get_content_id($comment) {
    return $comment->cid;
  }

  function uses_hook_link($teaser) {
    return $this->show_on_comment;
  }

  function get_labels_token_types() {
    return array('comment', 'node');
  }

  function replace_tokens($label, $contexts, $content_id) {
    if ($content_id) {
      if (($comment = $this->fetch_content($content_id)) && ($node = node_load($comment->nid))) {
        $contexts['node'] = $node;
        $contexts['comment'] = $comment;
      }
    }
    return parent::replace_tokens($label, $contexts, $content_id);
  }

  function get_flag_action($content_id) {
    $flag_action = parent::get_flag_action($content_id);
    $comment = $this->fetch_content($content_id);
    $flag_action->content_title = $comment->subject;
    $flag_action->content_url = _flag_url("node/$comment->nid/$comment->cid", "comment-$comment->cid");
    return $flag_action;
  }

  function get_relevant_action_objects($content_id) {
    $comment = $this->fetch_content($content_id);
    return array(
      'comment' => $comment,
      'node' => node_load($comment->nid),
    );
  }

  function rules_get_event_arguments_definition() {
    return array(
      'comment' => array(
        'type' => 'comment',
        'label' => t('flagged comment'),
        'handler' => 'flag_rules_get_event_argument',
      ),
      'node' => array(
        'type' => 'node',
        'label' => t("the flagged comment's content"),
        'handler' => 'flag_rules_get_comment_content',
      ),
    );
  }
  
  function rules_get_element_argument_definition() {
    return array('type' => 'comment', 'label' => t('Flagged comment'));
  }

  function get_views_info() {
    return array(
      'views table' => 'comments',
      'join field' => 'cid',
      'title field' => 'subject',
      'title' => t('Comment flag'),
      'help' => t('Limit results to only those comments flagged by a certain flag; Or display information about the flag set on a comment.'),
      'counter title' => t('Comment flag counter'),
      'counter help' => t('Include this to gain access to the flag counter field.'),
    );
  }

  function applies_to_content_id_array($content_ids) {
    $passed = array();
    $content_ids  = implode(',', array_map('intval', $content_ids));
    $placeholders = implode(',', array_fill(0, sizeof($this->types), "'%s'"));
    $result = db_query("SELECT cid FROM {comments} c INNER JOIN {node} n ON c.nid = n.nid WHERE cid IN ($content_ids) and n.type IN ($placeholders)", $this->types);
    while ($row = db_fetch_object($result)) {
      $passed[$row->cid] = TRUE;
    }
    return $passed;
  }
}

/**
 * Implements a user flag.
 */
class flag_user extends flag_flag {
  function default_options() {
    $options = parent::default_options();
    $options += array(
      'show_on_profile' => TRUE,
    );
    return $options;
  }

  function options_form(&$form) {
    parent::options_form($form);
    $form['types'] = array(
      // A user flag doesn't support node types. (Maybe will support roles instead, in the future.)
      '#type' => 'value',
      '#value' => array(0 => 0),
    );
    $form['display']['show_on_profile'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display link on user profile page'),
      '#default_value' => $this->show_on_profile,
      '#access' => empty($this->locked['show_on_profile']),
    );
  }

  function _load_content($content_id) {
    return user_load(array('uid' => $content_id));
  }

  function applies_to_content_object($user) {
    // This user flag doesn't currently support subtypes so we return TRUE for
    // any user.
    if ($user) {
      return TRUE;
    }
    return FALSE;
  }

  function get_content_id($user) {
    return $user->uid;
  }

  function uses_hook_link($teaser) {
    if ($this->show_on_profile){
      return TRUE;
    }
    return FALSE;
  }

  function get_labels_token_types() {
    return array('user');
  }

  function replace_tokens($label, $contexts, $content_id) {
    if ($content_id && ($user = $this->fetch_content($content_id))) {
      $contexts['user'] = $user;
    }
    return parent::replace_tokens($label, $contexts, $content_id);
  }

  function get_flag_action($content_id) {
    $flag_action = parent::get_flag_action($content_id);
    $user = $this->fetch_content($content_id);
    $flag_action->content_title = $user->name;
    $flag_action->content_url = _flag_url('user/' . $user->uid);
    return $flag_action;
  }

  function get_relevant_action_objects($content_id) {
    return array(
      'user' => $this->fetch_content($content_id),
    );
  }

  function rules_get_event_arguments_definition() {
    return array(
      'account' => array(
        'type' => 'user',
        'label' => t('flagged user'),
        'handler' => 'flag_rules_get_event_argument',
      ),
    );
  }
  
  function rules_get_element_argument_definition() {
    return array('type' => 'user', 'label' => t('Flagged user'));
  }

  function get_views_info() {
    return array(
      'views table' => 'users',
      'join field' => 'uid',
      'title field' => 'name',
      'title' => t('User flag'),
      'help' => t('Limit results to only those users flagged by a certain flag; Or display information about the flag set on a user.'),
      'counter title' => t('User flag counter'),
      'counter help' => t('Include this to gain access to the flag counter field.'),
    );
  }

  function applies_to_content_id_array($content_ids) {
    // This user flag doesn't currently support subtypes so all users are
    // applicable for flagging.
    $passed = array();
    foreach ($content_ids as $uid) {
      if ($uid) { // Exclude anonymous.
        $passed[$uid] = TRUE;
      }
    }
    return $passed;
  }

}

/**
 * A dummy flag to be used where the real implementation can't be found.
 */
class flag_broken extends flag_flag {
  function options_form(&$form) {
    $form = array();
    $form['error'] = array(
      '#value' => '<div class="error">'. t("The module providing this flag wasn't found, or this flag type, %type, isn't valid.", array('%type' => $this->content_type)) .'</div>',
    );
  }
}

// Returns TRUE if we're running under Drupal 5.
//
// I use this function because I don't want to maintain two versions of a file
// just because a handful of lines of code.
function _flag_is_drupal_5() {
  return !function_exists('theme_get_registry');
}

// That's ugly, but duplicating this logic is uglier.
function _flag_url($path, $fragment = NULL, $absolute = TRUE) {
  return _flag_is_drupal_5()
    ? url($path, NULL, $fragment, $absolute)
    : url($path, array('absolute' => TRUE, 'fragment' => $fragment));
}

