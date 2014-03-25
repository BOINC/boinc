
// JavaScript behaviors for the Image CAPTCHA
Drupal.behaviors.imageCaptcha = function (context) {

  // Add a click event to CAPTCHA images to reload the CAPTCHA image
  $(".captcha_image", context).click(function() {
    $(this).attr('src', $(this).attr('src').replace(/\?.*$/, '') + '?r=' + Math.random());
  })

};

// JavaScript behaviors for the Image CAPTCHA admin page
Drupal.behaviors.imageCaptchaAdmin = function (context) {

	// Helper function to show/hide noise level widget.
	var noise_level_shower = function(speed) {
		speed = (typeof speed == 'undefined') ? 'slow' : speed;
		if ($("#edit-image-captcha-dot-noise").is(":checked") || $("#edit-image-captcha-line-noise").is(":checked")) {
			$("#edit-image-captcha-noise-level-wrapper").show(speed);
		}
		else {
			$("#edit-image-captcha-noise-level-wrapper").hide(speed);
		}
	}
	// Add onclick handler to the dot and line noise check boxes.
	$("#edit-image-captcha-dot-noise").click(noise_level_shower);
	$("#edit-image-captcha-line-noise").click(noise_level_shower);
	// Show or hide appropriately on page load.
	noise_level_shower(0);

	// Helper function to show/hide smooth distortion widget.
	var smooth_distortion_shower = function(speed) {
		speed = (typeof speed == 'undefined') ? 'slow' : speed;
		if ($("#edit-image-captcha-distortion-amplitude").val() > 0) {
			$("#edit-image-captcha-bilinear-interpolation-wrapper").show(speed);
		}
		else {
			$("#edit-image-captcha-bilinear-interpolation-wrapper").hide(speed);
		}
	}
	// Add onchange handler to the distortion level select widget.
	$("#edit-image-captcha-distortion-amplitude").change(smooth_distortion_shower);
	// Show or hide appropriately on page load.
	smooth_distortion_shower(0)

};
