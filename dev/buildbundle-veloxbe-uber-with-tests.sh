#!/bin/bash -ex

BASEDIR=$(dirname $0)
source "$BASEDIR/builddeps-veloxbe.sh"
pwd

ls -al /etc/ssl/certs/java/cacerts || true

export MAVEN_OPTS="-Djavax.net.ssl.trustStore=/etc/ssl/certs/java/cacerts"

echo "Important Env vars; check them below to be doubly sure"
printenv | grep "MAVEN_OPTS"
printenv | grep "JAVA_HOME"

mvn clean package -Pbackends-velox -Pceleborn -Pspark-3.3 -e
