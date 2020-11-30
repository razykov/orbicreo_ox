#/bin/bash

SHARE_DIST=/usr/share/orbicreo/

cp bin/orbicreo /usr/bin/
cp share/orbicreo.h /usr/include/

mkdir -p $SHARE_DIST

cp share/main.c       $SHARE_DIST
cp share/recipe.json  $SHARE_DIST
cp share/version.json $SHARE_DIST
