
-------------------------------------------------------------------------------
Backup and Migrate 2 for Drupal 6.x
  by Ronan Dowling, Gorton Studios - ronan (at) gortonstudios (dot) com
-------------------------------------------------------------------------------

DESCRIPTION:
This module makes the task of backing up your Drupal database and migrating data
from one Drupal install to another easier. It provides a function to backup the
entire database to file or download, and to restore from a previous backup. You
can also schedule the backup operation. Compression of backup files is also
supported.

There are options to exclude the data from certain tables (such as cache or
search index tables) to increase efficiency by ignoring data that does not need
to be backed up or migrated.

The backup files are a list of SQL statements which can be executed with a tool
such as phpMyAdmin or the command-line mysql client.

-------------------------------------------------------------------------------

INSTALLATION:
* Put the module in your drupal modules directory and enable it in 
  admin/build/modules. 
* Go to admin/user/permissions and grant permission to any roles that need to be 
  able to backup or restore the databse.
* Configure and use the module at admin/content/backup_migrate

OPTIONAL:
* Enable token.module to allow token replacement in backup file names.
* To Backup to Amazon S3:
    - Download the S3 library from http://undesigned.org.za/2007/10/22/amazon-s3-php-class
      and place the file 'S3.php' in the includes directory in this module.
      The stable version (0.4.0 – 20th Jul 2009) works best with Backup and Migrate.

LIGHTTPD USERS:
Add the following code to your lighttp.conf to secure your backup directories:
  $HTTP["url"] =~ "^/sites/default/files/backup_migrate/" {
       url.access-deny = ( "" )
  }
You may need to adjust the path to reflect the actual path to the files.

IIS 7 USERS:
Add the following code to your web.config code to secire your backup directories:
<rule name="postinst-redirect" stopProcessing="true">
   <match url="sites/default/files/backup_migrate" />
   <action type="Rewrite" url=""/>
</rule>
You may need to adjust the path to reflect the actual path to the files.

-------------------------------------------------------------------------------

VERY IMPORTANT SECURITY NOTE:
Backup files may contain sensitive data and by default, are saved to your web
server in a directory normally accessible by the public. This could lead to a
very serious security vulnerability. Backup and Migrate attempts to protect
backup files using a .htaccess file, but this is not guaranteed to work on all
environments (and is guaranteed to fail on web servers that are not apache). You
should test to see if your backup files are publicly accessible, and if in doubt
do not save backups to the server, or use the destinations feature to save to a 
folder outside of your webroot.

OTHER WARNINGS:
A failed restore can destroy your database and therefore your entire Drupal
installation. ALWAYS TEST BACKUP FILES ON A TEST ENVIRONMENT FIRST. If in doubt
do not use this module.

This module has only be tested with MySQL and does not work with anyother dbms. 
If you have experiences with Postres or any other dbms and are willing to help 
test and modify the module to work with it, please contact the developer at 
ronan (at) gortonstudios (dot) com.

Make sure your php timeout is set high enough to complete a backup or restore
operation. Larger databases require more time. Also, while the module attempts
to keep memory needs to a minimum, a backup or restore will require
significantly more memory then most Drupal operations.

If your backup file contains the 'sessions' table all other users will be logged
out after you run a restore. To avoid this, exclude the sessions table when 
creating your backups. Be aware though that you will need to recreate the 
sessions table if you use this backup on an empty database.

Do not change the file extension of backup files or the restore function will be
unable to determine the compression type the file and will not function
correctly.

IF A RESTORE FAILS:
Don't panic, the restore file should work with phpMyAdmin's import function, or
with the mysql command line tool. If it does not, then it is likely corrupt; you
may panic now. MAKE SURE THAT THIS MODULE IS NOT YOUR ONLY FORM OF BACKUP.

-------------------------------------------------------------------------------

