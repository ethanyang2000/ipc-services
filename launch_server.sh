#!/bin/bash

n_sms_value=3
sms_size_value=64

export LD_LIBRARY_PATH=snappy-c:$LD_LIBRARY_PATH
./bin/service --n_sms "$n_sms_value" --sms_size "$sms_size_value"