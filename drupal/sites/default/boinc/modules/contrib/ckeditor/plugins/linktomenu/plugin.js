/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/
CKEDITOR.plugins.add( 'linktomenu',
{
	init : function( editor )
	{
		// Add the link and unlink buttons.
		editor.addCommand( 'linktomenu', new CKEDITOR.dialogCommand( 'linktomenu' ) );
		editor.ui.addButton( 'LinkToMenu',
			{
				label : 'Link to menu',
				icon : this.path + 'images/linktomenu.gif',
				command : 'linktomenu'
			} );
		CKEDITOR.dialog.add( 'linktomenu', this.path + 'dialogs/link.js' );

		// If the "menu" plugin is loaded, register the menu items.
		if ( editor.addMenuItems )
		{
			editor.addMenuItems(
				{
					linktomenu :
					{
						label : 'Link to menu',
						command : 'linktomenu',
						group : 'linktomenu',
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
