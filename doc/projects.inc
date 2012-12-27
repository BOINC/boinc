<?php

require_once("../inc/translation.inc");

// 0: name
// 1: web site URL
// 2: home
// 3: area
// 4: goals
// 5: logo
// 6: master URL (if different from web site)

$sensing = array(
    tra("Distributed sensing"),
    array(
        array(
            "Quake Catcher Network",
            "http://qcn.stanford.edu/sensor/",
            tra("Stanford University"),
            tra("Seismology"),
            tra("The Quake-Catcher Network is developing the world's largest seismic network using sensors attached to Internet-connected computers.  You must buy a sensor to participate."),
            ""
        ),
        array(
            "Radioactive@Home",
            "http://radioactiveathome.org/",
            tra("BOINC Poland Foundation"),
            tra("Environmental research"),
            tra("This project is creating a free and continuously updated map of radiation levels by using sensors connected to volunteers' computers.  You must buy a sensor to participate."),
            "radioactive.jpg",
        ),
    ),
    "<br><span class=\"note\">".tra("To participate in these projects you must buy a sensor and attach it to your computer.")."</span>",
);

$cogsci = array(
    tra("Cognitive science and artifical intelligence"),
    array(
        array(
            "MindModeling@Home",
            "http://mindmodeling.org/",
            "University of Dayton and Wright State University",
            "Cognitive Science",
            "MindModeling@Home uses computational cognitive process modeling to better understand the human mind, and specifically to study the mechanisms and processes that enable and moderate human performance and learning.",
            "mmlogo.gif"
        ),
        array(
            "FreeHAL",
            "http://www.freehal.net/freehal_at_home/",
            tra("Private"),
            tra("Artificial intelligence"),
            tra("Parse and convert semantic nets for use in FreeHAL, an artificial intelligence that uses semantic networks, stemmers, part of speech databases, and part of speech taggers in order to imitate human behavior in conversations.")
        ),
    ),
);

