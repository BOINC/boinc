$Id: README.txt,v 1.4 2008/11/04 18:55:59 karthik Exp $

og_notifications integrates OG with the notifications and messaging modules
family thereby enabling such features as group subscriptions, administrative
notifications etc.

The notifications and messaging modules extend beyond simple e-mail based
delivery systems and provide other avenues to contact recipients such as
private messages, simple alerts, and, if supported, even SMS. The delivery 
options are customisable by the end user.

INSTALLATION & CONFIGURATION
----------------------------
  * Install organic groups, messaging, token, notifications, notifications_lite,
  notifications_content and any other dependant modules prior to enabling
  og_notifications. Ensure that they are all up to date.

  * In addition to the above, install at least one messaging delivery module
  such as Simple Mail. It is also recommended that the notifications UI module
  is enabled to provide interface options.

  * Enable og_notifications. If this is an upgraded installation, all relevant
  data will be migrated over automatically.

  * The messaging and notifications modules can be configured via
  "admin/messaging". Besides all the generic options, settings particular to
  organic groups can be found in "admin/messaging/notifications/content".

  * The organic groups configuration page at "admin/og/og" contain further
  options for customising notification settings such as auto-subscription and
  default message templates.

  * Group pages now have a broadcast tab (previously the "e-mail" tab) which
  will allow privileged users to broadcast messages to all group members via
  the notifications module.  

  * End users can configure their individual preferences via their account 
  pages. These include auto-subscription and delivery options.

More information can be obtained from the documentation of the notification, 
messaging and other related modules.

CREDITS
-------
Authored by Karthik Kumar / Zen [ http://drupal.org/user/21209 ]
Sponsored by Kevin Millecam [Webwise Solutions]
