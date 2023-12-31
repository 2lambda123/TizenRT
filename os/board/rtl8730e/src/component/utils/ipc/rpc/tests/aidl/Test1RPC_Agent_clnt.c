/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <string.h> /* for memset */
#include <Test1RPC_Agent.h>
#include <rpc_client.h>

#include <RPCBaseDS.h>
#include <Test1RPCBaseDS.h>

HRESULT *
TEST1_RPC_ToSystem_ErrorStatus_0(TEST1_RPC_ERROR *argp, CLNT_STRUCT *clnt)
{
	RPC_STRUCT rpc;
	HRESULT *result = NULL ;
	long args_size = sizeof(TEST1_RPC_ERROR);


	// if NONBLOCK_MODE, dont need to alloc memory
	if (clnt->send_mode & BLOCK_MODE) {
		result = (HRESULT *) rpc_malloc(sizeof(HRESULT));
	}


	// prepare the RPC call structure
	// including programID, versionID, TaskID...
	rpc = RPC_PrepareCall(clnt, (int)result);


	if (RPC_ClientCall(&rpc, TEST1_RPC_ToSystem_ErrorStatus, clnt->send_mode,
					   (xdrproc_t) xdr_TEST1_RPC_ERROR, (caddr_t) argp, args_size)
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
