#!/bin/bash
#
# SRPCF - Simple Remote Procedire Command Framework
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

SRPCF_FILE="templates/cmds.txt"
TEMPLATE_FILE="templates/CMD_TEMP.c"

rm -f *.c
for cmd in `cat ${SRPCF_FILE}`
do
	echo "Generate SRPCF source code for ${cmd}"
	SRPCFNAME=${cmd}
	FILENAME=${cmd}.c
	cat $TEMPLATE_FILE | sed -e s/#FILENAME#/${FILENAME}/g | sed -e s/#SRPCFNAME#/${SRPCFNAME}/g > ${FILENAME}

done

