/**
 * JQuery functions used to determine whether the theme layout should
 * use a fixed-width navigation bar or a mobile hamburger style menu. 
 *
 * The functions determine the width of the navigation bar text. This
 * text is translated into many languages, and is not static. If the
 * text width is too wide for the screen, inject CSS to toggle between
 * using a hamburger menu (usehamburger) or a navigation bar
 * (usenavbar).
 */

/* 
 * The mobile and desktop minimum breakpoints set here must match the
 * breakpoints set in the CSS @media query. See file
 * css/responsive-media.css.
 *
 * N.B. These breakpoints may be different if you are using a sub-theme.
 */ 
var padding = 30;
var mobile_bp = 750;
var desktop_min = 980;

$(document).ready(function() {
  var window_width = $(window).width();
  var total_width = $('#main-menu ul').width() + $('#action-links ul').width();
    if ( ((total_width+padding) > window_width) || window_width <= mobile_bp ) {
    $('#header-wrapper div').addClass('usehamburger');
  } else {
    $('#header-wrapper div').addClass('usenavbar');
  }

});


$(window).resize(function() {
  var window_width = $(window).width();
  if ($('#header-wrapper div').hasClass('usenavbar')) {
    var total_width = $('#main-menu ul').width() + $('#action-links ul').width();
    if ((total_width+padding) > window_width || window_width <= mobile_bp ) {
      $('#header-wrapper div').removeClass('usenavbar');
      $('#header-wrapper div').addClass('usehamburger');
    }
  }	
  if ($('#header-wrapper div').hasClass('usehamburger') && (window_width >= desktop_min)) {
    $('#header-wrapper div').removeClass('usehamburger');
    $('#header-wrapper div').addClass('usenavbar');

    // Set CSS of mobile menu to display: none when window is resized.
    document.getElementById("navigation-mmt").style.display = "none";
  }
});
