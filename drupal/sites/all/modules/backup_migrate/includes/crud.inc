<?php

/**
 * @file
 * CRUD functions for backup and migrate types (schedules, profiles etc.).
 */

define('BACKUP_MIGRATE_STORAGE_NONE', 0);
define('BACKUP_MIGRATE_STORAGE_DB', 1);
define('BACKUP_MIGRATE_STORAGE_OVERRIDEN', 2);

/**
 * Return a list of CRUD types in the module.
 */
function backup_migrate_crud_types() {
  $out = array(
    'destination' => array(
      'class' => 'backup_migrate_destination',
      'include' => 'destinations',
    ),
    'profile' => array(
      'class' => 'backup_migrate_profile',
      'include' => 'profiles',
    ),
    'schedule' => array(
      'class' => 'backup_migrate_schedule',
      'include' => 'schedules',
    ),
  );
  return $out;
}

/**
 * Get a generic object of the given type to be used for static-like functions.
 *
 * I'm not using actual static method calls since they don't work on variables prior to PHP 5.3.0
 */
function backup_migrate_crud_type_load($type) {
  $out = NULL;
  $types = backup_migrate_crud_types();
  if (!empty($types[$type])) {
    $info = $types[$type];
    if ($info['include']) {
      backup_migrate_include($info['include']);
    }
    $out = new $info['class'];
  }
  return $out;
}


/**
 * Get the menu items handled by the CRUD code.
 */
function backup_migrate_crud_menu() {
  $items = array();
  foreach (backup_migrate_crud_types() as $type => $info) {
    $type = backup_migrate_crud_type_load($type);
    $items += (array)$type->get_menu_items();
  }
  return $items;
}

/**
 * Page callback to create a new item.
 */
function backup_migrate_crud_ui_create() {
  if ($type = backup_migrate_crud_type_load(arg(BACKUP_MIGRATE_MENU_DEPTH))) {
    $item = $type->create(array());
    return drupal_get_form('backup_migrate_crud_edit_form', $item);
  }
}

/**
 * Page callback to list all items.
 */
function backup_migrate_crud_ui_list() {
  $out = '';
  if ($type = backup_migrate_crud_type_load(arg(BACKUP_MIGRATE_MENU_DEPTH))) {
    $out = $type->get_list();
  }
  return $out;
}

/**
 * Page callback to edit an item.
 */
function backup_migrate_crud_ui_edit($item_id = NULL) {
  if ($type = backup_migrate_crud_type_load(arg(BACKUP_MIGRATE_MENU_DEPTH))) {
    if ($item_id && $item = $type->item($item_id)) {
      return drupal_get_form('backup_migrate_crud_edit_form', $item);
    }
    drupal_goto(BACKUP_MIGRATE_MENU_PATH. '/' . arg(BACKUP_MIGRATE_MENU_DEPTH));
  }
}

/**
 * A form callback to edit an item.
 */
function backup_migrate_crud_edit_form($form_state, $item) {
  $form = $item->edit_form();
  $form['item'] = array(
    '#type' => 'value',
    '#value' => $item,
  );
  $form['#validate'][]  = 'backup_migrate_crud_edit_form_validate';
  $form['#submit'][]    = 'backup_migrate_crud_edit_form_submit';

  return $form;
}

/**
 * Validate the item edit form.
 */
function backup_migrate_crud_edit_form_validate($form, &$form_state) {
  $item = $form_state['values']['item'];
  $item->edit_form_validate($form, $form_state);
}

/**
 * Submit the item edit form.
 */
function backup_migrate_crud_edit_form_submit($form, &$form_state) {
  $item = $form_state['values']['item'];
  $item->edit_form_submit($form, $form_state);
  if (empty($form_state['redirect'])) {
    $form_state['redirect'] = BACKUP_MIGRATE_MENU_PATH . '/'. $item->type_name;
  }
}


/**
 * Page callback to delete an item.
 */
