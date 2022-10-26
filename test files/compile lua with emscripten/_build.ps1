$env:Path += ";C:\msys64\usr\bin"
& make clean
& make generic CC='emcc -s WASM=1' AR='emar rcu' RANLIB='emranlib'

& echo '##################################'
& echo 'if succesful use "liblua.a"'
& echo ''

