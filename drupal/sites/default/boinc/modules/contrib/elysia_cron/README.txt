ELYSIA_CRON
by Eric Berdondini (gotheric)
<eric@void.it>

For installation instructions read INSTALL.TXT
For module developers API documetation read API.TXT

-----------------------------------------------------------------------------
FEATURES
-----------------------------------------------------------------------------

Elysia Cron extends Drupal standard cron, allowing a fine grain control over 
each task and several ways to add custom cron jobs to your site.

- Set the timings and frequencies of each cron task (you can run some jobs every
  day at a specified hour, other only monthly and so on...).
  For each task you can simply choose between some frequently used options 
  ("once a day", "once a month" ...), or use a powerful "linux crontab"-like 
  syntax to set the accurate timings. You can even define your frequently used 
  options to speed up site configuration.
- Parallel execution of cron task: you can group jobs in channels and execute 
  then simultaneously: so a task that takes a lot of time to execute won't block
  other tasks that need to be executed every 5 minutes...
- You can disable all tasks, an entire channel or a single task.
- Change the priority/order of task execution.
- Manual force the execution of a cron tasks.
- Detailed overview of cron status with time statistics for single tasks and 
  channels.

- Powerful API for module developers: you can define extra cron tasks for your 
  modules, each one with own default timings (site administrators can override 
  them by configuration, other modules via hook_alter). Elysia Cron 2.0 gives a 
  brand new API support (compatible with 1.0 version) with a lot of features.
- Administrators can define custom jobs (call to functions with parameters), via
  the "script" option.
- Several optimization for frequent cron calls and error handling.
- Protection from external cron calling by cron_key or allowed host list.

Elysia has no dependencies with contributed modules, and doesn't need to patch
the core: it can be used in minimal Drupal installation with only core modules.
It also can be used in a Drupal install profile.

3rd party integration:
- Ping feature, for external tracking services like host-tracker to tell whether
  cron is functioning properly on your site.
- Drush support: you can call "drush elysia-cron" to manually run extended cron.
- CTools support for exports/backup of task settings.
- Features support.

-----------------------------------------------------------------------------
USAGE EXAMPLES
-----------------------------------------------------------------------------

Elysia cron is usually used in large sites that needs performance optimization.

- Avoid drupal peak loads by distributing heavy load tasks during quiet periods 
  of the day: for example you may want to rebuild the XML Sitemap of your site 
  at 2:00AM in the morning, where usually only a few people are visiting your 
  site. You can even move some tasks to be executed only once a month (log 
  rotation, old records expiry...).

- If you have tasks that should be executed very often, but don't want to 
  execute ALL drupal cron tasks that often! For example, you may want to check 
  for emails that needs to be sent to your users every 2 minutes. Standard cron 
  is managed in a "monolithic" way, so even if you find out how to execute it 
  every 2 minutes, you will end in having all cron tasks executed so often, with
  a lot of performance problems.

- Fine tune cron cache management : drupal cron will invalidate variable cache 
  every cron run, and this is a great performance problem if you have a 
  frequently called task. Elysia cron optimize cache management, and doesn't 
  need to invalidate cache.

- Setup tasks that should be run at a precise time: for example if you want to 
  send a SimpleNews newsletter every monday at 9:00AM, you can do it.

- Parallel execution: if you have a task that takes a lot of time to execute, 
  you can setup a different channel for it so it won't block other tasks that 
  need to be executed every 5 minutes.

- Turn off (disable) a cron task/feature you don't need.

- Debug system cron problems. If your cron does not terminate correctly you can
  use extended logging, see at each cron timings and disable task to track down 
  the problem.

-----------------------------------------------------------------------------
CHANNELS
-----------------------------------------------------------------------------

Channels are groups of tasks. Each channel is a "parallel line" of execution
(= multiple channels can be executed simultaneously).
Tasks inside a channel will be executed sequentially (if they should).

WARNING: It's not recommended to create more than 2 or 3 channels.
Every channel will increase the delay between each cron check (of the same 
channel), because each cron call will cycle between all channels.
So, for example:
If you have 1 channel it will be checked once a minute.
If you have 2 channel each one will be checked every 2 minutes (almost usually, 
when the other one is running it will be checked once a minute).
It you have 10 channels there will be a check every 10 minutes... if you have
a job that should be executed every 5 minutes it won't do so!

-----------------------------------------------------------------------------
EXPORT VIA CTOOLS/FEATURES MODULE
-----------------------------------------------------------------------------

