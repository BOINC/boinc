$(document).ready( function() {
  // Hide the breadcrumb details, if no breadcrumb.
  $('#edit-zen-breadcrumb').change(
    function() {
      div = $('#div-zen-breadcrumb-collapse');
      if ($('#edit-zen-breadcrumb').val() == 'no') {
        div.slideUp('slow');
      } else if (div.css('display') == 'none') {
        div.slideDown('slow');
      }
    }
  );
  if ($('#edit-zen-breadcrumb').val() == 'no') {
    $('#div-zen-breadcrumb-collapse').css('display', 'none');
  }
  $('#edit-zen-breadcrumb-title').change(
    function() {
      checkbox = $('#edit-zen-breadcrumb-trailing');
      if ($('#edit-zen-breadcrumb-title').attr('checked')) {
        checkbox.attr('disabled', 'disabled');
      } else {
        checkbox.removeAttr('disabled');
      }
    }
  );
  $('#edit-zen-breadcrumb-title').change();
} );
