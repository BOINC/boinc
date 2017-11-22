/**
 * @file display_editor.js
 *
 * Contains the javascript for the Panels display editor.
 */

(function ($) {
  /** Delete pane button **/
  Drupal.Panels.bindClickDelete = function(context) {
    $('a.pane-delete:not(.pane-delete-processed)', context)
      .addClass('pane-delete-processed')
      .click(function() {
      if (confirm('Remove this pane?')) {
        var id = '#' + $(this).attr('id').replace('pane-delete-', '');
        $(id).remove();
        Drupal.Panels.Draggable.savePositions();
      }
      return false;
    });
  };

  Drupal.Panels.bindPortlet = function() {
    var handle = $(this).find('.panel-pane-collapsible > div.pane-title');
    var content = $(this).find('.panel-pane-collapsible > div.pane-content');
    if (content.length) {
      var toggle = $('<span class="toggle toggle-collapsed"></span>');
      handle.before(toggle);
      toggle.click(function() {
        content.slideToggle(20);
        toggle.toggleClass('toggle-collapsed');
      });
      handle.click(function() {
        content.slideToggle(20);
        toggle.toggleClass('toggle-collapsed');
      });
      content.hide();
    }
  };

  Drupal.Panels.Draggable = {
    // The draggable object
    object: null,

    // Where objects can be dropped
    dropzones: [],
    current_dropzone: null,

    // positions within dropzones where an object can be plazed
    landing_pads: [],
    current_pad: null,

    // Where the object is
    mouseOffset: { x: 0, y: 0 },
    windowOffset: { x: 0, y: 0 },
    offsetDivHeight: 0,

    // original settings to be restored
    original: {},
    // a placeholder so that if the object is let go but not over a drop zone,
    // it can be put back where it belongs
    placeholder: {},

    hoverclass: 'hoverclass',
    helperclass: 'helperclass',
    accept: 'div.panels-display',
    handle: 'div.grabber',
    draggable: 'div.panel-portlet',
    main: 'div#panels-dnd-main',

    // part of the id to remove to get just the number
    draggableId: 'panel-pane-',
    // What to add to the front of a the id to get the form id for a panel
    formId: 'input#edit-',

    maxWidth: 250,

    unsetDropZone: function() {
      $(this.current_dropzone.obj).removeClass(this.hoverclass);
      this.current_dropzone = null;
      for (var i in this.landing_pads) {
        $(this.landing_pads[i].obj).remove();
      }
      this.landing_pads = [];
      this.current_pad = null;
    },

    createLandingPad: function(where, append) {
      var obj = $('<div class="' + this.helperclass +'" id="' +
        $(where).attr('id') + '-dropzone">&nbsp;</div>');
      if (append) {
        $(where).append(obj);
      }
      else {
        $(where).before(obj);
      }
      var offset = $(obj).offset();

      $(obj).css({
        display: 'none'
      });
      this.landing_pads.push({
        centerX: offset.left + ($(obj).innerWidth() / 2),
        centerY: offset.top + ($(obj).innerHeight() / 2),
        obj: obj
      });
      return obj;
    },

    calculateDropZones: function(event, dropzone) {
      var dropzones = [];
      $(this.accept).each(function() {
        var offset = $(this).offset();
        offset.obj = this;
        offset.width = $(this).outerWidth();
        offset.height = $(this).outerHeight();
        dropzones.push(offset);
      });
      this.dropzones = dropzones;
    },

    reCalculateDropZones: function() {
      for (var i in this.dropzones) {
        offset = $(this.dropzones[i].obj).offset();
        offset.width = $(this.dropzones[i].obj).outerWidth();
        offset.height = $(this.dropzones[i].obj).outerHeight();
        $.extend(this.dropzones[i], offset);
      }
    },

    changeDropZone: function(new_dropzone) {
      // Unset our old dropzone.
      if (this.current_dropzone) {
        this.unsetDropZone();
      }

      // Set up our new dropzone.
      this.current_dropzone = new_dropzone;
      $(this.current_dropzone.obj).addClass(this.hoverclass);
      // add a landing pad
      this.createLandingPad(this.current_dropzone.obj, true);

      var that = this;
      // Create a landing pad before each existing portlet.
      $(this.current_dropzone.obj).find(this.draggable).each(function() {
        if (that.object.id != this.id) {
          that.createLandingPad(this, false);
        }
      });
    },

    findLandingPad: function(x, y) {
      var shortest_distance = null;
      var nearest_pad = null;
      // find the nearest pad.
      for (var i in this.landing_pads) {
        // This isn't the real distance, this is the square of the
        // distance -- no point in spending processing time on
        // sqrt.
        var dstx = Math.abs(x - this.landing_pads[i].centerX);
        var dsty = Math.abs(y - this.landing_pads[i].centerY);
        var distance =  (dstx * dstx) + (dsty * dsty);
        if (shortest_distance == null || distance < shortest_distance) {
          shortest_distance = distance;
          nearest_pad = this.landing_pads[i];
        }
      }
      if (nearest_pad != this.current_pad) {
        if (this.current_pad) {
          $(this.current_pad.obj).hide();
        }
        this.current_pad = nearest_pad;
        $(nearest_pad.obj).show();
      }
    },

    findDropZone: function(x, y) {
      // Go through our dropzones and see if we're over one.
      var new_dropzone = null;
      for (var i in this.dropzones) {
  //      console.log('x:' + x + ' left:' + this.dropzones[i].left + ' right: ' + this.dropzones[i].left + this.dropzones[i].width);
        if (this.dropzones[i].left < x &&
          x < this.dropzones[i].left + this.dropzones[i].width &&
          this.dropzones[i].top < y &&
          y < this.dropzones[i].top + this.dropzones[i].height) {
            new_dropzone = this.dropzones[i];
            break;
        }
      }
      // If we're over one, see if it's different.
      if (new_dropzone) {
        var changed = false;
        if (!this.current_dropzone || new_dropzone.obj.id != this.current_dropzone.obj.id) {
          this.changeDropZone(new_dropzone);
          changed = true;
        }
        this.findLandingPad(x, y);
        if (changed)  {
          // recalculate the size of our drop zones due to the fact that we're drawing landing pads.
          this.reCalculateDropZones();
        }
      }
      // If we're not over one, be sure to unhilite one if we were just
      // over it.
      else if (this.current_dropzone) {
        this.unsetDropZone();
      }
    },

    /** save button clicked, or pane deleted **/
    savePositions: function() {
      var draggable = Drupal.Panels.Draggable;
      $(draggable.accept).each(function() {
        var val = '';
        $(this).find(draggable.draggable).each(function() {
          if (val) {
            val += ',';
          }

          val += this.id.replace(draggable.draggableId, '');
        });
        // Note: _ is replaced with - because Drupal automatically does this
        // with form ids.
        $(draggable.formId + this.id.replace(/_/g, '-')).val(val);
      });
      return false;
    }
  };

  Drupal.Panels.DraggableHandler = function() {
    $(this).addClass('panel-draggable');
    var draggable = Drupal.Panels.Draggable;
    var scrollBuffer = 10;
    var scrollDistance = 10;
    var scrollTimer = 30;

    getMouseOffset = function(docPos, mousePos, windowPos) {
      return { x: mousePos.x - docPos.x + windowPos.x, y: mousePos.y - docPos.y + windowPos.y};
    };

    getMousePos = function(ev) {
      ev = ev || window.event;

      if (ev.pageX || ev.pageY) {
        return { x:ev.pageX, y:ev.pageY };
      }
      return {
        x:ev.clientX + document.body.scrollLeft - document.body.clientLeft,
        y:ev.clientY + document.body.scrollTop  - document.body.clientTop
      };
    };

    getPosition = function(e) {
      /*
      if (document.defaultView && document.defaultView.getComputedStyle) {
        var css = document.defaultView.getComputedStyle(e, null);
        return {
          x: parseInt(css.getPropertyValue('left')),
          y: parseInt(css.getPropertyValue('top'))
        };
      }
      */
      var left = 0;
      var top  = 0;

      while (e.offsetParent) {
        left += e.offsetLeft;
        top  += e.offsetTop;
        e     = e.offsetParent;
      }

      left += e.offsetLeft;
      top  += e.offsetTop;

      return { x:left, y:top };
    };

    mouseUp = function(e) {
      clearTimeout(draggable.timeoutId);
      draggable.dropzones = [];

      if (draggable.current_pad) {
        // Drop the object where we're hovering
        $(draggable.object).insertAfter($(draggable.current_pad.obj));
        Drupal.Panels.changed($(draggable.object));
      }
      else {
        // or put it back where it came from
        $(draggable.object).insertAfter(draggable.placeholder);
      }
      // remove the placeholder
      draggable.placeholder.remove();

      // restore original settings.
      $(draggable.object).css(draggable.original);
      if (draggable.current_dropzone) {
        draggable.unsetDropZone();
      }

      $(document).unbind('mouseup').unbind('mousemove');
      draggable.savePositions();
    };

    mouseMove = function(e) {
      draggable.mousePos = getMousePos(e);

      draggable.findDropZone(draggable.mousePos.x, draggable.mousePos.y);

      var windowMoved = parseInt(draggable.offsetDivHeight - $(draggable.main).innerHeight());

      draggable.object.style.top = draggable.mousePos.y - draggable.mouseOffset.y + windowMoved + 'px';
      draggable.object.style.left = draggable.mousePos.x - draggable.mouseOffset.x  + 'px';
      $(draggable.object).toggleClass('moving');
    };

    mouseDown = function(e) {
      // If we mouse-downed over something clickable, don't drag!
      if (e.target.nodeName == 'A' || e.target.nodeName == 'INPUT' || e.target.parentNode.nodeName == 'A' || e.target.nodeName.nodeName == 'INPUT') {
        return;
      }

      draggable.object = $(this).parent(draggable.draggable).get(0);

      // create a placeholder so we can put this object back if dropped in an invalid location.
      draggable.placeholder = $('<div class="draggable-placeholder-object" style="display:none"></div>"');
      $(draggable.object).after(draggable.placeholder);

      // Store original CSS so we can put it back.
      draggable.original = {
        position: $(draggable.object).css('position'),
        width: 'auto',
        left: $(draggable.object).css('left'),
        top: $(draggable.object).css('top'),
        'z-index': $(draggable.object).css('z-index'),
        'margin-bottom': $(draggable.object).css('margin-bottom'),
        'margin-top': $(draggable.object).css('margin-top'),
        'margin-left': $(draggable.object).css('margin-left'),
        'margin-right': $(draggable.object).css('margin-right'),
        'padding-bottom': $(draggable.object).css('padding-bottom'),
        'padding-top': $(draggable.object).css('padding-top'),
        'padding-left': $(draggable.object).css('padding-left'),
        'padding-right': $(draggable.object).css('padding-right')
      };

      draggable.mousePos = getMousePos(e);
      var originalPos = $(draggable.object).offset();
      var width = Math.min($(draggable.object).innerWidth(), draggable.maxWidth);

      draggable.offsetDivHeight = $(draggable.main).innerHeight();
      draggable.findDropZone(draggable.mousePos.x, draggable.mousePos.y);

      // Make copies of these because in FF3, they actually change when we
      // move the item, whereas they did not in FF2.

      if (e.layerX || e.layerY) {
        var layerX = e.layerX;
        var layerY = e.layerY;
      }
      else if (e.originalEvent && e.originalEvent.layerX) {
        var layerX = e.originalEvent.layerX;
        var layerY = e.originalEvent.layerY;
      }

      // Make the draggable relative, get it out of the way and make it
      // invisible.
      $(draggable.object).css({
        position: 'relative',
        'z-index': 100,
        width: width + 'px',
        'margin-bottom': (-1 * parseInt($(draggable.object).outerHeight())) + 'px',
        'margin-top': 0,
        'margin-left': 0,
        'margin-right': (-1 * parseInt($(draggable.object).outerWidth())) + 'px',
        'padding-bottom': 0,
        'padding-top': 0,
        'padding-left': 0,
        'padding-right': 0,
        'left': 0,
        'top': 0
      })
        .insertAfter($(draggable.main));
      var newPos = $(draggable.object).offset();

      var windowOffset = { left: originalPos.left - newPos.left, top: originalPos.top - newPos.top }

      // if they grabbed outside the area where we make the draggable smaller, move it
      // closer to the cursor.
      if (layerX != 'undefined' && layerX > width) {
        windowOffset.left += layerX - 10;
      }
      else if (layerX != 'undefined' && e.offsetX > width) {
        windowOffset.left += e.offsetX - 10;
      }

      // This is stored so we can move with it.
      draggable.mouseOffset = { x: draggable.mousePos.x - windowOffset.left, y: draggable.mousePos.y - windowOffset.top};
      draggable.offsetDivHeight = $(draggable.main).innerHeight();

      draggable.object.style.top = windowOffset.top + 'px';
      draggable.object.style.left = windowOffset.left + 'px';
      $(document).unbind('mouseup').unbind('mousemove').mouseup(mouseUp).mousemove(mouseMove);

      draggable.calculateDropZones(draggable.mousePos, e);
      draggable.timeoutId = setTimeout('timer()', scrollTimer);
      return false;
    };

    timer = function() {
      if (!draggable.timeCount) {
        draggable.timeCount = 0;
      }
      draggable.timeCount = draggable.timeCount + 1;
      var left = $(window).scrollLeft();
      var right = left + $(window).width();
      var top = $(window).scrollTop();
      var bottom = top + $(window).height();

      if (draggable.mousePos.x < left + scrollBuffer && left > 0) {
        window.scrollTo(left - scrollDistance, top);
        draggable.mousePos.x -= scrollDistance;
        draggable.object.style.top = draggable.mousePos.y - draggable.mouseOffset.y + 'px';
      }
      else if (draggable.mousePos.x > right - scrollBuffer) {
        window.scrollTo(left + scrollDistance, top);
        draggable.mousePos.x += scrollDistance;
        draggable.object.style.top = draggable.mousePos.y - draggable.mouseOffset.y + 'px';
      }
      else if (draggable.mousePos.y < top + scrollBuffer && top > 0) {
        window.scrollTo(left, top - scrollDistance);
        draggable.mousePos.y -= scrollDistance;
        draggable.object.style.top = draggable.mousePos.y - draggable.mouseOffset.y + 'px';
      }
      else if (draggable.mousePos.y > bottom - scrollBuffer) {
        window.scrollTo(left, top + scrollDistance);
        draggable.mousePos.y += scrollDistance;
        draggable.object.style.top = draggable.mousePos.y - draggable.mouseOffset.y + 'px';
      }

      draggable.timeoutId = setTimeout('timer()', scrollTimer);
    }

    $(this).mousedown(mouseDown);
  };

  $.fn.extend({
    panelsDraggable: Drupal.Panels.DraggableHandler
  });

  /**
   * Implement Drupal behavior for autoattach
   */
  Drupal.behaviors.PanelsDisplayEditor = function(context) {
    // Show javascript only items.
    $('span#panels-js-only').css('display', 'inline');

    $('#panels-dnd-main div.panel-pane:not(.panel-portlet)')
      .addClass('panel-portlet')
      .each(Drupal.Panels.bindPortlet);

    // The above doesn't work if context IS the pane, so do this to catch that.
    if ($(context).hasClass('panel-pane') && !$(context).hasClass('panel-portlet')) {
      $(context)
        .addClass('panel-portlet')
        .each(Drupal.Panels.bindPortlet);
    }

    // Make draggables and make sure their positions are saved.
    $(context).find('div.grabber:not(.panel-draggable)').panelsDraggable();
    Drupal.Panels.Draggable.savePositions();

    // Bind buttons.
    $('input#panels-hide-all', context).click(Drupal.Panels.clickHideAll);
    $('input#panels-show-all', context).click(Drupal.Panels.clickShowAll);

    Drupal.Panels.bindClickDelete(context);

    $('#panels-live-preview-button:not(.panels-preview-processed)')
      .addClass('panels-preview-processed')
      .click(function () {
        if (!$('#panels-preview').size()) {
          $('#panels-dnd-main').parents('form').after('<div id="panels-preview"></div>');
        }

        $('#panels-preview').html(Drupal.theme('CToolsModalThrobber'));
      });

    var setTitleClass = function () {
      if ($('#edit-display-title-hide-title').val() == 2) {
        $('#panels-dnd-main').removeClass('panels-set-title-hide');
      }
      else {
        $('#panels-dnd-main').addClass('panels-set-title-hide');
      }
    }

    // The panes have an option to set the display title, but only if
    // a select is set to the proper value. This sets a class on the
    // main edit div so that the option to set the display title
    // is hidden if that is not selected, and visible if it is.
    $('#edit-display-title-hide-title:not(.panels-title-processed)')
      .addClass('panels-title-processed')
      .change(setTitleClass);

    setTitleClass();
  };

  /**
   * AJAX responder command to render the preview.
   */
  Drupal.CTools.AJAX.commands.panel_preview = function(command) {
    $('#panels-preview').html(command.output);
  }

})(jQuery);
