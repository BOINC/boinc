<?php

// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2020 University of California
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

// job/project/preference keywords, master list

// NEVER REUSE IDS

define('KW_CATEGORY_SCIENCE', 0);
define('KW_CATEGORY_LOC', 1);

define('KW_ASTRONOMY',      1);
define('KW_SETI',           2);
define('KW_PULSARS',        3);
define('KW_GW',             4);
define('KW_COSMOLOGY',      5);
define('KW_PHYSICS',        6);
define('KW_PARTICLE_PHYSICS', 7);
define('KW_NANOSCIENCE',    8);
define('KW_BIOMED',         9);
define('KW_DRUG_DISCOVERY', 10);
define('KW_PROTEINS',       11);
define('KW_GENETICS',       12);
define('KW_DISEASE',        13);
define('KW_CANCER',         14);
define('KW_MATH_CS',        15);
define('KW_AI',             16);
define('KW_OCEANIA',        17);
define('KW_AUSTRALIA',      18);

define('KW_EUROPE',         20);
define('KW_GERMANY',        21);
define('KW_ASIA',           22);
define('KW_AMERICAS',       23);
define('KW_US',             24);
define('KW_UCB',            25);
define('KW_AEI',            26);
define('KW_CERN',           27);
define('KW_UW',             28);
define('KW_EARTH_SCI',      29);
define('KW_SPAIN',          30);
define('KW_SAN_JORGE',      31);
define('KW_NUMBER_THEORY',  32);
define('KW_CRYPTO',         33);
define('KW_ENV_RESEARCH',   34);
define('KW_CLIMATE',        35);
define('KW_CZECH',          36);
define('KW_CHARLES_PRAGUE', 37);
define('KW_RECHENKRAFT',    38);
define('KW_RHEINMAIN',      39);
define('KW_HUNGARY',        40);
define('KW_IRELAND',        41);
define('KW_UC_DUBLIN',      42);
define('KW_POLAND',         43);
define('KW_RUSSIA',         44);
define('KW_SW_STATE_RUSSIA', 45);
define('KW_RAS',            46);
define('KW_PRBB',           47);
define('KW_UK',             48);
define('KW_OXFORD',         49);
define('KW_CHINA',          50);
define('KW_U_DAYTON',       51);
define('KW_WRIGHT_STATE',   52);
define('KW_USC',            53);
define('KW_FULLERTON',      54);
define('KW_ARIZONA_STATE',  55);
define('KW_U_ILLINOIS',     56);
define('KW_U_WARSAW',       57);
define('KW_RPI',            58);
define('KW_INTERNATIONAL',  59);
define('KW_UND',            60);
define('KW_HOLLAND',        61);
define('KW_CHEMISTRY',      62);
define('KW_GAMES',          63);
define('KW_VIRUS',          64);
define('KW_FRANCE',         65);
define('KW_CANADA',         66);
define('KW_PSU',            67);
define('KW_SU',             68);
define('KW_SINAI_U',        69);
define('KW_AFRICA',         70);
define('KW_EGYPT',          71);

$job_keywords = array();

function keyword($symbol, $category, $level, $parent, $name) {
    global $job_keywords;
    $x = (object)array(
        'category'=>$category,
        'level'=>$level,
        'parent'=>$parent,
        'name'=>$name,
        'symbol'=>$symbol
    );
    $job_keywords[constant($symbol)] = $x;
}

keyword('KW_ASTRONOMY', KW_CATEGORY_SCIENCE, 0, 0,
    'Astronomy'
);
    keyword('KW_SETI', KW_CATEGORY_SCIENCE, 1, KW_ASTRONOMY,
        'Search for Extraterrestrial Intelligence'
    );
    keyword('KW_PULSARS', KW_CATEGORY_SCIENCE, 1, KW_ASTRONOMY,
        'Detection and study of pulsars'
    );
    keyword('KW_GW', KW_CATEGORY_SCIENCE, 1, KW_ASTRONOMY,
        'Detection and study of gravitational waves'
    );
    keyword('KW_COSMOLOGY', KW_CATEGORY_SCIENCE, 1, KW_ASTRONOMY,
        'Cosmology'
    );
keyword('KW_CHEMISTRY', KW_CATEGORY_SCIENCE, 0, 0,
    'Chemistry'
);
keyword('KW_PHYSICS', KW_CATEGORY_SCIENCE, 0, 0,
    'Physics'
);
    keyword('KW_PARTICLE_PHYSICS', KW_CATEGORY_SCIENCE, 1, KW_PHYSICS,
        'Particle physics'
    );
    keyword('KW_NANOSCIENCE', KW_CATEGORY_SCIENCE, 1, KW_PHYSICS,
        'Nanoscience'
    );
