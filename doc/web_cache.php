<?php
require_once("docutil.php");

page_head("Web page caching");
echo "
Some pages on your project's web site are
accessed often and require lots of database access to generate.
To keep this from bogging down your server,
BOINC caches these pages.
This cache is in PROJECT/html/cache/*;
a one-level hashed directory hierarchy is used
to deal with large-directory performance problems.

<h2>Caching configuration</h2>
<p>
The file html/project/cache_parameters.inc
contains a number of parameters related to caching:
";
list_start();
list_item("TEAM_PAGE_TTL", "Cache life of team pages; default 1 hour");
list_item("USER_PAGE_TTL", "Cache life of user pages; default 1 hour");
list_item("USER_HOST_TTL", "Cache life of user host list; default 1 hour");
list_item("USER_PROFILE_TTL", "Cache life of profiles; default 1 hour");
list_item("TOP_PAGES_TTL", "Cache life of user/team/host lists; default 12 hours");
list_item("INDEX_PAGE_TTL", "Cache life of main page; default 1 hour");
list_item("MAX_CACHE_USAGE", "Max cache size; default 100 MB");
list_item("MIN_FREE_SPACE", "Min free space on device; default 100 MB");
list_item("CACHE_SIZE_CHECK_FREQ", "Check cache size on every N user accesses to cached pages; default 1000");
list_end();
echo "
<h2>Caching and translation</h2>
BOINC uses several web-page caching systems,
which support language translation in different ways.

<ul>
<li>
<b>Pre-generated</b>:
Pages are updated from time to time, and do not support translation.
The system used for building profiles is a pre-generated cache.

<li>
<b>Fullpage cache</b>:
This cache system simply takes the output of a page
and saves it for the future.
It uses the <code>start_cache()</code> and
<code>end_cache()</code> functions in <code>cache.inc</code>.
The pages may not be translation-aware
(otherwise some users will see the wrong language).

<li>
<b>Fullpage cache with translation</b>:
You can make the language part of the cache filename.
(To do this, you need to adapt the code in cache.inc).
This can be inefficient because it stores a
separate copy of the page for each language.

<li>
<b>Object cache</b>:
This stores the data used to create the page
and recreates the page every time (using any language you'd like).
Use <code>get_cached_data()</code> in <code>cache.inc</code>.
This is perfect for pages that are accessed commonly
and by people from many nationalities (currently the top-X pages support it).
</ul>

<p>
If something shows up in the wrong language
it's probably because a page that was previously not being translated
got translation abilities but wasn't moved to the proper cache type.
Also if a page-piece that is included is now translatable,
all pages that make use of this piece should now either
use fullpage caching with translation,
or use an object cache (nicer but takes a few more lines of coding).

";

page_tail();
?>
