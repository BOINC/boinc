/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

/**
 * @file Plugin for inserting files from imce without image dialog
 */
( function() {
  CKEDITOR.plugins.add( 'imce',
  {
    init: function( editor )
    {
      //adding button 
      editor.ui.addButton( 'IMCE',
      {
        label: 'IMCE',
        command: 'IMCEWindow',
        icon: this.path + 'images/icon.png'
      });
    
      //opening imce window
      editor.addCommand( 'IMCEWindow', {
        exec : function () {
          var width = editor.config.filebrowserWindowWidth || '80%',
            height = editor.config.filebrowserWindowHeight || '70%';
          
          editor.popup(Drupal.settings.basePath + 'index.php?q=imce\x26app=ckeditor|sendto@ckeditor_setFile|&CKEditorFuncNum=' + editor._.filebrowserFnIMCE, width, height);
        }
      });
      
      //add editor function
      editor._.filebrowserFnIMCE = CKEDITOR.tools.addFunction( setFile, editor )
      
      //function which receive imce response
      window.ckeditor_setFile = function (file, win) {
        var cfunc = win.location.href.split('&');
      
        for (var x in cfunc) {
          if (cfunc[x].match(/^CKEditorFuncNum=\d+$/)) {
            cfunc = cfunc[x].split('=');
            break;
          }
        }
        
        CKEDITOR.tools.callFunction(cfunc[1], file);
        win.close();
      };
    }
  } );
  function setFile(file) {
    //checking if it is image
    if (file.width != 0 && file.height != 0) {
      this.insertHtml('<img src="' + file.url + '" style="width:' + file.width + 'px;height:' + file.height + 'px;" alt="' + file.name + '" />');
    } else {
      this.insertHtml('<a href="' + file.url + '">' + file.name + '</a>');
    }
  }
} )();
