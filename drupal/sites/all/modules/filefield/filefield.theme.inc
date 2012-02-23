<?php
// $Id: filefield.theme.inc,v 1.12 2010/06/30 01:37:29 quicksketch Exp $

/**
 * @file
 * Theme functions used for normal file output.
 *
 * Uses content.module to store the fid and field specific metadata,
 * and Drupal's {files} table to store the actual file data.
 */

/**
 * Return an image with an appropriate icon for the given file.
 *
 * @param $file
 *   A file object for which to make an icon.
 */
function theme_filefield_icon($file) {
  if (is_object($file)) {
    $file = (array) $file;
  }
  $mime = check_plain($file['filemime']);

  $dashed_mime = strtr($mime, array('/' => '-', '+' => '-'));

  if ($icon_url = filefield_icon_url($file)) {
    return '<img class="filefield-icon field-icon-'. $dashed_mime .'"  alt="'. t('@mime icon', array('@mime' => $mime)) .'" src="'. $icon_url .'" />';
  }
}

/**
 * Given a file object, create a URL to a matching icon.
 *
 * @param $file
 *   A file object.
 * @param $theme
 *   Optional. The theme to be used for the icon. Defaults to the value of
 *   the "filefield_icon_theme" variable.
 * @return
 *   A URL string to the icon, or FALSE if an appropriate icon could not be
 *   found.
 */
function _filefield_icon_url($file, $theme = NULL) {
  global $base_url;

  if ($icon_path = _filefield_icon_path($file, $theme)) {
    return $base_url .'/'. $icon_path;
  }
  return FALSE;
}

/**
 * Given a file object, create a URL to a matching icon.
 *
 * @param $file
 *   A file object.
 * @param $theme
 *   Optional. The theme to be used for the icon. Defaults to the value of
 *   the "filefield_icon_theme" variable.
 * @return
 *   A string to the icon as a local path, or FALSE if an appropriate icon could
 *   not be found.
 */
function _filefield_icon_path($file, $theme = NULL) {
  if (!isset($theme)) {
    $theme = variable_get('filefield_icon_theme', 'default');
  }

  // If there's an icon matching the exact mimetype, go for it.
  $dashed_mime = strtr($file['filemime'], array('/' => '-'));
  if ($icon_path = _filefield_create_icon_path($dashed_mime, $theme)) {
    return $icon_path;
  }
  // For a couple of mimetypes, we can "manually" tell a generic icon.
  if ($generic_name = _filefield_generic_icon_map($file)) {
    if ($icon_path = _filefield_create_icon_path($generic_name, $theme)) {
      return $icon_path;
    }
  }
  // Use generic icons for each category that provides such icons.
  foreach (array('audio', 'image', 'text', 'video') as $category) {
    if (strpos($file['filemime'], $category .'/') === 0) {
      if ($icon_path = _filefield_create_icon_path($category .'-x-generic', $theme)) {
        return $icon_path;
      }
    }
  }
  // Try application-octet-stream as last fallback.
  if ($icon_path = _filefield_create_icon_path('application-octet-stream', $theme)) {
    return $icon_path;
  }
  // Sorry, no icon can be found...
  return FALSE;
}

/**
 * Internal function to convert a file icon theme name to a directory.
 */
function _filefield_icon_directory($theme = NULL) {
  static $sets;

  if (!isset($sets)) {
    $sets = module_invoke_all('filefield_icon_sets');
    drupal_alter('filefield_icon_sets', $sets);
  }

  if (!isset($theme) || !isset($sets[$theme])) {
    $theme = 'default';
  }

  return $sets[$theme];
}

function _filefield_create_icon_path($icon_name, $theme = NULL) {
  $icons_directory = _filefield_icon_directory($theme);
  $icon_path = $icons_directory .'/'. $icon_name .'.png';

  if (file_exists($icon_path)) {
    return $icon_path;
  }
  return FALSE;
}

