#include "mongoose.h"

static sig_atomic_t s_signal_received = 0;
static const char *s_http_port = "8443";
static const char *s_ssl_cert = "server.pem";
static const char *s_ssl_key = "server.key";

class WEBSOCK {
   public:

      WEBSOCK();
      void initiate(struct mg_mgr&);
};
