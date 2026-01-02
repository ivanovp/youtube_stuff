#!/bin/sh
sox hanglejatszo_proba.mp3 -r 8000 -c 2 -b 16 -e signed-integer -L hanglejatszo_proba.raw
./convert.py hanglejatszo_proba.raw i2s_data
