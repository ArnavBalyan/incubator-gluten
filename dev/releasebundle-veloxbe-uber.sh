#!/bin/bash -ex

echo "Build flags: ${OTHER_BUILD_FLAGS}"

# Link native libs once
ldconfig

# Compile Velox
velox_dir=$(dirname $(realpath "$0"))/../ep/build-velox/build/velox_ep
export VELOX_HOME=${velox_dir}
BASEDIR=$(dirname $0)
source "$BASEDIR/builddeps-veloxbe.sh"

# Update versions before deploy
velox_sha=$(git -C ${velox_dir} rev-parse --short=15 HEAD)
gluten_sha=$(git rev-parse --short=15 HEAD)
old_gluten_version=1.2.0-SNAPSHOT
new_gluten_version=1.2.0-${gluten_sha}-${velox_sha}
find ./* -name "*.xml" -type f -exec sed -i "s/${old_gluten_version}/${new_gluten_version}/g" {} +

# the deploy
CACERTS_PATH=/opt/java/cacerts && ls -al ${CACERTS_PATH}
export MAVEN_OPTS="-Xmx3000m -Djavax.net.ssl.trustStore=${CACERTS_PATH}"
mvn clean package -Pbackends-velox -Pceleborn -Pspark-3.3 -DskipTests -e
echo "Built jars successfully!"
du -sh package/target/*
mvn -s ./dev/settings.xml deploy -Pbackends-velox -Pceleborn -Pspark-3.3 -DskipTests -e
