Drupal.behaviors.facetapi = function(context) {
  var settings = Drupal.settings;
  // Ensures ALL soft limit select boxes are updated.
  // @see http://drupal.org/node/735528
  $('select[name="soft_limit"]').change(function() {
      $('select[name="soft_limit"]').val($(this).val());
    });

  // Ensures ALL nofollow checkboxes are updated.
  // @see http://drupal.org/node/735528
  $('input[name="nofollow"]').change(function() {
      if ($(this).attr('checked')) {
        $('input[name="nofollow"]').attr('checked', 'checked');
      }
      else {
        $('input[name="nofollow"]').removeAttr('checked');
      }
    });

  // Ensures ALL show expanded checkboxes are updated.
  // @see http://drupal.org/node/735528
  $('input[name="show_expanded"]').change(function() {
      if ($(this).attr('checked')) {
        $('input[name="show_expanded"]').attr('checked', 'checked');
      }
      else {
        $('input[name="show_expanded"]').removeAttr('checked');
      }
    });

  // Handles bug where input format fieldset is not hidden.
  // @see http://drupal.org/node/997826
  if ($('select[name="empty_behavior"]').val() != 'text') {
    $('fieldset#edit-empty-text-format').hide();
  }
  $('select[name="empty_behavior"]').change(function() {
      if ($(this).val() != 'text') {
        $('fieldset#edit-empty-text-format').hide();
      }
      else {
        $('fieldset#edit-empty-text-format').show();
      }
    });
}