$biomed = array(
    tra("Biology and Medicine"),
    array(
        array(
            "FightMalaria@Home",
            "http://boinc.ucd.ie/fmah/",
            tra("University College Dublin"),
            tra("Antimalarial drug discovery"),
            tra("The parasite that causes malaria continues to evolve resistance to available medication. We therefore urgently need to discover new drugs to replace existing drugs. Importantly, these new drugs need to target NEW proteins in the parasite. The FightMalaria@Home project is aimed at finding these new targets."),
            "FMAH_banner3_wt.png"
        ),
        array(
            "POEM@HOME",
            "http://boinc.fzk.de/poem/",
            tra("University of Karlsruhe (Germany)"),
            tra("Protein structure prediction"),
            tra("POEM@HOME uses a computational approach to predict the biologically active structure of proteins, to understand the signal-processing mechanisms when the proteins interact with one another, to understand diseases related to protein malfunction or aggregation, and to develop new drugs on the basis of the three-dimensions structure of biologically important proteins."),
            "poem.jpg"
        ),
        array(
            "Docking@Home",
            "http://docking.cis.udel.edu/",
            tra("University of Delaware"),
            tra("Study of protein - ligand interactions"),
            tra("Docking@Home has both bioscience and computer science goals. The project aims to further knowledge of the atomic details of protein-ligand interactions and, by doing so, will search for insights into the discovery of novel pharmaceuticals."),
            "docking.png"
        ),
        array(
            "GPUGrid.net",
            "http://www.gpugrid.net/",
            tra("Barcelona Biomedical Research Park (PRBB)"),
            tra("Molecular simulations of proteins"),
            tra("GPUGrid.net opens novel computational scenarios by the first full-atom molecular dynamics code (CellMD) specially optimized to run on NVIDIA GPUs. New biomedical applications suddenly become possible giving a new role to computational biology for biomedical research."),
            "gpugrid.png"
        ),
        array(
            "Superlink@Technion",
            "http://cbl-boinc-server2.cs.technion.ac.il/superlinkattechnion/",
            tra("Technion, Israel"),
            tra("Genetic linkage analysis"),
            tra("Superlink@Technion helps geneticists all over the world find disease-provoking genes causing some types of diabetes, hypertension (high blood pressure), cancer, schizophrenia and many others."),
            "superlink_logo.gif"
        ),
//        array(
//            "Proteins@Home",
//            "http://biology.polytechnique.fr/proteinsathome",
//            tra("Ecole Polytechnique, Paris"),
//            tra("Protein structure prediction"),
//            tra("By joining Proteins@Home, you will be part of a large-scale protein structure prediction project and help to advance an important area of science. By increasing our knowledge of proteins, you will contribute to a better understanding of many diseases and pathologies, and to progress in both medicine and technology. Proteins@Home is not for profit."),
//            "proteinsathome.gif"
//        ),
        array(
            "The Lattice Project",
            "http://boinc.umiacs.umd.edu/",
            tra("University of Maryland Center for Bioinformatics and Computational Biology"),
            tra("Life science research"),
            tra("The Lattice Project supplies computing power to scientists at the University of Maryland studying evolutionary relationships based on DNA sequence data; bacterial, plasmid, and virus protein sequences; and biological diversity in nature reserves. "),
            "lattice.gif"
        ),
        array(
            "Malariacontrol.net",
            "http://www.malariacontrol.net/",
            tra("The Swiss Tropical Institute"),
            tra("Epidemiology"),
            tra("Simulation models of the transmission dynamics and health effects of malaria are an important tool for malaria control. They can be used to determine optimal strategies for delivering mosquito nets, chemotherapy, or new vaccines which are currently under development and testing.  Such modeling is extremely computer intensive, requiring simulations of large human populations with a diverse set of parameters related to biological and social factors that influence the distribution of the disease. "),
            "africaathome.gif"
        ),
//        array(
//            "Tanpaku",
//            "http://issofty17.is.noda.tus.ac.jp/",
//            tra("Tokyo University of Science"),
//            tra("Biology"),
//            tra("To predict protein structure and function from genetic sequences, using the 'Brownian Dynamics' (BD) method.  This method enables us to simulate more efficiently than conventional methods."),
//            "tanpaku.jpg"
//        ),
//        array(
//            "Predictor@home",
//            "http://predictor.chem.lsa.umich.edu/",
//            tra("University of Michigan"),
//            tra("Biology"),
//            tra("Protein structure prediction starts from a sequence of amino acids and attempts to predict the folded, functioning, form of the protein.  Predicting the structure of an unknown protein is a critical problem in enabling structure-based drug design to treat new and existing diseases."),
//            "predictor.jpg"
//        ),
        array(
            "Rosetta@home",
            "http://boinc.bakerlab.org/rosetta/",
            tra("University of Washington"),
            tra("Biology"),
            tra("Determine the 3-dimensional shapes of proteins in research that may ultimately lead to finding cures for some major human diseases. By running Rosetta@home you will help us speed up and extend our research in ways we couldn't possibly attempt without your help. You will also be helping our efforts at designing new proteins to fight diseases such as HIV, Malaria, Cancer, and Alzheimer's"),
            "rosetta_at_home_logo.jpg"
        ),
        array(
            "SIMAP",
            "http://boincsimap.org/boincsimap/",
            tra("University of Vienna"),
            tra("Biology"),
            tra("Calculate similarities between proteins. SIMAP provides a public database of the resulting data, which plays a key role in many bioinformatics research projects."),
            "simaplogo.jpg"
        ),
    ),
);
$earth = array(
    tra("Earth Sciences"),
    array(
        array(
            "Virtual Prairie",
            "http://vcsc.cs.uh.edu/virtual-prairie/",
            tra("University of Houston"),
            tra("Study of botanical ecosystems"),
            tra("Provide ecological guidelines on the design of prairies with the best potential for water purification.")
        ),
        array(
            "Climateprediction.net",
            "http://climateprediction.net/",
            tra("Oxford University"),
            tra("Climate study"),
            tra("Investigate the approximations that have to be made in state-of-the-art climate models. By running the model thousands of times we hope to find out how the model responds to slight tweaks to these approximations - slight enough to not make the approximations any less realistic. This will allow us to improve our understanding of how sensitive our models are to small changes and also to things like changes in carbon dioxide and the sulphur cycle. This will allow us to explore how climate may change in the next century under a wide range of different scenarios."),
            "cpn_logo_world_1.jpg",
            "http://climateapps2.oucs.ox.ac.uk/cpdnboinc/"
        ),
    ),
);
$astro_phys_chem = array(
    tra("Astronomy/Physics/Chemistry"),
    array(
        array(
            "Constellation",
            "http://aerospaceresearch.net/constellation/",
            "<a href=http://rechenkraft.net/>Rechenkraft.net</a>, <a href=http://stuttgart.dglr.de/>DGLR</a>, <a href=http://selfnet.de/>Selfnet</a>, and <a href=http://shackspace.de/>shack</a>",
            "Aerospace-related science and engineering",
            "Constellation is a platform for aerospace-related simulations, including trajectory optimization of launchers, satellites and probes, simulation of Moon's near-surface exosphere, and analysis of dynamic systems of exploration-rovers",
            "constellation2.png"
        ),
        array(
            "eOn",
            "http://eon.ices.utexas.edu/eon2/",
            tra("University of Texas at Austin"),
            tra("Chemistry"),
            tra("A common problem in theoretical chemistry, condensed matter physics and materials science is the calculation of the time evolution of an atomic scale system where, for example, chemical reactions and/or diffusion occur. Generally the events of interest are quite rare (many orders of magnitude slower than the vibrational movements of the atoms), and therefore direct simulations, tracking every movement of the atoms, would take thousands of years of computer calculations on the fastest present day computer before a single event of interest can be expected to occur.  Our research group is interested in calculating the long time dynamics of systems.")
        ),
        array(
            "Orbit@home",
            "http://orbit.psi.edu/oah/",
            tra("Planetary Science Institute"),
            tra("Astronomy"),
            tra("Monitor and study the hazard posed by near-Earth asteroids"),
            "http://orbit.psi.edu/"
        ),
        array(
            "Cosmology@Home",
            "http://www.cosmologyathome.org/",
            tra("University of Illinois at Urbana-Champaign"),
            tra("Astronomy"),
            tra("The goal of Cosmology@Home is to search for the model that best describes our Universe and to find the range of models that agree with the available astronomical particle physics data."),
            "cosmo.jpg"
        ),
        array(
            "Milkyway@home",
            "http://milkyway.cs.rpi.edu/milkyway/",
            tra("Rensselaer Polytechnic Institute"),
            tra("Astronomy"),
            tra("The goal of Milkyway@Home is to create a highly accurate three dimensional model of the Milky Way galaxy using data gathered by the Sloan Digital Sky Survey."),
            "mw.png",
        ),
        array(
            "Leiden Classical",
            "http://boinc.gorlaeus.net/",
            tra("Leiden University, The Netherlands"),
            tra("Chemistry"),
            tra("Surface science calculations using Classical Dynamics.  Leiden Classical allows volunteers, students and other scientist to submit their personal calculations to the grid. Each user has his own personal queue for Classical Dynamics jobs. In this way students have used the grid to simulate liquid argon, or to test the validity of the ideal gas law by actually doing the simulations through the grid."),
            "leiden_classical.png"
        ),
        array(
            "uFluids@home",
            "http://www.ufluids.net/",
            tra("Purdue University"),
            tra("Physics/Aeronautics"),
            tra("The uFluids project simulates two-phase fluid behavior in microgravity and microfluidics problems. Our goal is to design better satellite propellant management devices and address two-phase flow in microchannel and MEMS devices."),
            "ufluids.gif"
        ),
        array(
            "Einstein@home",
            "http://einstein.phys.uwm.edu/",
            tra("Univ. of Wisconsin - Milwaukee, Max Planck Institute"),
            tra("Astrophysics"),
            tra("Search for spinning neutron stars (also called pulsars) using data from the LIGO and GEO gravitational wave detectors, and from the Arecibo radio observatory.  Einstein@Home is a World Year of Physics 2005 project supported by the American Physical Society (APS) and by a number of international organizations."),
            "einstein.jpg"
        ),
        array(
            "LHC@home",
            "http://lhcathomeclassic.cern.ch/sixtrack/",
            tra("CERN (European Organization for Nuclear Research)"),
            tra("Physics"),
            tra("The Large Hadron Collider (LHC) is a particle accelerator at CERN, the European Organization for Nuclear Research, the world's largest particle physics laboratory.  It is the most powerful instrument ever built to investigate on particles proprieties. LHC@home runs simulations to improve the design of LHC and its detectors."),
            "lhc.jpg"
        ),
        array(
            "LHC@home Test4Theory",
            "http://lhcathome2.cern.ch/test4theory/",
            tra("CERN (European Organization for Nuclear Research)"),
            tra("Physics"),
            tra("This project uses CERN-developed virtual machine technology for full-fledged LHC event physics simulation on volunteer computers.  Requires that you install VirtualBox on your computer"),
            "t4t.png"
        ),
        array(
            "SETI@home",
            "http://setiathome.berkeley.edu/",
            tra("University of California, Berkeley"),
            tra("Astrophysics, astrobiology"),
            tra("SETI (Search for Extraterrestrial Intelligence) is a scientific area whose goal is to detect intelligent life outside Earth. One approach, known as radio SETI, uses radio telescopes to listen for narrow-bandwidth radio signals from space. Such signals are not known to occur naturally, so a detection would provide evidence of extraterrestrial technology."),
            "seti_logo.png"
        ),
        array(
            "Quantum Monte Carlo at Home",
            "http://qah.uni-muenster.de/",
            tra("University of Muenster (Germany)"),
            tra("Chemistry"),
            tra("Study the structure and reactivity of molecules using Quantum Chemistry."),
            "logo_oben.jpg"
        ),
        array(
            "Spinhenge@home",
            "http://spin.fh-bielefeld.de/",
            tra("Bielefeld University of Applied Sciences"),
            tra("Chemical engineering and nanotechnology"),
            tra("The study of molecular magnets and controlled nanoscale magnetism.  These magnetic molecules may be used to develop tiny magnetic switches, with applications in medicine (such as local tumor chemotherapy) and biotechnology."),
            "rotating-fe30-h90px.gif"
        ),
    ),
);

