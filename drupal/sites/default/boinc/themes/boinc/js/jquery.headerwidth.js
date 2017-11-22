/**
 * Jquery function to change the header width dependning on whether or
 * not the document elements are too wide for the screen. Uses
 * clientWidth and scrollWidth, if the scrollWidth is larger than
 * clientWidth, then some element is too wide for the screen. The
 * header width is set to the scrollWidth.
 *
 * The actual HTML element set is div id=page. The CSS width is set to
 * the scrollWidth.
 */
function setheaderwidth() {
  var cw  = document.documentElement["clientWidth"];
  var swh = document.documentElement["scrollWidth"];
  var swb = document.body["scrollWidth"];
  var sw = Math.max(swh, swb);

  if (cw < sw) {
    $('#page').css('width', sw+'px');
  }
  else {
    $('#page').css('width', '');
  }
}

$(document).ready(function() {
  setheaderwidth();
});


$(window).resize(function() {
  setheaderwidth();
})
