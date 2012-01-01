#!/bin/bash
BASEDIR=$(dirname $0)
cd $BASEDIR

IDE=xcode
mkdir -p -v $(IDE)

cd $(IDE)
cp -v ../premake4/*.lua .
../premake4.osx $(IDE)