function backup_migrate_crud_ui_delete($item_id = NULL) {
  if ($type = backup_migrate_crud_type_load(arg(BACKUP_MIGRATE_MENU_DEPTH))) {
    if ($item_id && $item = $type->item($item_id)) {
      return drupal_get_form('backup_migrate_crud_delete_confirm_form', $item);
    }
    drupal_goto('admin/content/backup_migrate/'. arg(BACKUP_MIGRATE_MENU_DEPTH));
  }
}

/**
 * Ask confirmation for deletion of a item.
 */
function backup_migrate_crud_delete_confirm_form(&$form_state, $item) {
  $form['item'] = array(
    '#type' => 'value',
    '#value' => $item,
  );
  $message = $item->delete_confirm_message();
  return confirm_form($form, t('Are you sure?'), BACKUP_MIGRATE_MENU_PATH . '/'. $item->type_name, $message, t('Delete'), t('Cancel'));
}

/**
 * Delete a item after confirmation.
 */
function backup_migrate_crud_delete_confirm_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $item = $form_state['values']['item'];
    $item->delete();
  }
  $form_state['redirect'] = BACKUP_MIGRATE_MENU_PATH . "/". $item->type_name;
}

/**
 * Export an item.
 */
function backup_migrate_crud_ui_export($item_id = NULL) {
  if ($type = backup_migrate_crud_type_load(arg(BACKUP_MIGRATE_MENU_DEPTH))) {
    if ($item_id && $item = $type->item($item_id)) {
      return drupal_get_form('backup_migrate_crud_export_form', $item->export());
    }
    drupal_goto(BACKUP_MIGRATE_MENU_PATH . '/' . arg(BACKUP_MIGRATE_MENU_DEPTH));
  }
}

/**
 * Ask confirmation for deletion of a destination.
 */
function backup_migrate_crud_export_form(&$form_state, $export) {
  $form['export'] = array(
    '#title' => t('Exported content'),
    '#type' => 'textarea',
    '#rows' => min(30, count(explode("\n", $export))),
    '#value' => $export,
  );
  return $form;
}

/**
 * Get all items of the given type.
 */
function backup_migrate_crud_get_items($type) {
  if ($type = backup_migrate_crud_type_load($type)) {
    return $type->all_items();
  }
}


/**
 * Get an item of the specified type.
 */
function backup_migrate_crud_get_item($type, $id) {
  if ($type = backup_migrate_crud_type_load($type)) {
    return $type->item($id);
  }
}

/**
 * Create a new item of the given type.
 */
function backup_migrate_crud_create_item($type, $params) {
  if ($type = backup_migrate_crud_type_load($type)) {
    return $type->create($params);
  }
}

/**
 * A base class for items which can be stored in the database, listed, edited, deleted etc.
 */
class backup_migrate_item {
  var $db_table = '';
  var $type_name = '';
  var $storage = FALSE;
  var $default_values = array();
  var $singular = 'item';
  var $plural = 'items';

  /**
   * Constructor, set the basic info pulled from the db or generated programatically.
   */
  function __construct($params = array()) {
    $this->from_array($this->_merge_defaults((array)$params, (array)$this->get_default_values()));
  }

  /**
   * Merge parameters with the given defaults.
   *
   * Works like array_merge_recursive, but it doesn't turn scalar values into arrays.
   */
  function _merge_defaults($params, $defaults) {
    foreach ($defaults as $key => $val) {
      if (!isset($params[$key])) {
        $params[$key] = $val;
      }
      else if (is_array($params[$key])) {
        $params[$key] = $this->_merge_defaults($params[$key], $val);
      }
    }
    return $params;
  }

  /**
   * Get the default values for standard parameters.
   */
  function get_default_values() {
    return $this->default_values;
  }

  /**
   * Save the item to the database.
   */  
  function save() {
    if (!$this->get_id()) {
      $this->generate_id();
    }
    drupal_write_record($this->db_table, $this->to_array(), !empty($this->storage) ? $this->get_primary_key() : array());
  }

