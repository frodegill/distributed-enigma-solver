#!/bin/sh
cd client/ &&
ln -sf ../common/common.h &&
ln -sf ../common/common.c &&
make clean &&
cd ../server/ &&
ln -sf ../common/common.h &&
ln -sf ../common/common.c &&
make clean
