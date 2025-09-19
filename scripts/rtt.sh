#!/bin/bash

until nc localhost 19021 > data/output.bin; do
  sleep 1
done