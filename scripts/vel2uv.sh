#! /bin/sh

input=$1
u=$2
v=$3

(extg $input -iv 1 -v 1 $u; extg $input -iv 2 -v 1 $v)

