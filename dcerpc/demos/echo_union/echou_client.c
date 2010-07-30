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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#define getopt getopt_system

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dce/rpc.h>
#define DCETHREAD_CHECKED
#define DCETHREAD_USE_THROW
#include <dce/dcethread.h>
#include "echou.h"
#include <misc.h>

#undef getopt

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#define MAX_USER_INPUT 128
#define MAX_LINE 128

/*
 * Forward declarations
 */

static int
get_client_rpc_binding(rpc_binding_handle_t *, const char *,
		       rpc_if_handle_t, const char *);

/*
 * usage()
 */

static void usage(void)
{
  printf("usage:   echon_client [-h hostname] [-u] [-t] [-i number]\n");
  printf("         -u:  use UDP protocol \n");
  printf("         -t:  use TCP protocol (default) \n");
  printf("         -v:  more verbosity\n");
  printf("         -h:  specify host where RPC server lives \n");
  printf("         -i:  long int to be sent (5 - if not specified)\n");
  printf("         -f:  float to be sent (2.5 - if not specified)\n\n");
  exit(0);
}

int main(int argc, char *argv[])
{

  /*
   * command line processing and options stuff
   */

  extern char *optarg;
  extern int optind, opterr, optopt;
  int c;

  int verbose = 1;
  int use_udp = 0;
  int use_tcp = 0;
  char rpc_host[128] = "localhost";
  const char * protocol;

  /*
   * stuff needed to make RPC calls
   */

  unsigned32 status;
  rpc_binding_handle_t     echo_server;
  int ok;
  long int in_type = -1;
  EchoUnion *in_value = NULL;
  EchoUnion *out_value = NULL;

  /*
   * Process the cmd line args
   */

  while ((c = getopt(argc, argv, "h:utvi:f:s:")) != EOF)
  {
      switch (c)
      {
      case 'u':
          use_udp = 1;
          break;
      case 't':
          use_tcp = 1;
          break;
      case 'v':
          verbose = 0;
          break;
      case 'h':
          strncpy(rpc_host, optarg, sizeof(rpc_host)-1);
          break;
      case 'i':
          in_type = 1;
          in_value = malloc(sizeof(EchoUnion));
          in_value->integer = atoi(optarg);
          break;
      case 'f':
          in_type = 2;
          in_value = malloc(sizeof(EchoUnion));
          in_value->fp = (idl_short_float)atof(optarg);
          break;
      case 's':
          in_type = 3;
          in_value = malloc(sizeof(EchoUnion));
          in_value->str = (unsigned_char_p_t)optarg;
          printf("in_value is string @ %p\n", (void*) optarg);
          break;
      default:
          usage();
      }
  }

  if (!use_tcp && !use_udp) use_tcp=1;

  if (use_udp)
    protocol = "udp";
  else
    protocol = "tcp";

  /*
   * Get a binding handle to the server using the following params:
   *
   *  1. the hostname where the server lives
   *  2. the interface description structure of the IDL interface
   *  3. the desired transport protocol (UDP or TCP)
   */

  if (get_client_rpc_binding(&echo_server,
		      rpc_host,
		      echou_v1_0_c_ifspec,
		      protocol) == 0)
    {
      printf ("Couldnt obtain RPC server binding. exiting.\n");
      exit(1);
    }

  /*
   * Do the RPC call
   */

  printf ("calling server\n");
  ok = ReplyBack(echo_server, in_type, in_value, &out_value, &status);

  /*
   * Print the results
   */

  if (ok && status == error_status_ok)
    {
      printf ("got response from server. results: \n");

      if (out_value == NULL)
      {
          printf("out_value = [null]\n");
      }
      else if (in_type == 1)
      {
          printf("out_value = [int] %li\n", (long int) out_value->integer);
      }
      else if (in_type == 2)
      {
          printf("out_value = [float] %f\n", (double) out_value->fp);
      }
      else if (in_type == 3)
      {
          printf("out_value = [string@%p] %s\n", (void*) out_value->str, (char*) out_value->str);
          midl_user_free(out_value->str);
      }

      printf("\n===================================\n");

    }

  if (out_value)
      midl_user_free(out_value);

  if (status != error_status_ok)
      chk_dce_err(status, "ReverseIt()", "main()", 1);

  /*
   * Done. Now gracefully teardown the RPC binding to the server
   */

  rpc_binding_free(&echo_server, &status);
  exit(0);

}

/*==========================================================================
 *
 * get_client_rpc_binding()
 *
 *==========================================================================
 *
 * Gets a binding handle to an RPC interface.
 *
 * parameters:
 *
 *    [in/out]  binding_handle
 *    [in]      hostname       <- Internet hostname where server lives
 *    [in]      interface_uuid <- DCE Interface UUID for service
 *    [in]      protocol       <- "udp", "tcp" or "any"
 *
 *==========================================================================*/

static int
get_client_rpc_binding(
     rpc_binding_handle_t * binding_handle,
     const char * hostname,
     rpc_if_handle_t interface_spec,
     const char * protocol)
{
  char * resolved_binding;
  char * printable_uuid ATTRIBUTE_UNUSED;
  const char * protocol_family;
  char partial_string_binding[128];
  rpc_if_id_t interface ATTRIBUTE_UNUSED;
  idl_uuid_t ifc_uuid ATTRIBUTE_UNUSED;
  error_status_t status;

  /*
   * create a string binding given the command line parameters and
   * resolve it into a full binding handle using the endpoint mapper.
   *  The binding handle resolution is handled by the runtime library
   */

  if (strcmp(protocol, "udp")==0)
    protocol_family = "ncadg_ip_udp";
  else
    protocol_family = "ncacn_ip_tcp";

  sprintf(partial_string_binding, "%s:%s[]",
	  protocol_family,
	  hostname);

  rpc_binding_from_string_binding((unsigned char *)partial_string_binding,
				  binding_handle,
				  &status);
      chk_dce_err(status, "string2binding()", "get_client_rpc_binding", 1);

  /*
   * Resolve the partial binding handle using the endpoint mapper
   */

  rpc_ep_resolve_binding(*binding_handle,
			 interface_spec,
			 &status);
  chk_dce_err(status, "rpc_ep_resolve_binding()", "get_client_rpc_binding", 1);

/*
 * Get a printable rendition of the binding handle and echo to
 * the user.
 */

  rpc_binding_to_string_binding(*binding_handle,
				(unsigned char **)&resolved_binding,
				&status);
        chk_dce_err(status, "binding2string()", "get_client_rpc_binding", 1);

  printf("fully resolving binding for server is: %s\n", resolved_binding);

  return 1;
}
