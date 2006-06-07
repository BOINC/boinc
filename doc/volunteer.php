<?php
require_once('docutil.php');
page_head('Volunteer computing');
echo "
<h2>What is volunteer computing?</h2>
<p>
<b>Volunteer computing</b> is an arrangement in which people (<b>volunteers</b>)
provide computing resources to <b>projects</b>,
which use the resources to do distributed computing and/or storage.
<ul>
<li>
Volunteers are typically members of the general public
who own Internet-connected PCs.
Organizations such as schools and businesses
may also volunteer the use of their computers.
<li>
Projects are typically academic (university-based)
and do scientific research.
But there are exceptions; for example, GIMPS and distributed.net
(two major projects) are not academic.
</ul>
<p>
Several aspects of the project/volunteer relationship are worth noting:
<ul>
<li>
Volunteers are effectively anonymous;
although they may be required to register
and supply email address or other information,
there is no way for a project to link them to a real-world identity.
<li>
Because of their anonymity, volunteers are not <b>accountable</b> to projects.
If a volunteer misbehaves in some way
(for example, by intentionally returning incorrect computational results)
the project cannot prosecute or discipline the volunteer.
<li>
Volunteers must <b>trust</b> projects in several ways:
1) the volunteer trusts the project to
provide applications that don't damage their computer
or invade their privacy;
2) the volunteer trusts that the project is truthful about
what work is being done by its applications,
and how the resulting intellectual property will be used;
3) the volunteer trusts the project to follow proper security practices,
so that hackers cannot use the project as a vehicle for malicious activities.
</ul>

<p>
The first volunteer computing project was GIMPS
(Great Internet Mersenne Prime Search), which started in 1995.
Other early projects include distributed.net, SETI@home, and Folding@home.
Today there are at least 50 active projects.


<h2>Why is volunteer computing important?</h2>
<p>
It's important for several reasons:
<ul>
<li>
Because of the huge number of PCs in the world,
volunteer computing can (and does) supply more computing power to science
than does any other type of computing.
This computing power enables scientific research that
could not be done otherwise.
<p>
This advantage will increase over time,
because the laws of economics dictate that consumer electronics
(PCs and game consoles)
will advance faster than more specialized products,
and that there will simply be more of them.
<li>
Volunteer computing power can't be bought; it must be earned.
A research project that has limited funding but large public appeal
(such as SETI@home) can get huge computing power.
In contrast, traditional supercomputers are extremely expensive,
and are available only for applications that can afford them
(for example, nuclear weapon design and espionage).
<li>
Volunteer computing encourages public interest in science,
and provides the public with voice in determining the
directions of scientific research.
</ul>

<h2>How does it compare to 'Grid computing'?</h2>
<p>
It depends on how you define 'Grid computing'.
The term generally refers to the sharing of computing resources
within and between organizations, with the following properties:
<ul>
<li> Each organization can act as either producer or consumer of resources
(hence the anology with the electrical power grid,
in which electric companies can buy and sell power to/from
other companies, according to fluctuating demand).
<li> The organizations are mutually accountable.
If one organization misbehaves, the others can respond
by suing them or refusing to share resources with them.
</ul>

<p>
This is different from volunteer computing.
'Desktop grid' computing - which uses desktop PCs within an organization -
is superficially similar to volunteer computing,
but because it has accountability and lacks anonymity,
it is different at a deeper level.
<p>
If your definition of 'Grid computing' encompasses all distributed computing
(which is silly - there's already a perfectly good term for that)
then volunteer computing is a type of Grid computing.
<p>
For more information about Grid computing,
visit CERN's <a href=http://gridcafe.web.cern.ch/gridcafe/>Grid Caf&#233;</a>.

<h2>Is it the same as 'peer-to-peer computing'?</h2>
<p>
No.
'Peer-to-peer computing' describes systems such as
Napster, Gnutella, and Freenet,
in which files and other data are exchanged between 'peers' (i.e. PCs)
without the involvement of a central server.
This differs in several ways from volunteer computing:
<ul>
<li> Volunteer computing uses central servers.
There is typically no peer-to-peer communication.
<li> Peer-to-peer computing benefits the participants
(i.e. the people sharing files).
There's no notion of a 'project' to which resources are donated.
<li> Peer-to-peer computing actually involves storage and retrieval,
not computing<sup>1</sup>.
</ul>
<hr>
<sup>1</sup> An exception: <a href=http://gpu.sourceforge.net/>GPU</a>
(Global Processing Unit) is a Gnutella client that allows users
to share CPU resources.

";
?>
