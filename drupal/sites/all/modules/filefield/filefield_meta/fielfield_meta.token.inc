<?php
// $Id: fielfield_meta.token.inc,v 1.1 2010/06/16 22:06:44 quicksketch Exp $

/**
 * @file
 * Token integration for FileField Meta.
 */

/**
 * Implementation of hook_token_list().
 *
 * Provide a user readable list of FileField Meta tokens.
 */
function filefield_token_list($type = 'all') {
  if ($type == 'field' || $type == 'all') {
    $tokens['file']['filefield-width']               = t('File Video width');
    $tokens['file']['filefield-height']              = t('File Video height');
    $tokens['file']['filefield-duration']            = t('File Duration');
    $tokens['file']['filefield-audio-format']        = t('File Audio Format path');
    $tokens['file']['filefield-audio-sample-rate']   = t('File Audio sample rate');
    $tokens['file']['filefield-audio-channel-mode']  = t('File Audio channel mode (stereo, mono)');
    $tokens['file']['filefield-audio-bitrate']       = t('File Audio bitrate');
    $tokens['file']['filefield-audio-bitrate-mode']  = t('File Audio bitrate mode (cbr, vbr, abr...)');

    // ID3 tags.
    foreach (filefield_meta_tags() as $tag => $label) {
      $tokens['file']['filefield-tag-' . $tag] = t('File ID3 @tag tag', array('@tag' => $label));
    }

    return $tokens;
  }
}

/**
 * Implementation of hook_token_values().
 *
 * Provide the token values for a given file item.
 */
function filefield_token_values($type, $object = NULL) {
  $tokens = array();
  if ($type == 'field' && isset($object[0]['fid'])) {
    $item = $object[0];

    $tokens['filefield-width']               = $item['data']['width'] ;
    $tokens['filefield-height']              = $item['data']['height'] ;
    $tokens['filefield-duration']            = $item['data']['duration'] ;
    $tokens['filefield-audio-format']        = isset($item['data']['audio_format']) ? check_plain($item['data']['audio_format']) : '';
    $tokens['filefield-audio-sample-rate']   = isset($item['data']['sample_rate']) ? check_plain($item['data']['sample_rate']) : '';
    $tokens['filefield-audio-channel-mode']  = isset($item['data']['audio_channel_mode']) ? check_plain($item['data']['audio_channel_mode']) : '';
    $tokens['filefield-audio-bitrate']       = isset($item['data']['audio_bitrate']) ? check_plain($item['data']['audio_bitrate']) : '';
    $tokens['filefield-audio-bitrate-mode']  = isset($item['data']['audio_bitrate_mode']) ? check_plain($item['data']['audio_bitrate_mode']) : '';

    // ID3 tags.
    foreach (filefield_meta_tags() as $tag => $label) {
      $tokens['filefield-tag-title'] = isset($item['data']['tags'][$tag]) ? check_plain($item['data']['tags'][$tag]) : '';
    }
  }

  return $tokens;
}
