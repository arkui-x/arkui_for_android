#!/bin/sh
echo Start Buil
#./AcrossArkUITestSuite/gradlew tasks 

SETTING_GRADLE=settings.gradle
GRADLE_COMMAND=./gradlew

getAllModules() {
    echo $(cat $SETTING_GRADLE | grep "^i" | cut -f2 -d "'")
    #return $?
}
modules=$(getAllModules)
echo Get Result
echo $modules
for module in $modules
do
    echo Start Running Test module $module && $GRADLE_COMMAND $module:connectedAndroidTest && echo Finish Running Test module $module
done