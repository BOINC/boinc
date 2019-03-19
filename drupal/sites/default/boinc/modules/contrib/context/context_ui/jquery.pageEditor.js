
/**
 * Generic pageEditor plugin. Allows an editor DOM object to trigger
 * init, start, and end events. Implementors can check whether the
 * editor is currently editing and bind handlers for the events triggered
 * by the editor.
 */
(function($) {
  $.fn.pageEditor = function(method, data) {
    this.each(function() {
      switch (method) {
        case 'isEditing':
          return this.editing;
        case 'start':
          if (!this.inited) {
            this.inited = true;
            $(this).trigger('init.pageEditor', data);
          }
          this.editing = true;
          $(this).trigger('start.pageEditor', data);
          break;
        case 'end':
          if (!this.inited) {
            this.inited = true;
            $(this).trigger('init.pageEditor', data);
          }
          this.editing = false;
          $(this).trigger('end.pageEditor', data);
          break;
        default:
          this.inited = false;
          this.editing = false;
          break;
      }
    });
    return this;
  };
})(jQuery);
