#! /bin/bash

resolve () {
    sed 's/<soft_link>..\/..\/projects\/[^\/]*\//project\//; s/<\/soft_link>//' $1 | tr -d '\r\n'
}

$(resolve slot/worker) --nsecs 1 $(resolve slot/in) slot/out
