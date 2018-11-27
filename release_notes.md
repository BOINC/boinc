# Release Notes
## Introduction
The BOINC team is pleased to announce a new server release.
These release notes for v1.0.2 provide an overview of the new features, bugs fixed and any known issues.
The new features included in this release are: GDPR support and the Simple Attach mechanism.

GDPR is now supported by:
* Consent policies - Project administrators can configure consent policies to which users may agree. Consent can be given and withdrawn by users. By default, projects are configured with Enrollment and Statistics Export consent types; which may be enabled in the project ops page.
* Terms of use - The project Web site displays terms-of-use for users to agree to. The BOINC manager should also display the same terms-of-use. 
* Delete user - Users may delete their accounts. Projects may choose between delete types: soft-delete, hard-delete, or a project-defined delete.
* While not directly GDPR related, projects can now notify users more securely when they change email addresses. A notification will be sent to both the old and new emails. Users may revert the email change if they suspect malicious behavior. Additionally, users may not delete their accounts until 7 days after an email change.
For more details, please see the [GDPR compliance](https://boinc.berkeley.edu/trac/wiki/GdprCompliance) page.

The simple attach mechanism simplifies the join-up process for Windows users. In summary, the new-user process is:
* Visit the project or account manager Web site and click on *Join*.
* Enter email/password, click *OK*
* Click *Download*
* Click on the installer, choose defaults 
The improvements are that the user doesn't leave the project Web site (i.e. doesn't land on BOINC Web site) and also the user doesn't have to select the project from a a list of all projects. Further details, including how to configure this for a project,  can be found in the [Simple Attach](https://boinc.berkeley.edu/trac/wiki/SimpleAttach) documentation.
## Bugs Fixed
Please see the git [comparison with v0.9](https://github.com/BOINC/boinc/compare/server_release/0.9...server_release/1/1.0) for the details.

## Known issues
None at this time.
## Download
The server release can be obtain with the following commands. 
```
git clone https://github.com/BOINC/boinc.git
cd boinc
git checkout tags/server_release/1.0/1.0.1 -b server_release/1.0
```
The code and then be build as described in the [documentation](https://boinc.berkeley.edu/trac/wiki/BuildSystem). 
```
./_autosetup
./configure --disable-client --disable-manager
make
```
## Install
To setup a new BOINC server, please follow the [guide](https://boinc.berkeley.edu/trac/wiki/ServerIntro). If you are new to BOINC please ensure to review the [Technical Documentation](https://boinc.berkeley.edu/trac/wiki/ProjectMain) first. 
## Upgrade
An existing BOINC server can be upgraded with the [upgrade tool](https://boinc.berkeley.edu/trac/wiki/ToolUpgrade).    
