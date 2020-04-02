/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

CKEDITOR.dialog.add( 'linktonode', function( editor )
{
	//<!-- linktonode START -->
	CKEDITOR.scriptLoader.load( Drupal.settings.basePath + "misc/jquery.js");
	CKEDITOR.scriptLoader.load( Drupal.settings.ckeditor.module_path + "/plugins/linktonode/jscripts/functions.js", function() {
		//window.focus();
		loadCategories(null);
	});
	//<!-- linktonode END -->

	// Handles the event when the "Target" selection box is changed.
	var targetChanged = function()
	{
		var dialog = this.getDialog(),
			popupFeatures = dialog.getContentElement( 'target', 'popupFeatures' ),
			targetName = dialog.getContentElement( 'target', 'linkTargetName' ),
			value = this.getValue();

		if ( !popupFeatures || !targetName )
			return;

		popupFeatures = popupFeatures.getElement();

		if ( value == 'popup' )
		{
			popupFeatures.show();
			targetName.setLabel( editor.lang.link.targetPopupName );
		}
		else
		{
			popupFeatures.hide();
			targetName.setLabel( editor.lang.link.targetFrameName );
			this.getDialog().setValueOf( 'target', 'linkTargetName', value.charAt( 0 ) == '_' ? value : '' );
		}
	};

	// Loads the parameters in a selected link to the link dialog fields.
	var emailRegex = /^mailto:([^?]+)(?:\?(.+))?$/,
		emailSubjectRegex = /subject=([^;?:@&=$,\/]*)/,
		emailBodyRegex = /body=([^;?:@&=$,\/]*)/,
		anchorRegex = /^#(.*)$/,
		urlRegex = /^((?:http|https|ftp|news):\/\/)?(.*)$/,
		selectableTargets = /^(_(?:self|top|parent|blank))$/;

	var popupRegex =
		/\s*window.open\(\s*this\.href\s*,\s*(?:'([^']*)'|null)\s*,\s*'([^']*)'\s*\)\s*;\s*return\s*false;*\s*/;
	var popupFeaturesRegex = /(?:^|,)([^=]+)=(\d+|yes|no)/gi;

	var parseLink = function( editor, element )
	{
		var href = element ? ( element.getAttribute( '_cke_saved_href' ) || element.getAttribute( 'href' ) ) : '',
			emailMatch = '',
			anchorMatch = '',
			urlMatch = false,
			retval = {};

		if ( href )
		{
			emailMatch = href.match( emailRegex );
			anchorMatch = href.match( anchorRegex );
			urlMatch = href.match( urlRegex );
		}

		// Load the link type and URL.
		if ( emailMatch )
		{
			var subjectMatch = href.match( emailSubjectRegex ),
				bodyMatch = href.match( emailBodyRegex );
			retval.type = 'email';
			retval.email = {};
			retval.email.address = emailMatch[1];
			subjectMatch && ( retval.email.subject = decodeURIComponent( subjectMatch[1] ) );
			bodyMatch && ( retval.email.body = decodeURIComponent( bodyMatch[1] ) );
		}
		else if ( anchorMatch )
		{
			retval.type = 'anchor';
			retval.anchor = {};
			retval.anchor.name = retval.anchor.id = anchorMatch[1];
		}
		else if ( href && urlMatch )		// urlRegex matches empty strings, so need to check for href as well.
		{
			retval.type = 'url';
			retval.url = {};
			retval.url.protocol = urlMatch[1];
			retval.url.url = urlMatch[2];
		}
		else
			retval.type = 'url';

		// Load target and popup settings.
		if ( element )
		{
			var target = element.getAttribute( 'target' );
			retval.target = {};
			retval.adv = {};

			// IE BUG: target attribute is an empty string instead of null in IE if it's not set.
			if ( !target )
			{
				var onclick = element.getAttribute( '_cke_pa_onclick' ) || element.getAttribute( 'onclick' ),
					onclickMatch = onclick && onclick.match( popupRegex );
				if ( onclickMatch )
				{
					retval.target.type = 'popup';
					retval.target.name = onclickMatch[1];

					var featureMatch;
					while ( ( featureMatch = popupFeaturesRegex.exec( onclickMatch[2] ) ) )
					{
						if ( featureMatch[2] == 'yes' || featureMatch[2] == '1' )
							retval.target[ featureMatch[1] ] = true;
						else if ( isFinite( featureMatch[2] ) )
							retval.target[ featureMatch[1] ] = featureMatch[2];
					}
				}
			}
			else
			{
				var targetMatch = target.match( selectableTargets );
				if ( targetMatch )
					retval.target.type = retval.target.name = target;
				else
				{
					retval.target.type = 'frame';
					retval.target.name = target;
				}
			}

			var me = this;
			var advAttr = function( inputName, attrName )
			{
				var value = element.getAttribute( attrName );
				if ( value !== null )
					retval.adv[ inputName ] = value || '';
			};
			advAttr( 'advId', 'id' );
			advAttr( 'advLangDir', 'dir' );
			advAttr( 'advAccessKey', 'accessKey' );
			advAttr( 'advName', 'name' );
			advAttr( 'advLangCode', 'lang' );
			advAttr( 'advTabIndex', 'tabindex' );
			advAttr( 'advTitle', 'title' );
			advAttr( 'advContentType', 'type' );
			advAttr( 'advCSSClasses', 'class' );
			advAttr( 'advCharset', 'charset' );
			advAttr( 'advStyles', 'style' );
		}

		// Find out whether we have any anchors in the editor.
		// Get all IMG elements in CK document.
		var elements = editor.document.getElementsByTag( 'img' ),
			realAnchors = new CKEDITOR.dom.nodeList( editor.document.$.anchors ),
			anchors = retval.anchors = [];

		for( var i = 0; i < elements.count() ; i++ )
		{
			var item = elements.getItem( i );
			if ( item.getAttribute( '_cke_realelement' ) && item.getAttribute( '_cke_real_element_type' ) == 'anchor' )
			{
				anchors.push( editor.restoreRealElement( item ) );
			}
		}

		for ( i = 0 ; i < realAnchors.count() ; i++ )
			anchors.push( realAnchors.getItem( i ) );

		for ( i = 0 ; i < anchors.length ; i++ )
		{
			item = anchors[ i ];
			anchors[ i ] = { name : item.getAttribute( 'name' ), id : item.getAttribute( 'id' ) };
		}

		// Record down the selected element in the dialog.
		this._.selectedElement = element;

		return retval;
	};

	var setupParams = function( page, data )
	{
		if ( data[page] )
			this.setValue( data[page][this.id] || '' );
	};

	var setupPopupParams = function( data )
	{
		return setupParams.call( this, 'target', data );
	};

	var setupAdvParams = function( data )
	{
		return setupParams.call( this, 'adv', data );
	};

	var commitParams = function( page, data )
	{
		if ( !data[page] )
			data[page] = {};

		data[page][this.id] = this.getValue() || '';
	};

	var commitPopupParams = function( data )
	{
		return commitParams.call( this, 'target', data );
	};

	var commitAdvParams = function( data )
	{
		return commitParams.call( this, 'adv', data );
	};

	return {
		title : 'Link to node',
		minWidth : 350,
		minHeight : 230,
		contents : [
			{
				id : 'info',
				label : editor.lang.link.info,
				title : editor.lang.link.info,
				elements :
				[
					{
						type : 'vbox',
						id : 'urlOptions',
						children :
						[
							{
								type : 'hbox',
								widths : [ '25%', '75%' ],
								children :
								[
									{
										id : 'linkType',
										type : 'select',
										label : editor.lang.link.type,
										hidden : !Drupal.settings.ckeditor.linktocontent_node_select_type,
										'default' : 'url',
										items :
										[
											[ 'path', 'path' ],
											[ 'internal', 'internal' ],
										],
										onChange : function() {
											var url = this.getDialog().getContentElement( 'info', 'url' );
											if ( ( !Drupal.settings.ckeditor.linktocontent_node_select_type && Drupal.settings.ckeditor.linktocontent_node_path_filter )
													|| this.getValue() == 'internal' ) {
												url.setValue($('#txtUrlInternal').val());
											}
											else {
												url.setValue($('#txtUrlPath').val());
											}
										},
										setup : function( data )
										{
											if ( data.type )
												this.setValue( data.type );
										},
										commit : function( data )
										{
											data.type = this.getValue();
										}
									},
									{
										type : 'text',
										id : 'url',
										label : editor.lang.common.url,
										onLoad : function ()
										{
											this.allowOnChange = true;
										},
										validate : function()
										{
											var dialog = this.getDialog();
											var func = CKEDITOR.dialog.validate.notEmpty( editor.lang.link.noUrl );
											return func.apply( this );
										},
										setup : function( data )
										{
											this.allowOnChange = false;
											if ( data.url )
												this.setValue( data.url.url );
											this.allowOnChange = true;

											var linkType = this.getDialog().getContentElement( 'info', 'linkType' );
											if ( linkType && linkType.getValue() == 'url' )
												this.select();

										},
										commit : function( data )
										{
											if ( !data.url )
												data.url = {};

											data.url.url = this.getValue();
											this.allowOnChange = false;
										}
									}
								],
								setup : function( data )
								{
									if ( !this.getDialog().getContentElement( 'info', 'linkType' ) )
										this.getElement().show();
								}
							}
						]
					},
					{
						type : 'html',
						html : 
	'<link rel="stylesheet" href="' + Drupal.settings.ckeditor.module_path + '/plugins/linktonode/css/linktocontent.css" type="text/css" />' +
	'<div class="panel_wrap" id="list">' +
	'<div class="nodes">' +
	'<div id="nodelist" class="scrollable accessible">' +
		'<table cellspacing="0" summary="nodelist" style="width:100%">' +
		'<thead>' +
		'<tr>' +
			'<th style="font-weight:bold">Node title</th>' +
			'<th style="font-weight:bold">Created</th>' +
			'<th style="font-weight:bold">Author</th>' +
		'</tr>' +
		'</thead>' +
		'<tbody>' +
		'</tbody>' +
		'</table>' +
	'</div>' +
	'</div>' +
	'</div>' +
	'<input id="txtUrlPath" type="hidden" />' + 
	'<input id="txtUrlInternal" type="hidden" />' +
	'<input id="txtUrlText" type="hidden" />' +
	'<div id="statusImg"><img alt="loading" src="' + Drupal.settings.ckeditor.module_path + '/plugins/linktonode/images/loading.gif" />loading...</div>'
					}
				]
			},
			{
				id : 'target',
				label : editor.lang.link.target,
				title : editor.lang.link.target,
				elements :
				[
					{
						type : 'hbox',
						widths : [ '50%', '50%' ],
						children :
						[
							{
								type : 'select',
								id : 'linkTargetType',
								label : editor.lang.link.target,
								'default' : 'notSet',
								style : 'width : 100%;',
								'items' :
								[
									[ editor.lang.link.targetNotSet, 'notSet' ],
									[ editor.lang.link.targetFrame, 'frame' ],
									[ editor.lang.link.targetPopup, 'popup' ],
									[ editor.lang.link.targetNew, '_blank' ],
									[ editor.lang.link.targetTop, '_top' ],
									[ editor.lang.link.targetSelf, '_self' ],
									[ editor.lang.link.targetParent, '_parent' ]
								],
								onChange : targetChanged,
								setup : function( data )
								{
									if ( data.target )
										this.setValue( data.target.type );
								},
								commit : function( data )
								{
									if ( !data.target )
										data.target = {};

									data.target.type = this.getValue();
								}
							},
							{
								type : 'text',
								id : 'linkTargetName',
								label : editor.lang.link.targetFrameName,
								'default' : '',
								setup : function( data )
								{
									if ( data.target )
										this.setValue( data.target.name );
								},
								commit : function( data )
								{
									if ( !data.target )
										data.target = {};

									data.target.name = this.getValue();
								}
							}
						]
					},
					{
						type : 'vbox',
						width : 260,
						align : 'center',
						padding : 2,
						id : 'popupFeatures',
						children :
						[
							{
								type : 'html',
								html : CKEDITOR.tools.htmlEncode( editor.lang.link.popupFeatures )
							},
							{
								type : 'hbox',
								children :
								[
									{
										type : 'checkbox',
										id : 'resizable',
										label : editor.lang.link.popupResizable,
										setup : setupPopupParams,
										commit : commitPopupParams
									},
									{
										type : 'checkbox',
										id : 'status',
										label : editor.lang.link.popupStatusBar,
										setup : setupPopupParams,
										commit : commitPopupParams

									}
								]
							},
							{
								type : 'hbox',
								children :
								[
									{
										type : 'checkbox',
										id : 'location',
										label : editor.lang.link.popupLocationBar,
										setup : setupPopupParams,
										commit : commitPopupParams

									},
									{
										type : 'checkbox',
										id : 'toolbar',
										label : editor.lang.link.popupToolbar,
										setup : setupPopupParams,
										commit : commitPopupParams

									}
								]
							},
							{
								type : 'hbox',
								children :
								[
									{
										type : 'checkbox',
										id : 'menubar',
										label : editor.lang.link.popupMenuBar,
										setup : setupPopupParams,
										commit : commitPopupParams

									},
									{
										type : 'checkbox',
										id : 'fullscreen',
										label : editor.lang.link.popupFullScreen,
										setup : setupPopupParams,
										commit : commitPopupParams

									}
								]
							},
							{
								type : 'hbox',
								children :
								[
									{
										type : 'checkbox',
										id : 'scrollbars',
										label : editor.lang.link.popupScrollBars,
										setup : setupPopupParams,
										commit : commitPopupParams

									},
									{
										type : 'checkbox',
										id : 'dependent',
										label : editor.lang.link.popupDependent,
										setup : setupPopupParams,
										commit : commitPopupParams

									}
								]
							},
							{
								type : 'hbox',
								children :
								[
									{
										type :  'text',
										widths : [ '30%', '70%' ],
										labelLayout : 'horizontal',
										label : editor.lang.link.popupWidth,
										id : 'width',
										setup : setupPopupParams,
										commit : commitPopupParams

									},
									{
										type :  'text',
										labelLayout : 'horizontal',
										widths : [ '55%', '45%' ],
										label : editor.lang.link.popupLeft,
										id : 'left',
										setup : setupPopupParams,
										commit : commitPopupParams

									}
								]
							},
							{
								type : 'hbox',
								children :
								[
									{
										type :  'text',
										labelLayout : 'horizontal',
										widths : [ '30%', '70%' ],
										label : editor.lang.link.popupHeight,
										id : 'height',
										setup : setupPopupParams,
										commit : commitPopupParams

									},
									{
										type :  'text',
										labelLayout : 'horizontal',
										label : editor.lang.link.popupTop,
										widths : [ '55%', '45%' ],
										id : 'top',
										setup : setupPopupParams,
										commit : commitPopupParams

									}
								]
							}
						]
					}
				]
			},
			{
				id : 'advanced',
				label : editor.lang.link.advanced,
				title : editor.lang.link.advanced,
				elements :
				[
					{
						type : 'vbox',
						padding : 1,
						children :
						[
							{
								type : 'hbox',
								widths : [ '45%', '35%', '20%' ],
								children :
								[
									{
										type : 'text',
										id : 'advId',
										label : editor.lang.link.id,
										setup : setupAdvParams,
										commit : commitAdvParams
									},
									{
										type : 'select',
										id : 'advLangDir',
										label : editor.lang.link.langDir,
										'default' : '',
										style : 'width:110px',
										items :
										[
											[ editor.lang.link.langDirNotSet, '' ],
											[ editor.lang.link.langDirLTR, 'ltr' ],
											[ editor.lang.link.langDirRTL, 'rtl' ]
										],
										setup : setupAdvParams,
										commit : commitAdvParams
									},
									{
										type : 'text',
										id : 'advAccessKey',
										width : '80px',
										label : editor.lang.link.acccessKey,
										maxLength : 1,
										setup : setupAdvParams,
										commit : commitAdvParams

									}
								]
							},
							{
								type : 'hbox',
								widths : [ '45%', '35%', '20%' ],
								children :
								[
									{
										type : 'text',
										label : editor.lang.link.name,
										id : 'advName',
										setup : setupAdvParams,
										commit : commitAdvParams

									},
									{
										type : 'text',
										label : editor.lang.link.langCode,
										id : 'advLangCode',
										width : '110px',
										'default' : '',
										setup : setupAdvParams,
										commit : commitAdvParams

									},
									{
										type : 'text',
										label : editor.lang.link.tabIndex,
										id : 'advTabIndex',
										width : '80px',
										maxLength : 5,
										setup : setupAdvParams,
										commit : commitAdvParams

									}
								]
							}
						]
					},
					{
						type : 'vbox',
						padding : 1,
						children :
						[
							{
								type : 'hbox',
								widths : [ '45%', '55%' ],
								children :
								[
									{
										type : 'text',
										label : editor.lang.link.advisoryTitle,
										'default' : '',
										id : 'advTitle',
										setup : setupAdvParams,
										commit : commitAdvParams

									},
									{
										type : 'text',
										label : editor.lang.link.advisoryContentType,
										'default' : '',
										id : 'advContentType',
										setup : setupAdvParams,
										commit : commitAdvParams

									}
								]
							},
							{
								type : 'hbox',
								widths : [ '45%', '55%' ],
								children :
								[
									{
										type : 'text',
										label : editor.lang.link.cssClasses,
										'default' : '',
										id : 'advCSSClasses',
										setup : setupAdvParams,
										commit : commitAdvParams

									},
									{
										type : 'text',
										label : editor.lang.link.charset,
										'default' : '',
										id : 'advCharset',
										setup : setupAdvParams,
										commit : commitAdvParams

									}
								]
							},
							{
								type : 'hbox',
								children :
								[
									{
										type : 'text',
										label : editor.lang.link.styles,
										'default' : '',
										id : 'advStyles',
										setup : setupAdvParams,
										commit : commitAdvParams

									}
								]
							}
						]
					}
				]
			}
		],
		onShow : function()
		{
			this.fakeObj = false;

			var editor = this.getParentEditor(),
				selection = editor.getSelection(),
				ranges = selection.getRanges(),
				element = null,
				me = this;
			// Fill in all the relevant fields if there's already one link selected.
			if ( ranges.length == 1 )
			{

				var rangeRoot = ranges[0].getCommonAncestor( true );
				element = rangeRoot.getAscendant( 'a', true );
				if ( element && element.getAttribute( 'href' ) )
				{
					selection.selectElement( element );
				}
				else if ( ( element = rangeRoot.getAscendant( 'img', true ) ) &&
						 element.getAttribute( '_cke_real_element_type' ) &&
						 element.getAttribute( '_cke_real_element_type' ) == 'anchor' )
				{
					this.fakeObj = element;
					element = editor.restoreRealElement( this.fakeObj );
					selection.selectElement( this.fakeObj );
				}
				else
					element = null;
			}

			this.setupContent( parseLink.apply( this, [ editor, element ] ) );
		},
		onOk : function()
		{
			var attributes = { href : 'javascript:void(0)/*' + CKEDITOR.tools.getNextNumber() + '*/' },
				removeAttributes = [],
				data = { href : attributes.href },
				me = this, editor = this.getParentEditor();
			this.commitContent( data );
			//<!-- linktonode START -->
			attributes._cke_saved_href = this.getContentElement( 'info', 'url' ).getValue();
			//<!-- linktonode END -->

			// Popups and target.
			if ( data.target )
			{
				if ( data.target.type == 'popup' )
				{
					var onclickList = [ 'window.open(this.href, \'',
							data.target.name || '', '\', \'' ];
					var featureList = [ 'resizable', 'status', 'location', 'toolbar', 'menubar', 'fullscreen',
							'scrollbars', 'dependent' ];
					var featureLength = featureList.length;
					var addFeature = function( featureName )
					{
						if ( data.target[ featureName ] )
							featureList.push( featureName + '=' + data.target[ featureName ] );
					};

					for ( var i = 0 ; i < featureLength ; i++ )
						featureList[i] = featureList[i] + ( data.target[ featureList[i] ] ? '=yes' : '=no' ) ;
					addFeature( 'width' );
					addFeature( 'left' );
					addFeature( 'height' );
					addFeature( 'top' );

					onclickList.push( featureList.join( ',' ), '\'); return false;' );
					attributes[ CKEDITOR.env.ie || CKEDITOR.env.webkit ? '_cke_pa_onclick' : 'onclick' ] = onclickList.join( '' );
				}
				else
				{
					if ( data.target.type != 'notSet' && data.target.name )
						attributes.target = data.target.name;
					removeAttributes.push( '_cke_pa_onclick', 'onclick' );
				}
			}

			// Advanced attributes.
			if ( data.adv )
			{
				var advAttr = function( inputName, attrName )
				{
					var value = data.adv[ inputName ];
					if ( value )
						attributes[attrName] = value;
					else
						removeAttributes.push( attrName );
				};

				if ( this._.selectedElement )
					advAttr( 'advId', 'id' );
				advAttr( 'advLangDir', 'dir' );
				advAttr( 'advAccessKey', 'accessKey' );
				advAttr( 'advName', 'name' );
				advAttr( 'advLangCode', 'lang' );
				advAttr( 'advTabIndex', 'tabindex' );
				advAttr( 'advTitle', 'title' );
				advAttr( 'advContentType', 'type' );
				advAttr( 'advCSSClasses', 'class' );
				advAttr( 'advCharset', 'charset' );
				advAttr( 'advStyles', 'style' );
			}

			if ( !this._.selectedElement )
			{
				// Create element if current selection is collapsed.
				var selection = editor.getSelection(),
					ranges = selection.getRanges();
				if ( ranges.length == 1 && ranges[0].collapsed )
				{
					// <!-- linktonode START -->
					var text = new CKEDITOR.dom.text( $('#txtUrlText').val() || attributes._cke_saved_href, editor.document );
					// <!-- linktonode END -->
					ranges[0].insertNode( text );
					ranges[0].selectNodeContents( text );
					selection.selectRanges( ranges );
				}

				// Apply style.
				var style = new CKEDITOR.style( { element : 'a', attributes : attributes } );
				style.type = CKEDITOR.STYLE_INLINE;		// need to override... dunno why.
				style.apply( editor.document );

				// Id. Apply only to the first link.
				if ( data.adv && data.adv.advId )
				{
					var links = this.getParentEditor().document.$.getElementsByTagName( 'a' );
					for ( i = 0 ; i < links.length ; i++ )
					{
						if ( links[i].href == attributes.href )
						{
							links[i].id = data.adv.advId;
							break;
						}
					}
				}
			}
			else
			{
				// We're only editing an existing link, so just overwrite the attributes.
				var element = this._.selectedElement;

				// IE BUG: Setting the name attribute to an existing link doesn't work.
				// Must re-create the link from weired syntax to workaround.
				if ( CKEDITOR.env.ie && attributes.name != element.getAttribute( 'name' ) )
				{
					var newElement = new CKEDITOR.dom.element( '<a name="' + CKEDITOR.tools.htmlEncode( attributes.name ) + '">',
							editor.document );

					selection = editor.getSelection();

					element.moveChildren( newElement );
					element.copyAttributes( newElement, { name : 1 } );
					newElement.replace( element );
					element = newElement;

					selection.selectElement( element );
				}

				element.setAttributes( attributes );
				element.removeAttributes( removeAttributes );

				// Make the element display as an anchor if a name has been set.
				if ( element.getAttribute( 'name' ) )
					element.addClass( 'cke_anchor' );
				else
					element.removeClass( 'cke_anchor' );

				if ( this.fakeObj )
					editor.createFakeElement( element, 'cke_anchor', 'anchor' ).replace( this.fakeObj );

				delete this._.selectedElement;
			}
		},
		onLoad : function()
		{
			CKEDITOR._._linkToNodeDialog = this;

			if ( !editor.config.linkShowAdvancedTab )
				this.hidePage( 'advanced' );		//Hide Advanded tab.

			if ( !editor.config.linkShowTargetTab )
				this.hidePage( 'target' );		//Hide Target tab.

		}
	};
} );
