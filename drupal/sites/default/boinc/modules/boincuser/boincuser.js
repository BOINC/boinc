// Javascript for disabling the submit button, enables it when
// terms-of-use checkbox is checked.

$(document).ready(function() {
  // Disable submit on initial page load
  $("#edit-submit").attr("disabled", "disabled");
  // On checkbox change, enable submit button
  $("#edit-termsofuse-agreeTOU").change(function () {
    if ($("#edit-termsofuse-agreeTOU").is(":checked")) {
      // enable submit
      $("#edit-submit").removeAttr("disabled");
    } else {
      // disable submit
       $("#edit-submit").attr("disabled", "disabled");
    }
  })
});
