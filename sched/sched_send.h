extern int send_work(
    SCHEDULER_REQUEST&, SCHEDULER_REPLY&, PLATFORM&, SCHED_SHMEM&
);

extern int add_result_to_reply(
    DB_RESULT& result, WORKUNIT& wu, SCHEDULER_REPLY& reply, PLATFORM&,
    WORK_REQ& wreq, APP* app, APP_VERSION* avp
);

extern bool anonymous(PLATFORM&);

extern bool find_app_version(
    WORK_REQ& wreq, WORKUNIT& wu, PLATFORM& platform, SCHED_SHMEM& ss,
    APP*& app, APP_VERSION*& avp
);

extern bool app_core_compatible(WORK_REQ& wreq, APP_VERSION& av);
