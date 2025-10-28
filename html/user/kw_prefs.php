<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

require_once("../inc/util.inc");
require_once("../inc/keywords.inc");
require_once("../inc/kw_prefs.inc");

// Interactive page for editing keyword prefs
//
// The PHP generates rows for all keywords, and hides the appropriate ones.
// The Javascript hides and unhides rows as needed.
//
// element IDs for keyword K
//
// rowK: the <tr> row element
// textK: the plus or minus sign, or spaces
// radioK: the radio buttons

// for each kw K
// nprefs(K) = how many descendants of K have nonzero pref
// expanded(K): true if user chose to expand K
//      persists if ancestors are contracted;
//      doesn't persist if nprefs becomes nonzero
//
// actions:
// click on expand or contract: set or clear expanded(K)
// radio: recompute nprefs for all ancestors
//
// render:
//  for each terminal node K
//      if nprefs(parent)
//          unhide
//      else if all ancesters are either nprefs<>0 or expanded
//          unhide
//      else
//          hide
//
//  for each nonterminal node K
//      if nprefs(K)
//          expanded=true
//          unhide
//          button=none
//      else if nprefs(parent)
//          unhide
//          set button according to expanded
//      else if all ancestors are expanded
//          set button according to expanded
//      else
//          hide

// for each keyword K:
// - generate list of K's children
//
function keyword_setup($uprefs) {
    global $job_keywords;
    foreach ($job_keywords as $id=>$k) {
        $job_keywords[$id]->children = array();
        $job_keywords[$id]->expand = 0;
    }
    foreach ($job_keywords as $id=>$k) {
        if (!$k->parent) continue;
        $job_keywords[$k->parent]->children[] = $id;
    }
    foreach ($uprefs as $id=>$n) {
        while (1) {
            $id = $job_keywords[$id]->parent;
            if (!$id) break;
            $job_keywords[$id]->expand = true;
        }
    }
}

// output a keyword and (recursively) its descendants.
// If its parent is expanded, show it, else hide
//
function generate_html_kw($id, $uprefs) {
    global $job_keywords;
    $kw = $job_keywords[$id];
    $u = $uprefs[$id];
    $yes_checked = ($u == KW_YES)?"checked":"";
    $no_checked = ($u == KW_NO)?"checked":"";
    $maybe_checked = ($u == KW_MAYBE)?"checked":"";

    echo sprintf('<tr id="%s" hidden>%s', "row$id", "\n");
    echo sprintf('   <td width=50%% id="%s"></td>%s', "text$id", "\n");
    echo sprintf('   <td><input onclick="radio(%d, 1)" type="radio" name="%s" id="%s" value="%d" %s></td>%s',
        $id, "radio$id", "radio$id"."_0", KW_YES, $yes_checked, "\n"
    );
    echo sprintf('   <td><input onclick="radio(%d, 0)" type="radio" name="%s" id="%s" value="%d" %s></td>%s',
        $id, "radio$id", "radio$id"."_1", KW_MAYBE, $maybe_checked, "\n"
    );
    echo sprintf('   <td><input onclick="radio(%d, -1)" type="radio" name="%s" id="%s" value="%d" %s></td>%s',
        $id, "radio$id", "radio$id"."_2", KW_NO, $no_checked, "\n"
    );
    echo "</tr>\n";

    foreach ($kw->children as $k) {
        generate_html_kw($k, $uprefs);
    }
}

function generate_html_category($category, $uprefs) {
    global $job_keywords;
    row_heading_array(array(
        '',
        tra('Prefer'),
        tra('As needed'),
        tra('Never')),
        null,
        'bg-default'
    );
    foreach ($job_keywords as $id=>$k) {
        if ($k->category != $category) continue;
        if ($k->parent) continue;
        generate_html_kw($id, $uprefs);
    }
}

