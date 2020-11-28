#/bin/bash

SHARE_DIST=/usr/share/orbicreo/

cp bin/orbicreo /usr/bin/

mkdir -p $SHARE_DIST

cp share/* $SHARE_DIST
