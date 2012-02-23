<?php
// $Id: template.zen.inc,v 1.3 2009/08/24 09:16:01 johnalbin Exp $

/**
 * @file
 * Add stylesheets that are only needed when Zen is the enabled theme.
 *
 * Don't do something this dumb in your sub-theme. Stylesheets should be added
 * using your sub-theme's .info file. If you desire styles that apply
 * conditionally, you can conditionally add a "body class" in the
 * preprocess_page function. For example, see how wireframes.css is handled in
 * zen_preprocess_page() and wireframes.css.
 */

/**
 * If the user is silly and enables Zen as the theme, manually add some stylesheets.
 */
function _zen_preprocess_page(&$vars, $hook) {
  $directory = _zen_path() . '/zen-internals/css';

  // Add default styles.
  if (theme_get_setting('zen_layout') == 'zen-columns-fixed') {
    drupal_add_css($directory . '/zen-fixed.css', 'theme', 'all');
  }
  else {
    drupal_add_css($directory . '/zen-liquid.css', 'theme', 'all');
  }
  // Add print styles.
  drupal_add_css($directory . '/print.css', 'theme', 'print');

  // Regenerate the stylesheets.
  $vars['css'] = drupal_add_css();
  $vars['styles'] = drupal_get_css();

  // Add IE styles.
  $query_string = '?'. substr(variable_get('css_js_query_string', '0'), 0, 1);
  $base_path = base_path() . $directory;
  $vars['styles'] .= <<< IE_STYLES
<!--[if IE]><link type="text/css" rel="stylesheet" media="all" href="$base_path/ie.css$query_string" /><![endif]-->
<!--[if lte IE 6]><link type="text/css" rel="stylesheet" media="all" href="$base_path/ie6.css$query_string" /><![endif]-->

IE_STYLES;
}
