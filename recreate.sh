#!/bin/bash
function error_exit { echo "$1" 1>&2; exit 1; }
./clean.sh || error_exit "Failed to clean up."
./execute.sh || error_exit "Failed to build project."
