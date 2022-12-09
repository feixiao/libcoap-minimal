/* minimal CoAP server
 *
 * Copyright (C) 2018-2021 Olaf Bergmann <bergmann@tzi.org>
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <getopt.h>
#include <iostream>
#include "common.hh"

int
main(int argc, char** argv) {
  coap_set_log_level(LOG_DEBUG);
  coap_log(LOG_INFO, "CoAP Server run...\n");



  char *optstr = (char*)"i:";
  struct option opts[] = {
      {"ip", required_argument, NULL, 'i'},
      {0, 0, 0, 0},
  };

  int opt;
  std::string address;
    while((opt = getopt_long(argc, argv, optstr, opts, NULL)) != -1){
        switch(opt) {
            case 'i':
                address = std::string(optarg);
                printf("optarg : %s , address : %s \n", optarg, address.c_str());
                break;    
            case '?':
                if(strchr(optstr, optopt) == NULL){
                    fprintf(stderr, "unknown option '-%c'\n", optopt);
                }else{
                    fprintf(stderr, "option requires an argument '-%c'\n", optopt);
                }
                return 1;
        }
    }

  if(address.empty()) {
    address = "localhost";
  }

  coap_context_t  *ctx = nullptr;
  coap_address_t dst;
  coap_resource_t *resource = nullptr;
  coap_endpoint_t *endpoint = nullptr;
  int result = EXIT_FAILURE;;
  coap_str_const_t *ruri = coap_make_str_const("hello");
  coap_startup();

  /* resolve destination address where server should be sent */
  if (resolve_address(address.c_str(), "5683", &dst) < 0) {
    coap_log(LOG_CRIT, "failed to resolve address\n");
    goto finish;
  }

  /* create CoAP context and a client session */
  ctx = coap_new_context(nullptr);

  if (!ctx || !(endpoint = coap_new_endpoint(ctx, &dst, COAP_PROTO_UDP))) {
    coap_log(LOG_EMERG, "cannot initialize context\n");
    goto finish;
  }

  resource = coap_resource_init(ruri, 0);
  coap_register_handler(resource, COAP_REQUEST_GET,
                        [](auto, auto,
                           const coap_pdu_t *request,
                           auto,
                           coap_pdu_t *response) {

                          printf("handler COAP_REQUEST_GET\n");
                          coap_show_pdu(LOG_WARNING, request);
                          coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
                          coap_add_data(response, 5,
                                        (const uint8_t *)"world");
                          coap_show_pdu(LOG_WARNING, response);
                        });
  coap_add_resource(ctx, resource);

  while (true) { coap_io_process(ctx, COAP_IO_WAIT); }

  result = EXIT_SUCCESS;
 finish:

  coap_free_context(ctx);
  coap_cleanup();

  return result;
}
