#!/bin/bash
BASEDIR=$(dirname $0)
cd $BASEDIR

IDE="xcode3"
mkdir -p -v $IDE

cd $IDE
cp -v ../premake4/*.lua .
../premake4/premake4.osx $IDE
