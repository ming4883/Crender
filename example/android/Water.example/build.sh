#!/bin/sh

# Invoke ndk-build
echo pwd is: $(pwd)

${NDKROOT}/ndk-build NDK_MODULE_PATH=../../.. "$@"

if [ $? != 0 ] ; then
	echo "ERROR: NDK build failed."
	exit 1
fi

# clean previous assets
if [ -d assets ]; then
	echo cleaning assets...
	rm -r assets
fi


if [[ $@ == *clean* ]]
then
	exit 0
fi

# copy assets
if [ ! -d assets ]; then
	echo creating assets...
	mkdir assets
fi

bash assets.sh

echo "build.sh done"

