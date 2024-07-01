#!/bin/bash -ex

ldconfig

echo "Build flags: ${OTHER_BUILD_FLAGS}"

export VELOX_HOME=$(dirname $(realpath "$0"))/../ep/build-velox/build/velox_ep

BASEDIR=$(dirname $0)
source "$BASEDIR/builddeps-veloxbe.sh"
pwd

CACERTS_PATH=/opt/java/cacerts
ls -al ${CACERTS_PATH}
export MAVEN_OPTS="-Djavax.net.ssl.trustStore=${CACERTS_PATH}"

echo "Important Env vars; check them below to be doubly sure"
printenv | grep "MAVEN_OPTS"
printenv | grep "JAVA_HOME"
printenv | grep "VELOX_HOME"

mvn clean package -Pbackends-velox -Pceleborn -Pspark-3.3 -DskipTests -e
