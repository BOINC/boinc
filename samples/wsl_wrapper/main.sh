#! /bin/bash

# Shim for running worker with wsl_wrapper or docker_wrapper.
# Resolves exec/in/out filenames, and passes cmdline args to worker.

resolve () {
    sed 's/<soft_link>//; s/<\/soft_link>//' $1 | tr -d '\r\n'
}

$(resolve worker) $@ $(resolve in) $(resolve out)
