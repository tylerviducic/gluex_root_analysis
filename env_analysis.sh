#!/bin/bash

# SET ROOT_ANALYSIS_HOME
SCRIPT_PATH=${BASH_SOURCE[0]}
if [[ -h $SCRIPT_PATH ]]; then
	SCRIPT_PATH=$(readlink $SCRIPT_PATH)
fi
export ROOT_ANALYSIS_HOME=$(cd $(dirname $SCRIPT_PATH); pwd)

# SET BMS_OSNAME, IF NOT SET ALREADY # COPIED FROM sim-recon
if [[ $BMS_OSNAME == "" ]]; then
	export BMS_OSNAME=`$ROOT_ANALYSIS_HOME/osrelease.pl`
fi

if [[ `uname` != "Darwin" ]]; then

   # SET LIBRARY, PATH
   if [[ $LD_LIBRARY_PATH == "" ]]; then
       export LD_LIBRARY_PATH
   fi
   export LD_LIBRARY_PATH=$ROOT_ANALYSIS_HOME/$BMS_OSNAME/lib:$LD_LIBRARY_PATH
else
    
   # SET LIBRARY, PATH
   if [[ $DYLD_LIBRARY_PATH == "" ]]; then
       export DYLD_LIBRARY_PATH
   fi
   export DYLD_LIBRARY_PATH=$ROOT_ANALYSIS_HOME/$BMS_OSNAME/lib:$DYLD_LIBRARY_PATH
fi

export PATH=$ROOT_ANALYSIS_HOME/$BMS_OSNAME/bin:$PATH
