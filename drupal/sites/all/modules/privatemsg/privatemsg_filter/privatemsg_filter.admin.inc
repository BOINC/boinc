<?php
// $Id: privatemsg_filter.admin.inc,v 1.1.2.2 2009/11/06 13:06:26 berdir Exp $

/**
 * @file
 * Admin menu callbacks for privatemsg_filter module.
 */

/**
 * Display a list of tags to administer them.
 */
function privatemsg_tags_admin() {
  drupal_set_title(t('Tags'));

  $header = array(
    'tag' => array(
      'data'    => t('Tag'),
      'field'   => 'tag',
      'class'   => 'privatemsg-filter-header-tag',
      'sort'    => 'asc',
      '#weight' => -40,
    ),
    'usage' => array(
      'data'    => t('Usage'),
      'field'   => 'count',
      'class'   => 'privatemsg-filter-header-usage',
      '#weight' => -30,
    ),
    'public' => array(
      'data'    => t('Public'),
      'field'   => 'public',
      'class'   => 'privatemsg-filter-header-public',
      '#weight' => -20,
    ),
    'operations' => array(
      'data'    => t('Operations'),
      'class'   => 'privatemsg-filter-header-operations',
      '#weight' => -10,
    ),
  );
  $query = _privatemsg_assemble_query(array('tags', 'privatemsg_filter'));
  $result = pager_query($query['query'] . tablesort_sql($header), 100);

  $rows = array();
  while ($tag = db_fetch_array($result)) {
    $row = array();
    $row['tag'] = check_plain($tag['tag']);
    $row['count'] = $tag['count'];
    $row['public'] = $tag['public'] ? t('Yes') : '-';
    $row['operations'] = l(t('edit'), 'admin/settings/messages/tags/edit/' . $tag['tag_id']);
    $row['operations'] .= ' ' . l(t('delete'), 'admin/settings/messages/tags/delete/' . $tag['tag_id']);

    $rows[] = $row;
  }

  if (empty($rows)) {
    // Display a message if now tags are available.
    $rows[] = array(array('data' => t('No tags available.'), 'colspan' => count($header)));
  }

  return theme('table', $header, $rows) . theme('pager');
}

/**
 * A form to edit existing or create new tags.
 *
 * @param $form_state
 *   Form state array
 * @param $tag_id
 *   Tag Id if an edit form should be displayed, NULL for a create
 *   new tag form.
 */
function privatemsg_tags_form(&$form_state, $tag_id = NULL) {
  $form = array();

  if ($tag_id) {
    $tag = db_fetch_array(db_query('SELECT * from {pm_tags} pmt WHERE pmt.tag_id = %d', $tag_id));
    $form['tag_id'] = array(
      '#value' => $tag_id,
      '#type' => 'value',
    );
    drupal_set_title(t('Edit @tag', array('@tag' => $tag['tag'])));
  }

  $form['tag'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => empty($tag_id) ? NULL : $tag['tag'],
  );

  $form['public'] = array(
    '#title' => t('Public'),
    '#type' => 'checkbox',
    '#default_value' => empty($tag_id) ? TRUE : $tag['public'],
    '#description' => t('Public tags are visible for all users, private tags are only visible if they use them.'),
  );

  $form['submit'] = array(
    '#value' => empty($tag_id) ? t('Create tag') : t('Save tag'),
    '#type'  => 'submit',
  );

  return $form;
}

/**
 * Form validate function for tags admin page.
 *
 * Make sure that a tag name is unique.
 */
function privatemsg_tags_form_validate($form, &$form_state) {
  $tag_id = isset($form_state['values']['tag_id']) ? $form_state['values']['tag_id'] : 0;
  if ($tag_id) {
    // We are editing an existing tag, exclude the current tag from the search.
    $exists = db_result(db_query("SELECT 1 FROM {pm_tags} WHERE tag = '%s' AND tag_id <> %d", $form_state['values']['tag'], $tag_id));
  }
  else {
    $exists = db_result(db_query("SELECT 1 FROM {pm_tags} WHERE tag = '%s'", $form_state['values']['tag']));
  }
  if ($exists) {
    form_set_error('tag', t('Tag already exists, choose a different name.'));
  }
}

/**
 * Form submit handler for add/edit forms.
 */
function privatemsg_tags_form_submit($form, &$form_state) {
  if (!empty($form_state['values']['tag_id'])) {
    // Tag already exists, update the existing tag.
    db_query("UPDATE {pm_tags} SET tag = '%s', public = %d WHERE tag_id = %d", $form_state['values']['tag'], $form_state['values']['public'], $form_state['values']['tag_id']);
    drupal_set_message(t('Tag updated.'));
    $form_state['redirect'] = 'admin/settings/messages/tags';
  }
  else {
    // Tag does not yet exist, create a new one.
    db_query("INSERT INTO {pm_tags} (tag, public) VALUES ('%s', %d)", $form_state['values']['tag'], $form_state['values']['public']);
    drupal_set_message(t('Tag created.'));
  }
}

/**
 * Confirmation form for deleting tags.
 *
 * @param $tag_id
 *   Id of the tag that should be deleted.
 */
function privatemsg_filter_tags_delete(&$form_state, $tag_id) {
  $form['tag_id'] = array(
    '#type' => 'value',
    '#value' => $tag_id,
  );
  return confirm_form($form,
    t('Are you sure you want to delete?'),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/settings/messages/tags',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for the confirm delete form.
 */
function privatemsg_filter_tags_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    db_query('DELETE FROM {pm_tags} WHERE tag_id = %d', $form_state['values']['tag_id']);
    db_query('DELETE FROM {pm_tags_index} WHERE tag_id = %d', $form_state['values']['tag_id']);

    drupal_set_message(t('Tag has been deleted'));
  }
  $form_state['redirect'] = 'admin/settings/messages/tags';
}