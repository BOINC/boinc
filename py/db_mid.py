## $Id$

## database middle-end.  This file is not required for normal database use.
## it is useful for debugging scripts; e.g.
##
##    print database.Users[1]
##
## will look prettier if you import this module.

import database

def MixIn(pyClass, mixInClass):
    pyClass.__bases__ = (mixInClass,) + pyClass.__bases__

class front_Project:
    def __repr__(self):
        return '<Project#%d %s>'%(self.id, self.short_name)

class front_Platform:
    def __repr__(self):
        return '<Platform#%d %s>'%(self.id, self.name)

class front_CoreVersion:
    def __repr__(self):
        return '<CoreVersion#%d %s>'%(self.id, self.version_num)

class front_App:
    def __repr__(self):
        return '<App#%d %s>'%(self.id, self.name)

class front_AppVersion:
    def __repr__(self):
        return '<AppVersion#%d %s %s>'%(self.id, self.app.name, self.version_num)

class front_User:
    def __repr__(self):
        return '<User#%d %s %s>'%(self.id, self.name, self.email_addr)

class front_Team:
    def __repr__(self):
        return '<Team#%d %s>'%(self.id, self.name)

class front_Host:
    def __repr__(self):
        return '<Host#%d %s>'%(self.id, self.user.name, self.domain_name)

class front_Workunit:
    def __repr__(self):
        return '<WU#%d %s>'%(self.id, self.name)

class front_Result:
    def __repr__(self):
        return '<Result#%d %s WU#%d %s>'%(self.id, self.name, self.workunit.id, self.workunit.name)

class front_Workseq:
    def __repr__(self):
        return '<Workseq#%d>'%(self.id)

MixIn(database.Project, front_Project)
MixIn(database.Platform, front_Platform)
MixIn(database.CoreVersion, front_CoreVersion)
MixIn(database.App, front_App)
MixIn(database.AppVersion, front_AppVersion)
MixIn(database.User, front_User)
MixIn(database.Team, front_Team)
MixIn(database.Host, front_Host)
MixIn(database.Workunit, front_Workunit)
MixIn(database.Result, front_Result)
MixIn(database.Workseq, front_Workseq)