  /**
   * Delete the item from the database.
   */  
  function delete() {
    $keys = (array)$this->get_primary_key();
    db_query("DELETE FROM {{$this->db_table}} WHERE $keys[0] = '%s'", $this->get_id());
  }


  /**
   * Load an existing item from an array.
   */
  function from_array($params) {
    foreach ($params as $key => $value) {
      if (method_exists($this, 'set_'. $key)) {
        $this->{'set_'. $key}($value);
      }
      else {
        $this->{$key} = $value;
      }
    }
  }

  /**
   * Return as an array of values.
   */
  function to_array() {
    $out = array();
    // Return fields as specified in the schema.
    $schema = $this->get_schema();
    if (!empty($schema['fields']) && is_array($schema['fields'])) {
      foreach ($schema['fields'] as $field => $info) {
        $out[$field] = $this->get($field);
      }
    }
    return $out;
  }

  /**
   * Return as an exported array of values.
   */
  function export() {
    $out = $this->to_array();
    ob_start();
    var_export($out);
    $out = ob_get_contents();
    ob_end_clean();
    return $out;
  }

  /**
   * Load an existing item from an database (serialized) array.
   */
  function load_row($data) {
    $params = array();
    $schema = $this->get_schema();
    // Load fields as specified in the schema.
    foreach ($schema['fields'] as $field => $info) {
      $params[$field] = empty($info['serialize']) ? $data[$field] : unserialize(db_decode_blob($data[$field]));
    }
    $this->from_array($params);
  }


  /**
   * Decode a loaded db row (unserialize necessary fields).
   */
  function decode_db_row($data) {
    $params = array();
    $schema = $this->get_schema();
    // Load fields as specified in the schema.
    foreach ($schema['fields'] as $field => $info) {
      $params[$field] = empty($info['serialize']) ? $data[$field] : unserialize(db_decode_blob($data[$field]));
    }
    return $params;
  }

  /**
   * Return the fields which must be serialized before saving to the db.
   */
  function get_serialized_fields() {
    $out = array();
    $schema = $this->get_schema();
    foreach ($schema['fields'] as $field => $info) {
      if (!empty($info['serialize'])) {
        $out[] = $field;
      }
    }
    return $out;
  }

  /**
   * Get the primary key field title from the schema.
   */
  function get_primary_key() {
    $schema = $this->get_schema();
    return @$schema['primary key'];
  }

  /**
   * Get the schema for the item type.
   */
  function get_schema() {
    return drupal_get_schema($this->db_table);
  }

  /**
   * Get the primary id for this item (if any is set).
   *
   * We only handle single field keys since that's all we need.
   */
  function get_id() {
    $keys = (array)$this->get_primary_key();
    return !empty($this->{$keys[0]}) ? (string)$this->{$keys[0]} : '';
  }

  /**
   * Set the primary id for this item (if any is set).
   */
  function set_id($id) {
    $keys = (array)$this->get_primary_key();
    if (!empty($keys[0])) {
      return $this->{$keys[0]} = $id;
    }
    return NULL;
  }

  /**
   * Return a random (very very likely unique) string id for a new item.
   */
  function generate_id() {
    $this->set_id(md5(uniqid(mt_rand(), true)));
  }

  /**
   * Get the name of the item.
   */
  function get_name() {
    return @$this->name;
  }

  /**
   * Get the member with the given key.
   */  
  function get($key) {
    if (method_exists($this, 'get_'. $key)) {
      return $this->{'get_'. $key}();
    }
    return @$this->{$key};
  }

  /* UI Stuff */

