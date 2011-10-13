#echo pwd = $(pwd)
echo removing old assets...
if [ -d assets ]; then
    rm -r assets
fi

if [ ! -d assets ]; then
    mkdir assets
fi

echo copying assets...
cp ../../../media/Mesh.gles assets
cp ../../../media/monkey.obj assets