function _filefield_generic_icon_map($file) {
  switch ($file['filemime']) {
    // Word document types.
    case 'application/msword':
    case 'application/vnd.ms-word.document.macroEnabled.12':
    case 'application/vnd.oasis.opendocument.text':
    case 'application/vnd.oasis.opendocument.text-template':
    case 'application/vnd.oasis.opendocument.text-master':
    case 'application/vnd.oasis.opendocument.text-web':
    case 'application/vnd.openxmlformats-officedocument.wordprocessingml.document':
    case 'application/vnd.stardivision.writer':
    case 'application/vnd.sun.xml.writer':
    case 'application/vnd.sun.xml.writer.template':
    case 'application/vnd.sun.xml.writer.global':
    case 'application/vnd.wordperfect':
    case 'application/x-abiword':
    case 'application/x-applix-word':
    case 'application/x-kword':
    case 'application/x-kword-crypt':
      return 'x-office-document';

    // Spreadsheet document types.
    case 'application/vnd.ms-excel':
    case 'application/vnd.ms-excel.sheet.macroEnabled.12':
    case 'application/vnd.oasis.opendocument.spreadsheet':
    case 'application/vnd.oasis.opendocument.spreadsheet-template':
    case 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet':
    case 'application/vnd.stardivision.calc':
    case 'application/vnd.sun.xml.calc':
    case 'application/vnd.sun.xml.calc.template':
    case 'application/vnd.lotus-1-2-3':
    case 'application/x-applix-spreadsheet':
    case 'application/x-gnumeric':
    case 'application/x-kspread':
    case 'application/x-kspread-crypt':
      return 'x-office-spreadsheet';

    // Presentation document types.
    case 'application/vnd.ms-powerpoint':
    case 'application/vnd.ms-powerpoint.presentation.macroEnabled.12':
    case 'application/vnd.oasis.opendocument.presentation':
    case 'application/vnd.oasis.opendocument.presentation-template':
    case 'application/vnd.openxmlformats-officedocument.presentationml.presentation':
    case 'application/vnd.stardivision.impress':
    case 'application/vnd.sun.xml.impress':
    case 'application/vnd.sun.xml.impress.template':
    case 'application/x-kpresenter':
      return 'x-office-presentation';

    // Compressed archive types.
    case 'application/zip':
    case 'application/x-zip':
    case 'application/stuffit':
    case 'application/x-stuffit':
    case 'application/x-7z-compressed':
    case 'application/x-ace':
    case 'application/x-arj':
    case 'application/x-bzip':
    case 'application/x-bzip-compressed-tar':
    case 'application/x-compress':
    case 'application/x-compressed-tar':
    case 'application/x-cpio-compressed':
    case 'application/x-deb':
    case 'application/x-gzip':
    case 'application/x-java-archive':
    case 'application/x-lha':
    case 'application/x-lhz':
    case 'application/x-lzop':
    case 'application/x-rar':
    case 'application/x-rpm':
    case 'application/x-tzo':
    case 'application/x-tar':
    case 'application/x-tarz':
    case 'application/x-tgz':
      return 'package-x-generic';

    // Script file types.
    case 'application/ecmascript':
    case 'application/javascript':
    case 'application/mathematica':
    case 'application/vnd.mozilla.xul+xml':
    case 'application/x-asp':
    case 'application/x-awk':
    case 'application/x-cgi':
    case 'application/x-csh':
    case 'application/x-m4':
    case 'application/x-perl':
    case 'application/x-php':
    case 'application/x-ruby':
    case 'application/x-shellscript':
    case 'text/vnd.wap.wmlscript':
    case 'text/x-emacs-lisp':
    case 'text/x-haskell':
    case 'text/x-literate-haskell':
    case 'text/x-lua':
    case 'text/x-makefile':
    case 'text/x-matlab':
    case 'text/x-python':
    case 'text/x-sql':
    case 'text/x-tcl':
      return 'text-x-script';

    // HTML aliases.
    case 'application/xhtml+xml':
      return 'text-html';

    // Executable types.
    case 'application/x-macbinary':
    case 'application/x-ms-dos-executable':
    case 'application/x-pef-executable':
      return 'application-x-executable';

    default:
      return FALSE;
  }
}