  /**
   * Get the action links for a destination.
   */
  function get_action_links() {
    $out = array();

    $item_id = $this->get_id();

    if (@$this->storage == BACKUP_MIGRATE_STORAGE_DB || @$this->storage == BACKUP_MIGRATE_STORAGE_OVERRIDEN) {
      $out['edit'] = l(t("edit"), BACKUP_MIGRATE_MENU_PATH . "/$this->type_name/list/edit/$item_id");
    }
    else if (@$this->storage == BACKUP_MIGRATE_STORAGE_NONE) {
      $out['edit'] = l(t("override"), BACKUP_MIGRATE_MENU_PATH . "/$this->type_name/list/edit/$item_id");
    }
    if (@$this->storage == BACKUP_MIGRATE_STORAGE_DB) {
      $out['delete'] = l(t("delete"), BACKUP_MIGRATE_MENU_PATH . "/$this->type_name/list/delete/$item_id");
    }
    else if (@$this->storage == BACKUP_MIGRATE_STORAGE_OVERRIDEN) {
      $out['delete'] = l(t("revert"), BACKUP_MIGRATE_MENU_PATH . "/$this->type_name/list/delete/$item_id");
    }

    // Export link disabled until we have an import function.
    //$out['export'] = l(t("export"), BACKUP_MIGRATE_MENU_PATH . "/$this->type_name/list/export/$item_id");

    return $out;
  }

  /**
   * Get a table of all items of this type.
   */  
  function get_list() {
    $items = $this->all_items();
    $rows = array();
    foreach ($items as $item) {
      if ($row = $item->get_list_row()) {
        $rows[] = $row;
      }
    }
    if (count($rows)) {
      $out = theme('table', $this->get_list_header(), $rows);
    }
    else {
      $out = t('There are no !items to display.', array('!items' => $this->plural));
    }
    if (user_access('administer backup and migrate')) {
      $out .= ' '. l(t('Create a new !item', array('!item' => $this->singular)), 'admin/content/backup_migrate/'. $this->type_name .'/list/add');
    }
    return $out;
  }

  /**
   * Get the columns needed to list the type.
   */  
  function get_list_column_info() {
    return array(
      'actions' => array('title' => t('Operations'), 'html' => TRUE),
    );
  }

  /**
   * Get header for a lost of this type.
   */  
  function get_list_header() {
    $out = array();
    foreach ($this->get_list_column_info() as $key => $col) {
      $out[] = $col['title'];
    }
    return $out;
  }

  /**
   * Get a row of data to be used in a list of items of this type.
   */  
  function get_list_row() {
    $out = array();
    foreach ($this->get_list_column_info() as $key => $col) {
      $out[$key] = empty($col['html']) ? check_plain($this->get($key)) : $this->get($key);
    }
    return $out;
  }

  /**
   * Get the rendered action links for a destination.
   */
  function get_actions() {
    $links = $this->get_action_links();
    return implode(" | ", $links);
  }

  /**
   * Get the edit form for the item.
   */
  function edit_form() {
    $form = array();
    $form['item'] = array(
      '#type' => 'value',
      '#value' => $this,
    );
    $form['id'] = array(
      '#type' => 'value',
      '#value' => $this->get_id(),
    );
    $form['actions'] = array('#prefix' => '<div class="container-inline">', '#suffix' => '</div>', '#weight' => 99);
    $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save !type', array('!type' => t($this->singular))));
    $form['actions']['cancel'] = array('#value' => l(t('Cancel'), BACKUP_MIGRATE_MENU_PATH . '/destination'));
    return $form;
  }

  /**
   * Validate the edit form for the item.
   */
  function edit_form_validate($form, &$form_state) {
  }

  /**
   * Submit the edit form for the item.
   */
  function edit_form_submit($form, &$form_state) {
    $this->from_array($form_state['values']);
    $this->save();
    _backup_migrate_message('!type saved', array('!type' => t(ucwords($this->singular))));
  }

  /**
   * Get the message to send to the user when confirming the deletion of the item.
   */
  function delete_confirm_message() {
    return t('Are you sure you want to delete this !type?', array('!type' => t($item->singular)));
  }

  /* Static Functions */
 
   /**
   * This function is not supposed to be called. It is just here to help the po extractor out.
   */
  function strings() {
    // Help the pot extractor find these strings.
    t('List !type');
    t('Create !type');
    t('Delete !type');
    t('Edit !type');
    t('Export !type');
  }

