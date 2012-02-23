   ________)             ________)                  
  (, /     /)           (, /         ,           /) 
    /___, // _   _        /___, __      _ __   _(/  
 ) /     (/_(_(_(_/_   ) /     / (__(__(/_/ (_(_(_  
(_/            .-/    (_/                           
              (_/                                   
-----------------------------------------------------
Flag Friend utilizes the Flag module's API in order to provide a lightweight 
buddy system.

Installation Instructions

    * Download the module and place in your /sites/all/modules/contrib or 
      wherever you normally install your contrib modules.
    * Go to /admin/build/modules and enable Flag and Flag Friend in the "Flags"
      category/fieldset.
    * Go to /admin/build/flags and enable the 'friend' flag.
    * Some default values are entered for you in this flag's configuration. 
      Feel free to modify what is available.
    * If you have Views installed, you should see a new tab when you go to your
      /user/N page titled 'Friends'. Underneath you will have two sub-tabs 
      titled 'Flagged' and 'Pending'. Feel free to modify these views as you 
      see fit, but I wouldn't mess too much with the arguments and/or 
      relationships unless you really know what you are doing.
          o 'Friends' lists your current friend relationships.
          o 'Flagged' lists other users you have flagged and are awaiting a response from.
          o 'Pending' lists other users that have flagged you are are awaiting your response.

Depends on: Flag 6.x-1 but will also work with Flag's 6.x-2 branch.
Support for: Activity2, Popups API, Token, Views2, Advanced Profile Kit, Chaos Tools
There is also a sub-module included for access control called Flag Friend Access.