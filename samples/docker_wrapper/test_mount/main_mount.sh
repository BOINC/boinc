#! /bin/bash

resolve () {
    sed 's/<soft_link>//; s/<\/soft_link>//' $1 | tr -d '\r\n'
}

$(resolve worker) --nsecs 60 $(resolve in) $(resolve out)
