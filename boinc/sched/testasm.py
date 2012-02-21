!/usr/bin/env python

import boinc_path_config
from Boinc.assimilator import *

class TestAssimilator(Assimilator):
    """
    A minimal Assimilator example that dumps the result out to the log 
    """
    
    def __init__(self):
        Assimilator.__init__(self)
    
    def assimilate_handler(self, wu, results, canonical_result):
        """
        Assimilates a canonical result, in this case assimilation
        means dumping the contents of the result to the log.
        Also calls report_errors to log any problems present in the workunit (wu)
        """
        
        # check for valid wu.canonical_result
        if wu.canonical_result:
            # do application specific processing
            self.logNormal("[%s] Found canonical result\n", wu.name)
            result = self.get_file_path(canonical_result)
            for line in open(result, 'r').readlines():
                line = line.strip()
                self.logDebug("  [%s] Answer found %s %s\n", canonical_result.name, line[-32:], line[:-33])
        else:
            self.logNormal("[%s] No canonical result\n", wu.name)
        
        if self.report_errors(wu):
            # report_errors returns true if error state was present
            # perhaps add some special logic here
            # even if no logic is required, report_errors should be called
            pass    

# allow the module to be executed as an application
if __name__ == '__main__':
    asm = TestAssimilator()
    asm.run()
