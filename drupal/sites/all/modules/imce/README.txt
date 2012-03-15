
IMCE
http://drupal.org/project/imce
====================================

DESCRIPTION
-----------
IMCE is an image/file uploader and browser that supports personal directories and quota.
IMCE can easily be integrated into any WYSIWYG editor or any web application that needs a file browser.
See INTEGRATION METHODS for more information.

FEATURES
-----------
- Basic file operations: upload, delete
- Image(jpg, png, gif) operations: resize, create thumbnails, preview
- Support for private file system
- Configurable limits for user roles: file size per upload, directory quota, file extensions, and image dimensions
- Personal or shared folders for users
- Permissions per directory
- Ftp-like directory navigation
- File sorting by name, size, dimensions, date
- Tabbed interface for file operations
- Keyboard shortcuts(up, down, insert(or enter), delete, home, end, ctrl+A, R(esize), T(humbnails), U(pload)).
- Built-in support for inline image/file insertion into textareas
- Multiple file selection(using ctrl or shift)
- Ajax file operations
- Themable layout using tpl files

INSTALLATION
-----------
1) Copy imce directory to your modules directory
2) Enable the module at: /admin/build/modules
3) Create configuration profiles and assign them to user roles at /admin/settings/imce
4) Test it at /imce.
5) See imce-content.tpl.php for some configuration options such as inline previewing.
6) See INTEGRATION METHODS to make IMCE collaborate with your application if it's not already integrated.
Notes:
 - When you configure IMCE for inline image/file insertion into textareas there should appear an IMCE link under each textarea you specified.
 - If you are uploading files containing unicode characters, it is strongly recommended to use the transliteration module that sanitizes filenames by converting characters from unicode to us-ascii. http://drupal.org/project/transliteration
 - If you are using CCK, you may want to check the File field sources module at http://drupal.org/project/filefield_sources


FREQUENTLY FACED ISSUES
-----------
- Inaccessible/invalid directory or subdirectory:
In some server configurations, manually(ftp or directly) created directories may not be writable by PHP(by IMCE). In this case, you have to set the chmod permissions of the directory to 0777 in order to make it writable by anyone. 
You should also make sure that in each configuration profile all of the defined directories are located under drupal's file system path which is usually "files".
And also if "safe mode restriction" is active in your server, don't expect IMCE to run flawlessly.

- Disappearing images after node submission:
Having nothing to do with IMCE, it appeared many times in issue queues. This is an input filtering issue that can be resolved by adding <img> tag into the default input format. Using Full HTML is another solution. See admin/settings/filters.

- Upload does not work in Opera
Jquery form plugin before version 2.09 has problems with Opera 9.2+. Replace Drupal's misc/jquery.form.js with the one at http://jquery.malsup.com/form/#download

- IMCE may have problem working with Google Analytics and Secure pages modules. Just make sure to add *imce* path to the exceptions list of these modules.


INTEGRATION METHODS
-----------

Here are the applications that already integrated IMCE.

WYSIWYG:
Install http://drupal.org/project/imce_wysiwyg bridge module and enable IMCE as a plug-in in WYSIWYG settings

BUEditor:
IMCE is integrated in image and link dialogs.

(F)CKeditor(without WYSIWYG): 
(F)ckeditor profile->File browser settings->IMCE integration

If your application is not one of the above, please keep reading in order to learn how to integrate IMCE.

Let's create a CASE and embody the IMCE integration on it:
- An application named myApp
- Has an url field for file url:
  <input type="text" name="urlField" id="urlField">
- Has a browse button with click event(inline or set by jQuery): (This can be a text link or anything that is clickable)
  <input type="button" value="Browse" onclick="openFileBrowser()">
  
Now let's go through the integration methods and define the openFileBrowser function that opens IMCE and makes it fill our url field on file selection.


INTEGRATION BY URL
-----------
When IMCE is opened using an url that contains &app=applicationName|fileProperty1@FieldId1|fileProperty2@FieldId2|...
the specified fields are filled with the specified properties of the selected file.

Avaliable file properties are: url, name, size(formatted), width, height, date(formatted), bytes(integer size in bytes), time(integer date timestamp), id(file id for newly uploaded files, 0 or integer), relpath(rawurlencoded path relative to file directory path.)

In our CASE, we should open IMCE using this URL: /imce?app=myApp|url@urlField which contains our application name and our url field id

function openFileBrowser() {
  window.open('/imce?app=myApp|url@urlField', '', 'width=760,height=560,resizable=1');
}

