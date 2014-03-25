
// Javascript behaviors for general CAPTCHA functionality.
Drupal.behaviors.captcha = function (context) {

  // Turn off autocompletion for the CAPTCHA response field.
  // We do it here with Javascript (instead of directly in the markup)
  // because this autocomplete attribute is not standard and
  // it would break (X)HTML compliance.
  $("#edit-captcha-response").attr("autocomplete", "off");

};


// JavaScript behaviors for the CAPTCHA admin page
Drupal.behaviors.captchaAdmin = function (context) {

	// Add onclick handler to checkbox for adding a CAPTCHA description
	// so that the textfields for the CAPTCHA description are hidden
	// when no description should be added.
	$("#edit-captcha-add-captcha-description").click(function() {
		if ($("#edit-captcha-add-captcha-description").is(":checked")) {
			// Show the CAPTCHA description textfield(s).
			$("#edit-captcha-description-wrapper").show("slow");
		}
		else {
			// Hide the CAPTCHA description textfield(s).
			$("#edit-captcha-description-wrapper").hide("slow");
		}
	});
	// Hide the CAPTCHA description textfields if option is disabled on page load.
	if (!$("#edit-captcha-add-captcha-description").is(":checked")) {
		$("#edit-captcha-description-wrapper").hide();
	}

};
