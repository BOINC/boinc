class DCAPI_Backend:
    def __init__(self):
        self.master_keys = [("WorkingDirectory", True),
                            ("InstanceUUID", True),
                            ("LogLevel", False),
                            ("LogFile", False)]
        self.client_keys = [("SendCfgKeys", False),
                            ("LogLevel", False)]
    
    def master_defaults(self):
        pass
    
    def get_master_keys(self):
        return self.master_keys

    def get_client_keys(self):
        return self.client_keys
