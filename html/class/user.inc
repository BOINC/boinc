<?php

class User {
	var $id;
	var $create_time;
	var $name;
	var $posts;
        var $has_profile;

	function User($id = -1, $create_time = 0, $name = "", $posts = -1, $has_profile = 0) {
		$vars = get_class_vars('User');
		foreach ($vars as $var => $value)
			$this->$var = $$var;
	}
}

function getUser($userID) {
	$sql = "SELECT * FROM user WHERE id = ".$userID;
	return getNextUser(sql_query($sql));
}

function getUserByAuth($auth) {
	$sql = "SELECT * FROM user WHERE authenticator = '$auth'";
	return getNextUser(sql_query($sql));
}

function getNextUser($result) {
	$user = sql_fetch_array($result);
	if (!$user)
		return false;
	foreach ($user as $var => $value)
            $user[$var] = stripslashes($value);
        
        // Check whether or not this user has a profile.
        $sql = "SELECT * FROM profile WHERE userid = " . $user['id'];
        $result2 = sql_query($sql);
        
        if (sql_num_rows($result2) > 0)
            $user['has_profile'] = 1;
        
        return new User($user['id'], $user['create_time'], $user['name'], $user['posts'], $user['has_profile']);
}


?>