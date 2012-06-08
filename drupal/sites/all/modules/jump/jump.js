Drupal.behaviors.jumpJumpOnClick = function(context) {
  // if js is running then hide the go button
  $('form.jump-quickly.js-enabled input.form-submit').css('display', 'none');
  // Match "<protocol>://".
  var protocol_re = /^[a-z]+:\/\/.*/;
  // We watch the whole select field here because ie and safari do not do
  // well with binding click handlers on option elements.
  $('form.jump-quickly.js-enabled select').change(function() {
    // Don't jump if clicking on the title option, when enabled.
    if ($(this).hasClass('first-no-jump') && $('option:selected:first-child', this).attr('value') == '') return false;
    // Create destination,
    //   if it is an absolute url use as is
    //   else concatenate base path and value.
    var dest = (protocol_re.test(dest)) ? $(this).attr('value') : Drupal.settings.basePath + $(this).attr('value');
    window.location = dest;
    return false;
  });
};
