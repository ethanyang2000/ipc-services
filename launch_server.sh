#!/bin/bash

n_sms_value=1
sms_size_value=32

export LD_LIBRARY_PATH=snappy-c:$LD_LIBRARY_PATH
./bin/service --n_sms "$n_sms_value" --sms_size "$sms_size_value"

if [ $? -ne 0 ]; then
    echo "Error: Server failed to start."
    exit 1
fi

echo "Server started successfully."
exit 0
