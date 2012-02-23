
Drupal.flexible = Drupal.flexible || {};

Drupal.flexible.splitters = [];

/**
 * Fix the height of all splitters to be the same as the items they are
 * splitting.
 */
Drupal.flexible.fixHeight = function() {
  for (i in Drupal.flexible.splitters) {
    Drupal.flexible.splitters[i].fixHeight();
  }
}

Drupal.behaviors.flexibleAdmin = function(context) {
  // Show/hide layout manager button
  $('input#panels-flexible-toggle-layout:not(.panels-flexible-processed)', context)
    .addClass('panels-flexible-processed')
    .click(function() {
      $('.panel-flexible-admin')
        .toggleClass('panel-flexible-no-edit-layout')
        .toggleClass('panel-flexible-edit-layout');

      if ($('.panel-flexible-admin').hasClass('panel-flexible-edit-layout')) {
        $(this).val(Drupal.t('Hide layout designer'));
        Drupal.flexible.fixHeight();
      }
      else {
        $(this).val(Drupal.t('Show layout designer'));
      }
      return false;
    });

  // Window splitter behavior.
  $('div.panels-flexible-splitter:not(.panels-splitter-processed)', context)
    .addClass('panels-splitter-processed')
    .each(function() {
      Drupal.flexible.splitters.push(new Drupal.flexible.splitter($(this)));
    });

  // Sometimes the splitter IS the context and the above syntax won't
  // catch that.
  if ($(context).hasClass('panels-flexible-splitter')) {
    $(context)
      .addClass('panels-splitter-processed')
      .each(function() {
        Drupal.flexible.splitters.push(new Drupal.flexible.splitter($(this)));
      });
  }

  Drupal.flexible.fixHeight();
};

