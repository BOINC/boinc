<?php

// Set this so we can tell that the file has been included at some point.
define('CTOOLS_AJAX_INCLUDED', 1);

/**
 * @file
 * Utilize the CTools AJAX responder.
 *
 * The AJAX responder is a javascript tool to make it very easy to do complicated
 * operations as a response to AJAX requests. When links are attached to the ajax
 * responder, the server sends back a packet of JSON data; this packet is an
 * array of commands to carry out.
 *
 * The command names correlate to functions in the responder space, making it
 * relatively easy for applications to provide their own commands to do whatever
 * spiffy functionality is necessary.
 *
 * Each command is an object. $object->command is the type of command and
 * will be used to find the function (it will correlate directly to
 * a function in the Drupal.CTools.AJAX.Command space). The object can
 * contain any other data that the command needs to process.
 *
 * Built in commands include:
 * - replace
 *   - selector: The CSS selector. This can be any selector jquery uses in $().
 *   - data: The data to use with the jquery replace() function.
 *
 * - prepend
 *   - selector: The CSS selector. This can be any selector jquery uses in $().
 *   - data: The data to use with the jquery prepend() function.
 *
 * - append
 *   - selector: The CSS selector. This can be any selector jquery uses in $().
 *   - data: The data to use with the jquery append() function.
 *
 * - after
 *   - selector: The CSS selector. This can be any selector jquery uses in $().
 *   - data: The data to use with the jquery after() function.
 *
 * - before
 *   - selector: The CSS selector. This can be any selector jquery uses in $().
 *   - data: The data to use with the jquery before() function.
 *
 * - remove
 *   - selector: The CSS selector. This can be any selector jquery uses in $().
 *
 * - changed
 *   - selector: The CSS selector. This selector will have 'changed' added as a class.
 *   - star: If set, will add a star to this selector. It must be within the 'selector' above.
 *
 * - alert
 *   - title: The title of the alert.
 *   - data: The data in the alert.
 *
 * - css
 *   - selector: The CSS selector to add CSS to.
 *   - argument: An array of 'key': 'value' CSS selectors to set.
 *
 * - attr
 *   - selector: The CSS selector. This can be any selector jquery uses in $().
 *   - name: The name or key of the data attached to this selector.
 *   - value: The value of the data.
 *
 * - settings
 *   - argument: An array of settings to add to Drupal.settings via $.extend
 *
 * - data
 *   - selector: The CSS selector. This can be any selector jquery uses in $().
 *   - name: The name or key of the data attached to this selector.
 *   - value: The value of the data. Not just limited to strings can be any format.
 *
 * - redirect
 *   - url: The url to be redirected to. This can be an absolute URL or a Drupal path.
 *
 * - reload
 *
 * - submit
 *   - selector: The CSS selector to identify the form for submission. This can
 *     be any selector jquery uses in $().
 *
 * Commands are usually created with a couple of helper functions, so they
 * look like this:
 *
 * @code
 *   $commands = array();
 *   $commands[] = ctools_ajax_command_replace('#ctools-object-1', 'some html here');
 *   $commands[] = ctools_ajax_command_changed('#ctools-object-1');
 *   ctools_ajax_render($commands); // this function exits.
 * @endcode
 */

/**
 * Render an image as a button link. This will automatically apply an AJAX class
 * to the link and add the appropriate javascript to make this happen.
 *
 * @param $image
 *   The path to an image to use that will be sent to theme('image') for rendering.
 * @param $dest
 *   The destination of the link.
 * @param $alt
 *   The alt text of the link.
 * @param $class
 *   Any class to apply to the link. @todo this should be a options array.
 */
function ctools_ajax_image_button($image, $dest, $alt, $class = '') {
  return ctools_ajax_text_button(theme('image', $image), $dest, $alt, $class);
}

/**
 * Render text as a link. This will automatically apply an AJAX class
 * to the link and add the appropriate javascript to make this happen.
 *
 * Note: 'html' => true so be sure any text is vetted! Chances are these kinds of buttons will
 * not use user input so this is a very minor concern.
 *
 * @param $image
 *   The path to an image to use that will be sent to theme('image') for rendering.
 * @param $dest
 *   The destination of the link.
 * @param $alt
 *   The alt text of the link.
 * @param $class
 *   Any class to apply to the link. @todo this should be a options array.
 * @param $type
 *   A type to use, in case a different behavior should be attached. Defaults
 *   to ctools-use-ajax.
 */
