#!/bin/bash
basepath=$(cd `dirname $0`; pwd)
APP_HOME=$basepath/..

JAVA=/usr/bin/java

JVM_OPT="$JVM_OPT -Djava.library.path=$APP_HOME/bin"
JVM_OPT="$JVM_OPT -classpath"
JVM_OPT="$JVM_OPT $APP_HOME/bin/CodeCraft-2021.jar"

$JAVA  $JVM_OPT com.huawei.java.main.Main 2>&1
exit