Drupal.flexible.splitter = function($splitter) {
  var splitter = this;

  this.fixHeight = function() {
    // Set the splitter height to the shorter of the two:
    $splitter.height(Math.max(this.left.outerHeight(), this.right.outerHeight()));
  }

  function splitterStart(event) {
    // Show splitting classes.
//    splitter.left.addClass('flexible-splitting');	// Safari selects A/B text on a move
//    splitter.right.addClass('flexible-splitting');	// Safari selects A/B text on a move
//    splitter.splitter.addClass('flexible-splitter-splitting');

    // Bind motion events.
    $(document)
      .bind("mousemove", splitterMove)
      .bind("mouseup", splitterEnd);

    // Calculate some data about our split regions:
    splitter.getSizes();

    // The X coordinate where we clicked.
    splitter.startX = event.pageX;

    // The current sizes of the left/right panes.
    splitter.currentLeft = parseFloat(splitter.left_width) * parseFloat(splitter.left_scale);
    splitter.currentRight = parseFloat(splitter.right_width) * parseFloat(splitter.right_scale);

    // The starting sizes of the left right panes.
    splitter.startLeft = splitter.currentLeft;
    splitter.startRight = splitter.currentRight;

    if (splitter.left_width_type == splitter.right_width_type) {
      // If they're the same type, add the two together so we know how
      // much space we have for splitting.
      splitter.max = splitter.startLeft + splitter.startRight;

      // calculate unit size and min/max width.
      if (splitter.left_width_type == '%') {
        splitter.left_total = splitter.left.width() / (splitter.left_width / 100);
        // One pixel is equivalent to what percentage of the total?
        splitter.left_unit = (1 / splitter.left_total) * 100;
        splitter.left_min = 5; // minimum % we'll use.
      }
      else {
        splitter.left_unit = 1;
        splitter.left_min = 25; // minimum pixels we'll use.
      }
      if (splitter.right_width_type == '%') {
        splitter.right_total = splitter.right.width() / (splitter.right_width / 100);
        // One pixel is equivalent to what percentage of the total?
        splitter.right_unit = (1 / splitter.right_total) * 100;
        splitter.right_min = 5; // minimum % we'll use.
      }
      else {
        splitter.right_unit = 1;
        splitter.right_min = 25; // minimum pixels we'll use.
      }
    }
    else {
      // Figure out the parent blob's width and set the max to that
      splitter.parent = $splitter.parent().parent();

      if (splitter.left_width_type != 'px') {
        // Only the 'px' side can resize.
        splitter.left_unit = 0;
        splitter.right_unit = 1;
        splitter.right_min = 25;
        splitter.right_padding = parseInt(splitter.parent.css('padding-right'));
        splitter.right_parent = parseInt(splitter.right.parent().css('margin-right'));
        splitter.max = splitter.right.width() + splitter.left.parent().width() -
          (splitter.left.siblings(':not(.panels-flexible-splitter)').length * 25) - 25;
      }
      else {
        splitter.right_unit = 0;
        splitter.left_unit = 1;
        splitter.left_min = 25;
        splitter.left_padding = parseInt(splitter.parent.css('padding-left'));
        splitter.left_parent = parseInt(splitter.left.parent().css('margin-left'));
        if (splitter.right_id) {
          splitter.max = splitter.left.width() + splitter.right.parent().width() -
            (splitter.right.siblings(':not(.panels-flexible-splitter)').length * 25) - 25;
        }
        else {
          var subtract = 0;
          splitter.left.siblings(':not(.panels-flexible-splitter)').each(function() { subtract += $(this).width()});
          splitter.max = splitter.left.parent().width() - subtract;
        }
      }
    }

    var offset = $(splitter.splitter).offset();

    // Create boxes to display widths left and right of the mouse pointer.
    // Create left box only if left box is mobile.
    if (splitter.left_unit) {
      splitter.left_box = $('<div class="flexible-splitter-hover-box">&nbsp;</div>');
      $('body').append(splitter.left_box);
      splitter.left_box.css('top', offset.top);
      splitter.left_box.css('left', event.pageX - 65);

    if (splitter.left_width_type == '%') {
        var left = splitter.currentLeft / splitter.left_scale;
        splitter.left_box.html(left.toFixed(2) + splitter.left_width_type);
      }
      else {
        // make sure pixel values are always whole integers.
        splitter.currentLeft = parseInt(splitter.currentLeft);
        splitter.left_box.html(splitter.currentLeft + splitter.left_width_type);
      }
    }

    // Create the right box if the right side is mobile.
    if (splitter.right_unit) {
      splitter.right_box = $('<div class="flexible-splitter-hover-box"></div>');
      $('body').append(splitter.right_box);
      splitter.right_box.css('top', offset.top);
      splitter.right_box.css('left', event.pageX + 5);
      if (splitter.right_width_type == '%') {
        var right = splitter.currentRight / splitter.right_scale;
        splitter.right_box.html(right.toFixed(2) + splitter.right_width_type);
      }
      else {
        // make sure pixel values are always whole integers.
        splitter.currentRight = parseInt(splitter.currentRight);
        splitter.right_box.html(splitter.currentRight + splitter.right_width_type);
      }
    }

    return false;
  };

  function splitterMove(event) {
    var diff = splitter.startX - event.pageX;
    var moved = 0;
    // Bah, javascript has no logical xor operator
    if ((splitter.left_unit && !splitter.right_unit) ||
      (!splitter.left_unit && splitter.right_unit)) {
      // This happens when one side is fixed and the other side is fluid. The
      // fixed side actually adjusts while the fluid side does not. However,
      // in order to move the fluid side we have to adjust the padding
      // on our parent object.
      if (splitter.left_unit) {
        // Only the left box is allowed to move.
        splitter.currentLeft = splitter.startLeft - diff;

        if (splitter.currentLeft < splitter.left_min) {
          splitter.currentLeft = splitter.left_min;
        }
        if (splitter.currentLeft > splitter.max) {
          splitter.currentLeft = splitter.max;
        }

        // If the shift key is pressed, go with 1% or 10px boundaries.
        if (event.shiftKey) {
          splitter.currentLeft = parseInt(splitter.currentLeft / 10) * 10;
        }
        moved = (splitter.startLeft - splitter.currentLeft);
      }
      else {
        // Only the left box is allowed to move.
        splitter.currentRight = splitter.startRight + diff;

        if (splitter.currentRight < splitter.right_min) {
          splitter.currentRight = splitter.right_min;
        }
        if (splitter.currentRight > splitter.max) {
          splitter.currentRight = splitter.max;
        }

        // If the shift key is pressed, go with 1% or 10px boundaries.
        if (event.shiftKey) {
          splitter.currentRight = parseInt(splitter.currentRight / 10) * 10;
        }
        moved = (splitter.currentRight - splitter.startRight);
      }
    }
    else {
      // If they are both the same type, do this..
      // Adjust the left side by the amount we moved.
      var left = -1 * diff * splitter.left_unit;

      splitter.currentLeft = splitter.startLeft + left;

      if (splitter.currentLeft < splitter.left_min) {
        splitter.currentLeft = splitter.left_min;
      }
      if (splitter.currentLeft > splitter.max - splitter.right_min) {
        splitter.currentLeft = splitter.max - splitter.right_min;
      }

      // If the shift key is pressed, go with 1% or 10px boundaries.
      if (event.shiftKey) {
        if (splitter.left_width_type == '%') {
          splitter.currentLeft = parseInt(splitter.currentLeft / splitter.left_scale) * splitter.left_scale;
        }
        else {
          splitter.currentLeft = parseInt(splitter.currentLeft / 10) * 10;
        }
      }

      // Now automatically make the right side to be the other half.
      splitter.currentRight = splitter.max - splitter.currentLeft;

      // recalculate how far we've moved into pixels so we can adjust our visible
      // boxes.
      moved = (splitter.startLeft - splitter.currentLeft) / splitter.left_unit;
    }

    if (splitter.left_unit) {
      splitter.left_box.css('left', splitter.startX - 65 - moved);
      if (splitter.left_width_type == '%') {
        var left = splitter.currentLeft / splitter.left_scale;
        splitter.left_box.html(left.toFixed(2) + splitter.left_width_type);
      }
      else {
        splitter.left_box.html(parseInt(splitter.currentLeft) + splitter.left_width_type);
      }

      // Finally actually move the left side
      splitter.left.css('width', splitter.currentLeft + splitter.left_width_type);
    }
    else {
      // if not moving the left side, adjust the parent padding instead.
      splitter.parent.css('padding-right', (splitter.right_padding + moved) + 'px');
      splitter.right.parent().css('margin-right', (splitter.right_parent - moved) + 'px');
    }

    if (splitter.right_unit) {
      splitter.right_box.css('left', splitter.startX + 5 - moved);
      if (splitter.right_width_type == '%') {
        var right = splitter.currentRight / splitter.right_scale;
        splitter.right_box.html(right.toFixed(2) + splitter.right_width_type);
      }
      else {
        splitter.right_box.html(parseInt(splitter.currentRight) + splitter.right_width_type);
      }

      // Finally actually move the right side
      splitter.right.css('width', splitter.currentRight + splitter.right_width_type);
    }
    else {
      // if not moving the right side, adjust the parent padding instead.
      splitter.parent.css('padding-left', (splitter.left_padding - moved) + 'px');
      splitter.left.parent().css('margin-left', (splitter.left_parent + moved) + 'px');
      if (jQuery.browser.msie) {
        splitter.left.parent().css('left', splitter.currentLeft);
      }
    }
    return false;
  };

  function splitterEnd(event) {
    if (splitter.left_unit) {
      splitter.left_box.remove();
    }

    if (splitter.right_unit) {
      splitter.right_box.remove();
    }

    splitter.left.removeClass("flexible-splitting");	// Safari selects A/B text on a move
    splitter.right.removeClass("flexible-splitting");	// Safari selects A/B text on a move
    splitter.splitter.removeClass("flexible-splitter-splitting");	// Safari selects A/B text on a move
    splitter.left.css("-webkit-user-select", "text");	// let Safari select text again
    splitter.right.css("-webkit-user-select", "text");	// let Safari select text again

    if (splitter.left_unit) {
      splitter.left_width = splitter.currentLeft / parseFloat(splitter.left_scale);
    }

    if (splitter.right_unit) {
      splitter.right_width = splitter.currentRight / parseFloat(splitter.right_scale);
    }

    splitter.putSizes();
    Drupal.flexible.fixHeight();

    $(document)
      .unbind("mousemove", splitterMove)
      .unbind("mouseup", splitterEnd);

    // Store the data on the server.
    $.ajax({
      type: "POST",
      url: Drupal.settings.flexible.resize,
      data: {
        'left': splitter.left_id,
        'left_width': splitter.left_width,
        'right': splitter.right_id,
        'right_width': splitter.right_width
      },
      global: true,
      success: Drupal.CTools.AJAX.respond,
      error: function() {
        alert("An error occurred while attempting to process " + Drupal.settings.flexible.resize);
      },
      dataType: 'json'
    });
  };

  this.getSizes = function() {
    splitter.left_width = $splitter.children('.panels-flexible-splitter-left-width').html();
    splitter.left_scale = $splitter.children('.panels-flexible-splitter-left-scale').html();
    splitter.left_width_type = $splitter.children('.panels-flexible-splitter-left-width-type').html();
    splitter.right_width = $splitter.children('.panels-flexible-splitter-right-width').html();
    splitter.right_scale = $splitter.children('.panels-flexible-splitter-right-scale').html();
    splitter.right_width_type = $splitter.children('.panels-flexible-splitter-right-width-type').html();
  };

  this.putSizes = function() {
    $(splitter.left_class + '-width').html(splitter.left_width);
    if (splitter.left_class != splitter.right_class) {
      $(splitter.right_class + '-width').html(splitter.right_width);
    }
  }

  splitter.splitter = $splitter;
  splitter.left_class = $splitter.children('.panels-flexible-splitter-left').html();
  splitter.left_id = $splitter.children('.panels-flexible-splitter-left-id').html();
  splitter.left = $(splitter.left_class);
  splitter.right_class = $splitter.children('.panels-flexible-splitter-right').html();
  splitter.right_id = $splitter.children('.panels-flexible-splitter-right-id').html();
  splitter.right = $(splitter.right_class);

  $splitter
    .bind("mousedown", splitterStart);

};

/**
 * Provide an AJAX response command to allow the server to request
 * height fixing.
 */
Drupal.CTools.AJAX.commands.flexible_fix_height = function() {
  Drupal.flexible.fixHeight();
};

/**
 * Provide an AJAX response command to fix the first/last bits of a
 * group.
 */
Drupal.CTools.AJAX.commands.flexible_fix_firstlast = function(data) {
  $(data.selector + ' > div > .' + data.base)
    .removeClass(data.base + '-first')
    .removeClass(data.base + '-last');

  $(data.selector + ' > div > .' + data.base + ':first')
    .addClass(data.base + '-first');
  $(data.selector + ' > div > .' + data.base + ':last')
    .addClass(data.base + '-last');
};