function ctools_ajax_text_button($text, $dest, $alt, $class = '', $type = 'ctools-use-ajax') {
  return l($text, $dest, array('html' => TRUE, 'attributes' => array('class' => "$type $class", 'title' => $alt)));
}

/**
 * Create a command array for the error case.
 */
function ctools_ajax_command_error($error = '') {
  return array(
    'command' => 'alert',
    'title' => t('Error'),
    'text' => $error ? $error : t('Server reports invalid input error.'),
  );
}

/**
 * Create a replace command for the AJAX responder.
 *
 * The replace command will replace a portion of the current document
 * with the specified HTML.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $html
 *   The data to use with the jquery replace() function.
 */
function ctools_ajax_command_replace($selector, $html) {
  return array(
    'command' => 'replace',
    'selector' => $selector,
    'data' => $html,
  );
}

/**
 * Set the HTML of a given selector to the given data.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $html
 *   The data to use with the jquery replace() function.
 */
function ctools_ajax_command_html($selector, $html) {
  return array(
    'command' => 'html',
    'selector' => $selector,
    'data' => $html,
  );
}

/**
 * Create a prepend command for the AJAX responder.
 *
 * This will prepend the HTML to the specified selector.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $html
 *   The data to use with the jquery prepend() function.
 */
function ctools_ajax_command_prepend($selector, $html) {
  return array(
    'command' => 'prepend',
    'selector' => $selector,
    'data' => $html,
  );
}

/**
 * Create an append command for the AJAX responder.
 *
 * This will append the HTML to the specified selector.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $html
 *   The data to use with the jquery append() function.
 */
function ctools_ajax_command_append($selector, $html) {
  return array(
    'command' => 'append',
    'selector' => $selector,
    'data' => $html,
  );
}

/**
 * Create an after command for the AJAX responder.
 *
 * This will add the HTML after the specified selector.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $html
 *   The data to use with the jquery after() function.
 */
function ctools_ajax_command_after($selector, $html) {
  return array(
    'command' => 'after',
    'selector' => $selector,
    'data' => $html,
  );
}

/**
 * Create a before command for the AJAX responder.
 *
 * This will add the HTML before the specified selector.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $html
 *   The data to use with the jquery before() function.
 */
function ctools_ajax_command_before($selector, $html) {
  return array(
    'command' => 'before',
    'selector' => $selector,
    'data' => $html,
  );
}

/**
 * Create a remove command for the AJAX responder.
 *
 * This will remove the specified selector and everything within it.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 */
function ctools_ajax_command_remove($selector) {
  return array(
    'command' => 'remove',
    'selector' => $selector,
  );
}

/**
 * Create a changed command for the AJAX responder.
 *
 * This will mark an item as 'changed'.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $star
 *   An optional CSS selector which must be inside $selector. If specified,
 *   a star will be appended.
 */
function ctools_ajax_command_changed($selector, $star = '') {
  return array(
    'command' => 'changed',
    'selector' => $selector,
    'star' => $star,
  );
}

/**
 * Create a css command for the AJAX responder.
 *
 * This will directly add CSS to the page.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $argument
 *   An array of key: value pairs to set in the CSS for the selector.
 */
function ctools_ajax_command_css($selector, $argument) {
  return array(
    'command' => 'css',
    'selector' => $selector,
    'argument' => $argument,
  );
}

/**
 * Create a settings command for the AJAX responder.
 *
 * This will add CSS files to the output. Files that have already
 * been processed will not be processed again.
 *
 * @param $argument
 *   An array of CSS files.
 */
function ctools_ajax_command_css_files($argument) {
  return array(
    'command' => 'css_files',
    'argument' => $argument,
  );
}

/**
 * Create a settings command for the AJAX responder.
 *
 * This will extend Drupal.settings with the given array.
 *
 * @param $argument
 *   An array of key: value pairs to add to the settings.
 */
function ctools_ajax_command_settings($argument) {
  return array(
    'command' => 'settings',
    'argument' => $argument,
  );
}

/**
 * Create a settings command for the AJAX responder.
 *
 * This will add javascript files to the output. Files that have already
 * been processed will not be processed again.
 *
 * @param $argument
 *   An array of javascript files.
 */
function ctools_ajax_command_scripts($argument) {
  return array(
    'command' => 'scripts',
    'argument' => $argument,
  );
}

