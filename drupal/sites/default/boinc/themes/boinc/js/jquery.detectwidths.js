/**
 * JQuery functions used to determine whether the theme layout should
 * use a fixed-width navigation bar or a mobile hamburger style menu.
 *
 * The functions determine the width of the navigation bar text. This
 * text is translated into many languages, and is not static. If the
 * text width is too wide for the screen, inject CSS to toggle between
 * using a hamburger menu (usehamburger) or a navigation bar
 * (usenavbar).
 *
 * If the menubar text is so wide it will not fit on the desktop width
 * site (980 px), then the CSS class (oversizemenubar) will be
 * used. In addition, the oversize flag will be set. This flag informs
 * the resize function not to swap out the hamburger menu
 * (userhamburger) for the navigation bar, as the latter will be too
 * wide for the layout.
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
var oversize = 0;

/*
 * jQuery plugin from http://www.foliotek.com/devblog/getting-the-width-of-a-hidden-element-with-jquery-using-width/
 * Obtains the width of the hidden item.
 * Returns a dim object containing the items heights and widths.
 */
(function ($) {
$.fn.getHiddenDimensions = function (includeMargin) {
    var $item = this,
    props = { display: 'block' },
    dim = { width: 0, height: 0, innerWidth: 0, innerHeight: 0, outerWidth: 0, outerHeight: 0 },
    $hiddenParents = $item.parents().andSelf().not(':visible'),
    includeMargin = (includeMargin == null) ? false : includeMargin;

    var oldProps = [];
    $hiddenParents.each(function () {
        var old = {};

        for (var name in props) {
            old[name] = this.style[name];
            this.style[name] = props[name];
        }

        oldProps.push(old);
    });

    dim.width = $item.width();
    dim.outerWidth = $item.outerWidth(includeMargin);
    dim.innerWidth = $item.innerWidth();
    dim.height = $item.height();
    dim.innerHeight = $item.innerHeight();
    dim.outerHeight = $item.outerHeight(includeMargin);

    $hiddenParents.each(function (i) {
        var old = oldProps[i];
        for (var name in props) {
            this.style[name] = old[name];
        }
    });

    return dim;
}
}(jQuery));


/**
 * Helper function to obtain the width of the navigation bar elements,
 * regardless of its visibility.
 *
 * Returns the total width of the two menu items plus padding.
 */
function getNavWidth() {
  var maindims   = $('#main-menu ul').getHiddenDimensions();
  var actiondims = $('#action-links ul').getHiddenDimensions();
  var total_width = maindims.outerWidth + actiondims.outerWidth + padding;
  return total_width;
}

$(document).ready(function() {
  var window_width = $(window).width();
  var total_width = getNavWidth();
  if ( ((total_width) > Math.min(window_width, desktop_min)) || window_width <= mobile_bp ) {
    $('#header-wrapper div').addClass('usehamburger');
  } else {
    $('#header-wrapper div').addClass('usenavbar');
  }
  if (total_width > desktop_min) {
    oversize = 1;
    if (window_width > desktop_min) {
      $('#header-wrapper div').addClass('oversizemenubar');
    }
  }
});


$(window).resize(function() {
  var window_width = $(window).width();
  var total_width = getNavWidth();

  if (total_width > desktop_min) {
    oversize = 1;
    if (window_width > desktop_min) {
      $('#header-wrapper div').addClass('oversizemenubar');
    } else {
      $('#header-wrapper div').removeClass('oversizemenubar');
    }
  }

  if ($('#header-wrapper div').hasClass('usenavbar')) {
    if ( total_width > Math.min(window_width, desktop_min) || window_width <= mobile_bp ) {
      $('#header-wrapper div').removeClass('usenavbar');
      $('#header-wrapper div').addClass('usehamburger');
    }
  }
  if ($('#header-wrapper div').hasClass('usehamburger')) {
    if ( (window_width >= desktop_min) && !oversize ) {
      $('#header-wrapper div').removeClass('usehamburger');
      $('#header-wrapper div').addClass('usenavbar');
    }
    // Set CSS of mobile menu to display: none when window is resized.
    document.getElementById("navigation-mmt").style.display = "none";
  }
});
