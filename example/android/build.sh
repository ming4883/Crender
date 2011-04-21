ndk-build NDK_MODULE_PATH=../..

if [ -d assets ]; then
    rm -r assets
fi

if [ ! -d assets ]; then
    mkdir assets
fi
cp ../../media/*.gles assets
cp ../../media/monkey.obj assets
