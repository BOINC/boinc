#!/usr/bin/env python
'''
Generic Assimilator framework
'''

import os, re, signal, sys, time, hashlib
import boinc_path_config
from Boinc import database, boinc_db, boinc_project_path, configxml, sched_messages

# Peter Norvig's Abstract base class hack
def abstract():
    """
    This function is not necessary, but provides
    a nice error message when Abstract methods are not
    overridden by child classes.
    See: http://norvig.com/python-iaq.html for details. 
    """
    import inspect
    # get the name of the calling function off the stack
    caller = inspect.getouterframes(inspect.currentframe())[1][3]
    raise NotImplementedError(caller + ' must be implemented in subclass')

class Assimilator():
    '''
    Use this class to create new pure-Python Assimilators.
    To create a new assimilator:
      1) call __init__ from the new child class' __init__ method
      2) override the assimilate_handler method
      3) add the standard if __name__ == "__main__" bootstrap (see end of this file)
    '''

    def __init__(self):
        # Be sure to call Assimilator.__init__(self) from child classes
        
        # HACK: this belongs in boinc_db.py!
        boinc_db.WU_ERROR_NO_CANONICAL_RESULT = 32
        
        # initialize member vars
        self.config = None
        self.STOP_TRIGGER_FILENAME = boinc_project_path.project_path('stop_daemons')
        self.caught_sig_int = False
        self.log=sched_messages.SchedMessages()
        self.pass_count = 0
        self.update_db = True
        self.noinsert = False
        self.wu_id_mod = 0
        self.wu_id_remainder = 0
        self.one_pass = False
        self.one_pass_N_WU = 0
        self.appname = ''
        self.sleep_interval = 10
    
    def check_stop_trigger(self):
        """
        Stops the daemon when not running in one_pass mode
        There are two cases when the daemon will stop:
           1) if the SIGINT signal is received 
           2) if the stop trigger file is present
        """
        
        try:
            junk = open(self.STOP_TRIGGER_FILENAME, 'r')
        except IOError:
            if self.caught_sig_int:
                self.logCritical("Caught SIGINT\n")
                sys.exit(1)
        else:
            self.logCritical("Found stop trigger\n")
            sys.exit(1)
    
    def sigint_handler(self, sig, stack):
        """
        This method handles the SIGINT signal. It sets a flag
        but waits to exit until check_stop_trigger is called 
        """
        self.logDebug("Handled SIGINT\n")
        self.caught_sig_int = True
    
    def filename_hash(self, name, hash_fanout):
        """
        Accepts a filename (without path) and the hash fanout. 
        Returns the directory bucket where the file will reside.
        The hash fanout is typically provided by the project config file.        
        """
        h = hex(int(hashlib.md5(name).hexdigest()[:8], 16) % hash_fanout)[2:]
        
        # check for the long L suffix. It seems like it should 
        # never be present but that isn't the case
        if h.endswith('L'):
            h = h[:-1]
        return h
    
    def get_file_path(self, result):
        """
        Accepts a result object and returns the relative path to the file.
        This method accounts for file hashing and includes the directory 
        bucket in the path returned.
        """
        name = re.search('<file_name>(.*)</file_name>',result.xml_doc_in).group(1)
        fanout = int(self.config.uldl_dir_fanout)
        hashed = self.filename_hash(name, fanout)
        updir = self.config.upload_dir
        result = os.path.join(updir,hashed,name)
        return result
    
    def assimilate_handler(self, wu, results, canonical_result):
        """
        This method is called for each workunit (wu) that needs to be
        processed. A canonical result is not guarenteed and several error
        conditions may be present on the wu. Call report_errors(wu) when
        overriding this method.
        
        Note that the -noinsert flag (self.noinsert) must be accounted for when
        overriding this method.
        """
        abstract()
        
    def report_errors(self, wu):
        """
        Writes error logs based on the workunit (wu) error_mask field.
        Returns True if errors were present, False otherwise.
        """
        if wu.error_mask&boinc_db.WU_ERROR_COULDNT_SEND_RESULT:
            self.logCritical("[%s] Error: couldn't send a result\n", wu.name)
            return True
        if wu.error_mask&boinc_db.WU_ERROR_TOO_MANY_ERROR_RESULTS:
            self.logCritical("[%s] Error: too many error results\n", wu.name)
            return True
        if wu.error_mask&boinc_db.WU_ERROR_TOO_MANY_TOTAL_RESULTS:
            self.logCritical("[%s] Error: too many total results\n", wu.name)
            return True
        if wu.error_mask&boinc_db.WU_ERROR_TOO_MANY_SUCCESS_RESULTS:
            self.logCritical("[%s] Error: too many success results\n", wu.name)
            return True
        return False
    
    def do_pass(self, app):
        """
        This method scans the database for workunits that need to be 
        assimilated. It handles all processing rules passed in on the command  
        line, except for -noinsert, which must be handled in assimilate_handler.
        Calls check_stop_trigger before doing any work.
        """

        did_something=False
        # check for stop trigger
        self.check_stop_trigger()
        self.pass_count += 1
        n = 0
    
        units = database.Workunits.find(app=app,assimilate_state=boinc_db.ASSIMILATE_READY)
    
        self.logDebug("pass %d, units %d\n", self.pass_count, len(units))
    
        # look for workunits with correct appid and 
        # assimilate_state==ASSIMILATE_READY
        for wu in units:
            # if the user has turned on the WU mod flag, adhere to it
            if self.wu_id_mod > 0 and self.wu_id_remainder > 0:
                if wu.id % self.wu_id_mod != self.wu_id_remainder:
                    continue
            
            # track how many jobs have been processed
            # stop if the limit is reached
            n += 1
            if self.one_pass_N_WU > 0 and n > self.one_pass_N_WU: 
                return did_something
    
            # only mark as dirty if the database is modified
            if self.update_db:
                did_something=True
    
            canonical_result = None
            results = None
            self.logDebug("[%s] assimilating: state=%d\n", wu.name, wu.assimilate_state)
            results = database.Results.find(workunit=wu)

            # look for canonical result for workunit in results
            for result in results:
                if result == wu.canonical_result:
                    canonical_result=result
    
            if canonical_result == None and wu.error_mask == 0:
                # If no canonical result found and WU had no other errors,
                # something is wrong, e.g. result records got deleted prematurely.
                # This is probably unrecoverable, so mark the WU as having
                # an assimilation error and keep going.
                wu.error_mask = boinc_db.WU_ERROR_NO_CANONICAL_RESULT
                wu.commit()
                
            # assimilate handler
            self.assimilate_handler(wu, results, canonical_result)
    
            # TODO: check for DEFER_ASSIMILATION as a return value from assimilate_handler
            
            if self.update_db:
                # tag wu as ASSIMILATE_DONE
                wu.assimilate_state = boinc_db.ASSIMILATE_DONE
                wu.transition_time = int(time.time())
                wu.commit()
    
        # return did something result
        return did_something

    def parse_args(self, args):
        """
        Parses arguments provided on the command line and sets
        those argument values as member variables. Arguments
        are parsed as their true types, so integers will be ints, 
        not strings.
        """
        
        args.reverse()
        while(len(args)):
            arg = args.pop()
            if arg == '-sleep_interval':
                arg = args.pop()
                self.sleep_interval = float(arg)
            elif arg == '-one_pass':
                self.one_pass = True
            elif arg == '-one_pass_N_WU':
                arg = args.pop()
                self.one_pass_N_WU = int(arg)
            elif arg == '-noinsert':
                self.noinsert = True
            elif arg == '-dont_update_db':
                self.update_db = False
            elif arg == '-mod':
                self.wu_id_mod = int(args.pop())
                self.wu_id_remainder = int(args.pop())
            elif arg == '-d':
                arg = args.pop()
                self.log.set_debug_level(arg)
            elif arg == '-app':
                arg = args.pop()
                self.appname = arg
            else:
                self.logCritical("Unrecognized arg: %s\n", arg)

    def run(self):
        """
        This function runs the class in a loop unless the
        one_pass or one_pass_WU_N flags are set. Before execution
        parse_args() is called, the xml config file is loaded and
        the SIGINT signal is hooked to the sigint_handler method.
        """
        self.parse_args(sys.argv[1:])
        self.config = configxml.default_config().config

        # retrieve app where name = app.name
        database.connect()
        app=database.Apps.find1(name=self.appname)
        database.close()
        
        signal.signal(signal.SIGINT, self.sigint_handler)
        
        # do one pass or execute main loop
        if self.one_pass:
            self.do_pass(app)
        else:
            # main loop
            while(1):
                database.connect()
                workdone = self.do_pass(app)
                database.close()
                if not workdone:
                    time.sleep(self.sleep_interval)
    
    def _writeLog(self, mode, *args):
        """
        A private helper function for writeing to the log
        """
        self.log.printf(mode, *args)
        
    def logCritical(self, *messageArgs):
        """
        A helper function for logging critical messages
        """
        self._writeLog(sched_messages.CRITICAL, *messageArgs)
    
    def logNormal(self, *messageArgs):
        """
        A helper function for logging normal messages
        """
        self._writeLog(sched_messages.NORMAL, *messageArgs)
    
    def logDebug(self, *messageArgs):
        """
        A helper function for logging debug messages
        """
        self._writeLog(sched_messages.DEBUG, *messageArgs)

# --------------------------------------------
# Add the following to your assimilator file:

#if __name__ == '__main__':
#    asm = YourAssimilator()
#    asm.run()

