$Id: README.txt,v 1.1.2.11 2010/01/12 12:08:49 fago Exp $


-----------------------
Content Profile Module
-----------------------
by Wolfgang Ziegler, nuppla@zites.net

With this module you can build user profiles with drupal's content types.


Installation 
------------
 * Copy the module's directory to your modules directory and activate the module.
 
 Usage:
--------
 * There will be a new content type "profile". Customize its settings at
   admin/content/types.
 * At the bottom of each content type edit form, there is a checkbox, which allows
   you to mark a content type as profile.
 * When you edit a profile content type there will be a further tab "Content profile",
   which provides content profile specific settings.
   

 Warning:
---------
 The module uses drupal's content or "nodes" for user profiles, so the access
 permissions applied to view the content profiles are the regular node related
 permissions.
 That means the "access user profiles" permission of the user module still
 applies only to the user account pages at "user/UID" but not to content profiles,
 which can be viewed at node/NID too. Still you can use any regular node access
 module to restrict access to your content profiles, e.g. you may use the Content
 Access module for that (http://drupal.org/project/content_access).



Content profiles per role:
--------------------------
You may, but you need not, mark multiple content types as profile. By customizing 
the permissions of a content type, this allows you to create different profiles for
different roles.


Hints:
------

 * When using content profiles the "title" field is sometimes annoying. You can rename
   it at the content types settings or hide it in the form and auto generate a title by
   using the auto nodetitle module http://drupal.org/project/auto_nodetitle.
   
 * If you want to link to a content profile of a user, you can always link to the path
   "user/UID/profile/TYPE" where UID is the users id and TYPE the machine readable content
   type name, an example path would be "user/1/profile/profile".
   This path is working regardless the user has already profile content created or not.

 * If you want to theme your content profile, you can do it like with any other content.
   Read http://drupal.org/node/266817.
   
 * If you want a content profile to be private while your site content should be available
   to the public, you need a module that allows configuring more fine grained access control
   permissions, e.g. the module Content Access (http://drupal.org/project/content_access)
   allows you to that.
   
 * There is also rules integration which is useful for customizing the behaviour of the
   module. See below for more.



Theming: Easily use profile information in your templates! 
-----------------------------------------------------------
Content Profile adds a new variable $content_profile to most templates related to users.
So this variable allows easy access to the data contained in the users' profiles.
Furthermore it does its job fast by lazy-loading and caching the needed content profile
nodes.

The $content_profile variable is available in the page, node, comment, user_name,
user_profile, user_signature, search_result and some other templates. 

$content_profile lets you access all variables of a profile, which are you used to
have in a common node template. See http://drupal.org/node/11816.

So in any of these templates you may use the $content_profile like this:

<?php
 // Just output the title of the content profile of type 'profile'
 // If there is no such profile, it will output nothing.
 echo $content_profile->get_variable('profile', 'title');

 // Get all variables of the content profile of type 'profile'
 $variables = $content_profile->get_variables('profile');
 
 // Print out a list of all available variables
 // If the user has no profile created yet, $variables will be FALSE.
 print_r($variables);

 if ($variables) {
   // Print the title and the content.
   echo $variables['title'];
   echo $variables['content'];
 }
 else {
   // No profile created yet.
 }
 
 // $content_profile also allows you to easily display the usual content profile's view
 // supporting the same parameters as node_view().
 echo $content_profile->get_view('profile');

?>

 Check the source of content_profile.theme_vars.inc to see what methods $content_profile
 supports else.


Adding $content_profile to further templates
--------------------------------------------

If you miss $content_profile in some templates containing user information (id), just
fill a issue in content profile's queue so we can add it to the module.
Furthermore you may let content_profile its variable to your custom templates by specifying
the setting 'content_profile_extra_templates' in your site's settings.php.

E.g. you may add:
  $conf['content_profile_extra_templates'] = array('my_template');

Where 'my_template' has to be the key of your template's entry in the theme_registry (hook_theme()).



Rules integration
------------------

There is some integration to the rules module (http://drupal.org/project/rules), which offers
a condition to check whether a user has already created a profile of a certain type. Then it
offers an action for loading the content profile of a user, which makes it available to token
replacements as well as to all other existing rules actions which deal with content.

So this integration allows one to build some profile related rules with the rules module. As
example the module ships with one deactivated default rule:

  "Redirect to profile creation page, if users have no profile."
  
If you activate it at the rules "Triggered rules" page, it's going to be evaluated when a user
logs in. Of course you can also alter the default rule and customize it so that it fits your needs,
e.g. you could remove the redirect action so that only a message is displayed.




---------------------------------------------
Content Profile User Registration Integration
----------------------------------------------

There is a small extension module shipping with the main module, which allows one to enable
registration integration per content profile. 


This module builds upon the main content profile module. It allows to integrate
the form of one or more content profile into the user registration page.


Installation 
------------
 * Activiate the module.
 * Be sure to read the usage notes below!
 
 
 Usage:
--------
 * When you edit a profile content type there will be a further tab "Content profile",
   which provides content profile specific settings. There is now a new field group
   called "User Registration" which allows you to enable this feature for a content profile.
   
 * You need not grant anonymous users access to create the content profile. If you would do so,
   anonymous users would be able to create anonymous profiles even without registering.
   
 * If you use the "Content permissions" module, which comes with CCK, make sure you grant access
   to fields that should appear for anonymous users.
   
 * The weight of the profile (configurable at the content profile settings) controls the position
   of the form elements on the registration page. 

 * You may also hide some form elements at the settings. Basically it allows you to hide non-required
   CCK fields as well as the title. If the title is hidden, it is set to the user's name.
    
 * For more control over the title use the "Automatic Nodetitles" module, which can be found
   at http://drupal.org/project/auto_nodetitle. It integrates fine with this module. 

 * Hiding required CCK fields is not supported, as the created content node would have empty
   required fields afterwards, which in affect would make it impossible even for admins to edit
   the content node.
  
 * So the "Hide other form elements" option allows you to hide all form elements not listed there,
   but required CCK fields always stay.
   
   However, you can still hide required CCK fields by restricting anonymous access to them by using the 
   "Content permissions" module of CCK. But be aware of this issue - maybe also restrict access for
   other roles accordingly.

 * If you want to hide the "body" field, just remove it from the content type in general at the content
   type's settings page. Then instead of this, just create a CCK textfield, which can be hidden.  

 * You can enable the registration integration for multiple profiles - however be aware that
   shared form elements like the title only appear once and all created profile nodes get the same
   values assigned.
   
 * For multiple registration paths for different roles, the AutoAssignRole module might help you:
   http://drupal.org/project/autoassignrole. It comes with Content Profile Registration Integration,
   so that you can select the profiles which should appear on each AutoAssignRole path (configurable
   at the content profile settings). You'll need a version of AutoAssignRole released later than 
   June 4, 2009. 

 * If you want to prepopulate some other form elements, maybe hidden CCK fields you can use the rules
   module for that. See http://drupal.org/project/rules.
   Just configure a rule, that reacts on the creation of the content profile (event) and populates
   your fields value (action).

 * Putting file uploads on the registration form is not supported and probably won't work right.
 
 * The CCK "Add more fields" feature is only working for users with javascript turned on in the
   registration form. Users without javascript won't be able to add more fields. Interested developers
   can find the related issue (in drupal itself) here: http://drupal.org/node/634984
 
 
  
-----------------------
Content Profile Tokens
-----------------------

Original author: @author Ádám Lippai - Oghma ltd. (lippai.adam@oghma.hu)


This is a small module that adds content profile tokens for textfields and number CCK fields for
a user as well as to the 'flag friend' modules' requester and requestee.

Warning: This module slows down the generation of users tokens, thus it might have some performance
         implications for your site. Use it with caution. 

Installation 
------------
 * Activiate the module.
 