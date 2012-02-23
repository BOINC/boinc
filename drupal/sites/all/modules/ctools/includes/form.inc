<?php
// $Id: form.inc,v 1.11.2.3 2010/08/20 22:16:57 merlinofchaos Exp $

/**
 * @file
 * CTools' replacements for Drupal's form functions.
 *
 * Primarily, ctools_build_form is meant to be a replacement for drupal_get_form().
 *
 * Instead of sending arguments through to the form builder, a form state array
 * is passed through. This form_state can contain any arguments needed, and it can
 * also be used to return data to the calling function.
 *
 * This can allow cleaner separation of the form from the storage mechanism.
 */

/**
 * Build a form, similar to drupal_get_form(). However, arguments
 * to the form builder are not sent through. Instead, the $form_state
 * can be given all the necessary data to fully utilize the form.
 */
function ctools_build_form($form_id, &$form_state) {
  // Ensure that we have some defaults.

  // These are defaults only; if already set they will not be overridden.
  $form_state += array('storage' => NULL, 'submitted' => FALSE, 'input' => $_POST, 'method' => 'post');

  $args = isset($form_state['args']) ? $form_state['args'] : array();
  $cacheable = FALSE;

  if (isset($_SESSION['batch_form_state'])) {
    // We've been redirected here after a batch processing : the form has
    // already been processed, so we grab the post-process $form_state value
    // and move on to form display. See _batch_finished() function.
    $form_state = $_SESSION['batch_form_state'];
    unset($_SESSION['batch_form_state']);
  }
  else {
    // If the incoming $form_state['input'] contains a form_build_id, we'll check the
    // cache for a copy of the form in question. If it's there, we don't
    // have to rebuild the form to proceed. In addition, if there is stored
    // form_state data from a previous step, we'll retrieve it so it can
    // be passed on to the form processing code.
    if (isset($form_state['input']['form_id']) && $form_state['input']['form_id'] == $form_id && !empty($form_state['input']['form_build_id'])) {
      $form_build_id = $form_state['input']['form_build_id'];
      $form = form_get_cache($form_build_id, $form_state);
      if (!empty($form['#no_cache']) || empty($form)) {
        unset($form);
      }
    }

    // If the previous bit of code didn't result in a populated $form
    // object, we're hitting the form for the first time and we need
    // to build it from scratch.
    if (!isset($form)) {
      $form_state['post'] = $form_state['input'];
      // This allows us to do some interesting form embedding stuff without
      // messing up the form IDs too badly.
      if (isset($form_state['wrapper callback']) && function_exists($form_state['wrapper callback'])) {
        // If there is a wrapper callback, we do not use drupal_retrieve_form.
        // Instead, we call $form_id builder function directly. This means the args
        // are *different* for forms used this way, which may not be ideal but
        // is necessary right now.
        $form = array();
        $form_state['wrapper callback']($form, $form_state);
        if (function_exists($form_id)) {
          $form_id($form, $form_state);
        }
      }
      else {
        // Use a copy of the function's arguments for manipulation
        $args_temp = $args;
        array_unshift($args_temp, 'placeholder');
        $args_temp[0] = &$form_state; // replaces the placeholder.
        array_unshift($args_temp, $form_id);

        $form = call_user_func_array('drupal_retrieve_form', $args_temp);
      }

      $form_build_id = 'form-' . md5(mt_rand());
      $form['#build_id'] = $form_build_id;


      if ($form_state['method'] == 'get' && !isset($form['#method'])) {
        $form['#method'] = 'get';
      }

      drupal_prepare_form($form_id, $form, $form_state);
      // Store a copy of the unprocessed form for caching and indicate that it
      // is cacheable if #cache will be set.
      $original_form = $form;
      $cacheable = TRUE;
      unset($form_state['post']);
    }
    $form['#post'] = $form_state['input'];

    // Now that we know we have a form, we'll process it (validating,
    // submitting, and handling the results returned by its submission
    // handlers. Submit handlers accumulate data in the form_state by
    // altering the $form_state variable, which is passed into them by
    // reference.
    ctools_process_form($form_id, $form, $form_state);
    // If we were told not to redirect, but not told to re-render, return
    // here.
    if (!empty($form_state['executed']) && empty($form_state['rerender'])) {
      return;
    }
    if ($cacheable && !empty($form['#cache']) && empty($form['#no_cache'])) {
      // Caching is done past drupal_process_form so #process callbacks can
      // set #cache. By not sending the form state, we avoid storing
      // $form_state['storage'].
      form_set_cache($form_build_id, $original_form, NULL);
    }
  }

  // Most simple, single-step forms will be finished by this point --
  // drupal_process_form() usually redirects to another page (or to
  // a 'fresh' copy of the form) once processing is complete. If one
  // of the form's handlers has set $form_state['redirect'] to FALSE,
  // the form will simply be re-rendered with the values still in its
  // fields.
  //
  // If $form_state['storage'] or $form_state['rebuild'] have been
  // set by any submit or validate handlers, however, we know that
  // we're in a complex multi-part process of some sort and the form's
  // workflow is NOT complete. We need to construct a fresh copy of
  // the form, passing in the latest $form_state in addition to any
  // other variables passed into drupal_get_form().
  //
  // If this function is being used to perform an '#ahah' callback
  // to rebuild some or all of a ctools wizard form step, be sure that
  // $form_state['wrapper callback'], $form_state['form_info'],
  // $form_state['step'], $form_state['no_redirect'], and $form_state['rebuild']
  // are properly set.

  if (!empty($form_state['rebuild']) || !empty($form_state['storage'])) {
    $form = ctools_rebuild_form($form_id, $form_state, $args, $form_build_id);
  }

  // If whoever is calling this wants the $form array (so that it can render it
  // another way, for example) then return it.
  if (!empty($form_state['want form'])) {
    return $form;
  }

  // Do not render certain items if requested not to:
  if (!empty($form_state['drop tokens'])) {
    unset($form['#id']);
    unset($form['#build_id']);
    unset($form['#token']);
    unset($form['form_token']);
  }
  // If we haven't redirected to a new location by now, we want to
  // render whatever form array is currently in hand.
  // Do not render certain items if requested not to:
  if (!empty($form_state['drop tokens'])) {
    unset($form['form_id']);
    unset($form['form_build_id']);
    unset($form['form_token']);
  }
  return drupal_render_form($form_id, $form);
}

