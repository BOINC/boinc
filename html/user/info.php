<?php
require_once('include.php');
doHeader('Rules and Policies');
?>

<h2>Rules and Policies</h2>

<h3>Run <?php echo $cfg['project'] ?> only on authorized computers</h3>

<p>
Run <?php echo $cfg['project'] ?> only on computers that you own, or for which you have obtained the owner's permission.
Some companies and schools have policies that prohibit using their computers for projects such as <?php echo $cfg['project'] ?>.
</p>

<h3>How <?php echo $cfg['project'] ?> will use your computer</h3>

<p>
When you run <?php echo $cfg['project'] ?> on your computer, it will use part of the computer's CPU power, disk space, and network bandwidth.
You can control how much of your resources are used by <?php echo $cfg['project'] ?>, and when it uses them.
</p>

<p>
The work done by your computer contributes to the academic nonprofit research being performed by <?php echo $cfg['project'] ?>.
The current research is described <a href="research.html">here</a>.
The application programs may change from time to time.
</p>

<h3>Privacy policy</h3>

<p>
Your account on <?php echo $cfg['project'] ?> is identified by a name that you choose.
This name may be shown on the <?php echo $cfg['project'] ?> web site, along with a summary of the work your computer has done for <?php echo $cfg['project'] ?> and other BOINC projects.
If you want to be anonymous, choose a name that doesn't reveal your identity.
</p>

<p>
If you participate in <?php echo $cfg['project'] ?>, information about your computer (such as its processor type, amount of memory, etc.) will be recorded by <?php echo $cfg['project'] ?> and used to decide what type of work to assign to your computer.
This information will also be shown on <?php echo $cfg['project'] ?>'s web site.
Nothing that reveals your computer's location (e.g. its domain name or network address) will be shown.
</p>

<p>
To participate in <?php echo $cfg['project'] ?>, you must give an address where you receive email.
This address will not be shown on the <?php echo $cfg['project'] ?> web site or shared with organizations.
<?php echo $cfg['project'] ?> may send you periodic newsletters; however, you can choose not to be sent these at any time.
</p>

<h3>Is it safe to run <?php echo $cfg['project'] ?>?</h3>

<p>
Any time you download a program through the Internet you are taking a chance: the program might have dangerous errors, or the download server might have been hacked.
<?php echo $cfg['project'] ?> has made efforts to minimize these risks.
We have tested our applications carefully.
Our servers are behind a firewall and are configured for high security.
To ensure the safety of program downloads, all executable files are digitally signed on a secure computer not connected to the Internet.
</p>

<p>
<?php echo $cfg['project'] ?> was developed at the University of California at Berkeley, by members of the SETI@home project.
</p>

<h3>Liability</h3>

<p>
<?php echo $cfg['project'] ?> assumes no liability for damage to your computer, loss of data, or any other event or condition that may occur as a result of participating in <?php echo $cfg['project'] ?>.
</p>

<h3>Other BOINC projects</h3>

<p>
Other projects use the same platform, BOINC, as <?php echo $cfg['project'] ?>.
You may want to consider participating in one or more of these projects.
By doing so, your computer will do useful work even when <?php echo $cfg['project'] ?> has no work available for it.
</p>

<p>
These other projects are not associated with <?php echo $cfg['project'] ?>, and we cannot vouch for their security practices or the nature of their research.
Join them at your own risk.
</p>

<?php
doFooter();
?>