That's all we need. Leave the rest to IMCE.
It will automatically create an operation tab named "Send to myApp" that sends the file url to our url field.
Clicking the files in preview do the same thing as well.

- What if we had another field for another file property e.g, Size: <input type="text" id="file-size"> ?
- We should have opened imce using this URL: /imce?app=myApp|url@urlField|size@file-size


- USING imceload:
You can point a predefined function to be executed when IMCE loads.
When the URL is like "app=myApp|imceload@myOnloadFunc", IMCE looks for "myOnloadFunc" in the parent window and executes it with the window parameter referring to IMCE window.
function myOnloadFunc (win) {//any method of imce is available through win.imce
  win.imce.setSendTo('Give it to myApplication baby', myFileHandler);//you should also define myFileHandler
}

- USING sendto:
You can point a predefined function to which the selected files are sent.
When the URL is like "app=myApp|sendto@myFileHandler", IMCE calls "myFileHandler" function of the parent window with file and window parameters.
function myFileHandler (file, win) {
  $('#urlFieldId').val(file.url);//insert file url into the url field
  win.close();//close IMCE
}
Usually sendto method is easier to implement, on the other hand imceload method is more flexible as you manually add your sento operator and also can do any modification before IMCE shows up.


ADVANCED INTEGRATION
-----------
In case:
- Your application wants to go beyond the simple "give me that file property" interaction with IMCE.
- Your application wants IMCE to send multiple files to it.(e.g., a gallery application)
- Your application wants to gain total control over IMCE.
Then you should consider applying advanced integration.

The initial step of advanced integration is the same as imceload-integration above.

We open IMCE and set its onload function:

window.open('/imce?app=myApp|imceload@initiateMyApp', '', 'width=760,height=560,resizable=1'); //initiateMyApp(win) will run when imce loads

Now we define our initiator function in which we do the necessary manipulations to IMCE interface:

initiateMyApp = function (win) {
  var imce = win.imce;
  ...use imce methods to add/remove/change things...
}

- Allright, but what do we add/romeve/change in IMCE ?
- Depends on our goal. Here are some properties and methods that can help us to achieve it:

Hooks
imce.hooks.load: an array of functions that run after imce loads. they are called with the window parameter.
imce.hooks.list: an array of functions that run while processing the file list. each row of the file list is sent to these functions.
imce.hooks.navigate: an array of functions that run after a directory is loaded. parameters sent are data(from ajax or cache), old_directory, cached(boolean that states the data is from the cache or not).
imce.hooks.cache: an array of functions that run just before a new directory is loaded. parameters are cached_data and new_directory.

Directory related properties
imce.tree: stores the directory list where imce.tree['.'] is the root element.

Directory related methods
imce.dirAdd(directory_name, parent_element, clickable): adds directory_name under parent_element. ex: imce.dirAdd('foo', imce.dir['.'], true)
imce.dirSubdirs(directory_name, subdirectories): adds each subdirectory in subdirectories array under directory_name. ex: imce.dirSubdirs('foo', ['bar', 'baz'])

File related properties
imce.findex: indexed array of files(table rows that contain file properties.)
imce.fids: object containing file_id(file name)-file(row) pairs.
imce.selected: object containing currently selected file_id(file name)-file(row) pairs.

File related methods
imce.fileAdd(file): adds the file object to the list. file object has the properties; name, size(bytes), width, height, date(timestamp), fsize(formatted), fdate(formatted)
imce.fileRemove(fiile_id): removes the file having the file_id from the list.
imce.fileGet(file_id). returns the file object having the file_id. file object contains name, url, size, bytes, width, height, date, time, id(file id for newly uploaded files, 0 or integer), relpath(rawurlencoded path relative to file directory path.)

File operations
imce.opAdd(op): adds an operation tab to the interface. op contains name, title, content(optional), func(optional onclick function)
imce.opEnable(name), imce.opDisable(name): enable/disable operation tabs.

Miscellaneous
imce.setMessage(msg, type): logs a message of the type(status, warning, error)

NOTES:
- All URL strings in the examples start with "/" considering the base path is "/".
In case your drupal is running on a sub directory e.g, http://localhost/drupal, these URLs should start with "/drupal/".
There is a safer solution that does not require manual URL fixing: If the Drupal javascript object is avaliable in your page you can use Drupal.settings.basePath at the beginning of URLs (Drupal.settings.basePath+'?q=imce....'). Note that, this won't work with multilingual paths with language prefixes.
- file and directory ids(names) used in imce.js are url encoded forms of original names. They are decoded using imce.decode and displayed in the lists.