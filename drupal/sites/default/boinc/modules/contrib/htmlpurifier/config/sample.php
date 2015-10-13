<?php

/**
 * @file
 * This file is a sample advanced PHP configuration file for the HTML Purifier
 * filter module. In reality, this file would be named N.php, where N is the
 * integer identifying the filter this is configuring. The configure page
 * for HTML Purifier (advanced) will tell you what file to copy this to.
 * 
 * See this URI:
 *
 *    http://htmlpurifier.org/live/configdoc/plain.html
 * 
 * For full information about permitted directives. The most interesting ones
 * for custom configuration are ones with the 'mixed' type, as they cannot
 * be configured using the webform.
 * 
 * @note
 *    A number of directives have been predefined for you in order to better
 *    work with Drupal. You can see what these defaults in the
 *    _htmlpurifier_get_config() function in htmlpurifier.module.php.
 * 
 * @warning
 *    Please be mindful of the version of HTML Purifier you have installed!
 *    All of the docs linked to are for the latest version of HTML Purifier;
 *    your installation may or may not be up-to-date.
 */

/**
 * Accepts an HTMLPurifier_Config configuration object and configures it.
 * 
 * @param $config
 *    Instance of HTMLPurifier_Config to modify. See
 *        http://htmlpurifier.org/doxygen/html/classHTMLPurifier__Config.html
 *    for a full API.
 * 
 * @note
 *    No return value is needed, as PHP objects are passed by reference.
 */
function htmlpurifier_config_N($config) {
  // Set your configuration here:
  $config->set('Core', 'Lexer', 'DirectLex');
  // $config->set('Namespace', 'Directive', $value);
  
  // Advanced users:
  // $def = $config->getDefinition('HTML');
  // For more information about this, see:
  //    http://htmlpurifier.org/docs/enduser-customize.html
}

