# set params
NDK_ROOT=/c/NVPACK/android-ndk-r8
COCOS2DX_ROOT=/e/cocos2dx20
GAME_ROOT=$COCOS2DX_ROOT/CutTheRopex
GAME_ANDROID_ROOT=$GAME_ROOT/proj.android
RESOURCE_ROOT=$GAME_ROOT/Resources

buildexternalsfromsource=

usage(){
cat << EOF
usage: $0 [options]

Build C/C++ native code using Android NDK

OPTIONS:
   -s	Build externals from source
   -h	this help
EOF
}

while getopts "s" OPTION; do
	case "$OPTION" in
		s)
			buildexternalsfromsource=1
			;;
		h)
			usage
			exit 0
			;;
	esac
done

# make sure assets is exist
if [ -d $GAME_ANDROID_ROOT/assets ]; then
    rm -rf $GAME_ANDROID_ROOT/assets
fi

mkdir $GAME_ANDROID_ROOT/assets

# copy resources
for file in $RESOURCE_ROOT/*
do
    if [ -d "$file" ]; then
        cp -rf "$file" $GAME_ANDROID_ROOT/assets
    fi

    if [ -f "$file" ]; then
        cp "$file" $GAME_ANDROID_ROOT/assets
    fi
done

if [[ $buildexternalsfromsource ]]; then
    echo "Building external dependencies from source"
    $NDK_ROOT/ndk-build -C $GAME_ANDROID_ROOT \
        NDK_MODULE_PATH=${COCOS2DX_ROOT}:${COCOS2DX_ROOT}/cocos2dx/platform/third_party/android/source
else
    echo "Using prebuilt externals"
    $NDK_ROOT/ndk-build -C $GAME_ANDROID_ROOT \
        NDK_MODULE_PATH=${COCOS2DX_ROOT}:${COCOS2DX_ROOT}/cocos2dx/platform/third_party/android/prebuilt
fi