$mixed = array(
    tra("Multiple applications"),
    array(
        array(
            "CAS@home",
            "http://casathome.ihep.ac.cn/",
            tra("Chinese Academy of Sciences"),
            tra("Physics, biochemistry, and others"),
            tra("The objective of CAS@home is to encourage and assist scientists in China to adopt the technologies of volunteer computing and volunteer thinking for their research."),
            "cas_at_home.jpg"
        ),
        array(
            "Yoyo@home",
            "http://www.rechenkraft.net/yoyo/",
            tra("Private"),
            tra("Mathematics, physics, evolution"),
            tra("Yoyo@home is an adapter between BOINC and several existing volunteer computing projects: ECM, Muon, Evolution@home, and distributed.net")
        ),
        array(
            "EDGeS@Home",
            "http://home.edges-grid.eu/home/",
            tra("MTA-SZTAKI Laboratory of Parallel and Distributed Systems (Hungary)"),
            tra("European research projects"),
            tra("The EDGeS@Home Beta project integrates volunteer computing into the service grid network of Europe by allowing service grids to send workunits to be processed by the volunteers of this project. The scientific projects covered by the project include math, physics, biology, etc."),
            "logo_edges.png"
        ),
        array(
            "Ibercivis",
            "http://registro.ibercivis.es/",
            tra("Spanish universities and research centers"),
            tra("Various Spanish research projects"),
            tra("Research in physics, material science, and biomedicine"),
            "cabecera2.jpg"
        ),
        array(
            "World Community Grid",
            "http://www.worldcommunitygrid.org/",
            tra("IBM Corporate Community Relations"),
            tra("Humanitarian research on disease, natural disasters and hunger."),
            tra("To further critical non-profit research on some of humanity's most pressing problems by creating the world's largest volunteer computing grid.  Research includes HIV/AIDS, cancer, muscular dystrophy, dengue fever, and many more."),
            "wcg.jpg",
        ),
    ),
);
$math = array(
    tra("Mathematics, computing, and games"),
    array(
        array(
            "OProject@Home",
            "http://oproject.info/",
            tra("Private"),
            tra("Mathematics, Physics, Artificial Intelligence"),
            tra("Simulation of quantum computing; Goldbach's conjecture."),
            "oproject_logo.png"
        ),
        array(
            "NumberFields@home",
            "http://stat.la.asu.edu/NumberFields/",
            "Arizona State University, school of Mathematics",
            "Mathematics",
            "NumberFields@home searches for fields with special properties. The primary application of this research is in the realm of algebraic number theory. Number theorists can mine the data for interesting patterns to help them formulate conjectures about number fields. Ultimately, this research will lead to a deeper understanding of the profound properties of numbers, the basic building blocks of all mathematics.",
            "nf_banner_10.jpg"
        ),
        array(
            "SAT@home",
            "http://sat.isa.ru/pdsat/",
            "Institute for Systems Analysis and Institute for System Dynamics and Control Theory, Russian Academy of Science",
            "Computer Science",
            "Solve hard and practically important problems (discrete functions inversion problems, discrete optimization, bioinformatics, etc.) that can be effectively reduced to Boolean satisfiability problem.",
            "sat_logo.png"
        ),
        array(
            "Mersenne@home",
            "http://mersenneathome.net/",
            tra("Private"),
            tra("Mathematics"),
            tra("Mersenne@home searches for 'Mersenne primes' - prime numbers of the form 2<sup>p</sup>-1."),

            "mersenne_logo80.png"
        ),
        array(
            "DistrRTgen",
            "http://boinc.freerainbowtables.com/distrrtgen/",
            "Private",
            tra("Cryptography"),
            "To give the world's security experts the best tools available for detecting weak hashes. This can help them to force developers to use more secure methods of password protection.",
            "freerainbowtables_logo.png",
        ),
        array(
            "Enigma@Home",
            "http://www.enigmaathome.net/",
            tra("Private"),
            tra("Cryptography"),
            tra("Attempt to decode 3 original Enigma messages. The signals were intercepted in the North Atlantic in 1942 and are believed to be unbroken.")
        ),
        array(
            "Collatz Conjecture",
            "http://boinc.thesonntags.com/collatz/",
            tra("Private"),
            tra("Mathematics"),
            tra("Study the Collatz Conjecture, an unsolved conjecture in mathematics"),
        ),
        array(
            "NFS@home",
            "http://escatter11.fullerton.edu/nfs/",
            tra("California State University Fullerton"),
            tra("Factorization of large integers"),
            tra("NFS@Home is a research project that uses Internet-connected computers to do the lattice sieving step in the Number Field Sieve factorization of large integers. As a young school student, you gained your first experience at breaking an integer into prime factors, such as 15 = 3 * 5 or 35 = 5 * 7. NFS@Home is a continuation of that experience, only with integers that are hundreds of digits long."),
            "NFS_Logo.jpg"
        ),
        array(
            "VTU@home",
            "http://boinc.vgtu.lt/vtuathome/",
            tra("Vilnius Gediminas Technical University and Kaunas University of Technology (Lithuania)"),
            tra("Software testing"),
            tra("The aim of this project is to provide a powerful distributed computing platform for scientists of Vilnius Gediminas Technical University (VGTU) as well as others Lithuanian academic institutions.  Current applications involve the study of Monte-Carlo based software testing."),
            "vtuathome-white.gif",
        ),
//        array(
//            "AQUA@home",
//            "http://aqua.dwavesys.com/",
//            tra("D-Wave Systems, Inc."),
//            tra("Quantum computing"),
//            tra("D-Wave's AQUA (Adiabatic QUantum Algorithms) is a research project whose goal is to predict the performance of superconducting adiabatic quantum computers on a variety of hard problems arising in fields ranging from materials science to machine learning. AQUA@home uses Internet-connected computers to help design and analyze quantum computing algorithms, using Quantum Monte Carlo techniques."),
//            "logo_dwave2.png"
//        ),
//        array(
//            "SHA-1 Collision Search Graz",
//            "http://boinc.iaik.tugraz.at/sha1_coll_search/",
//            tra("Graz University of Technology (Austria)"),
//            tra("Cryptography"),
//            tra("Search for collisions (weaknesses) of the widely-used SHA-1 hash function."),
//            "LogoKryptoGroup_klein.jpg"
//        ),
        array(
            "ABC@home",
            "http://abcathome.com/",
            tra("Mathematical Institute of Leiden University / Kennislink"),
            tra("Mathematics"),
            tra("Search for 'abc-triples': positive integers a,b,c such that a+b=c, a &lt; b &lt; c, a,b,c have no common divisors and c > rad(abc), where rad(n) is the product of the distinct prime factors of n. The ABC conjecture says that there are only finitely many a,b,c such that log(c)/log(rad(abc)) > h for any real h > 1. The ABC conjecture is currently one of the greatest open problems in mathematics. If it is proven to be true, a lot of other open problems can be answered directly from it."),
            "abclogo.jpg"
        ),
        array(
            "PrimeGrid",
            "http://www.primegrid.com/",
            tra("Private"),
            tra("Cryptography"),
            tra("Primegrid is generating a public sequential prime number database, and is searching for large twin primes of the form k*2<sup>n</sup>+1 and k*2<sup>n</sup>-1"),
            "primegrid_logo.png"
        ),
        array(
            "primaboinca",
            "http://www.primaboinca.com/",
            tra("Hochschule RheinMain University of Applied Sciences"),
            tra("Mathematics"),
            tra("Search for counterexamples to two conjectures related to the identification of prime numbers"),
            "logo_primaboinca.gif"
        ),
        array(
            "SZTAKI Desktop Grid",
            "http://szdg.lpds.sztaki.hu/szdg/",
            tra("MTA-SZTAKI Laboratory of Parallel and Distributed Systems (Hungary)"),
            tra("Mathematics"),
            tra("Find all the generalized binary number systems (in which bases are matrices and digits are vectors) up to dimension 11."),
            "szdg1_small.jpg"
        ),
//        array(
//            "Riesel Sieve",
//            "http://boinc.rieselsieve.com/",
//            tra("Riesel Sieve community"),
//            tra("Mathematics"),
//            tra("Find prime numbers of the form k*2<sup>n</sup>-1"),
//            ""
//        ),
//        array(
//            "Rectilinear Crossing Number",
//            "http://dist.ist.tugraz.at/cape5/",
//            tra("Graz University of Technology (Austria)"),
//            tra("Mathematics"),
//            tra("What is the least number of crossings a straight-edge drawing of the complete graph on top of a set of n points in the plane obtains?  From very recent (not even published yet) mathematical considerations the rectilinear crossing numbers for n=19 and n=21 are also known. So the most tantalizing problem now is to determine the true value for n=18, which is the main focus of this project."),
//            ""
//        ),
        array(
            "Chess960@home",
            "http://www.chess960athome.org/alpha/",
            tra("Chess-960.org"),
            tra("Game-playing"),
            tra("This project studies Chess 960, a variant of orthodox chess. In classical chess the starting position of the game never changes.  In Chess 960, just before the start of every game, the initial configuration of the chess pieces is determined randomly."),
            "chess960athome.jpg"
        ),
//        array(
//            "NQueens@home",
//            "http://nqueens.ing.udec.cl/",
//            tra("Universidad de Concepción, Chile"),
//            tra("Mathematics"),
//            tra("The eight queens problem consists of trying to place eight queens on a chessboard so that no queen attacks any other queen.  I has long been known that there are 92 solutions, of which 12 are distinct. NQueens@home studies the generalization to N queens on an NxN board, for N=19 and greater."),
//            "NQueens-Logo1b.png"
//        ),
        array(
            "sudoku@vtaiwan",
            "http://sudoku.nctu.edu.tw/",
            "National Chiao Tung University, Taiwan",
            tra("Mathematics"),
            "Sudoku@vtaiwan seeks to solve the minimum Sudoku problem, a well-known problem in mathematics and computer science.",
            "sudoku.png"
        ),
        array(
            "Surveill@Home",
            "http://surveill.dei.uc.pt/surveill",
            "University of Coimbra, Portugal",
            "Web performance",
            tra("Surveill@Home is a research project that conducts end-to-end fine-grained monitoring of web sites.  The project will deploy thousands of probes, each of which repeatedly performs transactions on web sites.  This approach provides end-user failure and performance statistics."),
            "surveill.png"
        ),
    ),
);

$areas = array($biomed, $astro_phys_chem, $math, $earth, $mixed, $cogsci, $sensing);

?>
