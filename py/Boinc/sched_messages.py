import sys, time 

CRITICAL=0
NORMAL=1
DEBUG=2

class SchedMessages:
    def __init__(self):
        self.debug_level = 0

    def set_debug_level(self, level):
        self.debug_level = int(level)

    def printf(self, kind, format, *args):
        if kind <= self.debug_level:
            if kind==CRITICAL:
                kind = "CRITICAL"
            elif kind==NORMAL:
                kind = "normal  "
            elif kind==DEBUG:
                kind = "debug   "
            else:
                kind = "*** internal error: invalid MessageKind ***";
            sys.stderr.write("%s [%s] " % (time.strftime("%Y/%m/%d %H:%M:%S", time.localtime()), kind))
            sys.stderr.write(format % args)