  /**
   * Get the menu items for manipulating this type.
   */
  function get_menu_items() {
    $type = $this->type_name;
    $items[BACKUP_MIGRATE_MENU_PATH . '/' . $type] = array(
      'title' => ucwords($this->plural),
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('crud', 'backup_migrate_crud_ui_list', TRUE),
      'access arguments' => array('administer backup and migrate'),
      'weight' => 2,
      'type' => MENU_LOCAL_TASK,
    );
    $items[BACKUP_MIGRATE_MENU_PATH . '/' . $type .'/list'] = array(
      'title' => 'List !type',
      'title arguments' => array('!type' => t(ucwords($this->plural))),
      'weight' => 1,
      'type' => MENU_DEFAULT_LOCAL_TASK,
    );
    $items[BACKUP_MIGRATE_MENU_PATH . '/' . $type .'/list/add'] = array(
      'title' => 'Create !type',
      'title arguments' => array('!type' => t(ucwords($this->singular))),
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('crud', 'backup_migrate_crud_ui_create', TRUE),
      'access arguments' => array('administer backup and migrate'),
      'weight' => 2,
      'type' => MENU_LOCAL_TASK,
    );
    $items[BACKUP_MIGRATE_MENU_PATH . '/' . $type .'/list/delete'] = array(
      'title' => 'Delete !type',
      'title arguments' => array('!type' => t(ucwords($this->singular))),
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('crud', 'backup_migrate_crud_ui_delete', TRUE),
      'access arguments' => array('administer backup and migrate'),
      'type' => MENU_LOCAL_TASK,
    );
    $items[BACKUP_MIGRATE_MENU_PATH . '/' . $type .'/list/edit'] = array(
      'title' => 'Edit !type',
      'title arguments' => array('!type' => t(ucwords($this->singular))),
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('crud', 'backup_migrate_crud_ui_edit', TRUE),
      'access arguments' => array('administer backup and migrate'),
      'type' => MENU_LOCAL_TASK,
    );
    $items[BACKUP_MIGRATE_MENU_PATH . '/' . $type .'/list/export'] = array(
      'title' => 'Export !type',
      'title arguments' => array('!type' => t(ucwords($this->singular))),
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('crud', 'backup_migrate_crud_ui_export', TRUE),
      'access arguments' => array('administer backup and migrate'),
      'type' => MENU_LOCAL_TASK,
    );
    return $items;
  }


  /**
   * Create a new items with the given input. Doesn't load the parameters, but could use them to determine what type to create.
   */
  function create($params = array()) {
    $type = get_class($this);
    return new $type($params);
  }

  /**
   * Get all of the given items.
   */
  function all_items() {
    static $cache = array();

    // Allow other modules to declare destinations programatically.
    $items = module_invoke_all($this->db_table);

    // Get any items stored as a variable. This allows destinations to be defined in settings.php
    $defaults = (array)variable_get($this->db_table .'_defaults', array());
    foreach ($defaults as $info) {
      if (is_array($info) && $item = $this->create($info)) {
        $items[$item->get_id()] = $item;
      }
    }

    // Get the items from the db.
    $result = db_query("SELECT * FROM {{$this->db_table}}");
    while ($info = db_fetch_array($result)) {
      $info = $this->decode_db_row($info);
      if ($item = $this->create($info)) {
        $item->storage = empty($items[$item->get_id()]) ? BACKUP_MIGRATE_STORAGE_DB : BACKUP_MIGRATE_STORAGE_OVERRIDEN;
        $items[$item->get_id()] = $item;
      }
    }

    // Allow other modules to alter the items. This should maybe be before the db override code above
    // but then the filters are not able to set defaults for missing values. Other modules should just
    // be careful not to overwrite the user's UI changes in an unexpected way.
    drupal_alter($this->db_table, $items);

    return $items;
  }

  /**
   * A particular item.
   */
  function item($item_id) {
    $items = $this->all_items();
    return !empty($items[$item_id]) ? $items[$item_id] : NULL;
  }
}