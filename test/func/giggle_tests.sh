#!/bin/bash

test -e ssshtest || wget -q https://raw.githubusercontent.com/ryanlayer/ssshtest/master/ssshtest

source ssshtest

STOP_ON_FAIL=0


INDEX=../../bin/index

run usage_test $INDEX 
assert_in_stderr "index: usage"
assert_no_stdout 
