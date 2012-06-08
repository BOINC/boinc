
Description
-----------
Drupal 6's Forms API is great, but its AHAH support (a technique similar to
AJAX) is lacking. You have to write a menu callback for every AHAH-enabled
form item of your form. This is time consuming, hard to maintain and hard to
write tests for. Not to mention that the code for those menu callbacks is not
easy to understand, yet always the same. ("What's that piece of code for?" –
"I'm not sure, it's just necessary…") See my blog post about this if you want
more details: http://wimleers.com/blog/ahah-helper-module.

This module simplifies that. It allows you to:
1) not write any menu callback at all.
2) still not write any JavaScript at all.
3) have a sole, central form definition function that has some if-tests to
   support a changing form based on the user's input, i.e. by checking
   $form_state['values'] and/or $form_state['storage']. This is in fact the
   exact same system you've been applying if you've already written multi-step
   forms. This makes sense, because AHAH forms are in fact normal multi-step
   forms, that just happen to be updatable through AHAH as well.
   You still have to use the #ahah property and set a wrapper, but you provide
   a "magical path" that will automatically rebuild and render the desired
   part of the form. If the part of the form that you want to be rendered is
   $form['fapi']['rocks'] then you would do
   'path' => ahah_helper_path(array('fapi', 'rocks')) and that's it.
   Adding graceful degradation just became really easy: just create buttons
   with the appropriate text, set '#submit' => array('ahah_helper_submit'),
   and off you go. You'd probably create such a button for every AHAH-powered
   form item. The exact same code will be used as when JavaScript would be
   enabled. (If you've got a AHAH-powered select called 'Usage', you'd
   probably name the button 'Update usage'. You get the point.)
   And thanks to these buttons, writing functional tests now becomes trivial
   as well. Because the same code is used when JavaScript is disabled (through
   the buttons) or enabled (through AHAH callbacks), just press the buttons in
   your tests and you'll be fine!
4) skip form validation for all AHAH updates that do not call validate or
   submit callbacks.
5) have new AHAH-powered form items added in an AHAH callback (previously not
   supported).

Look at the included ahah_helper_demo module for an example.


Dependencies
------------
None.


Installation
------------
1) Place this module directory in your modules folder (this will usually be
"sites/all/modules/").

2) Enable the module.


Sponsors
--------
Dries Buytaert & Benjamin Schrauwen of http://mollom.com.


Author
------
Wim Leers

* mail: work@wimleers.com
* website: http://wimleers.com/work

The author can be contacted for paid customizations of this module as well as
Drupal consulting and development.
