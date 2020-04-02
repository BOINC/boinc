/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/
CKEDITOR.plugins.add( 'linktonode',
{
	init : function( editor )
	{
		// Add the link and unlink buttons.
		editor.addCommand( 'linktonode', new CKEDITOR.dialogCommand( 'linktonode' ) );
		editor.ui.addButton( 'LinkToNode',
			{
				label : 'Link to node',
				icon : this.path + 'images/linktonode.gif',
				command : 'linktonode'
			} );
		CKEDITOR.dialog.add( 'linktonode', this.path + 'dialogs/link.js' );

		// If the "menu" plugin is loaded, register the menu items.
		if ( editor.addMenuItems )
		{
			editor.addMenuItems(
				{
					linktomenu :
					{
						label : 'Link to node',
						command : 'linktonode',
						group : 'linktonode',
						order : 1
					}
				});
		}

		// If the "contextmenu" plugin is loaded, register the listeners.
		if ( editor.contextMenu )
		{
			editor.contextMenu.addListener( function( element, selection )
				{
					if ( !element )
						return null;

					var isAnchor = ( element.is( 'img' ) && element.getAttribute( '_cke_real_element_type' ) == 'anchor' );

					if ( !isAnchor )
					{
						if ( !( element = element.getAscendant( 'a', true ) ) )
							return null;

						isAnchor = ( element.getAttribute( 'name' ) && !element.getAttribute( 'href' ) );
					}

					return { linktomenu : CKEDITOR.TRISTATE_OFF };
				});
		}
	}
} );