/**
 * Create a data command for the AJAX responder.
 *
 * This will attach the name=value pair of data to the selector via
 * jquery's data cache.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 * @param $name
 *   The name or key: of the data attached to this selector.
 * @param $value
 *  The value of the data. Not just limited to strings can be any format.
 */
function ctools_ajax_command_data($selector, $name, $value) {
  return array(
    'command' => 'data',
    'selector' => $selector,
    'name' => $name,
    'value' => $value,
  );
}

 /**
  * Set a single property to a value, on all matched elements.
  *
  * @param $selector
  *   The CSS selector. This can be any selector jquery uses in $().
  * @param $name
  *   The name or key: of the data attached to this selector.
  * @param $value
  *  The value of the data.
  */
 function ctools_ajax_command_attr($selector, $name, $value) {
   return array(
     'command' => 'attr',
     'selector' => $selector,
     'name' => $name,
     'value' => $value,
   );
 }

/**
 * Force a table to be restriped.
 *
 * This is usually used after a table has been modified by a replace or append
 * command.
 *
 * @param $selector
 *   The CSS selector. This can be any selector jquery uses in $().
 */
function ctools_ajax_command_restripe($selector) {
  return array(
    'command' => 'restripe',
    'selector' => $selector,
  );
}

/**
 * Force a client-side redirect.
 *
 * @param $url
 *   The url to be redirected to. This can be an absolute URL or a
 *   Drupal path.
 * @param $delay
 *   A delay before applying the redirection, in milliseconds.
 * @param $options
 *   An array of options to pass to the url() function.
 * @param $new_window
 *   A bool to determine if the URL should open in a new window.
 */
function ctools_ajax_command_redirect($url, $delay = 0, $options = array(), $new_window = FALSE) {
  return array(
    'command' => 'redirect',
    'url' => url($url, $options),
    'delay' => $delay,
    'new_window' => $new_window,
  );
}

/**
 * Force a reload of the current page.
 */
function ctools_ajax_command_reload() {
  return array(
    'command' => 'reload',
  );
}

/**
 * Submit a form.
 *
 * This is useful for submitting a parent form after a child form has finished
 * processing in a modal overlay.
 *
 * @param $selector
 *   The CSS selector to identify the form for submission. This can be any
 *   selector jquery uses in $().
 */
function ctools_ajax_command_submit($selector) {
  return array(
    'command' => 'submit',
    'selector' => $selector,
  );
}

/**
 * Render a commands array into JSON and immediately hand this back
 * to the AJAX requester.
 */
function ctools_ajax_render($commands = array()) {
  // Although ajax_deliver() does this, some contributed and custom modules
  // render AJAX responses without using that delivery callback.
  ctools_ajax_set_verification_header();

  $js_files = array();
  $settings = ctools_process_js_files($js_files, 'header');
  $settings += ctools_process_js_files($js_files, 'footer');

  $query_string = '?'. substr(variable_get('css_js_query_string', '0'), 0, 1);
  $css = drupal_add_css();
  foreach ($css as $media => $types) {
    // If CSS preprocessing is off, we still need to output the styles.
    // Additionally, go through any remaining styles if CSS preprocessing is on and output the non-cached ones.
    foreach ($types as $type => $files) {
      if ($type == 'module') {
        // Setup theme overrides for module styles.
        $theme_styles = array();
        foreach (array_keys($css[$media]['theme']) as $theme_style) {
          $theme_styles[] = basename($theme_style);
        }
      }
      // The theme stuff should already be added and because of admin themes,
      // this could cause different CSS to be added.
      if ($type != 'theme') {
        foreach ($types[$type] as $file => $preprocess) {
          // If the theme supplies its own style using the name of the module style, skip its inclusion.
          // This includes any RTL styles associated with its main LTR counterpart.
          if ($type == 'module' && in_array(str_replace('-rtl.css', '.css', basename($file)), $theme_styles)) {
            // Unset the file to prevent its inclusion when CSS aggregation is enabled.
            unset($types[$type][$file]);
            continue;
          }
          // Only include the stylesheet if it exists.
          if (file_exists($file)) {
            $css_files[] = array(
              'file' => base_path() . $file . $query_string,
              'media' => $media,
            );
          }
        }
      }
    }
  }

  if (!empty($js_files)) {
    array_unshift($commands, ctools_ajax_command_scripts(array_keys($js_files)));
  }

  if (!empty($css_files)) {
    array_unshift($commands, ctools_ajax_command_css_files($css_files));
  }

  if (!empty($settings)) {
    array_unshift($commands, ctools_ajax_command_settings(call_user_func_array('array_merge_recursive', $settings)));
  }

  if (!empty($_REQUEST['ctools_multipart'])) {
    // We don't use drupal_json here because the header is not true. We're not really
    // returning JSON, strictly-speaking, but rather JSON content wrapped in a <textarea>
    // as per the "file uploads" example here: http://malsup.com/jquery/form/#code-samples
    echo '<textarea>' . drupal_to_js($commands) . '</textarea>';
  }
  else {
    drupal_json($commands);
  }
  exit;
}

