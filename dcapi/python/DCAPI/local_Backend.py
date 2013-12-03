from DCAPI.DCAPI_Backend import DCAPI_Backend

class local_Backend(DCAPI_Backend):
    """DC-API deployment interface for local execution"""

    def __init__(self):
        DCAPI_Backend.__init__(self)
        self.master_keys.extend([("SleepingInterval", False)])
        self.client_keys.extend([("ClientMessageBox", False),
                                 ("MasterMessageBox", False),
                                 ("SubresultBox", False),
                                 ("SystemMessageBox", False),
                                 ("Executable", True),
                                 ("LeaveFiles", False),
                                 ("CheckpointFile", False),
                                 ("SavedOutputs", False)])
