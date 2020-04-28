/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

CKEDITOR.dialog.add( 'mediaembedDialog', function( editor ) {
  var numberRegex = /^\d+(?:\.\d+)?$/;
  var cssifyLength = function( length )
  {
    if ( numberRegex.test( length ) )
      return length + 'px';
    return length;
  }
  return {
    title : Drupal.t('Embed Media Dialog'),
    minWidth : 400,
    minHeight : 200,
    contents : [
    {
      id : 'mediaTab',
      label : Drupal.t('Embed media code'),
      title : Drupal.t('Embed media code'),
      elements :
      [
      {
        id : 'embed',
        type : 'textarea',
        rows : 9,
        label : Drupal.t('Paste embed code here')
      }
      ]
    }
    ],
    onOk : function() {
      var editor = this.getParentEditor();
      var content = this.getValueOf( 'mediaTab', 'embed' );
      if ( content.length>0 ) {
        var realElement = CKEDITOR.dom.element.createFromHtml('<div class="media_embed"></div>');
        realElement.setHtml(content);
        var fakeElement = editor.createFakeElement( realElement , 'cke_mediaembed', 'div', true);
        var matches = content.match(/width=(["']?)(\d+)\1/i);
        if (matches && matches.length == 3) {
          fakeElement.setStyle('width', cssifyLength(matches[2]));
        }
        matches = content.match(/height=([\"\']?)(\d+)\1/i);
        if (matches && matches.length == 3) {
          fakeElement.setStyle('height', cssifyLength(matches[2]));
        }
        editor.insertElement(fakeElement);
      }
    }
  };
});

