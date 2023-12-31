/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <string.h> /* for memset */
#include <Test2RPC_Agent.h>
#include <rpc_client.h>

#include <RPCBaseDS.h>
#include <Test2RPCBaseDS.h>

HRESULT *
TEST2_RPC_ToSystem_Streaming_Callback_0(TEST2_RPC_STREAMING_RESULT *argp, CLNT_STRUCT *clnt)
{
	RPC_STRUCT rpc;
	HRESULT *result = NULL ;
	long args_size = sizeof(TEST2_RPC_STREAMING_RESULT);


	// if NONBLOCK_MODE, dont need to alloc memory
	if (clnt->send_mode & BLOCK_MODE) {
		result = (HRESULT *) rpc_malloc(sizeof(HRESULT));
	}


	// prepare the RPC call structure
	// including programID, versionID, TaskID...
	rpc = RPC_PrepareCall(clnt, (int)result);


	if (RPC_ClientCall(&rpc, TEST2_RPC_ToSystem_Streaming_Callback, clnt->send_mode,
					   (xdrproc_t) xdr_TEST2_RPC_STREAMING_RESULT, (caddr_t) argp, args_size)
		!= 0) {
		if (result) {
			rpc_free(result);
		}
		return (HRESULT *) - 1;
	}


	//for blocking use
	if (clnt->send_mode & BLOCK_MODE) {
		XDR xdrs;

		WaitReply();
		xdrmem_create(&xdrs, (char *)result, sizeof(HRESULT), XDR_DECODE);
		if (!xdr_HRESULT(&xdrs, result)) {
			return (HRESULT *) - 1;
		}
		return result;
	}

	return 0;

}

HRESULT *
TEST2_RPC_ToSystem_Recognizer_Callback_0(TEST2_RPC_RECOGNIZER_RESULT *argp, CLNT_STRUCT *clnt)
{
	RPC_STRUCT rpc;
	HRESULT *result = NULL ;
	long args_size = sizeof(TEST2_RPC_RECOGNIZER_RESULT);


	// if NONBLOCK_MODE, dont need to alloc memory
	if (clnt->send_mode & BLOCK_MODE) {
		result = (HRESULT *) rpc_malloc(sizeof(HRESULT));
	}


	// prepare the RPC call structure
	// including programID, versionID, TaskID...
	rpc = RPC_PrepareCall(clnt, (int)result);


	if (RPC_ClientCall(&rpc, TEST2_RPC_ToSystem_Recognizer_Callback, clnt->send_mode,
					   (xdrproc_t) xdr_TEST2_RPC_RECOGNIZER_RESULT, (caddr_t) argp, args_size)
		!= 0) {
		if (result) {
			rpc_free(result);
		}
		return (HRESULT *) - 1;
	}


	//for blocking use
	if (clnt->send_mode & BLOCK_MODE) {
		XDR xdrs;

		WaitReply();
		xdrmem_create(&xdrs, (char *)result, sizeof(HRESULT), XDR_DECODE);
		if (!xdr_HRESULT(&xdrs, result)) {
			return (HRESULT *) - 1;
		}
		return result;
	}

	return 0;

}

HRESULT *
TEST2_RPC_ToSystem_Error_0(TEST2_RPC_ERROR_STATE *argp, CLNT_STRUCT *clnt)
{
	RPC_STRUCT rpc;
	HRESULT *result = NULL ;
	long args_size = sizeof(TEST2_RPC_ERROR_STATE);


	// if NONBLOCK_MODE, dont need to alloc memory
	if (clnt->send_mode & BLOCK_MODE) {
		result = (HRESULT *) rpc_malloc(sizeof(HRESULT));
	}


	// prepare the RPC call structure
	// including programID, versionID, TaskID...
	rpc = RPC_PrepareCall(clnt, (int)result);


	if (RPC_ClientCall(&rpc, TEST2_RPC_ToSystem_Error, clnt->send_mode,
					   (xdrproc_t) xdr_TEST2_RPC_ERROR_STATE, (caddr_t) argp, args_size)
		!= 0) {
		if (result) {
			rpc_free(result);
		}
		return (HRESULT *) - 1;
	}


	//for blocking use
	if (clnt->send_mode & BLOCK_MODE) {
		XDR xdrs;

		WaitReply();
		xdrmem_create(&xdrs, (char *)result, sizeof(HRESULT), XDR_DECODE);
		if (!xdr_HRESULT(&xdrs, result)) {
			return (HRESULT *) - 1;
		}
		return result;
	}

	return 0;

}
