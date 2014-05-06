#!/bin/sh

mkdir target
mkdir extras
cd extras

#building the Openal
echo ""
echo "##############################"
echo "#      Building OpenAL       #"
echo "##############################"
echo ""
wget http://kcat.strangesoft.net/openal-releases/openal-soft-1.15.1.tar.bz2
tar xpjf openal-soft-1.15.1.tar.bz2
cd openal-soft-1.15.1
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$PWD/../../../target ../
make all && make install
cd ../../

#building the libcurl
echo ""
echo "##############################"
echo "#      Building LibCURL      #"
echo "##############################"
echo ""
wget http://curl.haxx.se/download/curl-7.36.0.tar.gz
tar xvzf curl-7.36.0.tar.gz
cd curl-7.36.0
./configure --prefix=$PWD/../../target
make && make install
cd ../

#building the libflac
echo ""
echo "##############################"
echo "#       Building LibFLAC        #"
echo "##############################"
echo ""
wget http://downloads.xiph.org/releases/flac/flac-1.3.0.tar.xz
tar xf flac-1.3.0.tar.xz
cd flac-1.3.0
./configure --prefix=$PWD/../../target
make && make install
cd ../

#building the unittest++
echo ""
echo "##############################"
echo "#    Building UnitTest++     #"
echo "##############################"
echo ""
git clone https://github.com/unittest-cpp/unittest-cpp.git UnitTest++
cd UnitTest++
cd builds
cmake -DCMAKE_INSTALL_PREFIX=$PWD/../../../target/ ../
make all && make install
cd ../../

#building the scons
echo ""
echo "##############################"
echo "#      Building Scons        #"
echo "##############################"
echo ""
wget http://prdownloads.sourceforge.net/scons/scons-2.3.0.tar.gz
tar xvzf scons-2.3.0.tar.gz
cd scons-2.3.0
python setup.py install --prefix=$PWD/../../target
cd ../


#building the libjsconcpp
echo ""
echo "##############################"
echo "#      Building Jsoncpp      #"
echo "##############################"
echo ""
wget "http://downloads.sourceforge.net/project/jsoncpp/jsoncpp/0.5.0/jsoncpp-src-0.5.0.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fjsoncpp%2F&ts=1294425421&use_mirror=freefr" -O jsoncpp-src-0.5.0.tar.gz
tar xvzf jsoncpp-src-0.5.0.tar.gz
cd jsoncpp-src-0.5.0
python $PWD/../../target/bin/scons platform=linux-gcc check
cp ./libs/linux-gcc-4.6/libjson_linux-gcc-4.6_libmt.so $PWD/../../target/lib/libjsoncpp.so
cp ./libs/linux-gcc-4.6/libjson_linux-gcc-4.6_libmt.a $PWD/../../target/lib/libjsoncpp.a
cp ./include/json -r $PWD/../../target/include/
cd ../


cd ../