/**
 * Sets a response header for ajax.js to trust the response body.
 *
 * It is not safe to invoke JS commands within user-uploaded files, so this
 * header protects against those being invoked.
 *
 * @see Drupal.ajax.ajax.options.success()
 */
function ctools_ajax_set_verification_header() {
  $set = &ctools_static(__FUNCTION__);

  if (!isset($set)) {
    // User-uploaded files cannot set any response headers, so the token value
    // does not need to be hard to guess.
    drupal_set_header('X-Drupal-Ajax-Token: 1');
    $set = TRUE;
  }
}

/**
 * Send an error response back via AJAX and immediately exit.
 */
function ctools_ajax_render_error($error = '') {
  $commands = array();
  $commands[] = ctools_ajax_command_error($error);
  ctools_ajax_render($commands);
}

/**
 * Associate a URL to a form element with a hidden form.
 *
 * This is a helper function to easily associate a URL with a form element
 * which can be used for different ajax functionality.
 *
 * You would call this function on a form element in the form function like this:
 *
 * @code
 *   $form['example'] = array(
 *     '#title' => t('Example'),
 *     '#type' => 'select',
 *     '#options' => array(1 => 'One', 2 => 'Two', 3 => 'Three'),
 *     '#default_value' => 1,
 *   );
 *   ctools_ajax_associate_url_to_element($form, $form['example'], 'example/ajax/urlpath');
 * @endcode
 *
 * The AJAX request will POST the value of the form element in the
 * "ctools_changed" parameter (i.e. $_POST['ctools_changed']).
 *
 * @param &$form
 *   Reference to the form element. This is required to have the #id and
 *   #attribute elements populated and to create the hidden form element for
 *   each select.
 * @param &$form_element
 *   The form element we are going to take action on.
 * @param $dest
 *   The URL to associate the form element to.
 * @param $type
 *   Optional; A type to use, in case a different behavior should be attached.
 *   If empty the type will be set to "ctools-use-ajax" for submit elements and
 *   "ctools-use-ajax-onchange" for other elements.
 */
function ctools_ajax_associate_url_to_element(&$form, &$form_element, $dest, $type = '') {
  drupal_add_js('misc/jquery.form.js', 'core');
  if (!isset($form_element['#id'])) {
    //Create a unique ID to associate $form_element and hidden elements since we dont have an ID
    $form_element['#id'] = uniqid('ctools-ajax-url-');

    if (empty($type)) {
      $type = $form_element['#type'] == 'submit' ? 'ctools-use-ajax' : 'ctools-use-ajax-onchange';
    }

    if (empty($form_element['#attributes']['class'])) {
      $form_element['#attributes']['class'] = $type;
    }
    else {
      $form_element['#attributes']['class'] .= " $type";
    }
  }

  //Add hidden form element to hold base URL
  $form[$form_element['#id'] . '-url'] = array(
    '#type' => 'hidden',
    '#value' => $dest,
    '#attributes' => array('class' =>  $form_element['#id'] . '-url'),
  );
}

/**
 * Function that controls if ctools_ajax_page_preprocess() will run.
 *
 * Useful if not using Drupal's core CSS/JS handling & you wish to override it.
 * If skipping the page_preprocess function you must provide the scripts and css
 * files loaded on this page as a JS setting in the footer under CToolsAJAX.
 *
 * @param $value
 *   Bool; set to FALSE to disable ctools_ajax_page_preprocess() from running.
 */
function ctools_ajax_run_page_preprocess($value = TRUE) {
  $run_hook = &ctools_static('ctools_ajax_page_preprocess', TRUE);
  $run_hook = $value;
}

/**
 * Implement hook_preprocess_page. Process variables for page.tpl.php
 *
 * @param $variables
 *   The existing theme data structure.
 */
