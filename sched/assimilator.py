#!/usr/bin/python
'''
Python implementation of an assimilator
Contributed by Stephen Pellicer
'''

import os, re, signal, sys, time, hashlib
import database, boinc_db, configxml, sched_messages

STOP_TRIGGER_FILENAME = os.path.join('..', 'stop_servers')
caught_sig_int = False
log_messages=sched_messages.SchedMessages()

def check_stop_trigger():
    global caught_sig_int, log_messages
    try:
        junk = open(STOP_TRIGGER_FILENAME, 'r')
    except IOError:
        if caught_sig_int:
            log_messages.printf(sched_messages.CRITICAL, "Caught SIGINT\n")
            sys.exit(1)
    else:
        log_messages.printf(sched_messages.CRITICAL, "Found stop trigger\n")
        sys.exit(1)

def sigint_handler(sig, stack):
    global caught_sig_int
    log_messages.printf(sched_messages.DEBUG, "Handled SIGINT\n")
    caught_sig_int = True

def filename_hash(name, hash_fanout):
    h = hex(int(hashlib.md5(name).hexdigest()[:8], 16) % hash_fanout)[2:]
    if h.endswith('L'):
        h = h[:-1]
    return h

def get_file_path(result):
    name = re.search('<file_name>(.*)</file_name>',result.xml_doc_in).group(1)
    fanout = int(config.uldl_dir_fanout)
    hashed = filename_hash(name, fanout)
    updir = config.upload_dir
    result = os.path.join(updir,hashed,name)
    return result

def assimilate_handler(wu, results, canonical_result):
    # check for valid wu.canonical_resultid
    if wu.canonical_result:
        # do application specific processing
        log_messages.printf(sched_messages.NORMAL, "[%s] Found canonical result\n", wu.name)
        result = get_file_path(canonical_result)
        for line in open(result, 'r').readlines():
            line = line.strip()
            log_messages.printf(sched_messages.DEBUG, "  [%s] Answer found %s %s\n", canonical_result.name, line[-32:], line[:-33])
    else:
        log_messages.printf(sched_messages.NORMAL, "[%s] No canonical result\n", wu.name)
            
    if wu.error_mask&boinc_db.WU_ERROR_COULDNT_SEND_RESULT:
        log_messages.printf(sched_messages.CRITICAL, "[%s] Error: couldn't send a result\n", wu.name)
    if wu.error_mask&boinc_db.WU_ERROR_TOO_MANY_ERROR_RESULTS:
        log_messages.printf(sched_messages.CRITICAL, "[%s] Error: too many error results\n", wu.name)
    if wu.error_mask&boinc_db.WU_ERROR_TOO_MANY_TOTAL_RESULTS:
        log_messages.printf(sched_messages.CRITICAL, "[%s] Error: too many total results\n", wu.name)
    if wu.error_mask&boinc_db.WU_ERROR_TOO_MANY_SUCCESS_RESULTS:
        log_messages.printf(sched_messages.CRITICAL, "[%s] Error: too many success results\n", wu.name)
    # check for error conditions

pass_count = 0

def do_pass(app):
    global pass_count
    did_something=False
    # check for stop trigger
    check_stop_trigger()
    pass_count += 1

    units = database.Workunits.find(app=app,assimilate_state=boinc_db.ASSIMILATE_READY)

    log_messages.printf(sched_messages.DEBUG, "pass %d, units %d\n", pass_count, len(units))

    # look for workunits with correct appid and 
    # assimilate_state==ASSIMILATE_READY
    for wu in units:
        did_something=True
        canonical_result=None
        results=None
        log_messages.printf(sched_messages.DEBUG, "[%s] assimilating: state=%d\n", wu.name, wu.assimilate_state)
        results = database.Results.find(workunit=wu)
        # look for canonical result for workunit in results
        for result in results:
            if result == wu.canonical_result:
                canonical_result=result

        # assimilate handler
        assimilate_handler(wu, results, canonical_result)

        # tag wu as ASSIMILATE_DONE
        wu.assimilate_state = boinc_db.ASSIMILATE_DONE
        wu.transition_time = int(time.time())
        wu.commit()

        # set wu transition_time

    # return did something result
    return did_something

# main function
asynch = False
one_pass = False
appname = ''

# check for asynch one_pass, debug, app
args = sys.argv[1:]
args.reverse()
while(len(args)):
    arg = args.pop()
    if arg == '-asynch':
        asynch = True
    elif arg == '-one_pass':
        one_pass = True
    elif arg == '-d':
        arg = args.pop()
        log_messages.set_debug_level(arg)
    elif arg == '-app':
        arg = args.pop()
        appname = arg
    else:
        log_messages.printf(sched_messages.CRITICAL, "Unrecognized arg: %s\n", arg)
        
config = configxml.default_config().config
database.connect()

# fork if asynch
if(asynch):
    # add fork code
    pass

# retrieve app where name = app.name
app=database.Apps.find1(name=appname)
signal.signal(signal.SIGINT, sigint_handler)

# do one pass or execute main loop
if one_pass:
    do_pass(app)
else:
    # main loop
    while(1):
	database.close()
	database.connect()
        if not do_pass(app):
            time.sleep(10)
