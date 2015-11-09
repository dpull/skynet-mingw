mklink /j 3rd skynet\3rd
mklink /j lualib-src skynet\lualib-src
mklink /j service-src skynet\service-src
mklink /j skynet-src skynet\skynet-src

Set Path=C:\MinGW\msys\1.0\bin;C:\MinGW\bin
make -f AutoBuild