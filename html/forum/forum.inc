<?php

require_once('../include/database.inc');

class Category {
	var $id;
	var $name;

	function Category($id = -1, $name = '') {
		$vars = get_class_vars('Category');
		foreach ($vars as $var => $value)
			$this->$var = $$var;
	}

	function getForums() {
		$sql = 'SELECT * FROM forum WHERE category = '.$this->id.' ORDER BY orderID ASC';
		return sql_query($sql);
	}
}

class Forum {
	var	$id;
	var $category;
	var $orderID;
	var $title;
	var $description;
	var $timestamp;
	var $threads;
	var $posts;

	function Forum($id = -1, $category = -1, $orderID = -1, $title = '', $description = '', $timestamp = 0, $threads = 0, $posts = 0) {
		$vars = get_class_vars('Forum');
		foreach ($vars as $var => $value)
			$this->$var = $$var;
	}

	function getThreads($min = -1, $nRec = -1) {
		$sql = 'SELECT * FROM thread WHERE forum = '.$this->id.' ORDER BY timestamp DESC';
		if ($min > -1) {
			$sql .= ' LIMIT '.$min;
			if ($nRec > -1)
				$sql .= ', '.$nRec;
		} elseif ($nRec > -1)
			$sql .= ' LIMIT '.$nRec;
		return sql_query($sql);
	}
}

class Thread {
	var $id;
	var $forum;
	var $owner;

	var $title;

	var $timestamp;
	var $views;
	var $replies;

	function Thread($id = -1, $forum = -1, $owner = -1, $title = '', $timestamp = 0, $views = 0, $replies = 0) {
		$vars = get_class_vars('Thread');
		foreach ($vars as $var => $value)
			$this->$var = $$var;
	}

	function getPosts($min = -1, $nRec = -1) {
		$sql = 'SELECT * FROM post WHERE thread = '.$this->id.' ORDER BY id ASC';
		if ($min > -1) {
			$sql .= ' LIMIT '.$min;
			if ($nRec > -1)
				$sql .= ', '.$nRec;
		} elseif ($nRec > -1)
			$sql .= ' LIMIT '.$nRec;
		return sql_query($sql);
	}

	function post($content) {
		$title = addslashes(htmlentities($this->title));
		$content = addslashes(htmlentities($content));

		$sql = "INSERT INTO thread (forum, owner, title, timestamp) VALUES (".$this->forum.", ".$this->owner.", '".$title."', UNIX_TIMESTAMP())";
		$result = sql_query($sql);
		if (!$result)
			return false;
		$this->id = sql_insert_id();

		$post = new Post();
		$post->thread = $this->id;
		$post->user = $this->owner;
		$post->content = $content;
		$postID = $post->insert();

		$sql = "UPDATE user SET posts = posts + 1 WHERE id = ".$this->owner." LIMIT 1";
		sql_query($sql);

		$sql = "UPDATE forum SET threads = threads + 1, posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = ".$this->forum." LIMIT 1";
		sql_query($sql);
		
		return $thread->id;
	}

	function reply($user = -1, $content = "") {
		$content = addslashes(htmlentities($content));

		$post = new Post();
		$post->thread = $this->id;
		$post->user = $user;
		$post->content = $content;
		$postID = $post->insert();

		$sql = "UPDATE user SET posts = posts + 1 WHERE id = ".$user." LIMIT 1";
		sql_query($sql);

		$sql = "UPDATE thread SET replies = replies + 1, timestamp = UNIX_TIMESTAMP() WHERE id = ".$this->id." LIMIT 1";
		sql_query($sql);

		$sql = "UPDATE forum SET posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = ".$this->forum." LIMIT 1";
		sql_query($sql);
	}

	function incView() {
		$sql = "UPDATE thread SET views = views + 1 WHERE id = ".$this->id." LIMIT 1";
		sql_query($sql);
	}
}

class Post {
	var $id;
	var $thread;
	var $user;
	var $timestamp;
	var $content;

	function Post($id = -1, $thread = -1, $user = -1, $timestamp = 0, $content = "") {
		$vars = get_class_vars('Post');
		foreach ($vars as $var => $value)
			$this->$var = $$var;
	}

	function insert() {
		$sql = "INSERT INTO post (thread, user, timestamp, content) VALUES (".$this->thread.", ".$this->user.", UNIX_TIMESTAMP(), '".$this->content."')";
		$result = sql_query($sql);
		if (!$result)
			return false;
		return ($this->id = sql_insert_id());
	}
}

/* group database functions */

function getCategories() {
	$langID = (!empty($_SESSION['lang']['id']))?$_SESSION['lang']['id']:1;
	$sql = "SELECT * FROM category WHERE lang = ".$langID." ORDER BY orderID ASC";
	return sql_query($sql);
}

function getNextCategory($result) {
	$category = sql_fetch_array($result);
	if (!$category)
		return false;
	foreach ($category as $var => $value)
		$category[$var] = stripslashes($value);
	return new Category($category['id'], $category['name']);
}

function getNextForum($result) {
	$forum = sql_fetch_array($result);
	if (!$forum)
		return false;
	foreach ($forum as $var => $value)
		$forum[$var] = stripslashes($value);
	return new Forum($forum['id'], $forum['category'], $forum['orderID'], $forum['title'], $forum['description'], $forum['timestamp'], $forum['threads'], $forum['posts']);
}

function getNextThread($result) {
	$thread = sql_fetch_array($result);
	if (!$thread)
		return false;
	foreach ($thread as $var => $value)
		$thread[$var] = stripslashes($value);
	return new Thread($thread['id'], $thread['forum'], $thread['owner'], $thread['title'], $thread['timestamp'], $thread['views'], $thread['replies']);
}

function getNextPost($result) {
	$post = sql_fetch_array($result);
	if (!$post)
		return false;
	foreach ($post as $var => $value)
		$post[$var] = stripslashes($value);
	return new Post($post['id'], $post['thread'], $post['user'], $post['timestamp'], $post['content']);
}

/* specific database functions */

function getForum($forumID) {
	$sql = "SELECT * FROM forum WHERE id = ".$forumID;
	return getNextForum(sql_query($sql));
}

function getThread($threadID) {
	$sql = "SELECT * FROM thread WHERE id = ".$threadID;
	return getNextThread(sql_query($sql));
}

?>