keyword('KW_BIOMED', KW_CATEGORY_SCIENCE, 0, 0,
    'Biology and Medicine'
);
    keyword('KW_DRUG_DISCOVERY', KW_CATEGORY_SCIENCE, 1, KW_BIOMED,
        'Drug discovery'
    );
    keyword('KW_PROTEINS', KW_CATEGORY_SCIENCE, 1, KW_BIOMED,
        'Protein research'
    );
    keyword('KW_GENETICS', KW_CATEGORY_SCIENCE, 1, KW_BIOMED,
        'Genetics and Phylogeny'
    );
    keyword('KW_DISEASE', KW_CATEGORY_SCIENCE, 1, KW_BIOMED,
        'Disease research'
    );
        keyword('KW_CANCER', KW_CATEGORY_SCIENCE, 2, KW_DISEASE,
            'Cancer research'
        );
        keyword('KW_VIRUS', KW_CATEGORY_SCIENCE, 2, KW_DISEASE,
            'COVID-19 and virology'
        );
keyword('KW_MATH_CS', KW_CATEGORY_SCIENCE, 0, 0,
    'Mathematics and Computer Science'
);
    keyword('KW_NUMBER_THEORY', KW_CATEGORY_SCIENCE, 1, KW_MATH_CS,
        'Number Theory'
    );
    keyword('KW_CRYPTO', KW_CATEGORY_SCIENCE, 1, KW_MATH_CS,
        'Cryptography'
    );
    keyword('KW_GAMES', KW_CATEGORY_SCIENCE, 1, KW_MATH_CS,
        'Games'
    );
keyword('KW_AI', KW_CATEGORY_SCIENCE, 0, 0,
    'Artificial Intelligence and Cognitive Science'
);
keyword('KW_EARTH_SCI', KW_CATEGORY_SCIENCE, 0, 0,
    'Earth sciences'
);
    keyword('KW_ENV_RESEARCH', KW_CATEGORY_SCIENCE, 1, KW_EARTH_SCI,
        'Environmental research'
    );
    keyword('KW_CLIMATE', KW_CATEGORY_SCIENCE, 1, KW_EARTH_SCI,
        'Climate research'
    );

// Locations

keyword('KW_INTERNATIONAL', KW_CATEGORY_LOC, 0, 0,
    'International'
);
keyword('KW_EUROPE', KW_CATEGORY_LOC, 0, 0,
    'Europe'
);
    keyword('KW_CERN', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'CERN'
    );
    keyword('KW_CZECH', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Czech Republic'
    );
        keyword('KW_CHARLES_PRAGUE', KW_CATEGORY_LOC, 2, KW_CZECH,
            'Charles University in Prague'
        );
    keyword('KW_FRANCE', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'France'
    );
    keyword('KW_GERMANY', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Germany'
    );
        keyword('KW_AEI', KW_CATEGORY_LOC, 2, KW_GERMANY,
            'Albert Einstein Institute for Gravitational Physics'
        );
        keyword('KW_RECHENKRAFT', KW_CATEGORY_LOC, 2, KW_GERMANY,
            'Rechenkraft.net e.V'
        );
        keyword('KW_RHEINMAIN', KW_CATEGORY_LOC, 2, KW_GERMANY,
            'Hochschule RheinMain University of Applied Sciences'
        );
    keyword('KW_HOLLAND', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Holland'
    );
    keyword('KW_HUNGARY', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Hungary'
    );
    keyword('KW_IRELAND', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Ireland'
    );
        keyword('KW_UC_DUBLIN', KW_CATEGORY_LOC, 2, KW_IRELAND,
            'University College Dublin'
        );
    keyword('KW_POLAND', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Poland'
    );
        keyword('KW_U_WARSAW', KW_CATEGORY_LOC, 2, KW_POLAND,
            'University of Warsaw'
        );
    keyword('KW_RUSSIA', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Russia'
    );
        keyword('KW_SW_STATE_RUSSIA', KW_CATEGORY_LOC, 2, KW_RUSSIA,
            'Southwest State University, Russia'
        );
        keyword('KW_RAS', KW_CATEGORY_LOC, 2, KW_RUSSIA,
            'Russian Academy of Science'
        );
    keyword('KW_SPAIN', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Spain'
    );
        keyword('KW_SAN_JORGE', KW_CATEGORY_LOC, 2, KW_SPAIN,
            'San Jorge University, Zaragoza, Spain'
        );
        keyword('KW_PRBB', KW_CATEGORY_LOC, 2, KW_SPAIN,
            'Barcelona Biomedical Research Park'
        );
    keyword('KW_UK', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'United Kingdom'
    );
        keyword('KW_OXFORD', KW_CATEGORY_LOC, 2, KW_UK,
            'Oxford University'
        );