/**
 * ctools' replacement of drupal_rebuild_form.
 *
 * This change merely respects a form's wishes not to be cached.
 */
function ctools_rebuild_form($form_id, &$form_state, $args, $form_build_id = NULL) {
  // Remove the first argument. This is $form_id.when called from
  // drupal_get_form and the original $form_state when called from some AHAH
  // callback. Neither is needed. After that, put in the current state.
  array_unshift($args, 'placeholder');
  $args[0] = &$form_state; // Replaces placeholder.
  // And the form_id.
  array_unshift($args, $form_id);

  if (isset($form_state['wrapper callback']) && function_exists($form_state['wrapper callback'])) {
    // If there is a wrapper callback, we do not use drupal_retrieve_form.
    // Instead, we call $form_id builder function directly. This means the args
    // are *different* for forms used this way, which may not be ideal but
    // is necessary right now.
    $form = array();
    $form_state['wrapper callback']($form, $form_state);
    if (function_exists($form_id)) {
      $form_id($form, $form_state);
    }
  }
  else {
    $form = call_user_func_array('drupal_retrieve_form', $args);
  }

  if (!isset($form_build_id)) {
    // We need a new build_id for the new version of the form.
    $form_build_id = 'form-' . md5(mt_rand());
  }
  $form['#build_id'] = $form_build_id;
  // Flush form ID element cache because we may be rebuilding
  // a form in a way that Drupal's FAPI isn't used to which
  // causes unnecessary form ID changes.
  form_clean_id(NULL, TRUE);
  drupal_prepare_form($form_id, $form, $form_state);

  if (empty($form['#no_cache'])) {
    // Now, we cache the form structure so it can be retrieved later for
    // validation. If $form_state['storage'] is populated, we'll also cache
    // it so that it can be used to resume complex multi-step processes.
    form_set_cache($form_build_id, $form, $form_state);
  }

  // Originally this called drupal_process_form, but all that happens there
  // is form_builder and then submission; and the rebuilt form is not
  // allowed to submit. Therefore, just do this:
  $form['#post'] = $form_state['input'];
  $form = form_builder($form_id, $form, $form_state);

  return $form;
}

/**
 * ctools' replacement for drupal_process_form that accepts commands
 * not to redirect, as well as forcing processing of 'get' method forms.
 */
