/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: srpcf.h
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//
// Definitions
//
#define SRPCF_CODE_REVISION			"1.0.0"
#define SRPCF_SPEC_REVISION			"1.0.0"

#define SRPCF_FUNC_MAXLEN          	100
#define SRPCF_HELPER_PREFIX    		"srpcfHelper_"
#define SRPCF_ERROR_PREFIX			"srpcfError_"
#define SRPCF_PARSER_PREFIX    		"srpcfParser_"
#define SRPCF_EXECUTOR_PREFIX		"srpcfExecutor_"

#define SRPCF_SUPPORT( NAME )		{ NAME, FALSE, #NAME }
#define SRPCF_SUPPORT_END			{ XR_END_SRPCF, FALSE, NULL }


//
// Enumernations
//
typedef enum _srpcfCmds {

	// Start of SRPCF commands
	XR_START_SRPCF = 0,

	// Help
	xrHelp,

	// Others
	xrCpuInfo,
	xrPciList,
	xrRtcDateSet,
	xrRtcDateShow,
	xrRtcSet,
	xrRtcShow,
	xrDateShow,
	xrTimeShow,

	// End of SRPCF commands
	XR_END_SRPCF,

} srpcfCmds;