keyword('KW_ASIA', KW_CATEGORY_LOC, 0, 0,
    'Asia'
);
    keyword('KW_CHINA', KW_CATEGORY_LOC, 1, KW_ASIA,
        'China'
    );
keyword('KW_OCEANIA', KW_CATEGORY_LOC, 0, 0,
    'Oceania'
);
    keyword('KW_AUSTRALIA', KW_CATEGORY_LOC, 1, KW_OCEANIA,
        'Australia'
    );

keyword('KW_AFRICA', KW_CATEGORY_LOC, 0, 0,
    'Africa'
);
    keyword('KW_EGYPT', KW_CATEGORY_LOC, 1, KW_AFRICA,
        'Egypt'
    );
        keyword('KW_SINAI_U', KW_CATEGORY_LOC, 2, KW_EGYPT,
            'Sinai University'
        );

keyword('KW_AMERICAS', KW_CATEGORY_LOC, 0, 0,
    'Americas'
);
    keyword('KW_CANADA', KW_CATEGORY_LOC, 1, KW_AMERICAS,
        'Canada'
    );
    keyword('KW_US', KW_CATEGORY_LOC, 1, KW_AMERICAS,
        'United States'
    );
        keyword('KW_UCB', KW_CATEGORY_LOC, 2, KW_US,
            'University of California, Berkeley'
        );
        keyword('KW_UW', KW_CATEGORY_LOC, 2, KW_US,
            'University of Washington'
        );
        keyword('KW_U_DAYTON', KW_CATEGORY_LOC, 2, KW_US,
            'University of Dayton'
        );
        keyword('KW_WRIGHT_STATE', KW_CATEGORY_LOC, 2, KW_US,
            'Wright State University'
        );
        keyword('KW_USC', KW_CATEGORY_LOC, 2, KW_US,
            'University of Southern California'
        );
        keyword('KW_FULLERTON', KW_CATEGORY_LOC, 2, KW_US,
            'California State University Fullerton'
        );
        keyword('KW_ARIZONA_STATE', KW_CATEGORY_LOC, 2, KW_US,
            'Arizona State University'
        );
        keyword('KW_U_ILLINOIS', KW_CATEGORY_LOC, 2, KW_US,
            'University of Illinois'
        );
        keyword('KW_RPI', KW_CATEGORY_LOC, 2, KW_US,
            'Rensselaer Polytechnic Institute'
        );
        keyword('KW_UND', KW_CATEGORY_LOC, 2, KW_US,
            'University of North Dakota'
        );
        keyword('KW_PSU', KW_CATEGORY_LOC, 2, KW_US,
            'Penn State University'
        );
        keyword('KW_SU', KW_CATEGORY_LOC, 2, KW_US,
            'Shenandoah University'
        );

// return a list of [value, name] pairs for the given category
//
function keyword_select_options($category) {
    global $job_keywords;
    $opts = [];
    kw_children();
    foreach ($job_keywords as $id=>$k) {
        if ($k->category != $category) continue;
        if ($k->parent) continue;
        $opts = array_merge($opts, kw_options($id));
    }
    return $opts;
}

function kw_children() {
    global $job_keywords;
    foreach ($job_keywords as $id=>$k) {
        $job_keywords[$id]->children = array();
        $job_keywords[$id]->expand = 0;
    }
    foreach ($job_keywords as $id=>$k) {
        if (!$k->parent) continue;
        $job_keywords[$k->parent]->children[] = $id;
    }
}

function kw_options($id, $indent='') {
    global $job_keywords;
    $kw = $job_keywords[$id];
    $opts = [[$id, "$indent$kw->name"]];
    $indent .= '&nbsp;&nbsp;&nbsp;&nbsp;';
    foreach ($kw->children as $k) {
        $opts = array_merge($opts, kw_options($k, $indent));
    }
    return $opts;
}

function kw_id_to_name($id) {
    global $job_keywords;
    $kw = $job_keywords[$id];
    return $kw->name;
}

function kw_array_to_str($kws, $sep='<br>') {
    $x = array_map('strval', $kws);
    $x = array_map('kw_id_to_name', $x);
    return implode($sep, $x);
}

?>
