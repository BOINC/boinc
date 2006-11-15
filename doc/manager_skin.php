<?php
require_once("docutil.php");
page_head("Creating a skin for the BOINC Manager");
echo "
This document describes how to customize the appearance
of the BOINC Manager simple GUI (available in 5.8+).

<h3>Contents</h3>
<ul>
    <li><a href=\"#Skin\">Skin</a>
        <ul>
            <li><a href=\"#Localization\">Localization</a>
            <li><a href=\"#Layout\">Layout</a>
            <li><a href=\"#Samples\">Samples</a>
        </ul>
    <li><a href=\"#SkinCollections\">Skin Collections</a>
        <ul>
            <li><a href=\"#Simple\">Simple</a>
            <li><a href=\"#Advanced\">Advanced</a>
            <li><a href=\"#Wizards\">Wizards</a>
        </ul>
    <li><a href=\"#SkinBasicElements\">Skin Basic Elements</a>
        <ul>
            <li><a href=\"#Image\">Image</a>
            <li><a href=\"#Icon\">Icon</a>
            <li><a href=\"#SimpleTab\">Simple Tab</a>
            <li><a href=\"#SimpleLink\">Simple Link</a>
            <li><a href=\"#SimpleButton\">Simple Button</a>
        </ul>
</ul>

<h3><a name=\"Skin\">Skin</a></h3>
<p>
Skins allow you to customize how an interface looks.
<p>
A skin.xml file needs to be created for any customized skin.
This document describes the layout of the skin.xml file.
<p>
A skin.xml file should be contained within its
own directory under the 'skins' directory.
<p>
An example would look like this:
"; block_start(); echo "
+ /BOINC
|
+---+ /Skins
    |
    +---+ /Custom Skin
        |
        +--- skin.xml
"; block_end(); echo "
<p>

<h4><a name=\"Localization\">Localization</a></h4>
<p>
BOINC Manager attempts to find and use localized skin references.
It falls back to a suitable language
and if none can be found it'll fall back to English.
Overriding the
language being searched for can be done in the Advanced GUI's option dialog.
<p>
If the prefered language is 'pt_BR' and it cannot be found then the next language searched for is 'pt'.
If 'pt cannot be found then the manager resorts to 'en'.

