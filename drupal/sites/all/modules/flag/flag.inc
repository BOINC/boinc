<?php

/**
 * @file
 * Implements various flags. Uses object oriented style inspired by that
 * of Views 2.
 */

/**
 * Implementation of hook_flag_definitions().
 *
 * Defines the flag types this module implements.
 *
 * @return
 *   An "array of arrays", keyed by content-types. The 'handler' slot
 *   should point to the PHP class implementing this flag.
 */
function flag_flag_definitions() {
  $definitions = array(
    'node' => array(
      'title' => t('Nodes'),
      'description' => t("Nodes are a Drupal site's primary content."),
      'handler' => 'flag_node',
    ),
    'user' => array(
      'title' => t('Users'),
      'description' => t('Users who have created accounts on your site.'),
      'handler' => 'flag_user',
    ),
  );

  if (module_exists('comment')) {
    $definitions['comment'] = array(
      'title' => t('Comments'),
      'description' => t('Comments are responses to node content.'),
      'handler' => 'flag_comment',
    );
  }

  return $definitions;
}

/**
 * Returns a flag definition.
 */
function flag_fetch_definition($content_type = NULL) {
  static $defintions;
  if (!isset($defintions)) {
    $defintions = module_invoke_all('flag_definitions');
    drupal_alter('flag_definitions', $defintions);
    if (!isset($defintions['node'])) {
      // We want our API to be available in hook_install, but our module is not
      // enabled by then, so let's load our implementation directly:
      $defintions += flag_flag_definitions();
    }
  }

  if (isset($content_type)) {
    if (isset($defintions[$content_type])) {
      return $defintions[$content_type];
    }
  }
  else {
    return $defintions;
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

  /**
   * The database ID.
   *
   * NULL for flags that haven't been saved to the database yet.
   *
   * @var integer
   */
  var $fid = NULL;

  /**
   * The content type (aka entity type) this flag works with.
   *
   * @var string
   */
  var $content_type = NULL;

  /**
   * The flag's "machine readable" name.
   *
   * @var string
   */
  var $name = '';

  /**
   * The human-readable title for this flag.
   *
   * @var string
   */
  var $title = '';

  /**
   * Whether this flag state should act as a single toggle to all users.
   *
   * @var bool
   */
  var $global = FALSE;

  /**
   * The sub-types, AKA bundles, this flag applies to.
   *
   * @var array
   */
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
   */
  static function factory_by_row($row) {
    $flag = flag_create_handler($row->content_type);

    // Lump all data unto the object...
    foreach ($row as $field => $value) {
      $flag->$field = $value;
    }
    // ...but skip the following two.
    unset($flag->options, $flag->type);

    // Populate the options with the defaults.
    $options = (array) unserialize($row->options);
    $options += $flag->options();

    // Make the unserialized options accessible as normal properties.
    foreach ($options as $option => $value) {
      $flag->$option = $value;
    }

    if (!empty($row->type)) {
      // The loop loading from the database should further populate this property.
      $flag->types[] = $row->type;
    }

    return $flag;
  }

  /**
   * Create a complete flag (except an FID) from an array definition.
   */
  static function factory_by_array($config) {
    $flag = flag_create_handler($config['content_type']);

    foreach ($config as $option => $value) {
      $flag->$option = $value;
    }

    if (isset($config['locked']) && is_array($config['locked'])) {
      $flag->locked = drupal_map_assoc($config['locked']);
    }

    return $flag;
  }

  /**
   * Another factory method. Returns a new, "empty" flag; e.g., one suitable for
   * the "Add new flag" page.
   */
  static function factory_by_content_type($content_type) {
    return flag_create_handler($content_type);
  }

  /**
   * Declares the options this flag supports, and their default values.
   *
   * Derived classes should want to override this.
   */
  function options() {
    $options = array(
      // The text for the "flag this" link for this flag.
      'flag_short' => '',
      // The description of the "flag this" link.
      'flag_long' => '',
      // Message displayed after flagging content.
      'flag_message' => '',
      // Likewise but for unflagged.
      'unflag_short' => '',
      'unflag_long' => '',
      'unflag_message' => '',
      'unflag_denied_text' => '',
      // The link type used by the flag, as defined in hook_flag_link_types().
      'link_type' => 'toggle',
      'roles' => array(
        'flag' => array(DRUPAL_AUTHENTICATED_RID),
        'unflag' => array(DRUPAL_AUTHENTICATED_RID),
      ),
      'weight' => 0,
    );

    // Merge in options from the current link type.
    $link_type = $this->get_link_type();
    $options = array_merge($options, $link_type['options']);

    // Allow other modules to change the flag options.
    drupal_alter('flag_options', $options, $this);
    return $options;
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
    $options = $this->options();
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
    $this->roles['flag'] = array_values(array_filter($this->roles['flag']));
    $this->roles['unflag'] = array_values(array_filter($this->roles['unflag']));
    $this->types = array_values(array_filter($this->types));
    // Clear internal titles cache:
    $this->get_title(NULL, TRUE);
  }

  /**
   * Validates this flag's options.
   *
   * @return
   *   A list of errors encountered while validating this flag's options.
   */
  function validate() {
    // TODO: It might be nice if this used automatic method discovery rather
    // than hard-coding the list of validate functions.
    return array_merge_recursive(
      $this->validate_name(),
      $this->validate_access()
    );
  }

  /**
   * Validates that the current flag's name is valid.
   *
   * @return
   *   A list of errors encountered while validating this flag's name.
   */
  function validate_name() {
    $errors = array();

    // Ensure a safe machine name.
    if (!preg_match('/^[a-z_][a-z0-9_]*$/', $this->name)) {
      $errors['name'][] = array(
        'error' => 'flag_name_characters',
        'message' => t('The flag name may only contain lowercase letters, underscores, and numbers.'),
      );
    }
    // Ensure the machine name is unique.
    $flag = flag_get_flag($this->name);
    if (!empty($flag) && (!isset($this->fid) || $flag->fid != $this->fid)) {
      $errors['name'][] = array(
        'error' => 'flag_name_unique',
        'message' => t('Flag names must be unique. This flag name is already in use.'),
      );
    }

    return $errors;
  }

  /**
   * Validates that the current flag's access settings are valid.
   */
  function validate_access() {
    $errors = array();

    // Require an unflag access denied message a role is not allowed to unflag.
    if (empty($this->unflag_denied_text)) {
      foreach ($this->roles['flag'] as $key => $rid) {
        if ($rid && empty($this->roles['unflag'][$key])) {
          $errors['unflag_denied_text'][] = array(
            'error' => 'flag_denied_text_required',
            'message' => t('The "Unflag not allowed text" is required if any user roles are not allowed to unflag.'),
          );
          break;
        }
      }
    }

    // Do not allow unflag access without flag access.
    foreach ($this->roles['unflag'] as $key => $rid) {
      if ($rid && empty($this->roles['flag'][$key])) {
        $errors['roles'][] = array(
          'error' => 'flag_roles_unflag',
          'message' => t('Any user role that has the ability to unflag must also have the ability to flag.'),
        );
        break;
      }
    }

    return $errors;
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
   * @defgroup access Access control
   * @{
   */

  /**
   * Returns TRUE if the flag applies to the given content.
   *
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
   * Determines whether the user has access to use this flag.
   *
   * @param $action
   *   Optional. The action to test, either "flag" or "unflag". If none given,
   *   "flag" will be tested, which is the minimum permission to use a flag.
   * @param $account
   *   Optional. The user object. If none given, the current user will be used.
   *
   * @return
   *   Boolean TRUE if the user is allowed to flag/unflag. FALSE otherwise.
   */
  function user_access($action = 'flag', $account = NULL) {
    if (!isset($account)) {
      $account = $GLOBALS['user'];
    }

    // Anonymous user can't use this system unless Session API is installed.
    if ($account->uid == 0 && !module_exists('session_api')) {
      return FALSE;
    }

    $matched_roles = array_intersect($this->roles[$action], array_keys($account->roles));
    return !empty($matched_roles) || $account->uid == 1;
  }

  /**
   * Determines whether the user may flag, or unflag, the given content.
   *
   * This method typically should not be overridden by child classes. Instead
   * they should implement type_access(), which is called by this method.
   *
   * @param $content_id
   *   The content ID to flag/unflag.
   * @param $action
   *   The action to test. Either 'flag' or 'unflag'. Leave NULL to determine
   *   by flag status.
   * @param $account
   *   The user on whose behalf to test the flagging action. Leave NULL for the
   *   current user.
   *
   * @return
   *   Boolean TRUE if the user is allowed to flag/unflag the given content.
   *   FALSE otherwise.
   */
  function access($content_id, $action = NULL, $account = NULL) {
    if (!isset($account)) {
      $account = $GLOBALS['user'];
    }

    if (isset($content_id) && !$this->applies_to_content_id($content_id)) {
      // Flag does not apply to this content.
      return FALSE;
    }

    if (!isset($action)) {
      $uid = $account->uid;
      $sid = flag_get_sid($uid);
      $action = $this->is_flagged($content_id, $uid, $sid) ? 'unflag' : 'flag';
    }

    // Base initial access on the user's basic permission to use this flag.
    $access = $this->user_access($action, $account);

    // Check for additional access rules provided by sub-classes.
    $child_access = $this->type_access($content_id, $action, $account);
    if (isset($child_access)) {
      $access = $child_access;
    }

    // Allow modules to disallow (or allow) access to flagging.
    $access_array = module_invoke_all('flag_access', $this, $content_id, $action, $account);

    foreach ($access_array as $set_access) {
      if (isset($set_access)) {
        $access = $set_access;
      }
    }

    return $access;
  }

  /**
   * Determine access to multiple objects.
   *
   * Similar to user_access() but works on multiple IDs at once. Called in the
   * pre_render() stage of the 'Flag links' field within Views to find out where
   * that link applies. The reason we do a separate DB query, and not lump this
   * test in the Views query, is to make 'many to one' tests possible without
   * interfering with the rows, and also to reduce the complexity of the code.
   *
   * This method typically should not be overridden by child classes. Instead
   * they should implement type_access_multiple(), which is called by this
   * method.
   *
   * @param $content_ids
   *   The array of content IDs to check. The keys are the content IDs, the
   *   values are the actions to test: either 'flag' or 'unflag'.
   * @param $account
   *   Optional. The account for which the actions will be compared against.
   *   If left empty, the current user will be used.
   *
   * @return
   *   An array whose keys are the object IDs and values are booleans indicating
   *   access.
   *
   * @see hook_flag_access_multiple()
   */
  function access_multiple($content_ids, $account = NULL) {
    $account = isset($account) ? $account : $GLOBALS['user'];
    $access = array();

    // First check basic user access for this action.
    foreach ($content_ids as $content_id => $action) {
      $access[$content_id] = $this->user_access($content_ids[$content_id], $account);
    }

    // Check for additional access rules provided by sub-classes.
    $child_access = $this->type_access_multiple($content_ids, $account);
    if (isset($child_access)) {
      foreach ($child_access as $content_id => $content_access) {
        if (isset($content_access)) {
          $access[$content_id] = $content_access;
        }
      }
    }

    // Merge in module-defined access.
    foreach (module_implements('flag_access_multiple') as $module) {
      $module_access = module_invoke($module, 'flag_access_multiple', $this, $content_ids, $account);
      foreach ($module_access as $content_id => $content_access) {
        if (isset($content_access)) {
          $access[$content_id] = $content_access;
        }
      }
    }

    return $access;
  }

  /**
   * Implements access() implemented by each child class.
   *
   * @abstract
   */
  function type_access($content_id, $action, $account) {
    return NULL;
  }

  /**
   * Implements access_multiple() implemented by each child class.
   *
   * @abstract
   */
  function type_access_multiple($content_ids, $account) {
    return array();
  }

  /**
   * @} End of "defgroup access".
   */

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
   * Returns TRUE if this flag requires anonymous user cookies.
   */
  function uses_anonymous_cookies() {
    global $user;
    return $user->uid == 0 && variable_get('cache', 0);
  }

  /**
   * Flags, or unflags, an item.
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
     // Get the user.
    if (!isset($account)) {
      $account = $GLOBALS['user'];
    }
    if (!$account) {
      return FALSE;
    }

    // Check access and applicability.
    if (!$skip_permission_check) {
      if (!$this->access($content_id, $action, $account)) {
        // User has no permission to flag/unflag this object.
        return FALSE;
      }
    }
    else {
      // We are skipping permission checks. However, at a minimum we must make
      // sure the flag applies to this content type:
      if (!$this->applies_to_content_id($content_id)) {
        return FALSE;
      }
    }

    // Clear various caches; We don't want code running after us to report
    // wrong counts or false flaggings.
    flag_get_counts(NULL, NULL, TRUE);
    flag_get_user_flags(NULL, NULL, NULL, NULL, TRUE);

    // Find out which user id to use.
    $uid = $this->global ? 0 : $account->uid;

    // Find out which session id to use.
    if ($this->global) {
      $sid = 0;
    }
    else {
      $sid = flag_get_sid($uid, TRUE);
      // Anonymous users must always have a session id.
      if ($sid == 0 && $account->uid == 0) {
        return FALSE;
      }
    }

    // Perform the flagging or unflagging of this flag.
    $flagged = $this->_is_flagged($content_id, $uid, $sid);
    if ($action == 'unflag') {
      if ($this->uses_anonymous_cookies()) {
        $this->_unflag_anonymous($content_id);
      }
      if ($flagged) {
        $fcid = $this->_unflag($content_id, $uid, $sid);
        module_invoke_all('flag', 'unflag', $this, $content_id, $account, $fcid);
      }
    }
    elseif ($action == 'flag') {
      if ($this->uses_anonymous_cookies()) {
        $this->_flag_anonymous($content_id);
      }
      if (!$flagged) {
        $fcid = $this->_flag($content_id, $uid, $sid);
        module_invoke_all('flag', 'flag', $this, $content_id, $account, $fcid);
      }
    }

    return TRUE;
  }

  /**
   * Determines if a certain user has flagged this content.
   *
   * Thanks to using a cache, inquiring several different flags about the same
   * item results in only one SQL query.
   *
   * @param $uid
   *   Optional. The user ID whose flags we're checking. If none given, the
   *   current user will be used.
   *
   * @return
   *   TRUE if the content is flagged, FALSE otherwise.
   */
  function is_flagged($content_id, $uid = NULL, $sid = NULL) {
    return (bool) $this->get_flagging_record($content_id, $uid, $sid);
  }

  /**
   * Returns the flagging record.
   *
   * This method returns the "flagging record": the {flag_content} record that
   * exists for each flagged item (for a certain user). If the item isn't
   * flagged, returns NULL. This method could be useful, for example, when you
   * want to find out the 'fcid' or 'timestamp' values.
   *
   * Thanks to using a cache, inquiring several different flags about the same
   * item results in only one SQL query.
   *
   * Parameters are the same as is_flagged()'s.
   */
  function get_flagging_record($content_id, $uid = NULL, $sid = NULL) {
    $uid = $this->global ? 0 : (!isset($uid) ? $GLOBALS['user']->uid : $uid);
    $sid = $this->global ? 0 : (!isset($sid) ? flag_get_sid($uid) : $sid);

    // flag_get_user_flags() does caching.
    $user_flags = flag_get_user_flags($this->content_type, $content_id, $uid, $sid);
    return isset($user_flags[$this->name]) ? $user_flags[$this->name] : NULL;
  }

  /**
   * Determines if a certain user has flagged this content.
   *
   * You probably shouldn't call this raw private method: call the
   * is_flagged() method instead.
   *
   * This method is similar to is_flagged() except that it does direct SQL and
   * doesn't do caching. Use it when you want to not affect the cache, or to
   * bypass it.
   *
   * @return
   *   If the content is flagged, returns the value of the 'fcid' column.
   *   Else, returns FALSE.
   *
   * @private
   */
  function _is_flagged($content_id, $uid, $sid) {
    return db_result(db_query("SELECT fcid FROM {flag_content} WHERE fid = %d AND uid = %d AND sid = %d AND content_id = %d", $this->fid, $uid, $sid, $content_id));
  }

  /**
   * A low-level method to flag content.
   *
   * You probably shouldn't call this raw private method: call the flag()
   * function instead.
   *
   * @return
   *   The 'fcid' column of the new {flag_content} record.
   *
   * @private
   */
  function _flag($content_id, $uid, $sid) {
    db_query("INSERT INTO {flag_content} (fid, content_type, content_id, uid, sid, timestamp) VALUES (%d, '%s', %d, %d, %d, %d)", $this->fid, $this->content_type, $content_id, $uid, $sid, time());
    $fcid = db_last_insert_id('flag_content', 'fcid');
    $this->_increase_count($content_id);
    return $fcid;
  }

  /**
   * A low-level method to unflag content.
   *
   * You probably shouldn't call this raw private method: call the flag()
   * function instead.
   *
   * @return
   *   If the content was flagged, returns the value of the now deleted 'fcid'
   *   column. Else, returns FALSE.
   *
   * @private
   */
  function _unflag($content_id, $uid, $sid) {
    $fcid = db_result(db_query("SELECT fcid FROM {flag_content} WHERE fid = %d AND content_id = %d AND uid = %d AND sid = %d", $this->fid, $content_id, $uid, $sid));
    if ($fcid) {
      db_query("DELETE FROM {flag_content} WHERE fcid = %d", $fcid);
      $this->_decrease_count($content_id);
    }
    return $fcid;
  }

  /**
   * Increases the flag count for a piece of content.
   *
   * @param $content_id
   *   For which item should the count be increased.
   * @param $number
   *   The amount of counts to increasing. Defaults to 1.
   *
   * @private
   */
  function _increase_count($content_id, $number = 1) {
    $time = time();
    db_query("UPDATE {flag_counts} SET count = count + %d, last_updated = %d WHERE fid = %d AND content_id = %d", $number, $time, $this->fid, $content_id);
    if (!db_affected_rows()) {
      db_query("INSERT INTO {flag_counts} (fid, content_type, content_id, count, last_updated) VALUES (%d, '%s', %d, %d, %d)", $this->fid, $this->content_type, $content_id, $number, $time);
    }
  }

  /**
   * Decreases the flag count for a piece of content.
   *
   * @param $content_id
   *   For which item should the count be descreased.
   * @param $number
   *   The amount of counts to decrease. Defaults to 1.
   *
   * @private
   */
  function _decrease_count($content_id, $number = 1) {
    $time = time();
    db_query("UPDATE {flag_counts} SET count = count - %d, last_updated = %d WHERE fid = %d AND content_id = %d", $number, $time, $this->fid, $content_id);
    // Delete rows with count 0, for data consistency and space-saving.
    db_query("DELETE FROM {flag_counts} WHERE fid = %d and content_id = %d AND count = %d", $this->fid, $content_id, 0);
  }

  /**
   * Set a cookie for anonymous users to record their flagging.
   *
   * @private
   */
  function _flag_anonymous($content_id) {
    $storage = FlagCookieStorage::factory($this);
    $storage->flag($content_id);
  }

  /**
   * Remove the cookie for anonymous users to record their unflagging.
   *
   * @private
   */
  function _unflag_anonymous($content_id) {
    $storage = FlagCookieStorage::factory($this);
    $storage->unflag($content_id);
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
   * For global flags, pass '0' as the user ID and session ID.
   */
  function get_user_count($uid, $sid = NULL) {
    if (!isset($sid)) {
      $sid = flag_get_sid($uid);
    }
    return db_result(db_query('SELECT COUNT(*) FROM {flag_content} WHERE fid = %d AND uid = %d AND sid = %d', $this->fid, $uid, $sid));
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
   * Get the link type for this flag.
   */
  function get_link_type() {
    $link_types = flag_get_link_types();
    return (isset($this->link_type) && isset($link_types[$this->link_type])) ? $link_types[$this->link_type] : $link_types['normal'];
  }

  /**
   * Replaces tokens in a label. Only the 'global' token context is recognized
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
    return array();
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
   * @} End of "addtogroup views".
   */

  /**
   * Saves a flag to the database. It is a wrapper around update() and insert().
   */
  function save() {
    if (isset($this->fid)) {
      $this->update();
      $this->is_new = FALSE;
    }
    else {
      $this->insert();
      $this->is_new = TRUE;
    }
    // Clear the page cache for anonymous users.
    cache_clear_all('*', 'cache_page', TRUE);
  }

  /**
   * Saves an existing flag to the database. Better use save().
   */
  function update() {
    db_query("UPDATE {flags} SET name = '%s', title = '%s', global = %d, options = '%s' WHERE fid = %d", $this->name, $this->title, $this->global, $this->get_serialized_options(), $this->fid);
    db_query("DELETE FROM {flag_types} WHERE fid = %d", $this->fid);
    foreach ($this->types as $type) {
      db_query("INSERT INTO {flag_types} (fid, type) VALUES (%d, '%s')", $this->fid, $type);
    }
  }

  /**
   * Saves a new flag to the database. Better use save().
   */
  function insert() {
    db_query("INSERT INTO {flags} (content_type, name, title, global, options) VALUES ('%s', '%s', '%s', %d, '%s')", $this->content_type, $this->name, $this->title, $this->global, $this->get_serialized_options());
    $this->fid = db_last_insert_id('flags', 'fid');
    foreach ($this->types as $type) {
      db_query("INSERT INTO {flag_types} (fid, type) VALUES (%d, '%s')", $this->fid, $type);
    }
  }

  /**
   * Options are stored serialized in the database.
   */
  function get_serialized_options() {
    $option_names = array_keys($this->options());
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
    module_invoke_all('flag_delete', $this);
  }

  /**
   * Returns TRUE if this flag's declared API version is compatible with this
   * module.
   *
   * An "incompatible" flag is one exported (and now being imported or exposed
   * via hook_flag_default_flags()) by a different version of the Flag module.
   * An incompatible flag should be treated as a "black box": it should not be
   * saved or exported because our code may not know to handle its internal
   * structure.
   */
  function is_compatible() {
    if (isset($this->fid)) {
      // Database flags are always compatible.
      return TRUE;
    }
    else {
      if (!isset($this->api_version)) {
        $this->api_version = 1;
      }
      return $this->api_version == FLAG_API_VERSION;
    }
  }

  /**
   * Finds the "default flag" corresponding to this flag.
   *
   * Flags defined in code ("default flags") can be overridden. This method
   * returns the default flag that is being overridden by $this. Returns NULL
   * if $this overrides no default flag.
   */
  function find_default_flag() {
    if ($this->fid) {
      $default_flags = flag_get_default_flags(TRUE);
      if (isset($default_flags[$this->name])) {
        return $default_flags[$this->name];
      }
    }
  }

  /**
   * Reverts an overriding flag to its default state.
   *
   * Note that $this isn't altered. To see the reverted flag you'll have to
   * call flag_get_flag($this->name) again.
   *
   * @return
   *   TRUE if the flag was reverted successfully; FALSE if there was an error;
   *   NULL if this flag overrides no default flag.
   */
  function revert() {
    if (($default_flag = $this->find_default_flag())) {
      if ($default_flag->is_compatible()) {
        $default_flag = clone $default_flag;
        $default_flag->fid = $this->fid;
        $default_flag->save();
        flag_get_flags(NULL, NULL, NULL, TRUE);
        return TRUE;
      }
      else {
        return FALSE;
      }
    }
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
   * Returns administrative menu path for carrying out some action.
   */
  function admin_path($action) {
    if ($action == 'edit') {
      // Since 'edit' is the default tab, we omit the action.
      return FLAG_ADMIN_PATH . '/manage/' . $this->name;
    }
    else {
      return FLAG_ADMIN_PATH . '/manage/' . $this->name . '/' . $action;
    }
  }

  /**
   * Renders a flag/unflag link. This is a wrapper around theme('flag') that,
   * in Drupal 6, easily channels the call to the right template file.
   *
   * For parameters docmentation, see theme_flag().
   */
  function theme($action, $content_id, $after_flagging = FALSE) {
    static $js_added = array();
    global $user;

    // If the flagging user is anonymous, set a boolean for the benefit of
    // JavaScript code. Currently, only our "anti-crawlers" mechanism uses it.
    if ($user->uid == 0 && !isset($js_added['anonymous'])) {
      $js_added['anonymous'] = TRUE;
      drupal_add_js(array('flag' => array('anonymous' => TRUE)), 'setting');
    }

    // If the flagging user is anonymous and the page cache is enabled, we
    // update the links through JavaScript.
    if ($this->uses_anonymous_cookies() && !$after_flagging) {
      if ($this->global) {
        // In case of global flags, the JavaScript template is to contain
        // the opposite of the current state.
        $js_action = ($action == 'flag' ? 'unflag' : 'flag');
      }
      else {
        // In case of non-global flags, we always show the "flag!" link,
        // and then replace it with the "unflag!" link through JavaScript.
        $js_action = 'unflag';
        $action = 'flag';
      }
      if (!isset($js_added[$this->name . '_' . $content_id])) {
        $js_added[$this->name . '_' . $content_id] = TRUE;
        $js_template = theme($this->theme_suggestions(), $this, $js_action, $content_id, $after_flagging);
        drupal_add_js(array('flag' => array('templates' => array($this->name . '_' . $content_id => $js_template))), 'setting');
      }
    }

    return theme($this->theme_suggestions(), $this, $action, $content_id, $after_flagging);
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
  function options() {
    $options = parent::options();
    $options += array(
      'show_on_page' => TRUE,
      'show_on_teaser' => TRUE,
      'show_on_form' => FALSE,
      'access_author' => '',
      'i18n' => 0,
    );
    return $options;
  }

  function options_form(&$form) {
    parent::options_form($form);
    // Support for i18n flagging requires Translation helpers module.
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

    $form['access']['access_author'] = array(
      '#type' => 'radios',
      '#title' => t('Flag access by content authorship'),
      '#options' => array(
        '' => t('No additional restrictions'),
        'own' => t('Users may only flag content they own'),
        'others' => t('Users may only flag content of others'),
      ),
      '#default_value' => $this->access_author,
      '#description' => t("Restrict access to this flag based on the user's ownership of the content. Users must also have access to the flag through the role settings."),
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

  function type_access_multiple($content_ids, $account) {
    $access = array();

    // Ensure that only flaggable node types are granted access. This avoids a
    // node_load() on every type, usually done by applies_to_content_id().
    $nids = implode(',', array_map('intval', array_keys($content_ids)));
    $placeholders = implode(',', array_fill(0, sizeof($this->types), "'%s'"));
    $result = db_query("SELECT nid as content_id FROM {node} WHERE nid IN ($nids) AND type NOT IN ($placeholders)", $this->types);
    while ($row = db_fetch_object($result)) {
      $access[$row->content_id] = FALSE;
    }

    return $access;
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

  // Instead of overriding is_flagged() we override get_flagging_record(),
  // which is the underlying method.
  function get_flagging_record($content_id, $uid = NULL, $sid = NULL) {
    $content_id = $this->get_translation_id($content_id);
    return parent::get_flagging_record($content_id, $uid, $sid);
  }

  function get_labels_token_types() {
    return array_merge(array('node'), parent::get_labels_token_types());
  }

  function replace_tokens($label, $contexts, $content_id) {
    if (is_numeric($content_id) && ($node = $this->fetch_content($content_id))) {
      $contexts['node'] = $node;
    }
    // Nodes accept the node-type as a $content_id in the case that a new node
    // is being created and a full node object does not yet exist.
    elseif (!empty($content_id) && ($type = node_get_types('type', $content_id))) {
      $content_id = NULL;
      $contexts['node'] = (object) array(
        'nid' => NULL,
        'type' => $type->type,
        'title' => '',
      );
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
      'node_author' => array(
        'type' => 'user',
        'label' => t('flagged content author'),
        'handler' => 'flag_rules_get_node_author',
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
}

/**
 * Implements a comment flag.
 */
class flag_comment extends flag_flag  {
  function options() {
    $options = parent::options();
    $options += array(
      'access_author' => '',
      'show_on_comment' => TRUE,
    );
    return $options;
  }

  function options_form(&$form) {
    parent::options_form($form);

    $form['access']['access_author'] = array(
      '#type' => 'radios',
      '#title' => t('Flag access by content authorship'),
      '#options' => array(
        '' => t('No additional restrictions'),
        'comment_own' => t('Users may only flag own comments'),
        'comment_others' => t('Users may only flag comments by others'),
        'node_own' => t('Users may only flag comments of nodes they own'),
        'node_others' => t('Users may only flag comments of nodes by others'),
      ),
      '#default_value' => $this->access_author,
      '#description' => t("Restrict access to this flag based on the user's ownership of the content. Users must also have access to the flag through the role settings."),
    );

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

  function type_access_multiple($content_ids, $account) {
    $access = array();

    // Ensure node types are granted access. This avoids a
    // node_load() on every type, usually done by applies_to_content_id().
    $cids = implode(',', array_map('intval', array_keys($content_ids)));
    $placeholders = implode(',', array_fill(0, sizeof($this->types), "'%s'"));
    $result = db_query("SELECT cid as content_id FROM {comments} c INNER JOIN {node} n ON c.nid = n.nid WHERE cid IN ($cids) and n.type NOT IN ($placeholders)", $this->types);
    while ($row = db_fetch_object($result)) {
      $access[$row->content_id] = FALSE;
    }

    return $access;
  }

  function get_content_id($comment) {
    // Store the comment object in the static cache, to avoid getting it
    // again unneedlessly.
    $this->remember_content($comment->cid, $comment);
    return $comment->cid;
  }

  function uses_hook_link($teaser) {
    return $this->show_on_comment;
  }

  function get_labels_token_types() {
    return array_merge(array('comment', 'node'), parent::get_labels_token_types());
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
      'comment_author' => array(
        'type' => 'user',
        'label' => t('flagged comment author'),
        'handler' => 'flag_rules_get_comment_author',
      ),
      'node' => array(
        'type' => 'node',
        'label' => t("the flagged comment's content"),
        'handler' => 'flag_rules_get_comment_content',
      ),
      'node_author' => array(
        'type' => 'user',
        'label' => t('commented content author'),
        'handler' => 'flag_rules_get_comment_node_author',
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
}

/**
 * Implements a user flag.
 */
class flag_user extends flag_flag {
  function options() {
    $options = parent::options();
    $options += array(
      'show_on_profile' => TRUE,
      'access_uid' => '',
    );
    return $options;
  }

  function options_form(&$form) {
    parent::options_form($form);
    $form['access']['types'] = array(
      // A user flag doesn't support node types.
      // TODO: Maybe support roles instead of node types.
      '#type' => 'value',
      '#value' => array(0 => 0),
    );
    $form['access']['access_uid'] = array(
      '#type' => 'checkbox',
      '#title' => t('Users may flag themselves'),
      '#description' => t('Disabling this option may be useful when setting up a "friend" flag, when a user flagging themself does not make sense.'),
      '#default_value' => $this->access_uid ? 0 : 1,
    );
    $form['display']['show_on_profile'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display link on user profile page'),
      '#default_value' => $this->show_on_profile,
      '#access' => empty($this->locked['show_on_profile']),
    );
  }

  function form_input($form_values) {
    parent::form_input($form_values);
    // The access_uid value is intentionally backwards from the UI, to avoid
    // confusion caused by checking a box to disable a feature.
    $this->access_uid = empty($form_values['access_uid']) ? 'others' : '';
  }

  function _load_content($content_id) {
    return user_load($content_id);
  }

  function applies_to_content_object($user) {
    // This user flag doesn't currently support subtypes so we return TRUE for
    // any user.
    if ($user) {
      return TRUE;
    }
    return FALSE;
  }

  function type_access($content_id, $action, $account) {
    // Prevent users from flagging themselves.
    if ($this->access_uid == 'others' && $content_id == $account->uid) {
      return FALSE;
    }
  }

  function type_access_multiple($content_ids, $account) {
    $access = array();

    // Exclude anonymous.
    if (array_key_exists(0, $content_ids)) {
      $access[0] = FALSE;
    }

    // Prevent users from flagging themselves.
    if ($this->access_uid == 'others' && array_key_exists($account->uid, $content_ids)) {
      $access[$account->uid] = FALSE;
    }

    return $access;
  }

  function get_content_id($user) {
    return $user->uid;
  }

  function uses_hook_link($teaser) {
    if ($this->show_on_profile) {
      return TRUE;
    }
    return FALSE;
  }

  function get_labels_token_types() {
    return array_merge(array('user'), parent::get_labels_token_types());
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

/**
 * A shortcut function to output the link URL.
 */
function _flag_url($path, $fragment = NULL, $absolute = TRUE) {
  return url($path, array('fragment' => $fragment, 'absolute' => $absolute));
}


/**
 * Utility class to handle cookies.
 *
 * Cookies are used to record flaggings for anonymous users on cached pages.
 *
 * This class contains only two instance methods. Usage example:
 * @code
 *   $storage = FlagCookieStorage::factory($flag);
 *   $storage->flag(145);
 *   $storage->unflag(17);
 * @endcode
 *
 * You may delete all the cookies with <code>FlagCookieStorage::drop()</code>.
 */
abstract class FlagCookieStorage {

  /**
   * Returns the actual storage object compatible with the flag.
   */
  static function factory($flag) {
    if ($flag->global) {
      return new FlagGlobalCookieStorage($flag);
    }
    else {
      return new FlagNonGlobalCookieStorage($flag);
    }
  }

  function __construct($flag) {
    $this->flag = $flag;
  }

  /**
   * "Flags" an item.
   *
   * It just records this fact in a cookie.
   */
  abstract function flag($content_id);

  /**
   * "Unflags" an item.
   *
   * It just records this fact in a cookie.
   */
  abstract function unflag($content_id);

  /**
   * Deletes all the cookies.
   *
   * (Etymology: "drop" as in "drop database".)
   */
  static function drop() {
    FlagGlobalCookieStorage::drop();
    FlagNonGlobalCookieStorage::drop();
  }
}

/**
 * Storage handler for global flags.
 */
class FlagGlobalCookieStorage extends FlagCookieStorage {

  function flag($content_id) {
    $cookie_key = $this->cookie_key($content_id);
    setcookie($cookie_key, 1, time() + $this->get_lifetime(), base_path());
    $_COOKIE[$cookie_key] = 1;
  }

  function unflag($content_id) {
    $cookie_key = $this->cookie_key($content_id);
    setcookie($cookie_key, 0, time() + $this->get_lifetime(), base_path());
    $_COOKIE[$cookie_key] = 0;
  }

  // Global flags persist for the length of the minimum cache lifetime.
  protected function get_lifetime() {
    $cookie_lifetime = variable_get('cache', 0) ? variable_get('cache_lifetime', 0) : -1;
    // Do not let the cookie lifetime be 0 (which is the no cache limit on
    // anonymous page caching), since it would expire immediately. Usually
    // the no cache limit means caches are cleared on cron, which usually runs
    // at least once an hour.
    if ($cookie_lifetime == 0) {
      $cookie_lifetime = 3600;
    }
    return $cookie_lifetime;
  }

  protected function cookie_key($content_id) {
    return 'flag_global_' . $this->flag->name . '_' . $content_id;
  }

  /**
   * Deletes all the global cookies.
   */
  static function drop() {
    foreach ($_COOKIE as $key => $value) {
      if (strpos($key, 'flag_global_') === 0) {
        setcookie($key, FALSE, 0, base_path());
        unset($_COOKIE[$key]);
      }
    }
  }
}
/**
 * Storage handler for non-global flags.
 */
class FlagNonGlobalCookieStorage extends FlagCookieStorage {

  // The anonymous per-user flaggings are stored in a single cookie, so that
  // all of them persist as long as the Drupal cookie lifetime.

  function __construct($flag) {
    parent::__construct($flag);
    $this->flaggings = isset($_COOKIE['flags']) ? explode(' ', $_COOKIE['flags']) : array();
  }

  function flag($content_id) {
    if (!$this->is_flagged($content_id)) {
      $this->flaggings[] = $this->cookie_key($content_id);
      $this->write();
    }
  }

  function unflag($content_id) {
    if (($index = $this->index_of($content_id)) !== FALSE) {
      unset($this->flaggings[$index]);
      $this->write();
    }
  }

  protected function get_lifetime() {
    return min((int) ini_get('session.cookie_lifetime'), (int) ini_get('session.gc_maxlifetime'));
  }

  protected function cookie_key($content_id) {
    return $this->flag->name . '_' . $content_id;
  }

  protected function write() {
    $serialized = implode(' ', array_filter($this->flaggings));
    setcookie('flags', $serialized, time() + $this->get_lifetime(), base_path());
    $_COOKIE['flags'] = $serialized;
  }

  protected function is_flagged($content_id) {
    return $this->index_of($content_id) !== FALSE;
  }

  protected function index_of($content_id) {
    return array_search($this->cookie_key($content_id), $this->flaggings);
  }

  /**
   * Deletes the cookie.
   */
  static function drop() {
    if (isset($_COOKIE['flags'])) {
      setcookie('flags', FALSE, 0, base_path());
      unset($_COOKIE['flags']);
    }
  }
}

