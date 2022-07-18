/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

/**
 * @file Plugin for inserting Drupal teaser and page breaks.
 */
var pluginRequires = [ 'fakeobjects' ];
if (Drupal.ckeditor_ver == 3) {
  pluginRequires = [ 'fakeobjects', 'htmldataprocessor' ];
}

CKEDITOR.plugins.add( 'drupalbreaks',
{
	requires  : pluginRequires,

	init : function( editor )
	{
    var addCssObj = CKEDITOR;

    if (Drupal.ckeditor_ver == 3) {
      addCssObj = editor;
    }
		// Add the styles that renders our fake objects.
		addCssObj.addCss(
			'img.cke_drupal_pagebreak,img.cke_drupal_break' +
			'{' +
				'background-image: url(' + CKEDITOR.getUrl( this.path + 'images/pagebreak.gif' ) + ');' +
				'background-position: center center;' +
				'background-repeat: no-repeat;' +
				'clear: both;' +
				'display: block;' +
				'float: none;' +
				'width: 100%;' +
				'border-top: #999999 1px dotted;' +
				'border-bottom: #999999 1px dotted;' +
				'height: 5px;' +
			'}' +
			'img.cke_drupal_break' +
			'{' +
				'border-top: #FF0000 1px dotted;' +
				'border-bottom: #FF0000 1px dotted;' +
			'}'
		);

		// Register the toolbar buttons.
		if ( Drupal.ckeditorTeaserInfo(editor.name) || Drupal.settings.ckeditor.teaser == editor.name ) {
			editor.ui.addButton( 'DrupalBreak',
				{
					label : Drupal.t('Insert Teaser Break'),
					icon : this.path + 'images/drupalbreak.png',
					command : 'drupalbreak'
				});

                        editor.ui.addButton( 'DrupalPageBreak',
                                {
                                        label : Drupal.t( 'Insert Page Break' ),
                                        icon : this.path + 'images/drupalpagebreak.png',
                                        command : 'drupalpagebreak'
                                });
		}

		// Register the commands.

		editor.addCommand( 'drupalbreak',
		{
			exec : function()
			{
				// There should be only one <!--break--> in document. So, look
				// for an image with class "cke_drupal_break" (the fake element).
				var images = editor.document.getElementsByTag( 'img' );
				for ( var i = 0, len = images.count() ; i < len ; i++ )
				{
					var img = images.getItem( i );
					if ( img.hasClass( 'cke_drupal_break' ) )
					{
						if ( confirm( Drupal.t( 'The document already contains a teaser break. Do you want to proceed by removing it first?' ) ) )
						{
							img.remove();
							break;
						}
						else
							return;
					}
				}

				insertComment( 'break' );
			}
		} );

		editor.addCommand( 'drupalpagebreak',
		{
			exec : function()
			{
				insertComment( 'pagebreak' );
			}
		} );

		// This function effectively inserts the comment into the editor.
		function insertComment( text )
		{
			// Create the fake element that will be inserted into the document.
			// The trick is declaring it as an <hr>, so it will behave like a
			// block element (and in effect it behaves much like an <hr>).
      		if ( !CKEDITOR.dom.comment.prototype.getAttribute ) {
        		CKEDITOR.dom.comment.prototype.getAttribute = function() { return ''; };
        		CKEDITOR.dom.comment.prototype.attributes = { align : '' };
      		}
			var fakeElement = editor.createFakeElement( new CKEDITOR.dom.comment( text ), 'cke_drupal_' + text, 'hr' );

			// This is the trick part. We can't use editor.insertElement()
			// because we need to put the comment directly at <body> level.
			// We need to do range manipulation for that.

			// Get a DOM range from the current selection.
			var range = editor.getSelection().getRanges()[0],
				elementsPath = new CKEDITOR.dom.elementPath( range.getCommonAncestor( true ) ),
				element = ( elementsPath.block && elementsPath.block.getParent() ) || elementsPath.blockLimit,
				hasMoved;

			// If we're not in <body> go moving the position to after the
			// elements until reaching it. This may happen when inside tables,
			// lists, blockquotes, etc.
			while ( element && element.getName() != 'body' )
			{
				range.moveToPosition( element, CKEDITOR.POSITION_AFTER_END );
				hasMoved = 1;
				element = element.getParent();
			}

			// Split the current block.
			if ( !hasMoved )
				range.splitBlock( 'p' );

			// Insert the fake element into the document.
			range.insertNode( fakeElement );

			// Now, we move the selection to the best possible place following
			// our fake element.
			var next = fakeElement;
			while ( ( next = next.getNext() ) && !range.moveToElementEditStart( next ) )
			{}

			range.select();
		}
	},

	afterInit : function( editor )
	{
		// Adds the comment processing rules to the data filter, so comments
		// are replaced by fake elements.
		editor.dataProcessor.dataFilter.addRules(
			{
				comment : function( value )
				{
          			if ( !CKEDITOR.htmlParser.comment.prototype.getAttribute ) {
           				CKEDITOR.htmlParser.comment.prototype.getAttribute = function() { return ''; };
           				CKEDITOR.htmlParser.comment.prototype.attributes = { align : '' };
          			}

					if ( value == 'break' || value == 'pagebreak' )
						return editor.createFakeParserElement( new CKEDITOR.htmlParser.comment( value ), 'cke_drupal_' + value, 'hr' );

					return value;
				}
			});
	}
});
