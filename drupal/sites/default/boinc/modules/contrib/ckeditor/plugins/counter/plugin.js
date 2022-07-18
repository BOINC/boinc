/**
 * @file Plugin to count symbols, symbols without blanks and words
 */
( function(){
  var emptyHtml = '<span class="cke_empty">&nbsp;</span>';
  CKEDITOR.plugins.add( 'counter',
  {
    init : function( editor )
    {
      var spaceId = 'cke_counter_' + editor.name;
      var spaceElement;
      var ckeditorEventThemeSpace = 'uiSpace';
      var getSpaceElement = function()
      {
        if ( !spaceElement )
          spaceElement = CKEDITOR.document.getById( spaceId );
        return spaceElement;
      };

      if (Drupal.ckeditor_ver == 3) {
        ckeditorEventThemeSpace = 'themeSpace';
      }

      editor.on( ckeditorEventThemeSpace, function( event )
      {
        if ( event.data.space == 'bottom' )
        {
          event.data.html +=
          '<span id="' + spaceId + '_label" class="cke_voice_label">Counter</span>' +
          '<div id="' + spaceId + '" class="cke_counter" role="group" aria-labelledby="' + spaceId + '_label">' + emptyHtml + '</div>';
        }
      });

      function count( ev )
      {
        var space = getSpaceElement();
        var text = ev.editor.getData();
        // decode HTML entities; it also removes HTML tags, but works only if jQuery is available
        text = jQuery('<div/>').html(text).text();
        // remove all redundant blank symbols
        text = text.replace(new RegExp('\\s+', 'g'), ' ');
        // remove all blank symbols at the start and at the end
        text = text.replace(new RegExp('(^\\s+)|(\\s+$)', 'g'), '');
        var symbols = text.length;
        var words = text.split(' ').length;
        //remove all blank symbols
        text = text.replace(new RegExp('\\s+', 'g'), '');
        var symbols_wo_blanks = text.length;

        space.setHtml( '<span class="cke_counter" style="float: right">' + symbols + ' / ' + symbols_wo_blanks + ' symbols; ' + words + ' words</span>' );
      }

      editor.on( 'dataReady', count );
      editor.on( 'blur', count );
      editor.on( 'focus', count );
    // Almost useless
    //editor.on( 'saveSnapshot', count );
    // Requires too much resources
    //editor.on( 'key', count );
    }
  });
})();
