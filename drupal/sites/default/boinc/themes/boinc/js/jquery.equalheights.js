/**
 * Equal Heights Plugin
 * Equalize the heights of elements. Great for columns or any elements
 * that need to be the same size (floats, etc).
 *
 * Version 1.0
 * Updated 12/10/2008
 *
 * Copyright (c) 2008 Rob Glazebrook (cssnewbie.com)
 *
 * Usage: $(object).equalHeights([minHeight], [maxHeight]);
 *
 * Example 1: $(".cols").equalHeights(); Sets all columns to the same height.
 * Example 2: $(".cols").equalHeights(400); Sets all cols to at least 400px tall.
 * Example 3: $(".cols").equalHeights(100,300); Cols are at least 100 but no more
 * than 300 pixels tall. Elements with too much content will gain a scrollbar.
 *
 */

(function($) {
  $.fn.equalHeights = function(minHeight, maxHeight) {

    tallest = (minHeight) ? minHeight : 0;
    primaryExclude = 0;
    primaryPadding = 0;
    secondaryPadding = 0;
    secondaryHeight = 0;
    secondaryExclude = 0;
    secondaryCount = 0;

    // Look at each element to find the tallest of them
    this.each(function() {
      if ($(this).hasClass('panel-secondary')) {
        // This is designed to line up multiple secondary panels with a single
        // primary panel; see how tall the secondary panels are combined
        secondaryCount++;
        thisHeight = $(this).outerHeight(true);
        // Test that child elements are somehow larger than the parent
        childrenHeight = 0;
        $(this).children().each(function() {
          childrenHeight = childrenHeight + $(this).outerHeight(true);
        });
        if (childrenHeight > thisHeight) {
          thisHeight = childrenHeight;
        }
        // Keep a running total, but we don't want to resize the adjustable
        // secondary panel to be the size of all of them together
        secondaryHeight = secondaryHeight + thisHeight;
        if ($(this).hasClass('no-resize')) {
          secondaryExclude = secondaryExclude + thisHeight;
        }
        else {
          secondaryPadding = parseInt($(this).css('paddingTop')) + parseInt($(this).css('paddingBottom'));
        }
      }
      else {
        // If not a secondary panel, things are simpler
        if ($(this).outerHeight(true) > tallest) {
          tallest = $(this).outerHeight(true);
        }
        if (!$(this).hasClass('panel-primary')) {
          // If this is a child of the primary panel, we need to see how much
          // smaller it is and exclude the difference when resizing later
          primaryExclude = $('.panel-primary').outerHeight() - $(this).outerHeight();
        }
        primaryPadding = parseInt($(this).css('paddingTop')) + parseInt($(this).css('paddingBottom'));
      }
    });
    // Account for any gaps between secondary panels
    panelGap = parseInt($('.panel-region-separator').css('marginBottom'));
    if ((secondaryHeight) && (tallest < secondaryHeight)) {
      tallest = secondaryHeight;
      if (secondaryCount > 1) {
        tallest = tallest + ((secondaryCount-1) * panelGap);
      }
    }
    secondaryExclude = secondaryExclude - (primaryPadding - secondaryPadding);
    if (secondaryCount > 1) {
      secondaryExclude = secondaryExclude + ((secondaryCount-1) * panelGap);
    }
    // If a maxHeight is set, be sure not to exceed it
    if ((maxHeight) && (tallest > maxHeight)) tallest = maxHeight;
    return this.each(function() {
      // Resize each element appropriately
      if (!$(this).hasClass('no-resize')) {
        if ($(this).hasClass('panel-primary')) {
          $(this).height(tallest).css("overflow","auto");
        }
        else if ($(this).hasClass('panel-secondary')) {
          $(this).height(tallest - secondaryExclude).css('overflow','auto');
        }
        else {
          $(this).height(tallest - primaryExclude).css("overflow","auto");
        }
      }
    });
  }
})(jQuery);