function ctools_process_form($form_id, &$form, &$form_state) {
  // submitting, and handling the results returned by its submission
  // handlers. Submit handlers accumulate data in the form_state by
  // altering the $form_state variable, which is passed into them by
  // reference.
  $form_state['values'] = array();

  // With $_GET, these forms are always submitted.
  if ($form_state['method'] == 'get') {
    if (!isset($form['#post']['form_build_id'])) {
      $form['#post']['form_build_id'] = $form['#build_id'];
    }
    if (!isset($form['#post']['form_id'])) {
      $form['#post']['form_id'] = $form_id;
    }
    if (!isset($form['#post']['form_token']) && isset($form['#token'])) {
      $form['#post']['form_token'] = drupal_get_token($form['#token']);
    }
  }

  $form = form_builder($form_id, $form, $form_state);
  // Only process the form if it is programmed or the form_id coming
  // from the POST data is set and matches the current form_id.

  if ((!empty($form['#programmed'])) || (!empty($form['#post']) && (isset($form['#post']['form_id']) && ($form['#post']['form_id'] == $form_id)))) {
    ctools_validate_form($form_id, $form, $form_state);

    // form_clean_id() maintains a cache of element IDs it has seen,
    // so it can prevent duplicates. We want to be sure we reset that
    // cache when a form is processed, so scenerios that result in
    // the form being built behind the scenes and again for the
    // browser don't increment all the element IDs needlessly.
    form_clean_id(NULL, TRUE);

    if ((!empty($form_state['submitted'])) && !form_get_errors() && empty($form_state['rebuild'])) {
      $form_state['redirect'] = NULL;
      form_execute_handlers('submit', $form, $form_state);

      // We'll clear out the cached copies of the form and its stored data
      // here, as we've finished with them. The in-memory copies are still
      // here, though.
      if (variable_get('cache', CACHE_DISABLED) == CACHE_DISABLED && !empty($form_state['values']['form_build_id'])) {
        cache_clear_all('form_' . $form_state['values']['form_build_id'], 'cache_form');
        cache_clear_all('storage_' . $form_state['values']['form_build_id'], 'cache_form');
      }

      // If batches were set in the submit handlers, we process them now,
      // possibly ending execution. We make sure we do not react to the batch
      // that is already being processed (if a batch operation performs a
      // drupal_execute).
      if ($batch = &batch_get() && !isset($batch['current_set'])) {
        // The batch uses its own copies of $form and $form_state for
        // late execution of submit handers and post-batch redirection.
        $batch['form'] = $form;
        $batch['form_state'] = $form_state;
        $batch['progressive'] = !$form['#programmed'];
        batch_process();
        // Execution continues only for programmatic forms.
        // For 'regular' forms, we get redirected to the batch processing
        // page. Form redirection will be handled in _batch_finished(),
        // after the batch is processed.
      }

      // If no submit handlers have populated the $form_state['storage']
      // bundle, and the $form_state['rebuild'] flag has not been set,
      // we're finished and should redirect to a new destination page
      // if one has been set (and a fresh, unpopulated copy of the form
      // if one hasn't). If the form was called by drupal_execute(),
      // however, we'll skip this and let the calling function examine
      // the resulting $form_state bundle itself.
      if (!$form['#programmed'] && empty($form_state['rebuild']) && empty($form_state['storage'])) {
        if (!empty($form_state['no_redirect'])) {
          $form_state['executed'] = TRUE;
        }
        else {
          drupal_redirect_form($form, $form_state['redirect']);
        }
      }
    }
  }
}

/**
 * The original version of drupal_validate_form does not have an override for
 * the static check to only validate a form id once. Unfortunately, we need
 * to be able to override this.
 */
function ctools_validate_form($form_id, $form, &$form_state) {
  static $validated_forms = array();

  if (isset($validated_forms[$form_id]) && empty($form_state['must_validate'])) {
    return;
  }

  // If the session token was set by drupal_prepare_form(), ensure that it
  // matches the current user's session.
  if (isset($form['#token'])) {
    if (!drupal_valid_token($form_state['values']['form_token'], $form['#token'])) {
      // Setting this error will cause the form to fail validation.
      form_set_error('form_token', t('Validation error, please try again. If this error persists, please contact the site administrator.'));
    }
  }

  if (!empty($form_state['clicked_button']['#skip validation'])) {
    return;
  }

  _form_validate($form, $form_state, $form_id);
  $validated_forms[$form_id] = TRUE;
}

