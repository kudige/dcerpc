/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Portions of this software have been released under the following terms:
 *
 * (c) Copyright 1989-1993 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989-1993 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989-1993 DIGITAL EQUIPMENT CORPORATION
 *
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 * permission to use, copy, modify, and distribute this file for any
 * purpose is hereby granted without fee, provided that the above
 * copyright notices and this notice appears in all source code copies,
 * and that none of the names of Open Software Foundation, Inc., Hewlett-
 * Packard Company, Apple Inc. or Digital Equipment Corporation be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Neither Open Software
 * Foundation, Inc., Hewlett-Packard Company, Apple Inc. nor Digital
 * Equipment Corporation makes any representations about the suitability
 * of this software for any purpose.
 *
 * Copyright (c) 2007, Novell, Inc. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Novell Inc. ("Apple") nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * 3. Neither the name of the Novell, Inc. nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * SAMR test
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <compat/dcerpc.h>
#include "samrt.h"
#include "misc.h"

#ifndef _WIN32
static void wait_for_signals(void);
#endif

/*
 *
 * A template DCE RPC server
 *
 * main() contains the basic calls needed to register an interface,
 * get communications endpoints, and register the endpoints
 * with the endpoint mapper.
 *
 * ReverseIt() implements the interface specified in echo.idl
 *
 */

int main(int ac ATTRIBUTE_UNUSED, char *av[] ATTRIBUTE_UNUSED)
{
  unsigned32 status;
  rpc_binding_vector_p_t     server_binding;
  char * string_binding;
  unsigned32 i;

  /*
   * Register the Interface with the local endpoint mapper (rpcd)
   */

  printf ("Registering server.... \n");
  rpc_server_register_if(samrt_v1_0_s_ifspec,
			 NULL,
			 NULL,
			 &status);
      chk_dce_err(status, "rpc_server_register_if()", "", 1);

      printf("registered.\nPreparing binding handle...\n");

      rpc_server_use_protseq((unsigned_char_p_t)"ncacn_ip_tcp",
	      rpc_c_protseq_max_calls_default, &status);

      chk_dce_err(status, "rpc_server_use_all_protseqs()", "", 1);
      rpc_server_inq_bindings(&server_binding, &status);
      chk_dce_err(status, "rpc_server_inq_bindings()", "", 1);

      /*
       * Register bindings with the endpoint mapper
       */

	printf("registering bindings with endpoint mapper\n");

  rpc_ep_register(samrt_v1_0_s_ifspec,
		  server_binding,
		  NULL,
		  (unsigned char *)"QDA application server",
		  &status);
      chk_dce_err(status, "rpc_ep_register()", "", 1);

	printf("registered.\n");

      /*
       * Print out the servers endpoints (TCP and UDP port numbers)
       */

  printf ("Server's communications endpoints are:\n");

    for (i=0; i<RPC_FIELD_COUNT(server_binding); i++)
    {
        rpc_binding_to_string_binding(RPC_FIELD_BINDING_H(server_binding)[i],
				    (unsigned char **)&string_binding,
				    &status
				    );
      if (string_binding)
		printf("\t%s\n",string_binding);
    }

#ifndef _WIN32
  /*
   * Start the signal waiting thread in background. This thread will
   * Catch SIGINT and gracefully shutdown the server.
   */

  wait_for_signals();
#endif

  /*
   * Begin listening for calls
   */

  printf ("listening for calls.... \n");

  DCETHREAD_TRY
    {
      rpc_server_listen(rpc_c_listen_max_calls_default, &status);
    }
  DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
      printf ("Server stoppped listening\n");
    }
  DCETHREAD_ENDTRY

    /*
     * If we reached this point, then the server was stopped, most likely
     * by the signal handler thread called rpc_mgmt_stop_server().
     * gracefully cleanup and unregister the bindings from the
     * endpoint mapper.
     */

#ifndef _WIN32
    /*
     * Kill the signal handling thread
     */

#endif

  printf ("Unregistering server from the endpoint mapper.... \n");
  rpc_ep_unregister(samrt_v1_0_s_ifspec,
		    server_binding,
		    NULL,
		    &status);
  chk_dce_err(status, "rpc_ep_unregister()", "", 0);

  /*
   * retire the binding information
   */

  printf("Cleaning up communications endpoints... \n");
  rpc_server_unregister_if(samrt_v1_0_s_ifspec,
			   NULL,
			   &status);
  chk_dce_err(status, "rpc_server_unregister_if()", "", 0);

  exit(0);

}

/*=========================================================================
 *
 * Server implementation of ReverseIt()
 *
 *=========================================================================*/

unsigned32
samrt_Connect(
     handle_t h,
     unsigned16 *system_name ATTRIBUTE_UNUSED,
     unsigned32 access_mask,
     void **handle)
{

  char * binding_info;
  error_status_t e;
  unsigned32 authn_protocol;
  void *mech_ctx;

  /*
   * Get some info about the client binding
   */

  rpc_binding_to_string_binding(h, (unsigned char **)&binding_info, &e);
  if (e == rpc_s_ok)
    {
      printf ("samrt_Connect() called by client: %s\n", binding_info);
    }

  mech_ctx = NULL;
  rpc_binding_inq_security_context(h, &authn_protocol, &mech_ctx, &e);

  printf("\n\nFunction samrt_Connect() -- input argments\n");

  printf("\taccess_mask = %d\n", access_mask);

  printf ("\n=========================================\n");

  *handle = NULL;
  return 0;

}

#ifndef _WIN32
/*=========================================================================
 *
 * wait_for_signals()
 *
 *
 * Set up the process environment to properly deal with signals.
 * By default, we isolate all threads from receiving asynchronous
 * signals. We create a thread that handles all async signals.
 * The signal handling actions are handled in the handler thread.
 *
 * For AIX, we cant use a thread that sigwaits() on a specific signal,
 * we use a plain old, lame old Unix signal handler.
 *
 *=========================================================================*/

void
wait_for_signals(void)
{
    sigset_t signals;

    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);

    dcethread_signal_to_interrupt(&signals, dcethread_self());
}

#endif
