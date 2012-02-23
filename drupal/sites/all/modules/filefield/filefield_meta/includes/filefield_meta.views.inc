<?php
// $Id: filefield_meta.views.inc,v 1.3 2010/06/16 22:06:44 quicksketch Exp $
/**
 * @file
 * Provide views data for filefield_meta.module.
 */

/**
 * @defgroup views_filefield_meta_module filefield_meta.module handlers
 *
 * Includes the tables 'node', 'node_revisions' and 'history'.
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function filefield_meta_views_data() {
  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['filefield_meta']['table']['group']  = t('File');

  // For other base tables, explain how we join
  $data['filefield_meta']['table']['join'] = array(
    // this explains how the 'filefield_meta' table (named in the line above)
    // links toward the files table.
    'files' => array(
      'left_table' => 'files', // Because this is a direct link it could be left out.
      'left_field' => 'fid',
      'field' => 'fid',
     ),
  );

  // ----------------------------------------------------------------
  // filefield_meta table -- fields

  // width
  $data['filefield_meta']['width'] = array(
    'title' => t('Video width'),
    'help' => t('Width of a video or image file in pixels.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // height
  $data['filefield_meta']['height'] = array(
    'title' => t('Video height'),
    'help' => t('Height of a video or image file in pixels.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // duration
  $data['filefield_meta']['duration'] = array(
    'title' => t('Duration'),
    'help' => t('The duration of audio or video files, in seconds.'),
    'field' => array(
      'handler' => 'filefield_meta_handler_field_duration',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // audio_format
  $data['filefield_meta']['audio_format'] = array(
    'title' => t('Audio format'),
    'help' => t('The audio format.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // audio_sample_rate
  $data['filefield_meta']['audio_sample_rate'] = array(
    'title' => t('Audio sample rate'),
    'help' => t('The sample rate of the audio.'),
    'field' => array(
      'handler' => 'filefield_meta_handler_field_samplerate',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // audio_channel_mode
  $data['filefield_meta']['audio_channel_mode'] = array(
    'title' => t('Audio channel mode'),
    'help' => t('The number of channels in the audio, by name (stereo or mono).'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // audio_bitrate
  $data['filefield_meta']['audio_bitrate'] = array(
    'title' => t('Audio bitrate'),
    'help' => t('The audio bitrate.'),
    'field' => array(
      'handler' => 'filefield_meta_handler_field_bitrate',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // audio_bitrate_mode
  $data['filefield_meta']['audio_bitrate_mode'] = array(
    'title' => t('Audio bitrate mode'),
    'help' => t('The kind of audio bitrate, such as VBR. Usually empty.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  // Tags.
  $data['filefield_meta']['tags'] = array(
    'title' => t('ID3 tags'),
    'help' => t('ID3 tags include embedded information such as artist, album, year, genre and other informaiton.'),
    'field' => array(
      'handler' => 'filefield_meta_handler_field_tags',
      'click sortable' => FALSE,
    ),
  );

  return $data;
}

/**
 * Implementation of hook_views_handlers().
 */
function filefield_meta_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'filefield_meta') . '/includes',
    ),
    'handlers' => array(
      // field handlers
      'filefield_meta_handler_field_bitrate' => array(
        'parent' => 'views_handler_field_numeric',
      ),
      'filefield_meta_handler_field_duration' => array(
        'parent' => 'views_handler_field_numeric',
      ),
      'filefield_meta_handler_field_samplerate' => array(
        'parent' => 'views_handler_field_numeric',
      ),
      'filefield_meta_handler_field_tags' => array(
        'parent' => 'views_handler_field_prerender_list',
      ),
    ),
  );
}

/**
 * @}
 */
