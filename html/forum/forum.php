<?php
require_once('../include.php');
require_once('forum.inc');
doHeader('Forum');

// Number of forum topics per page.
$n = 50;

/* sanitize variable */
$_GET['id'] = stripslashes(strip_tags($_GET['id']));
// Is this sanitization stuff really necessary?
$_GET['sort'] = stripslashes(strip_tags($_GET['sort']));

if (!array_key_exists('start', $_GET) || $_GET['start'] < 0)
	$_GET['start'] = 0;
        
$sort_style = $_GET['sort'];

if ($sort_style == NULL) {
    $sort_style = 'modified-new';
}
        
$forum = getForum($_GET['id']);
?>
<form action="forum.php" method="get">
<input type="hidden" name="id" value=<?php echo $forum->id ?>>
<table width=100% cellspacing=0 cellpadding=0>
  <tr valign="bottom">
    <td align="left" style="border:0px"> 

<p>
	<span class="title"><?php echo $forum->title ?></span>
	<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a>
</p>
<p><a href="post.php?id=<?php echo $_GET['id'] ?>">Post a New Thread / Question</a></p>
</td>
<td align="right" style="border:0px">
<select name="sort">
  <option <?php if ($sort_style == 'modified-new') echo 'selected' ?> value="modified-new">Most recent post first</option>
  <option <?php if ($sort_style == 'modified-old') echo 'selected' ?> value="modified-old">Least recent post first</option>
  <!--<option <?php if ($sort_style == 'activity-most') echo 'selected' ?> value="activity-most">Most recent activity first</option>-->
  <option <?php if ($sort_style == 'views-most') echo 'selected' ?> value="views-most">Most views first</option>
  <option <?php if ($sort_style == 'replies-most') echo 'selected' ?> value="replies-most">Most replies first</option>
</select>
<input type="submit" value="Sort">  
</td>
</tr></table>
</form>
<?php
if ($forum->threads > $n):
	$totalPages = floor($forum->threads / $n);
	$curPage = floor($_GET['start'] / $n);

	$pages = array(0, 1, 2);
	for ($i = -1 ; $i <= 1 ; $i++)
		if ($curPage + $i > 0 && $curPage + $i < $totalPages - 1)
			array_push($pages, $curPage + $i);
	for ($i = -3 ; $i <= -1 ; $i++)
		if ($totalPages + $i > 0)
			array_push($pages, $totalPages + $i);
	$pages = array_unique($pages);
	natsort($pages);
	$pages = array_values($pages);

	$gotoStr = '<p style="text-align:right">Goto page ';

	if ($curPage == 0)
		$gotoStr .= '<span style="font-size:larger; font-weight:bold">1</span>';
	else
		$gotoStr .= '<a href="forum.php?id='.$_GET['id'].'&start='.(($curPage-1)*$n).'">Previous</a> <a href="forum.php?id='.$_GET['id'].'&start=0">1</a>';

	for ($i = 1 ; $i < count($pages)-1 ; $i++) {
		if ($curPage == $pages[$i]) {
			$gotoStr .= ($i > 0 && $pages[$i-1] == $pages[$i] - 1)?', ':' ... ';
			$gotoStr .= '<span style="font-size:larger; font-weight:bold">'.($pages[$i]+1).'</span>';
		} else {
			$gotoStr .= ($i > 0 && $pages[$i-1] == $pages[$i] - 1)?', ':' ... ';
			$gotoStr .= '<a href="forum.php?id='.$_GET['id'].'&start='.($pages[$i]*$n).'">'.($pages[$i]+1).'</a>';
		}
	}

	if ($curPage == $totalPages-1)
		$gotoStr .= ', <span style="font-size:larger; font-weight:bold">'.$totalPages.'</span>';
	else
		$gotoStr .= ', <a href="forum.php?id='.$_GET['id'].'&start='.(($totalPages-1)*$n).'">'.$totalPages.'</a> <a href="forum.php?id='.$_GET['id'].'&start='.(($curPage+1)*$n).'">Next</a>';

	$gotoStr .= '</p>';

	echo $gotoStr;
endif;
?>

<p>
	<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
		<tr>
			<th>Titles</th>
			<th style="width: 50px">Replies</th>
			<th style="width: 150px">Author</th>
			<th style="width: 50px">Views</th>
			<th style="width: 170px">Last Post</th>
		</tr>
		<?php
		$threads = $forum->getThreads($_GET['start'], $n, $sort_style);
		while($thread = getNextThread($threads)):
			$user = getUser($thread->owner);
			?>
			<tr style="font-size:8pt; text-align:center">
				<td class="col1" style="font-size:10pt; text-align:left"><a href="thread.php?id=<?php echo $thread->id ?>"><b><?php echo stripslashes($thread->title) ?></b></a></td>
				<td class="col2"><?php echo $thread->replies ?></td>
				<td class="col3"><a href="../show_user.php?userid=<?php echo $thread->owner ?>"><?php echo $user->name ?></a></td>
				<td class="col2"><?php echo $thread->views ?></td>
				<td class="col3" style="text-align:right"><?php echo date('D M j, Y g:i a', $thread->timestamp) ?></td>
			</tr>
			<?php
		endwhile;
		?>
	</table>
</p>

<?php
if ($forum->threads > $n)
	echo $gotoStr;
?>

<?php
doFooter();
?>