#!/bin/sh

# link some skynet directories
need_link_dirs=(3rd examples lualib lualib-src service service-src skynet-src test)
for d in ${need_link_dirs[@]}; do
    rm -rf ${d}
    ln -s skynet/${d} ${d}
done


# modify skynet_start.c
SKYNET_START_FILE="skynet-src/skynet_start.c"

cp -rf platform/skynet_condition.c skynet-src/

# insert '#include "skynet_condition.h"' statement(sed insert new line after/before match not supported in mingw env)
awk '/^#include +<pthread.h>/{print "#include \"skynet_condition.h\"";}1' ${SKYNET_START_FILE} > ${SKYNET_START_FILE}.new 
cp -f ${SKYNET_START_FILE}.new ${SKYNET_START_FILE}

# update all pthread_cond_xxxx type&methods to skynet_cond_xxxx type&methods
sed -e 's/pthread_cond_t/skynet_cond_t/g' ${SKYNET_START_FILE} > ${SKYNET_START_FILE}.new && cp -f ${SKYNET_START_FILE}.new ${SKYNET_START_FILE}
sed -e 's/pthread_cond_init/skynet_cond_init/g' ${SKYNET_START_FILE} > ${SKYNET_START_FILE}.new && cp -f ${SKYNET_START_FILE}.new ${SKYNET_START_FILE}
sed -e 's/pthread_cond_wait/skynet_cond_wait/g' ${SKYNET_START_FILE} > ${SKYNET_START_FILE}.new && cp -f ${SKYNET_START_FILE}.new ${SKYNET_START_FILE}
sed -e 's/pthread_cond_signal/skynet_cond_signal/g' ${SKYNET_START_FILE} > ${SKYNET_START_FILE}.new && cp -f ${SKYNET_START_FILE}.new ${SKYNET_START_FILE}
sed -e 's/pthread_cond_broadcast/skynet_cond_broadcast/g' ${SKYNET_START_FILE} > ${SKYNET_START_FILE}.new && cp -f ${SKYNET_START_FILE}.new ${SKYNET_START_FILE}
sed -e 's/pthread_cond_destroy/skynet_cond_destroy/g' ${SKYNET_START_FILE} > ${SKYNET_START_FILE}.new && cp -f ${SKYNET_START_FILE}.new ${SKYNET_START_FILE}
rm -f "${SKYNET_START_FILE}".new

