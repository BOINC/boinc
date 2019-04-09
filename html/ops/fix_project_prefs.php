#!/usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// repair user.project_prefs field (does not validate project_prefs of all users!)
// * squash duplicate app_id entries (and optionally delete deprecated appids)
// * delete empty venue tags
// * migrate preset from attribute to tag (Drupal specific)
//
// produces lots of output in dry_run mode so always pipe it through less

require_once("../inc/boinc_db.inc");

// check your scheduler code if <apps_selected> is supported before turning this on
define('ADD_APPS_SELECTED_BLOCK', false);
// removes all currently deprecated apps from project_prefs
define('DELETE_DEPRECATED_APPS', false);

// test on all users first then change to false!
$dry_run = true;

// delete duplicate app_id entries and delete old ones
// returns an array of unique app_id entries
//
function clean_appids($appids) {
    $apps = BoincApp::enum("deprecated=0");
    $current_appids = array();
    foreach ($apps as $app) {
        $current_appids[] = $app->id;
    }
    $uniq = array_unique($appids);
    sort($uniq);
    if (DELETE_DEPRECATED_APPS) {
        $uniq = array_intersect($uniq, $current_appids);
    }
    return $uniq;
}

// extracts all app_id elements below $node, removes them
// and adds only the elements that remain after cleaning
// may add a surrounding apps_selected block if needed
//
function handle_project_specific_block($node) {
    $asnode = $node->getElementsByTagName("apps_selected");
    $length = $asnode->length;
    if ($length == 1) {
        $node = $asnode->item(0);
    } else if ($length == 0) {
        if (ADD_APPS_SELECTED_BLOCK) {
            // create a new apps_selected block and move app_id tags in there
            $asnode = $node->ownerDocument->createElement('apps_selected');
            $asnode = $node->appendChild($asnode);
            $appids = $node->getElementsByTagName("app_id");
            // changing the tree in a loop has strange results so do it separately
            $to_remove = array();
            $ids = array();
            foreach($appids as $elem) {
                $ids[] = $elem->nodeValue;
                $to_remove[] = $elem;
            }
            foreach ($to_remove as $elem) {
                $oldnode = $node->removeChild($elem);
            }
            foreach ($ids as $id) {
                $elem = $node->ownerDocument->createElement('app_id', $id);
                $asnode->appendChild($elem);
            }
            $node = $asnode;
        }
    } else {
        echo "more than one apps_selected block found; Exiting\n";
        exit(1);
    }
    $appids = $node->getElementsByTagName("app_id");
    if ($appids->length == 0 ) return;
    // changing the tree in a loop has strange results so do it separately
    $to_remove = array();
    $ids = array();
    foreach ($appids as $elem) {
        $ids[] = $elem->nodeValue;
        $to_remove[] = $elem;
    }
    foreach ($to_remove as $elem) {
        $oldnode = $node->removeChild($elem);
    }
    $cleaned = clean_appids($ids);
    foreach ($cleaned as $id) {
        $elem = $node->ownerDocument->createElement('app_id', $id);
        $node->appendChild($elem);
    }
}

// read project specific prefs into a DOMDocument tree and find nodes to fix
// returns fixed XML or false if something failed
function repair_prefs($prefs) {
    $prefs_dom = new DOMDocument();
    $prefs_dom->preserveWhiteSpace = false;
    $prefs_dom->formatOutput = true;
    $prefs_dom->validateOnParse = true;
    if (!$prefs_dom->loadXML($prefs)) {
        return false;
    }

    $root = $prefs_dom->firstChild;
    if ($root->hasChildNodes()) {
        $subNodes = $root->childNodes;
        $to_remove = array();
        foreach ($subNodes as $subNode) {
            // ignore empty text nodes which are basically whitespace from indentation (only relevant if preserveWhiteSpace is set to true above)
            if (($subNode->nodeType != XML_TEXT_NODE) || (($subNode->nodeType == XML_TEXT_NODE) &&(strlen(trim($subNode->wholeText))>=1))) {
                if ($subNode->nodeName == "venue") {
                    if (!$subNode->hasChildNodes()) {
                        // empty venue tag, clean this up too
                        $to_remove[] = $subNode;
                        echo "warning: empty venue tag removed\n";
                        continue;
                    }
                    if ($subNode->hasAttributes()) {
                        // transform old style preset attributes to tag
                        $preset = $subNode->getAttribute("preset");
                        if ($preset != "") {
                            $subNode->removeAttribute("preset");
                            $elem = $subNode->ownerDocument->createElement('preset', $preset);
                            $subNode->appendChild($elem);
                            echo "warning: transformed preset attribute to tag\n";
                        }
                    }
                    // check if venue has a project_specific subnode (its optional)
                    $psnode = $subNode->getElementsByTagName("project_specific");
                    if ($psnode->length == 1) {
                        handle_project_specific_block($psnode->item(0));
                    }
                }
                if ($subNode->nodeName == "project_specific") {
                    handle_project_specific_block($subNode);
                }
            }
        }
        foreach ($to_remove as $elem) {
            $oldnode = $root->removeChild($elem);
        }
    }

    return $prefs_dom->saveXML($prefs_dom->documentElement);
}

function process_set($users) {
    global $dry_run;
    foreach ($users as $user) {
        if (!$user->project_prefs) {
            //echo "repair not needed for user $user->id\n";
            continue;
        }
        // only parse XML if it contains something we want to fix
        if (strstr($user->project_prefs, "app_id")
            || strstr($user->project_prefs, "preset=")) {

            echo "repair started for user $user->id\n";
            $p = repair_prefs($user->project_prefs);
            if ($p) {
                if (!$dry_run) {
                    if ($user->update("project_prefs='$p'")) {
                        echo "project_prefs repaired and updated for user $user->id\n";
                    } else {
                        echo "failed to update project_prefs for user $user->id; Exiting\n";
                        exit(1);
                    }
                } else {
                    echo "repair succeeded for user $user->id\n";
                    echo "repaired prefs:\n$p\n";
                }
            } else {
                echo "repair failed for user $user->id\n";
                if ($dry_run) {
                    echo "original prefs:\n$user->project_prefs\n";
                }
            }
        }
    }
}

$n = 0;
$maxid = BoincUser::max("id");
if ($dry_run) {
    echo "Dry run only! No preferences will be updated\n";
}
while ($n <= $maxid) {
    $m = $n + 1000;
    $users = BoincUser::enum("id >= $n and id < $m");
    //echo "processing from $n\n";
    if (!$users) break;
    process_set($users);
    $n = $m;
}
?>
