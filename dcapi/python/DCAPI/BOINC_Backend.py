import os

from DCAPI.DCAPI_Backend import DCAPI_Backend

import boinc_path_config
from Boinc import configxml

class BOINC_Backend(DCAPI_Backend):
    """DC-API deployment interface for BOINC"""

    def __init__(self):
        DCAPI_Backend.__init__(self)
        try:
            self.config = configxml.default_config()
        except:
            raise SystemExit("Failed to locate/parse the BOINC project configuration")
        self.master_keys.extend([("ProjectRootDir", False)])
        self.client_keys.extend([("Redundancy", False),
                                 ("MaxOutputSize", False),
                                 ("MaxMemUsage", False),
                                 ("MaxDiskUsage", False),
                                 ("EstimatedFPOps", True),
                                 ("MaxFPOps", True),
                                 ("DelayBound", True)])

    def master_defaults(self):
        return {'ProjectRootDir': os.path.dirname(self.config.filename)} 
