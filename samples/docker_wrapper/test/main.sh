#! /bin/bash

resolve () {
    sed 's/<soft_link>..\/..\/projects\/[^\/]*\//\/project\//; s/<\/soft_link>//' $1 | tr -d '\r\n'
}

./worker $@ $(resolve in) out
