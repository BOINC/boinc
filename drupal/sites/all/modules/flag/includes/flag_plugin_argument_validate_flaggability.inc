<?php
// $Id: flag_plugin_argument_validate_flaggability.inc,v 1.1.2.3 2008/12/03 14:10:00 mooffie Exp $

/**
 * @file
 * Contains the flaggability validator handler.
 */

/**
 * Validates whether an argument is a flaggable/flagged object.
 *
 * @ingroup views
 */
class flag_plugin_argument_validate_flaggability extends views_plugin_argument_validate {

  function construct() {
    parent::construct();
    $this->flag_type = $this->definition['flag type'];
  }

  function validate_form(&$form, &$form_state) {
    $options = $this->flags_options();

    $form[$this->_option_name('flag_name')] = array(
      '#type' => 'radios',
       // Add an ID to the surrounding div because radios don't get IDs
       // as a whole group. This is needed for #dependency.
      '#prefix' => '<div><div id="edit-options-' . views_css_safe($this->_option_name('flag_name')) . '">',
      '#suffix' => '</div></div>',
      '#title' => t('Flag'),
      '#options' => $options,
      '#default_value' => $this->_get_option('flag_name', '*relationship*'),
      '#description' => t('Select the flag to validate against.'),
      '#process' => array('expand_radios', 'views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );
    if (!$options) {
      $form[$this->_option_name('flag_name')]['#description'] = '<p class="warning">' . t('No %type flags exist. You must first <a href="@create-url">create a %type flag</a> before being able to use one.', array('%type' => $this->flag_type, '@create-url' => 'admin/build/flags')) . '</p>';
    }

    $form[$this->_option_name('flag_test')] = array(
      '#type' => 'select',
      '#title' => t('Validate the @type only if', array('@type' => $this->flag_type)),
      '#options' => $this->tests_options(),
      '#default_value' => $this->_get_option('flag_test', 'flaggable'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    // This validator supports the "multiple IDs" syntax. It may not be
    // extremely useful, but similar validators do support this and it's a good
    // thing to be compatible.
    $form[$this->_option_name('flag_id_type')] = array(
      '#type' => 'select',
      '#title' => t('Argument type'),
      '#options' => array(
        'id' => t('ID'),
        'ids' => t('IDs separated by , or +'),
      ),
      '#default_value' => $this->_get_option('flag_id_type', 'id'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );
  }

  /**
   * Returns form #options for the flags. Returns empty array if no flags were
   * found.
   */
  function flags_options() {
    $flags = flag_get_flags($this->flag_type);
    if (!$flags) {
      return array();
    }
    else {
      foreach ($flags as $flag) {
        $options[$flag->name] = $flag->get_title();
      }
      $options['*relationship*'] = t('<em>Pick the first flag mentioned in the relationships.</em>');
      return $options;
    }
  }

  /**
   * Validator arguments are stored in the argument object, not here, so we
   * define a convenience method for fetching them.
   */
  function _get_option($option, $default) {
    $option = $this->_option_name($option);
    return isset($this->argument->options[$option]) ? $this->argument->options[$option] : $default;
  }

  function _option_name($option) {
    // We must embed the flag type in the option name or else validators of
    // different types will clash with each other. It's a trait of Views that all
    // validators on the system get their settings lumped onto the argument.
    // Examine the view's 'Export' output to understand.
    return 'validate_argument_' . $this->flag_type . '_' . $option;
  }

  /**
   * Declares all tests. This scheme makes it easy for derived classes to add
   * and remove tests.
   */
  function tests_info($which = NULL) {
    return array(
      'flaggable' => array(
        'title' => t('It is flaggable'),
        'callback' => 'test_flaggable',
      ),
      'flagged' => array(
        'title' => t('It is flagged at least once'),
        'callback' => 'test_flagged',
      ),
      'flagged_by_current_user' => array(
        'title' => t('It is flagged by the current user'),
        'callback' => 'test_flagged_by_current_user',
      ),
    );
  }

  function tests_options() {
    $options = array();
    foreach ($this->tests_info() as $id => $info) {
      $options[$id] = $info['title'];
    }
    return $options;
  }

  function get_flag() {
    $flag_name = $this->_get_option('flag_name', '*relationship*');

    if ($flag_name == '*relationship*') {
      // Pick the first flag mentioned in the relationships.
      foreach ($this->view->relationship as $id => $handler) {
        // Note: we can't do $handler->field, because the relationship handler's init() may overwrite it.
        if (strpos($handler->options['field'], 'flag') !== FALSE && !empty($handler->options['flag'])) {
          $flag = flag_get_flag($handler->options['flag']);
          if ($flag && $flag->content_type == $this->flag_type) {
            return $flag;
          }
        }
      }
    }

    return flag_get_flag($flag_name);
  }

  /**
   * Tests whether the argument is flaggable, or flagged, or flagged by current
   * user. These are three possible tests, and which of the three to actually
   * carry out is determined by 'flag_test'.
   */
  function validate_argument($argument) {
    $flag_test = $this->_get_option('flag_test', 'flaggable');
    $id_type   = $this->_get_option('flag_id_type', 'id');

    $flag = $this->get_flag();
    if (!$flag) {
      // Validator is misconfigured somehow.
      return TRUE;
    }

    if ($id_type == 'id') {
      if (!is_numeric($argument)) {
        // If a user is being smart and types several IDs where only one is
        // expected, we invalidate this.
        return FALSE;
      }
    }

    $ids = views_break_phrase($argument);
    if ($ids->value == array(-1)) {
      // Malformed argument syntax. Invalidate.
      return FALSE;
    }
    $ids = $ids->value;

    // Delegate the testing to the particual test method. $passed then
    // holds the IDs that passed the test.
    $tests_info = $this->tests_info();
    $method = $tests_info[$flag_test]['callback'];
    if (method_exists($this, $method)) {
      $passed = $this->$method($ids, $flag);
    }
    else {
      $passed = array();
    }

    // If some IDs exist that haven't $passed our test then validation fails.
    $failed = array_diff($ids, $passed);
    return empty($failed);
  }

  //
  // The actual tests. They return the IDs that passed.
  //

  function test_flaggable($ids, $flag) {
    return array_keys($flag->applies_to_content_id_array($ids));
  }

  function test_flagged($ids, $flag) {
    // view_break_phrase() is guaranteed to return only integers, so this is SQL safe.
    $flattened_ids = implode(',', $ids);
    return $this->_test_by_sql("SELECT content_id FROM {flag_counts} WHERE fid = %d AND content_id IN ($flattened_ids) AND count > 0", array($flag->fid));
  }

  function test_flagged_by_current_user($ids, $flag) {
    global $user;
    if (!$user->uid) {
      // Anonymous user
      return FALSE;
    }
    $flattened_ids = implode(',', $ids);
    return $this->_test_by_sql("SELECT content_id FROM {flag_content} WHERE fid = %d AND content_id IN ($flattened_ids) AND uid = %d", array($flag->fid, $user->uid));
  }

  // Helper: executes an SQL query and returns all the content_id's.
  function _test_by_sql($sql, $args) {
    $passed = array();
    $result = db_query($sql, $args);
    while ($row = db_fetch_object($result)) {
      $passed[] = $row->content_id;
    }
    return $passed;
  }
}