function generate_javascript($uprefs) {
    global $job_keywords;
    echo "<script>
        var ids = new Array;
        var names = new Array;
        var levels = new Array;
        var parent = new Array;
        var radio_value = new Array;
";
    foreach ($job_keywords as $id=>$k) {
        $val = $uprefs[$id];
        echo "
            names[$id] = '$k->name';
            levels[$id] = $k->level;
            parent[$id] = $k->parent;
            radio_value[$id] = $val;
            ids.push($id);
        ";
    }
    echo sprintf("var nkws = %d;\n", count($job_keywords));
    echo <<<EOT
    var rows = new Array;
    var texts = new Array;
    var expanded = new Array;
    var terminal = new Array;
    var nprefs = new Array;

    // initialize stuff

    for (i=0; i<nkws; i++) {
        terminal[ids[i]] = true;
        nprefs[ids[i]] = 0;
    }
    for (i=0; i<nkws; i++) {
        var j = ids[i];
        var rowid = "row"+j;
        var textid = "text"+j;
        rows[j] = document.getElementById(rowid);
        texts[j] = document.getElementById(textid);
        if (parent[j]) {
            terminal[parent[j]] = false;
        }
        expanded[j] = false;

        if (radio_value[j]) {
            k = j;
            while (1) {
                k = parent[k];
                if (!k) break;
                nprefs[k]++;
            }
        }
    }

    // Firefox doesn't set radio buttons correctly.
    //
    for (i=0; i<nkws; i++) {
        var j = ids[i];
        if (!terminal[j]) continue;
        for (k=0; k<3; k++) {
            id = "radio"+j+"_"+k;
            r = document.getElementById(id);
            //console.log(id, r);
            r.checked= (k == 1-radio_value[j]);
        }
    }

    var font_size = [120, 108, 92, 80];
    var indent = [0, 1.3, 2.6, 3.9];
    var button_indent = 1.0;

    // -1: show "contract" button
    // 0: no button
    // 1: show "expand" button
    //
    function set_expand(k, val) {
        //console.log('set_expand ', k, val);
        var level = levels[k];
        var x = '<span style="font-size:'+font_size[level]+'%">';
        x += '<span style="padding-left:'+indent[level]+'em"/>';
        if (val < 0) {
            x += '<a onclick="expand_contract('+k+')" id="t'+k+'" href=#/>&boxminus;</a> ';
        } else if (val == 0) {
            x += '<span style="padding-left:'+button_indent+'em"/>';
        } else {
            x += '<a onclick="expand_contract('+k+')" id="t'+k+'" href=#/>&boxplus;</a> ';
        }
        x += names[k];
        texts[k].innerHTML = x;
    }

    function radio(k, val) {
        //console.log('radio ', k, val, parent[k]);
        old_val = radio_value[k];
        radio_value[k] = val;
        inc = 0;
        if (val && !old_val) inc = 1;
        if (!val && old_val) inc = -1;
        if (inc) {
            while (1) {
                k = parent[k];
                if (!k) break;
                nprefs[k] += inc;
            }
        }
        render();
    }

    // click on expand/contract link
    //
    function expand_contract(k) {
        expanded[k] = !expanded[k];
        set_expand(k, expanded[k]?-1:1);
        var h = expanded[k]?false:true;
        //console.log('expand_contract ', k, h);
        render();
        return false;
    }

    // return true if all ancestrors of i are expanded or nprefs>0
    //
    function ancestors_expanded(i) {
        while (1) {
            i = parent[i];
            if (!i) break;
            if (!nprefs[i] && !expanded[i]) return false;
        }
        return true;
    }

    function render() {
        for (i=0; i<nkws; i++) {
            j = ids[i];
            if (terminal[j]) {
                set_expand(j, 0);
                if (nprefs[parent[j]]>0 || ancestors_expanded(j)) {
                    rows[j].hidden = false;
                } else {
                    rows[j].hidden = true;
                }
            } else {
                //console.log("nprefs ", j, nprefs[j]);
                if (nprefs[j]) {
                    expanded[j] = true;
                    rows[j].hidden = false;
                    set_expand(j, 0);
                } else {
                    p = parent[j];
                    if (p) {
                        if (nprefs[parent[j]]>0 || ancestors_expanded(j)) {
                            rows[j].hidden = false;
                            set_expand(j, expanded[j]?-1:1);
                        } else {
                            rows[j].hidden = true;
                        }
                    } else {
                        rows[j].hidden = false;
                        set_expand(j, expanded[j]?-1:1);
                    }
                }
            }
        }
    }

    render();

EOT;
    echo "</script>\n";
}

function prefs_edit_form($user, $show_saved) {
    global $job_keywords;

    [$yes, $no] = read_kw_prefs($user);

    // convert user prefs to a map id=>-1/0/1
    //
    $uprefs = array();
    foreach ($job_keywords as $id=>$kw) {
        $uprefs[$id] = 0;
    }
    foreach ($no as $id) {
        $uprefs[$id] = KW_NO;
    }
    foreach ($yes as $id) {
        $uprefs[$id] = KW_YES;
    }

    keyword_setup($uprefs);

    page_head(tra("Science and location preferences"));

    if ($show_saved) {
        echo '<span class="text-success">'.tra("Preferences saved.").'</span><p><p>';
    }

    echo tra("Select science areas and locations you do or don't want to support.  Click %1 for more detail, %2 for less.  When done, click the Save button at the bottom.",
        "<a>&boxplus;</a>",
        "<a>&boxminus;</a>"
    );
    echo "
        <p>
    ";
    form_start("kw_prefs.php");
    form_input_hidden('action', 'submit');
    echo "<h3>".tra("Science areas")."</h3>\n";
    start_table("table-striped");
    generate_html_category(KW_CATEGORY_SCIENCE, $uprefs);
    end_table();
    echo "<h3>".tra("Locations")."</h3>\n";
    start_table("table-striped");
    generate_html_category(KW_CATEGORY_LOC, $uprefs);
    end_table();
    echo sprintf(
        '<button type="submit" %s class="btn">%s</button>',
        button_style(),
        tra("Save")
    );
    form_end();
    generate_javascript($uprefs);
    page_tail();
}

function prefs_edit_action($user) {
    global $job_keywords;
    $yes = [];
    $no = [];
    foreach ($job_keywords as $id=>$kw) {
        $name = "radio$id";
        $val = get_int($name, true);
        switch ($val) {
        case KW_NO:
            $no[] = $id;
            break;
        case KW_YES:
            $yes[] = $id;
            break;
        }
    }
    write_kw_prefs($user, $yes, $no);
}

$user = get_logged_in_user();
$action = get_str('action', true);

if ($action == "submit") {
    prefs_edit_action($user);
    prefs_edit_form($user, true);
} else {
    prefs_edit_form($user, false);
}

?>
