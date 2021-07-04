## $Id$

## database middle-end.  This file is not required for normal database use.
## it is useful for debugging scripts; e.g.
##
##    print database.Users[1]
##
## will look prettier if you import this module.

from Boinc import database

def MixIn(pyClass, mixInClass):
    pyClass.__bases__ = (mixInClass,) + pyClass.__bases__

class front_Platform:
    def __repr__(self):
        return '<Platform#%s %s>'%(self.id, self.name)

class front_App:
    def __repr__(self):
        return '<App#%s %s>'%(self.id, self.name)

class front_AppVersion:
    def __repr__(self):
        return '<AppVersion#%s %s %s %s>'%(self.id, self.app.name,
                                           self.version_num, self.platform.name)

class front_User:
    def __repr__(self):
        return '<User#%s %s %s>'%(self.id, self.name, self.email_addr)

class front_Team:
    def __repr__(self):
        return '<Team#%s %s>'%(self.id, self.name)

class front_Host:
    def __repr__(self):
        return '<Host#%s %s %s>'%(self.id, self.user.name, self.domain_name)

class front_Workunit:
    def __repr__(self):
        return '<WU#%s %s>'%(self.id, self.name)

class front_Result:
    def __repr__(self):
        return '<Result#%s %s WU#%s %s>'%(self.id, self.name, self.workunit.id, self.workunit.name)

MixIn(database.Platform, front_Platform)
MixIn(database.App, front_App)
MixIn(database.AppVersion, front_AppVersion)
MixIn(database.User, front_User)
MixIn(database.Team, front_Team)
MixIn(database.Host, front_Host)
MixIn(database.Workunit, front_Workunit)
MixIn(database.Result, front_Result)
