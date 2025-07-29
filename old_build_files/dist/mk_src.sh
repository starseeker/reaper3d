#!/bin/sh

. ./mk_init.sh

cd $TMP


rm -rf ${VER}-src
mkdir ${VER}-src
cd ${VER}-src
mkdir ${DEST}

cp -R ${DIR}/src ${DEST}
cp -R ${DIR}/build ${DEST}
mkdir -p ${DEST}/plugins/win32

cp ${DIR}/LICENSE ${DEST}
cp ${DIR}/README ${DEST}
cp ${DIR}/INSTALL ${DEST}
cp ${DIR}/ChangeLog ${DEST}

if [ ! -f ${TMP}/win32-support.zip ]; then
    lynx -dump http://zarquon:88/~project/dist/win32-support.zip > ${TMP}/win32-support.zip
fi
unzip -q ${TMP}/win32-support.zip -d ${DEST}/build/vs

(cd ${DEST}/build/ac ; ./bootstrap.sh; touch stamp-h.in)

 
find ${DEST} \( -name '*.h' -or -name '*.c' -or -name '*.cpp' \
                -or -name 'read*' -or -name 'READ*' \
                -or -name 'INSTA*' -or -name 'LICENS*' \) -print0 \
        | xargs -0 conv.sh

if [ "$VER" = "linux" ]; then
    tar zcf ../${DEST}-src.tar.gz ${DEST}
else
    rm -f ../${DEST}-src.zip
    zip -q -r ../${DEST}-src.zip ${DEST}
fi

rm -r ${DEST}
cd ..
rmdir ${VER}-src

