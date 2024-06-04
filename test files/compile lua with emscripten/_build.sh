#!/bin/sh

# add emscripten to PATH (tested with emcc 3.1.60)
source $EMSDK/emsdk_env.sh

echo '##################################'
echo 'about to compile LUA...'
echo ''

make clean
make generic CC='emcc' AR='emar rcu' RANLIB='emranlib'

echo '##################################'
echo 'if succesful use "liblua.a"'
echo ''

