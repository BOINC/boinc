/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

/**
 * @file Plugin for inserting Drupal embeded media
 */
( function() {
  var numberRegex = /^\d+(?:\.\d+)?$/;
  var cssifyLength = function( length )
  {
    if ( numberRegex.test( length ) )
      return length + 'px';
    return length;
  }
  CKEDITOR.plugins.add( 'mediaembed',
  {
    requires : [ 'dialog', 'fakeobjects' ],
    init: function( editor )
    {
      var addCssObj = CKEDITOR;

      if (Drupal.ckeditor_ver == 3) {
        addCssObj = editor;
      }
      addCssObj.addCss(
        'img.cke_mediaembed' +
        '{' +
        'background-image: url(' + CKEDITOR.getUrl( this.path + 'images/placeholder.gif' ) + ');' +
        'background-position: center center;' +
        'background-repeat: no-repeat;' +
        'border: 1px solid #a9a9a9;' +
        'width: 80px;' +
        'height: 80px;' +
        '}'
        );

      editor.addCommand( 'mediaembedDialog', new CKEDITOR.dialogCommand( 'mediaembedDialog', { allowedContent : 'div(media_embed);iframe[*](*)' } ) );
      editor.ui.addButton( 'MediaEmbed',
      {
        label: 'Embed Media',
        command: 'mediaembedDialog',
        icon: this.path + 'images/icon.png'
      } );
      CKEDITOR.dialog.add( 'mediaembedDialog', this.path + 'dialogs/mediaembed.js' );
    },
    afterInit : function( editor )
    {
      var dataProcessor = editor.dataProcessor,
      dataFilter = dataProcessor && dataProcessor.dataFilter,
      htmlFilter = dataProcessor && dataProcessor.htmlFilter;

      if ( htmlFilter )
      {
        htmlFilter.addRules({
          elements :
          {
            'div' : function ( element ) {
              if( element.attributes['class'] == 'media_embed' ) {
                for (var x in element.children) {
                  if (typeof(element.children[x].attributes) != 'undefined') {
                    if (typeof(element.children[x].attributes.width) != undefined) {
                      element.children[x].attributes.width = element.attributes.width;
                    }
                    if (typeof(element.children[x].attributes.height) != undefined) {
                      element.children[x].attributes.height = element.attributes.height;
                    }
                  }
                }
              }
            }
          }
        });
      }
      if ( dataFilter )
      {
        dataFilter.addRules(
        {
          elements :
          {
            'div' : function( element )
            {
              var attributes = element.attributes,
              classId = attributes.classid && String( attributes.classid ).toLowerCase();

              if (element.attributes[ 'class' ] == 'media_embed') {
                var fakeElement = editor.createFakeParserElement(element, 'cke_mediaembed', 'div', true);
                var fakeStyle = fakeElement.attributes.style || '';
                if (element.children[0] && typeof(element.children[0].attributes) != 'undefined') {
                  var height = element.children[0].attributes.height,
                  width = element.children[0].attributes.width;
                }
                if ( typeof width != 'undefined' )
                  fakeStyle = fakeElement.attributes.style = fakeStyle + 'width:' + cssifyLength( width ) + ';';

                if ( typeof height != 'undefined' )
                  fakeStyle = fakeElement.attributes.style = fakeStyle + 'height:' + cssifyLength( height ) + ';';

                return fakeElement;
              }
              return element;
            }
          }
        },
        5);
      }
    }
  } );
} )();