<h4><a name=\"Layout\">Layout</a></h4>
<p>
The skin.xml file is described as follows:
"; block_start(); echo "
&lt;skin&gt;
    &lt;pt_BR&gt;
        &lt;<a href=\"#Simple\">simple</a> /&gt;
        [ &lt;<a href=\"#Advanced\">advanced</a> /&gt; ]
        [ &lt;<a href=\"#Wizards\">wizards</a> /&gt; ]
    &lt;/pt_BR&gt;
    &lt;pt&gt;
        &lt;<a href=\"#Simple\">simple</a> /&gt;
        [ &lt;<a href=\"#Advanced\">advanced</a> /&gt; ]
        [ &lt;<a href=\"#Wizards\">wizards</a> /&gt; ]
    &lt;/pt&gt;
    &lt;en&gt;
        &lt;<a href=\"#Simple\">simple</a> /&gt;
        [ &lt;<a href=\"#Advanced\">advanced</a> /&gt; ]
        [ &lt;<a href=\"#Wizards\">wizards</a> /&gt; ]
    &lt;/en&gt;
&lt;/skin&gt;
"; block_end(); echo "
<p>
Both the <a href=\"#Advanced\">advanced</a> and <a href=\"#Wizards\">wizards</a> collections are optional.
<p>

<h4><a name=\"Sample\">Sample</a></h4>
<p>
The World Community Grid skin can be found <a href=\"http://setiathome.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/clientgui/skins/World%20Community%20Grid/skin.xml?rev=1.2;content-type=text%2Fplain\">here</a>.
<p>

<h3><a name=\"SkinCollections\">Skin Collections</a></h3>

<h4><a name=\"Simple\">Simple</a></h4>
<p>
The Simple collection contains all the elements need to construct the Simple GUI.
<p>
The Simple collection contains the following elements:
<table width=100%>
  <tr>
    <th>Name</th>
    <th>Type</th>
    <th>Image Height</th>
    <th>Image Width</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>background_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>540px</td>
    <td>410px</td>
    <td>
      Used as the Simple GUI background image.<br>
      A background color should be specified for this element.
    </td>
  <tr>
  <tr>
    <td>spacer_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>11px</td>
    <td>2px</td>
    <td>Used to seperate the links on the bottom of the Simple GUI</td>
  <tr>
  <tr>
    <td>static_line_color</td>
    <td>String</td>
    <td></td>
    <td></td>
    <td>The color is represented as an RGB value with the token being ':'</td>
  <tr>
  <tr>
    <td>state_indicator_background_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>35px</td>
    <td>264px</td>
    <td></td>
  <tr>
  <tr>
    <td>connecting_indicator_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>15px</td>
    <td>14px</td>
    <td></td>
  <tr>
  <tr>
    <td>error_indicator_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>20px</td>
    <td>69px</td>
    <td></td>
  <tr>
  <tr>
    <td>workunit_active_tab</td>
    <td><a href=\"#SimpleTab\">Simple Tab</a></td>
    <td>16px</td>
    <td>16px</td>
    <td></td>
  <tr>
  <tr>
    <td>workunit_suspended_tab</td>
    <td><a href=\"#SimpleTab\">Simple Tab</a></td>
    <td>16px</td>
    <td>16px</td>
    <td></td>
  <tr>
  <tr>
    <td>workunit_tab_area_background_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>24px</td>
    <td>343px</td>
    <td></td>
  <tr>
  <tr>
    <td>workunit_area_background_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>314px</td>
    <td>343px</td>
    <td></td>
  <tr>
  <tr>
    <td>workunit_animation_background_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>146px</td>
    <td>294px</td>
    <td></td>
  <tr>
  <tr>
    <td>workunit_animation_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>126px</td>
    <td>290px</td>
    <td></td>
  <tr>
  <tr>
    <td>workunit_gauge_background_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>18px</td>
    <td>258px</td>
    <td></td>
  <tr>
  <tr>
    <td>workunit_gauge_progress_indicator_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>7px</td>
    <td>8px</td>
    <td></td>
  <tr>
  <tr>
    <td>project_area_background_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>113px</td>
    <td>343px</td>
    <td></td>
  <tr>
  <tr>
    <td>project_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>40px</td>
    <td>40px</td>
    <td>Default image to display for a project that does not currently have an image defined.</td>
  <tr>
  <tr>
    <td>attach_project_button</td>
    <td><a href=\"#SimpleButton\">Simple Button</a></td>
    <td>18px</td>
    <td>81px</td>
    <td></td>
  <tr>
  <tr>
    <td>right_arrow_button</td>
    <td><a href=\"#SimpleButton\">Simple Button</a></td>
    <td>20px</td>
    <td>20px</td>
    <td></td>
  <tr>
  <tr>
    <td>left_arrow_button</td>
    <td><a href=\"#SimpleButton\">Simple Button</a></td>
    <td>20px</td>
    <td>20px</td>
    <td></td>
  <tr>
  <tr>
    <td>save_button</td>
    <td><a href=\"#SimpleButton\">Simple Button</a></td>
    <td>16px</td>
    <td>57px</td>
    <td></td>
  <tr>
  <tr>
    <td>cancel_button</td>
    <td><a href=\"#SimpleButton\">Simple Button</a></td>
    <td>16px</td>
    <td>57px</td>
    <td></td>
  <tr>
  <tr>
    <td>close_button</td>
    <td><a href=\"#SimpleButton\">Simple Button</a></td>
    <td>16px</td>
    <td>57px</td>
    <td></td>
  <tr>
  <tr>
    <td>messages_link_image</td>
    <td><a href=\"#SimpleLink\">Simple Link</a></td>
    <td>20px</td>
    <td>70px</td>
    <td></td>
  <tr>
  <tr>
    <td>messages_alert_link_image</td>
    <td><a href=\"#SimpleLink\">Simple Link</a></td>
    <td>20px</td>
    <td>70px</td>
    <td></td>
  <tr>
  <tr>
    <td>suspend_link_image</td>
    <td><a href=\"#SimpleLink\">Simple Link</a></td>
    <td>20px</td>
    <td>59px</td>
    <td></td>
  <tr>
  <tr>
    <td>resume_link_image</td>
    <td><a href=\"#SimpleLink\">Simple Link</a></td>
    <td>20px</td>
    <td>59px</td>
    <td></td>
  <tr>
  <tr>
    <td>preferences_link_image</td>
    <td><a href=\"#SimpleLink\">Simple Link</a></td>
    <td>20px</td>
    <td>81px</td>
    <td></td>
  <tr>
  <tr>
    <td>advanced_link_image</td>
    <td><a href=\"#SimpleLink\">Simple Link</a></td>
    <td>20px</td>
    <td>101px</td>
    <td></td>
  <tr>
  <tr>
    <td>dialog_background_image</td>
    <td><a href=\"#Image\">Image</a></td>
    <td>600px</td>
    <td>800px</td>
    <td>
      The dialogs height and width change according to the configuraton of the computer 
      so the center of the image is what is drawn on to the dialog.
    </td>
  <tr>
</table>
<p>
The Simple collection is described as follows:
"; block_start(); echo "
&lt;simple&gt;
    &lt;background_image /&gt;
    &lt;spacer_image /&gt;
    ...
&lt;/simple&gt;
"; block_end(); echo "
<p>


<h4><a name=\"Advanced\">Advanced</a></h4>
<p>
The Advanced collection contains all the elements need to construct the Advanced GUI.
<p>
The Advanced collection contains the following elements:
<table width=100%>
  <tr>
    <th>Name</th>
    <th>Type</th>
    <th>Image Height</th>
    <th>Image Width</th>
    <th>Notes</th>
  </tr>
  <tr>
    <td>application_name</td>
    <td>String</td>
    <td></td>
    <td></td>
    <td>Displayed in title bar.</td>
  <tr>
  <tr>
    <td>application_icon</td>
    <td><a href=\"#Icon\">Icon</a></td>
    <td>16px</td>
    <td>16px</td>
    <td>Taskbar icon.</td>
  <tr>
  <tr>
    <td>application_disconnected_icon</td>
    <td><a href=\"#Icon\">Icon</a></td>
    <td>16px</td>
    <td>16px</td>
    <td>Taskbar icon when disconnected.</td>
  <tr>
  <tr>
    <td>application_snooze_icon</td>
    <td><a href=\"#Icon\">Icon</a></td>
    <td>16px</td>
    <td>16px</td>
    <td>Taskbar icon when snoozing.</td>
  <tr>
  <tr>
    <td>application_logo</td>
    <td>String</td>
    <td>50px</td>
    <td>50px</td>
    <td>Application logo that appears in the about box.</td>
  <tr>
  <tr>
    <td>company_name</td>
    <td>String</td>
    <td></td>
    <td></td>
    <td>Company associated with the release of the client software package.</td>
  <tr>
  <tr>
    <td>company_website</td>
    <td>String</td>
    <td></td>
    <td></td>
    <td>Company website.</td>
  <tr>
  <tr>
    <td>project_name</td>
    <td>String</td>
    <td></td>
    <td></td>
    <td>Shorter version of the application name, or a specific project who sponsored the client package.</td>
  <tr>
  <tr>
    <td>open_tab</td>
    <td>Number</td>
    <td></td>
    <td></td>
    <td>Which tab to open by default in the Advanced GUI. A value of '0' means open the last tab used.</td>
  <tr>
  <tr>
    <td>exit_message</td>
    <td>String</td>
    <td></td>
    <td></td>
    <td>What message should be sent to the user when they close BOINC manager.</td>
  <tr>
</table>
<p>
The Advanced collection is described as follows:
"; block_start(); echo "
&lt;advanced&gt;
    &lt;application_name /&gt;
    &lt;application_icon /&gt;
    ...
&lt;/advanced&gt;
"; block_end(); echo "
<p>


<h4><a name=\"Wizards\">Wizards</a></h4>
<p>
The Wizards collection consists of two elements which are broken out into the attach 
to project wizard and the attach to account manager wizard.
<p>
The attach to project element is descibed like this:
"; block_start(); echo "
&lt;attach_to_project&gt;
    &lt;title&gt;Attach to Project&lt;/title&gt;
    &lt;logo&gt;graphic/logo.png&lt;/logo&gt;
&lt;/attach_to_project&gt;
"; block_end(); echo "
<p>
The title is displayed in the wizard's title bar.
<p>
The logo is played on each wizard page and should have a height of 280px and a width of 115px. 
logos can be any of the following types: PNG, JPG, GIF, and BMP. The logo location should be 
specified as a path relative to the skin.xml description file. The path seperator should be a 
'/' for all platforms.
<p>
The attach to account manager element is descibed like this:
"; block_start(); echo "
&lt;attach_to_account_manager&gt;
    &lt;title&gt;Attach to Account Manager&lt;/title&gt;
    &lt;logo&gt;graphic/logo.png&lt;/logo&gt;
    &lt;account_info_message&gt;&lt;/account_info_message&gt;
&lt;/attach_to_account_manager&gt;
"; block_end(); echo "
<p>
The title is displayed in the wizard's title bar.
<p>
The logo is played on each wizard page and should have a height of 280px and a width of 115px. 
logos can be any of the following types: PNG, JPG, GIF, and BMP. The logo location should be 
specified as a path relative to the skin.xml description file. The path seperator should be a 
'/' for all platforms.
<p>
The account_info_message is text that is displayed when the user is asked for 
username/email address and password information for the account manager.
<p>
The Wizards collection is described as follows:
"; block_start(); echo "
&lt;wizards&gt;
    &lt;attach_to_project /&gt;
    &lt;attach_to_account_manager /&gt;
    ...
&lt;/wizards&gt;
"; block_end(); echo "
<p>


<h3><a name=\"SkinBasicElements\">Skin Basic Elements</a></h3>

<h4><a name=\"Image\">Image</a></h4>
<p>
Images are used for backgrounds and miscellaneous visual elements.
<p>
Images are described as follows:
"; block_start(); echo "
&lt;image&gt;
    &lt;imagesrc&gt;graphics/image.jpg&lt;/imagesrc&gt;
    [ &lt;background_color&gt;255:0:255&lt;/background_color&gt; ]
&lt;/image&gt;
"; block_end(); echo "
<p>
imagesrc describes the name and location of the image in question. Images 
can be any of the following types: PNG, JPG, GIF, and BMP. The image location
should be specified as a path relative to the skin.xml description file. The
path seperator should be a '/' for all platforms.
<p>
background_color is optional and describes the backgrond color that 
should be painted on to the dialog before the image is drawn over the top
of it. The color is represented as an RGB value with the token being ':'.
<p>


<h4><a name=\"Icon\">Icon</a></h4>
<p>
These elements are used to describe the taskbar icons in various states.
<p>
Icons are described as follows:
"; block_start(); echo "
&lt;image&gt;
    &lt;imagesrc&gt;graphics/image.jpg&lt;/imagesrc&gt; 
    &lt;transparency_mask&gt;255:0:255&lt;/transparency_mask&gt;
&lt;/image&gt;
"; block_end(); echo "
<p>
imagesrc describes the name and location of the image in question. Images 
can be any of the following types: PNG, JPG, GIF, and BMP. The image location
should be specified as a path relative to the skin.xml description file. The
path seperator should be a '/' for all platforms.
<p>
transparency_mask describes the backgrond color that should be used as the
transparency mask. The color is represented as an RGB value with the token 
being ':'.
<p>


<h4><a name=\"SimpleTab\">Simple Tab</a></h4>
<p>
The different types of simple tabs represent the different states an active
task can be displayed as.
<p>
Tabs are described as follows:
"; block_start(); echo "
&lt;tab&gt;
    &lt;imagesrc&gt;graphic/icon.png&lt;/imagesrc&gt;
    &lt;border_color&gt;51:102:102&lt;/border_color&gt;
    &lt;gradient_from_color&gt;51:102:102&lt;/gradient_from_color&gt;
    &lt;gradient_to_color&gt;134:179:176&lt;/gradient_to_color&gt;
&lt;/tab&gt;
"; block_end(); echo "
<p>
imagesrc describes the name and location of the image in question. Images 
can be any of the following types: PNG, JPG, GIF, and BMP. The image location
should be specified as a path relative to the skin.xml description file. The
path seperator should be a '/' for all platforms.
<p>
border_color describes the color that surrounds the tab. The color is 
represented as an RGB value with the token being ':'.
<p>
gradient_from_color describes the color that should start the gradient
effect. The color is represented as an RGB value with the token being ':'.
<p>
gradient_to_color describes the color that should finish the gradient
effect. The color is represented as an RGB value with the token being ':'.
<p>


<h4><a name=\"SimpleLink\">Simple Link</a></h4>
<p>
Links are images that cause an action to happen. Links do not change state
when clicked.
<p>
Links are described as follows:
"; block_start(); echo "
&lt;image&gt;
    &lt;imagesrc&gt;graphic/image.png&lt;/imagesrc&gt;
&lt;/image&gt;
"; block_end(); echo "
<p>
imagesrc describes the name and location of the image in question. Images 
can be any of the following types: PNG, JPG, GIF, and BMP. The image location
should be specified as a path relative to the skin.xml description file. The
path seperator should be a '/' for all platforms.
<p>


<h4><a name=\"SimpleButton\">Simple Button</a></h4>
<p>
Buttons are images that cause an action to happen. Buttons can be at rest or in
a clicked state. When the button is clicked it changes to the clicked image.
<p>
Buttons are described as follows:
"; block_start(); echo "
&lt;button&gt;
    &lt;imagesrc&gt;graphic/button.png&lt;/imagesrc&gt;
    &lt;imagesrc_clicked&gt;graphic/button_clicked.png&lt;/imagesrc_clicked&gt;
&lt;/button&gt;
"; block_end(); echo "
<p>
imagesrc describes the name and location of the image in question. Images 
can be any of the following types: PNG, JPG, GIF, and BMP. The image location
should be specified as a path relative to the skin.xml description file. The
path seperator should be a '/' for all platforms.
<p>
imagesrc_clicked describes the name and location of the image in question. Images 
can be any of the following types: PNG, JPG, GIF, and BMP. The image location
should be specified as a path relative to the skin.xml description file. The
path seperator should be a '/' for all platforms.
<p>

";
page_tail();
?>
