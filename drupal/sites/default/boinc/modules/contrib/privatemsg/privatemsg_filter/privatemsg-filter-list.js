Drupal.behaviors.tagActionButtons = function (context) {
  $("div.privatemsg-tag-add-submit").hide();
  $("div.privatemsg-tag-remove-submit").hide();

  $('#privatemsg-list #edit-tag-add').change(function () {
    $('#edit-tag-add-submit').click();
  });

  $('#privatemsg-list #edit-tag-remove').change(function () {
    $('#edit-tag-remove-submit').click();
  });
};