With 2.0 version of Elysia Cron you can use "Bulk Export" functionality of 
"Chaos Tools Suite" to export cron settings.
To use it simply enable all modules, go to Structure > Bulk Exporter and
select the tasks you want to export settings. You can also select all 
"elysia_cron" prefixed variables to export global options.
Than generate the module.

The generated code will set the new defaults of elysia cron settings. This way
you can simply enable it to use them, but you are free to override them in
the future using the normal settings page.
Note that if you want to delete all overrides, and return to exported settings,
you should do a "reset to defaults" from elysia cron settings page. 

You can also use "Features" module to create a Feature module will the settings
you need.
Note that if you want to delete the overridden settings it's preferable to use
the "reset to defaults" elysia cron's button.
You can use the "Revert components" Features's button, but this will reset also
all cron statistics (if you are not interested in them you can freely use that
button).

-----------------------------------------------------------------------------
DRUSH SUPPORT
-----------------------------------------------------------------------------

Elysia Cron 2.0 adds a simple support for Drush module.

You can execute the "elysia-cron" command to run cron.

-----------------------------------------------------------------------------
RULES AND SCRIPT SYNTAX
-----------------------------------------------------------------------------

1. FIELDS ORDER
---------------------------------

 +---------------- minute (0 - 59)
 |  +------------- hour (0 - 23)
 |  |  +---------- day of month (1 - 31)
 |  |  |  +------- month (1 - 12)
 |  |  |  |  +---- day of week (0 - 6) (Sunday=0)
 |  |  |  |  |
 *  *  *  *  *

Each of the patterns from the first five fields may be either * (an asterisk), 
which matches all legal values, or a list of elements separated by commas 
(see below).

For "day of the week" (field 5), 0 is considered Sunday, 6 is Saturday (7 is 
an illegal value)

A job is executed when the time/date specification fields all match the current 
time and date. There is one exception: if both "day of month" and "day of week" 
are restricted (not "*"), then either the "day of month" field (3) or the "day 
of week" field (5) must match the current day (even though the other of the two 
fields need not match the current day).

2. FIELDS OPERATOR
---------------------------------

There are several ways of specifying multiple date/time values in a field:

* The comma (',') operator specifies a list of values, for example: "1,3,4,7,8"
* The dash ('-') operator specifies a range of values, for example: "1-6", which 
  is equivalent to "1,2,3,4,5,6"
* The asterisk ('*') operator specifies all possible values for a field. For 
  example, an asterisk in the hour time field would be equivalent to 'every hour' 
  (subject to matching other specified fields).
* The slash ('/') operator (called "step") can be used to skip a given number of 
  values. For example, "*/3" in the hour time field is equivalent to 
  "0,3,6,9,12,15,18,21".

3. EXAMPLES
---------------------------------

 */15 * * * : Execute job every 15 minutes
 0 2,14 * * *: Execute job every day at 2:00 and 14:00
 0 2 * * 1-5: Execute job at 2:00 of every working day
 0 12 1 */2 1: Execute job every 2 month, at 12:00 of first day of the month OR
 at every monday.

4. SCRIPTS
---------------------------------

You can use the script section to easily create new jobs (by calling a php 
function) or to change the scheduling of an existing job.

Every line of the script can be a comment (if it starts with #) or a job 
definition.

The syntax of a job definition is:
<-> [rule] <ctx:CONTEXT> [job]

(Tokens betweens [] are mandatory)

* <->: a line starting with "-" means that the job is DISABLED.
* [rule]: a crontab schedule rule. See above.
* <ctx:CHANNEL>: set the channel of the task.
* [job]: could be the name of a supported job (for example: 'search_cron') or a
  function call, ending with ; (for example: 'process_queue();').

A comment on the line just preceding a job definition is considered the job 
description.

Remember that script OVERRIDES all settings on single jobs sections or channel 
sections of the configuration

5. EXAMPLE OF SCRIPT
---------------------------------

# Search indexing every 2 hours (i'm setting this as the job description)
0 */2 * * * search_cron

# I'll check for module status only on sunday nights 
# (and this is will not be the job description, see the empty line below)

0 2 * * 0 update_status_cron

# Trackback ping process every 15min and on a channel called "net"
*/15 * * * * ctx:net trackback_cron

# Disable node_cron (i must set the cron rule even if disabled)
- */15 * * * * node_cron

# Launch function send_summary_mail('test@test.com', false); every night
# And set its description to "Send daily summary"
# Send daily summary
0 1 * * *  send_summary_mail('test@test.com', false);

-----------------------------------------------------------------------------
CREDITS
-----------------------------------------------------------------------------

Elysia cron is a part of the Elysia project (but could be used stand alone
with no limitation).

Developing is sponsored by :
Void Labs s.n.c
http://www.void.it
