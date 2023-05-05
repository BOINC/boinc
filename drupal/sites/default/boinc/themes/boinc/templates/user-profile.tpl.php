<?php
// $Id: user-profile.tpl.php,v 1.2.2.2 2009/10/06 11:50:06 goba Exp $

/**
 * @file user-profile.tpl.php
 * Default theme implementation to present all user profile data.
 *
 * This template is used when viewing a registered member's profile page,
 * e.g., example.com/user/123. 123 being the users ID.
 *
 * By default, all user profile data is printed out with the $user_profile
 * variable. If there is a need to break it up you can use $profile instead.
 * It is keyed to the name of each category or other data attached to the
 * account. If it is a category it will contain all the profile items. By
 * default $profile['summary'] is provided which contains data on the user's
 * history. Other data can be included by modules. $profile['user_picture'] is
 * available by default showing the account picture.
 *
 * Also keep in mind that profile items and their categories can be defined by
 * site administrators. They are also available within $profile. For example,
 * if a site is configured with a category of "contact" with
 * fields for of addresses, phone numbers and other related info, then doing a
 * straight print of $profile['contact'] will output everything in the
 * category. This is useful for altering source order and adding custom
 * markup for the group.
 *
 * To check for all available data within $profile, use the code below.
 * @code
 *   print '<pre>'. check_plain(print_r($profile, 1)) .'</pre>';
 * @endcode
 *
 * Available variables:
 *   - $user_profile: All user profile data. Ready for print.
 *   - $profile: Keyed array of profile categories and their items or other data
 *     provided by modules.
 *
 * @see user-profile-category.tpl.php
 *   Where the html is handled for the group.
 * @see user-profile-item.tpl.php
 *   Where the html is handled for each item in the group.
 * @see template_preprocess_user_profile()
 */
?>
<?php
global $user;
drupal_set_title('Profile');
$account = user_load($account->uid);
$content_profile = content_profile_load('profile', $account->uid);
$name = check_plain($account->boincuser_name);
$boincid = check_plain($account->boincuser_id);
$join_date = date('d F Y', $account->created);
$country = check_plain($content_profile->field_country[0]['value']);
$website = check_plain($content_profile->field_url[0]['value']);
$background = check_markup($content_profile->field_background[0]['value'], $content_profile->format, FALSE);
$opinions = check_markup($content_profile->field_opinions[0]['value'], $content_profile->format, FALSE);
$user_links = array();
$user_links2l = array();
$profile_is_approved = ($content_profile->status AND !$content_profile->moderate);
$user_is_moderator = user_access('edit any profile content');
$is_own_profile = ($user->uid == $account->uid);

if ($user->uid AND ($user->uid != $account->uid)) {
  if (module_exists('private_messages')) {
  // if (function_exists('privatemsg_get_link')) {
    $user_links[] = array(
      'title' => bts('Send message', array(), NULL, 'boinc:private-message'),
      'href' => privatemsg_get_link(array($account))
    );
  }

  if (module_exists('friends')) {
    $flag = flag_get_flag('friend');
    $friend_status = flag_friend_determine_friend_status($flag, $account->uid, $user->uid);
    switch ($friend_status) {
    case FLAG_FRIEND_BOTH:
    case FLAG_FRIEND_FLAGGED:
      $user_links[] = array(
        'title' => bts('Remove friend', array(), NULL, 'boinc:friend-remove'),
        'href' => "flag/confirm/unfriend/friend/{$account->uid}"
      );
      break;
    case FLAG_FRIEND_PENDING:
      $user_links[] = array(
        'title' => bts('Cancel friend request', array(), NULL, 'boinc:friend-cancel'),
        'href' => "flag/confirm/unflag/friend/{$account->uid}"
      );
      break;
    case FLAG_FRIEND_APPROVAL:
       $user_links[] = array(
        'title' => bts('Approve friend request', array(), NULL, 'boinc:friend-approve'),
        'href' => "flag/confirm/flag/friend/{$account->uid}"
      );
      break;
    case FLAG_FRIEND_UNFLAGGED:
    default:
      $user_links[] = array(
        'title' => bts('Add as friend', array(), NULL, 'boinc:friend-add'),
        'href' => "flag/confirm/flag/friend/{$account->uid}"
      );
    }
  }

  if (module_exists('ignore_user')) {
    static $ignored;
    if ($user->uid != $account->uid && $account->uid != 0 && user_access('ignore user')) {
      if (!isset($ignored[$account->uid])) {
        $ignored[$account->uid] = db_result(db_query('SELECT COUNT(*) FROM {ignore_user} WHERE uid = %d AND iuid = %d', $user->uid, $account->uid));
      }
      if ($ignored[$account->uid] == 0) {
        $ignore_link = array(
          'title' => bts('Ignore user', array(), NULL, 'boinc:ignore-user-add'),
          'href' => 'account/prefs/privacy/ignore_user/add/'. $account->uid,
          'query' => 'destination='. $_GET['q'],
          'attributes' => array(
            'class' => 'ignore-user',
            'title' => bts('Add user to your ignore list', array(), NULL, 'boinc:ignore-user-add'),
          ),
        );
      }
      else {
        $ignore_link = array(
          'title' => bts('Stop Ignoring user', array(), NULL, 'boinc:ignore-user-add'),
          'href' => 'account/prefs/privacy/ignore_user/remove/'. $account->uid,
          'query' => 'destination='. $_GET['q'],
          'attributes' => array(
            'class' => 'ignore-user',
            'title' => bts('Remove user from your ignore list', array(), NULL, 'boinc:ignore-user-add'),
          ),
        );
      }
    $user_links2l[] = $ignore_link;
    }
  }

  if (user_access('assign community member role')
      OR user_access('assign all roles')) {
    if (array_search('community member', $account->roles)) {
      $user_links2l[] = array(
        'title' => bts('Ban user', array(), NULL, 'boinc:moderate-ban-user'),
        'href' => "moderate/user/{$account->uid}/ban"
      );
    }
    else {
      $user_links2l[] = array(
        'title' => bts('Lift user ban', array(), NULL, 'boinc:moderate-unban-user'),
        'href' => "user_control/{$account->uid}/lift-ban"
      );
    }
  }
}

