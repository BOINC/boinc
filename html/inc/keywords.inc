<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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
define('KW_DIABETES',       17);
define('KW_CANCER_PROSTATE', 18);
define('KW_CANCER_BREAST',  19);
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
        keyword('KW_DIABETES', KW_CATEGORY_SCIENCE, 2, KW_DISEASE,
            'Diabetes research'
        );
        keyword('KW_CANCER', KW_CATEGORY_SCIENCE, 2, KW_DISEASE,
            'Cancer research'
        );
            keyword('KW_CANCER_PROSTATE', KW_CATEGORY_SCIENCE, 3, KW_CANCER,
                'Prostate cancer research'
            );
            keyword('KW_CANCER_BREAST', KW_CATEGORY_SCIENCE, 3, KW_CANCER,
                'Breast cancer research'
            );
keyword('KW_MATH_CS', KW_CATEGORY_SCIENCE, 0, 0,
    'Mathematics and Computer Science'
);
keyword('KW_AI', KW_CATEGORY_SCIENCE, 0, 0,
    'Artificial Intelligence and Cognitive Science'
);
keyword('KW_EARTH_SCI', KW_CATEGORY_SCIENCE, 0, 0,
    'Earth sciences'
);

// Locations

keyword('KW_EUROPE', KW_CATEGORY_LOC, 0, 0,
    'Europe'
);
    keyword('KW_GERMANY', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'Germany'
    );
        keyword('KW_AEI', KW_CATEGORY_LOC, 2, KW_GERMANY,
            'Albert Einstein Institute for Gravitational Physics'
        );
    keyword('KW_CERN', KW_CATEGORY_LOC, 1, KW_EUROPE,
        'CERN'
    );
keyword('KW_ASIA', KW_CATEGORY_LOC, 0, 0,
    'Asia'
);
keyword('KW_AMERICAS', KW_CATEGORY_LOC, 0, 0,
    'The Americas'
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
?>
