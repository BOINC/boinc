<?php
// $Id: views_handler_field_contact_link.inc,v 1.2 2009/09/21 21:18:10 merlinofchaos Exp $
/**
 * A field that links to the user contact page, if access is permitted.
 */
class views_handler_field_contact_link extends views_handler_field_user_link {

  function option_definition() {
    $options = parent::option_definition();
    $options['link_display'] = array('default' => 'link', 'translatable' => FALSE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_display'] = array(
      '#title' => t('Type of link'),
      '#default_value' => $this->options['link_display'],
      '#type' => 'select',
      '#options' => array(
        'link' => t('Link'),
        'icon' => t('Icon'),
      ),
    );
    $form['text']['#title'] = t('Link label');
    $form['text']['#required'] = TRUE;
    $form['text']['#default_value'] = empty($this->options['text']) ? t('contact') : $this->options['text'];
  }

  function render($values) {
    global $user;
    $uid = $values->{$this->aliases['uid']};

    // Only registered users can view other registered user's contact page.
    if (empty($user->uid) || empty($uid)) {
      return;
    }

    $account = user_load($uid);
    if (empty($account)) {
      return;
    }

    // Check access when we pull up the user account so we know
    // if the user has made the contact page available.
    if (!_contact_user_tab_access($account) || empty($account->contact)) {
      return;
    }

    if ($this->options['link_display'] == 'icon') {
      return l(theme('image', 'misc/forum-new.png'), 'user/'. $account->uid .'/contact', array('html' => TRUE, 'attributes' => array('title' => t('Contact %user', array('%user' => $account->name)))));
    }
    else {
      return l($this->options['text'], 'user/'. $account->uid .'/contact', array('attributes' => array('title' => t('Contact %user', array('%user' => $account->name)))));
    }
  }
}

