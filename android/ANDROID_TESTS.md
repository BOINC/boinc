# BOINC Android GUI

* Choose-project wizard (access via the main menu or + icon in Projects screen). This lets you select a set of projects, then enter an email address and password. For each project:
  * If no account with that email exists, create one an attach to it.
  * If an account exists and the password doesn't match, show an error message.
  * If the project is down, retry later, and show the outcome in the Projects screen.
  * If an account exists and the password matches, attach to it.

  Test as many of these cases as possible.

* Add project using URL (access via the Choose-project wizard). This lets you enter a project URL, then an email address / password. Test as many of these cases as possible:
  * If you enter a URL that is not that of a BOINC project, you should get an error.
  * If you click "Forgot password?" you should go a web page asking for an email address, and then you should get an email address letting you reset your password.
  * If you click "Register" with the email address of an existing account, you should get an error.
  * If you click "Sign in" with an email address not that of an existing account, or the wrong password, you should get an error.
* Projects screen:
  * check project commands (update, suspend/resume, etc.) by tapping on a project and selecting menu item;
  * check that status text is updated accordingly.
  * check that information on project details screen is displayed correctly.
* Tasks screen:
  * check task commands (suspend/resume, abort) by tapping on a task and selecting menu item.
* Preferences screen:
  * verify that Autostart works (i.e. that BOINC starts up when the device is rebooted).
  * verify that Notifications works (the BOINC icon in the upper left hand of the screen should update when the client is running or suspended).
  * enable "Advanced preferences and controls", and verify that you see more preferences (e.g. CPU limits) in the preferences screen and more project commands (No New Tasks and Reset) in the Projects screen.
* Verify that orientation changes (screen rotations) result in expected behavior on all menus and screens.
* Verify that share/invite functions work as expected.
Verify that computing is suspended if battery charge is below 90%. This is shown in the Status tab, and the event log should have messages showing the charge level.
* Verify that computing is suspended if battery temperature exceeds 45 degrees Celsius (this may happen on some devices). This is shown in the Status tab, and the event log should have messages showing the temperature.
* Verify that the client properly detects when you are connected to USB or AC power (this is shown in the event log).
* Verify that the client properly detects when you are connected to a WiFi network (this is shown in the event log).

