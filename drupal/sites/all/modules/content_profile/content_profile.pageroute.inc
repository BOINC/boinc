<?php
// $Id: content_profile.pageroute.inc,v 1.1.2.1 2010/01/12 13:50:39 fago Exp $

/**
 * @file
 * Pageroute intergration classes
 */

include_once(drupal_get_path('module', 'pageroute') . '/pageroute.page_edit.inc');
include_once(drupal_get_path('module', 'pageroute') . '/pageroute.page_view.inc');

/**
 * theme_content_profile_pageroute_empty generates a message, if there is no content profile for the user.
 */
function theme_content_profile_pageroute_empty($type_name) {
  return '<div class="content-profile-empty">'.
    t('You have not created a @type yet. Go ahead and create one!', array('@type' => $type_name)) .'</div>';
}

/**
 * ContentProfilePageEditProfile displays a form that allows users to edit content profiles.
 */
class ContentProfilePageEditProfile extends PageroutePageEdit {

  public function getForm(&$form, &$form_state, &$args) {
    $args['hide_pageroute_buttons'] = FALSE;
    $args['default_target'] = PAGEROUTE_CURRENT;
    $page = &$form_state['page'];

    if (isset($form_state['node']) && $form_state['node']['type'] == $this->options['content-type']) {
      $node = $form_state['node'];
    }
    else {
      if (isset($page)) {
        $node = node_load(array(
        	'type' => $this->options['content-type'],
        	'uid' => pageroute_page_get_uid($page),
        ));
      }
    }
    if (empty($node)) {
      PageroutePageAdd::setNodeAddForm($form, $form_state, $page);
    }
    else {
      $this->setNodeEditForm($form, $form_state, $page, $args, $node);
    }
    parent::unsetForm($form);
  }

  public function getAdminForm($page, &$form) {

    $form['options']['content-type'] = array(
      '#type' => 'select',
      '#title' => t('Profile content type'),
      '#options' => content_profile_get_types('names'),
      '#default_value' => $page->options['content-type'],
      '#weight' => 2,
      '#description' => t('Select a content profile type.'),
    );

    PageroutePage::nodeUi($page, $form, TRUE);
  }

  public static function help() {
    return t('A page of this type will present a content profile node editing form of a configurable content-profile-type. It will edit the node with the id taken from the first argument of the pageroute. Furthermore this type can be configured to show a node adding form if the content profile is not existing. So you can build a pageroute that manages the creation and editing of content profiles.');
  }

  public static function info() {
    return array('name' => t('Content profile editing form'));
  }

  public function setUp() {}

  public static function getDefaultSubmitHandler($form) {
    return 'node_form_submit';
  }

}

/**
 * ContentProfilePageViewProfile displays a content profile.
 */
class ContentProfilePageViewProfile extends PageroutePageView {
  /*
   * Returns the page display for the configured node
   */
  public function getForm(&$form, &$form_state, &$args) {
    $page = &$form_state['page'];

    $args['hide_pageroute_buttons'] = FALSE;
    $args['default_target'] = PAGEROUTE_CURRENT;

    $node = node_load(array(
      'type' => $page->options['content-type'],
      'uid' => pageroute_page_get_uid($page),
    ));

    if ($node->nid && node_access('view', $node)) {
      if (empty($this->title)) {
        drupal_set_title(check_plain($node->title));
      }
      node_tag_new($node->nid);
      $form += array('pageroute-view' => array('#value' => node_view($node, FALSE, TRUE, FALSE)));
    }
    else {
      $type_name = node_get_types('name', $page->options['content-type']);
      $form += array('pageroute-view' => array('#value' => theme('content_profile_pageroute_empty', $type_name)));
    }
  }

  public function getAdminForm($page, &$form) {
    $form['options']['content-type'] = array(
      '#type' => 'select',
      '#title' => t('Profile content type'),
      '#options' => content_profile_get_types('names'),
      '#required' => TRUE,
      '#default_value' => $page->options['content-type'],
      '#weight' => 2,
      '#description' => t('You can only use content types marked as \'content profile\''),
    );
  }

  public static function help() {
    return t('The lonely node display page can be used to view this lonely node. This might be useful for displaying the lonely node aftercreation or update. There will be a (themeable) message if there is no node that can be displayed.');
  }

  public static function info() {
    return array('name' => t('Content profile display'));
  }
}