function ctools_ajax_page_preprocess(&$variables) {
  // See if we will be running this hook.
  $run_hook = &ctools_static(__FUNCTION__, TRUE);
  if (!$run_hook) {
    return;
  }

  $js_files = $css_files = array();
  ctools_process_js_files($js_files, 'header');
  ctools_process_js_files($js_files, 'footer');
  ctools_process_css_files($css_files, $variables['css']);

  // Add loaded JS and CSS information to the footer, so that an AJAX
  // request knows if they are already loaded.
  // For inline Javascript to validate as XHTML, all Javascript containing
  // XHTML needs to be wrapped in CDATA. To make that backwards compatible
  // with HTML 4, we need to comment out the CDATA-tag.
  $loaded = array('CToolsAJAX' => array('scripts' => $js_files, 'css' => $css_files));
  $embed_prefix = "\n<!--//--><![CDATA[//><!--\n";
  $embed_suffix = "\n//--><!]]>\n";
  $variables['closure'].= '<script type="text/javascript">' . $embed_prefix . 'jQuery.extend(Drupal.settings, ' . drupal_to_js($loaded) . ");" . $embed_suffix . "</script>\n";
}

/**
 * Create a list of javascript files that are on the page.
 *
 * @param $js_files
 *   Array of js files that are loaded on this page.
 * @param $scope
 *   String usually containing header or footer.
 * @param $scripts
 *   (Optional) array returned from drupal_add_js(). If NULL then it will load
 *   the array from drupal_add_js for the given scope.
 * @return array $settings
 *   The JS 'setting' array for the given scope.
 */
function ctools_process_js_files(&$js_files, $scope, $scripts = NULL) {
  // Automatically extract any 'settings' added via drupal_add_js() and make
  // them the first command.
  if (empty($scripts)) {
    $scripts = drupal_add_js(NULL, NULL, $scope);
  }

  // Get replacements that are going to be made by contrib modules and take
  // them into account so we don't double-load scripts.
  static $replacements = NULL;
  if (!isset($replacements)) {
    $replacements = module_invoke_all('js_replacements');
  }

  $settings = array();
  foreach ($scripts as $type => $data) {
    switch ($type) {
      case 'setting':
        $settings = $data;
        break;
      case 'inline':
      case 'theme':
        // Presently we ignore inline javascript.
        // Theme JS is already added and because of admin themes, this could add
        // improper JS to the page.
        break;
      default:
        // If JS preprocessing is off, we still need to output the scripts.
        // Additionally, go through any remaining scripts if JS preprocessing is on and output the non-cached ones.
        foreach ($data as $path => $info) {
          // If the script is being replaced, take that replacement into account.
          $final_path = isset($replacements[$type][$path]) ? $replacements[$type][$path] : $path;
          $js_files[base_path() . $final_path] = TRUE;
        }
    }
  }

  return $settings;
}

/**
 * Create a list of CSS files to add to the page.
 *
 * @param $css_files
 *   Array of css files that are loaded on this page. Passed by reference and
 *   previous values are wiped.
 * @param $css
 *   Array returned from drupal_add_css() or $variables['css'] from
 *   hook_preprocess_page.
 */
function ctools_process_css_files(&$css_files, $css) {
  // Go through all CSS files that are being added to the page and catalog them.
  $css_files = array();
  foreach ($css as $media => $types) {
    // If CSS preprocessing is off, we still need to output the styles.
    // Additionally, go through any remaining styles if CSS preprocessing is on and output the non-cached ones.
    foreach ($types as $type => $files) {
      if ($type == 'module') {
        // Setup theme overrides for module styles.
        $theme_styles = array();
        foreach (array_keys($css[$media]['theme']) as $theme_style) {
          $theme_styles[] = basename($theme_style);
        }
      }
      foreach ($types[$type] as $file => $preprocess) {
        // If the theme supplies its own style using the name of the module style, skip its inclusion.
        // This includes any RTL styles associated with its main LTR counterpart.
        if ($type == 'module' && in_array(str_replace('-rtl.css', '.css', basename($file)), $theme_styles)) {
          // Unset the file to prevent its inclusion when CSS aggregation is enabled.
          unset($types[$type][$file]);
          continue;
        }
        // Only include the stylesheet if it exists.
        if (file_exists($file)) {
          $css_files[base_path() . $file] = TRUE;
        }
      }
    }
  }
}