?>
<div class="user-profile">
  <div class="picture">
    <?php
      $user_image = boincuser_get_user_profile_image($account->uid, FALSE);
      print theme('imagefield_image', $user_image['image'], $user_image['alt'],
        $user_image['alt'], array(), false);
    ?>
  </div>
  <div class="general-info">
    <div class="name">
      <span class="label"></span>
      <span class="value"><?php print $name; ?></span>
    </div>
    <?php if ($account->status==1): ?>
      <div class="join-date">
        <span class="label"><?php print bts('Member since', array(), NULL, 'boinc:user-info'); ?>:</span>
        <span class="value"><?php print $join_date; ?></span>
      </div>
      <div class="country">
        <span class="label"><?php print bts('Country', array(), NULL, 'boinc:country-of-origin'); ?>:</span>
        <span class="value"><?php print $country; ?></span>
      </div>
      <div class="boincid">
        <span class="value"><?php print bts('BOINC ID', array(), NULL, 'boinc:boincid'); ?>:</span>
        <span class="value"><?php print $boincid; ?></span>
      </div>
      <?php if ($website AND ($profile_is_approved OR $user_is_moderator OR $is_own_profile)): ?>
        <div class="website">
          <span class="label"><?php print bts('Website', array(), NULL, 'boinc:website-of-user-of-team'); ?>:</span>
          <span class="value"><?php print l($website, (strpos($website, 'http') === false) ? "http://{$website}" : $website); ?></span>
        </div>
      <?php endif; ?>
      <?php if ($user->uid AND ($user->uid != $account->uid)): ?>
        <ul class="tab-list">
          <?php foreach ($user_links as $key => $link): ?>
            <li class="primary <?php print ($key == 0) ? 'first ' : ''; ?>tab<?php print ($key == count($user_links)-1) ? ' last' : ''; ?>">
              <?php print l($link['title'], $link['href'], array('query' => drupal_get_destination())); ?>
            </li>
          <?php endforeach; ?>
        </ul>
        <ul class="tab-list">
          <?php foreach ($user_links2l as $key => $link): ?>
            <li class="primary <?php print ($key == 0) ? 'first ' : ''; ?>tab<?php print ($key == count($user_links2l)-1) ? ' last' : ''; ?>">
              <?php print l($link['title'], $link['href'], array('query' => drupal_get_destination())); ?>
            </li>
          <?php endforeach; ?>
        </ul>
      <?php endif; ?>
    <?php endif; ?>
    <div class="clearfix"></div>
  </div>
  <?php if ($content_profile): ?>
    <?php if ($background OR $opinions): ?>
      <div class="bio">
        <?php if (!$profile_is_approved): ?>
          <div class="messages warning">
            <?php print bts('Profile awaiting moderator approval', array(), NULL, 'boinc:user-profile:-1:message-shown-when-awating-moderation'); ?>
          </div>
        <?php endif; ?>
        <?php if ($profile_is_approved OR $user_is_moderator OR $is_own_profile): ?>
          <?php if ($background): ?>
            <div class="background">
              <span class="label"><?php print bts('Background', array(), NULL, 'boinc:user-profile'); ?></span>
              <span class="value"><?php print $background; ?></span>
            </div>
          <?php endif; ?>
          <?php if ($opinions): ?>
            <div class="opinions">
              <span class="label"><?php print bts('Opinion', array(), NULL, 'boinc:user-profile'); ?></span>
              <span class="value"><?php print $opinions; ?></span>
            </div>
          <?php endif; ?>
        <?php endif; ?>
      </div>
    <?php endif; ?>
  <?php else: ?>
    <div class="messages warning">
      <?php print bts('Profile does not exist.', array(), NULL, 'boinc:user-profile:-1:message-shown-when-there-is-no-profile'); ?>
    </div>
  <?php endif; ?>
</div>
<?php
  $ddname = 'flag_abuse_reason-dropdown-user-' . $account->uid;
?>
<div class="dropdown">
  <div id="<?php print $ddname; ?>" class="dropdown-content">
    <?php print flag_create_link('abuse_user_1', $account->uid); ?>
    <?php print flag_create_link('abuse_user_2', $account->uid); ?>
    <?php print flag_create_link('abuse_user_3', $account->uid); ?>
    <?php print flag_create_link('abuse_user_4', $account->uid); ?>
    <?php print flag_create_link('abuse_user_5', $account->uid); ?>
  </div>
</div>
