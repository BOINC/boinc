<?php
// $Id: content.token.inc,v 1.5.2.14 2011/01/05 10:34:57 yched Exp $

/**
 * @file
 * Implementation of hook_content_build_modes
 * (on behalf of token.module)
 */
function token_content_build_modes() {
  return array(
    'token' => array(
      'title' => t('Token'),
      'build modes' => array(
        'token' => array(
          'title' => t('Token'),
          'views style' => FALSE,
        ),
      ),
    ),
  );
}

// Two helper functions that generate appropriate tokens for CCK-added fields.
function content_token_values($type, $object = NULL, $options = array()) {
  $tokens = array();
  if ($type == 'node') {
    // Prevent against invalid 'nodes' built by broken 3rd party code.
    if (isset($object->type)) {
      // Let PHP free the $node object when we are done. Working directly on the
      // incoming $object causes memory leak issues on long-running scripts such
      // as migrations. See http://drupal.org/node/736440.
      $node = drupal_clone($object);
      $content_type = content_types($node->type);
      $node->build_mode = 'token';
      $node->content = array();
      content_view($node);
      // The formatted values will only be known after the content has been rendered.
      drupal_render($node->content);
      content_alter($node);

      $field_types = _content_field_types();
      foreach ($content_type['fields'] as $field_name => $field) {
        $items = isset($node->{$field_name}) ? $node->{$field_name} : array();
        $function = $field_types[$field['type']]['module'] . '_token_values';
        if (!empty($items) && function_exists($function)) {
          $token_values = (array) $function('field', $items, $options);
          foreach ($token_values as $token => $value) {
            $tokens[$field_name .'-'. $token] = $value;
          }
        }
      }
    }
  }
  return $tokens;
}

function content_token_list($type = 'all') {
  if ($type == 'node' || $type == 'all') {
    $list = array();
    $field_types = _content_field_types();

    foreach (content_fields() as $field) {
      $sub_list = array();
      $function = $field_types[$field['type']]['module'] . '_token_list';
      if (function_exists($function)) {
        $sub_list = $function('field');
        foreach ($sub_list as $category => $token_list) {
          foreach ($token_list as $token => $description) {
            $list['CCK '. $category][$field['field_name'] .'-'. $token] = $description;
          }
        }
      }
    }

    return $list;
  }
}

if (module_exists('nodereference')) {
  function nodereference_token_list($type = 'all') {
    if ($type == 'field' || $type == 'all') {
      $tokens = array();

      $tokens['node reference']['nid']   = t('Referenced node ID');
      $tokens['node reference']['title'] = t('Referenced node title');
      $tokens['node reference']['title-raw'] = t('Referenced node unfiltered title. WARNING - raw user input.');
      $tokens['node reference']['link']  = t("Formatted html link to the referenced node.");
      $tokens['node reference']['path']  = t("Relative path alias to the referenced node.");
      $tokens['node reference']['url']  = t("Absolute path alias to the referenced node.");

      return $tokens;
    }
  }

  function nodereference_token_values($type, $object = NULL, $options = array()) {
    if ($type == 'field') {
      $item = $object[0];

      $title = is_numeric($item['nid']) ? _nodereference_titles($item['nid']) : '';
      $tokens['nid']   = $item['nid'];
      $tokens['title'] = $title ? check_plain($title) : '';
      $tokens['title-raw'] = $title;
      $tokens['link']  = isset($item['view']) ? $item['view'] : '';
      $tokens['path'] = is_numeric($item['nid']) ? url('node/' . $item['nid']) : '';
      $tokens['url'] = is_numeric($item['nid']) ? url('node/' . $item['nid'], array('absolute' => TRUE)) : '';

      return $tokens;
    }
  }
}

if (module_exists('number')) {
  function number_token_list($type = 'all') {
    if ($type == 'field' || $type == 'all') {
      $tokens = array();

      $tokens['number']['raw']       = t('Raw number value');
      $tokens['number']['formatted'] = t('Formatted number value');

      return $tokens;
    }
  }

  function number_token_values($type, $object = NULL, $options = array()) {
    if ($type == 'field') {
      $item = $object[0];

      $tokens['raw']       = $item['value'];
      $tokens['formatted'] = isset($item['view']) ? $item['view'] : '';

      return $tokens;
    }
  }
}

if (module_exists('text')) {
  function text_token_list($type = 'all') {
    if ($type == 'field' || $type == 'all') {
      $tokens = array();

      $tokens['text']['raw']       = t('Raw, unfiltered text');
      $tokens['text']['formatted'] = t('Formatted and filtered text');

      return $tokens;
    }
  }

  function text_token_values($type, $object = NULL, $options = array()) {
    if ($type == 'field') {
      $item = $object[0];

      $tokens['raw']  = $item['value'];
      $tokens['formatted'] = isset($item['view']) ? $item['view'] : '';
      return $tokens;
    }
  }
}

if (module_exists('userreference')) {
  function userreference_token_list($type = 'all') {
    if ($type == 'field' || $type == 'all') {
      $tokens = array();

      $tokens['user reference']['uid']   = t('Referenced user ID');
      $tokens['user reference']['name']  = t('Referenced user name');
      $tokens['user reference']['link']  = t('Formatted HTML link to referenced user');
      $tokens['user reference']['path']  = t("Relative path alias to the referenced user.");
      $tokens['user reference']['url']  = t("Absolute path alias to the referenced user.");

      return $tokens;
    }
  }

  function userreference_token_values($type, $object = NULL, $options = array()) {
    if ($type == 'field') {
      $item = $object[0];

      $tokens['uid']   = $item['uid'];
      $tokens['name']  = isset($item['view']) ? strip_tags($item['view']) : '';
      $tokens['link']  = isset($item['view']) ? $item['view'] : '';
      $tokens['path'] = is_numeric($item['uid']) ? url('user/' . $item['uid']) : '';
      $tokens['url'] = is_numeric($item['uid']) ? url('user/' . $item['uid'], array('absolute' => TRUE)) : '';

      return $tokens;
    }
  }
}
