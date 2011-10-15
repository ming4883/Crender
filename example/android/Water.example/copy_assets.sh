#echo pwd = $(pwd)
echo removing old assets...
if [ -d assets ]; then
    rm -r assets
fi

if [ ! -d assets ]; then
    mkdir assets
fi

echo copying assets...
cp ../../../media/Water.gles assets
#cp ../../../media/Reflection.gles assets
cp ../../../media/Common.gles assets
