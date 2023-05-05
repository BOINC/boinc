/*
 * Boinc Attach - (c) 2005 Yuri Dario
 *
 * Attach a new project to boinc client
*/

say 'Enter project URL: e.g. http://setiathome.berkeley.edu/'
url = linein()

say 'Enter project key: e.g. 9d48790f3c4e8fe255cf61f1b7a12480'
key = linein()

'boinc_client.exe -attach_project' url key

say 'Your project has been registered.'
say 'Now install your project client code.'

'pause'
