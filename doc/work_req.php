The work_req element of a scheduler RPC request
is in units of 'normalized CPU seconds'.

A request of X normalized CPU seconds
is asking for enough work to keep one CPU
busy for X seconds of wall-clock time.

In deciding how much work this is,
the scheduler must take into account:

1) the project's resource share fraction;
2) the host's on-fraction
3) the host's active-fraction.

For example, suppose a host has a 1 GFLOP/sec CPU,
the project's resource share fraction is 0.5,
the host's on-fraction is 0.8
and the host's active-fraction is 0.9.

Then the expected processing rate per CPU is

1*0.5*0.8*0.9 GFLOP/sec = 0.36 GFLOP/sec

Suppose the host requests 1000 seconds of work.
Then the scheduler should send it at least 360 GFLOPs of work.
