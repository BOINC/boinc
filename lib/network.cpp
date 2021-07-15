// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#if defined(_WIN32)
#include "boinc_win.h"
#include <fcntl.h>
#else
#include "config.h"
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#endif

#include "error_numbers.h"
#include "str_util.h"
#include "util.h"

#include "network.h"

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/engine.h>

using std::perror;
using std::sprintf;

#define RSA_KEY_BITS (4096)
#define CERT_DAYS (365)

#define REQ_DN_C "US"
#define REQ_DN_ST ""
#define REQ_DN_L ""
#define REQ_DN_O "Berkeley University"
#define REQ_DN_OU ""
#define REQ_DN_CN "BOINC"

const char* socket_error_str() {
    static char buf[80];
#if defined(_WIN32) && defined(USE_WINSOCK)
    int e = WSAGetLastError();
    switch (e) {
    case WSANOTINITIALISED:
        return "WSA not initialized";
    case WSAENETDOWN:
        return "the network subsystem has failed";
    case WSAHOST_NOT_FOUND:
        return "host name not found";
    case WSATRY_AGAIN:
        return "no response from server";
    case WSANO_RECOVERY:
        return "a nonrecoverable error occurred";
    case WSANO_DATA:
        return "valid name, no data record of requested type";
    case WSAEINPROGRESS:
        return "a blocking socket call in progress";
    case WSAEFAULT:
        return "invalid part of user address space";
    case WSAEINTR:
        return "a blocking socket call was canceled";
    case WSAENOTSOCK:
        return "not a socket";
    }
    snprintf(buf, sizeof(buf), "error %d", e);
    return buf;
#else
    switch (h_errno) {
    case HOST_NOT_FOUND:
        return "host not found";
    case NO_DATA:
        return "valid name, no data record of requested type";
    case NO_RECOVERY:
        return "a nonrecoverable error occurred";
    case TRY_AGAIN:
        return "host not found or server failure";
#ifdef NETDB_INTERNAL
    case NETDB_INTERNAL:
        snprintf(buf, sizeof(buf), "network internal error %d", errno);
        return buf;
#endif
    }
    snprintf(buf, sizeof(buf), "error %d", h_errno);
    return buf;
#endif
}

bool is_localhost(sockaddr_storage& s) {
#ifdef _WIN32
    if (ntohl(s.sin_addr.s_addr) == 0x7f000001) return true;
#else
    switch (s.ss_family) {
        case AF_INET: {
            sockaddr_in* sin = (sockaddr_in*)&s;
            return (ntohl(sin->sin_addr.s_addr) == 0x7f000001);
            break;
        }
        case AF_INET6: {
            sockaddr_in6* sin = (sockaddr_in6*)&s;
            char buf[256];
            inet_ntop(AF_INET6, (void*)(&sin->sin6_addr), buf, sizeof(buf));
            return (strcmp(buf, "::1") == 0);
            break;
        }
    }
#endif
    return false;
}

bool same_ip_addr(sockaddr_storage& s1, sockaddr_storage& s2) {
#ifdef _WIN32
    return (s1.sin_addr.s_addr == s2.sin_addr.s_addr);
#else
    if (s1.ss_family != s2.ss_family) return false;
    switch (s1.ss_family) {
        case AF_INET: {
            sockaddr_in* sin1 = (sockaddr_in*)&s1;
            sockaddr_in* sin2 = (sockaddr_in*)&s2;
            return (memcmp((void*)(&sin1->sin_addr), (void*)(&sin2->sin_addr), sizeof(in_addr)) == 0);
            break;
        }
        case AF_INET6: {
            sockaddr_in6* sin1 = (sockaddr_in6*)&s1;
            sockaddr_in6* sin2 = (sockaddr_in6*)&s2;
            return (memcmp((void*)(&sin1->sin6_addr), (void*)(&sin2->sin6_addr), sizeof(in6_addr)) == 0);
            break;
        }
    }
    return false;
#endif
}

int resolve_hostname(const char* hostname, sockaddr_storage &ip_addr) {
#ifdef _WIN32
    hostent* hep;
    hep = gethostbyname(hostname);
    if (!hep) {
        return ERR_GETHOSTBYNAME;
    }
    for (int i=0; ; i++) {
        if (!hep->h_addr_list[i]) break;
        ip_addr.sin_family = AF_INET;
        ip_addr.sin_addr.s_addr = *(int*)hep->h_addr_list[i];
        if ((ip_addr.sin_addr.s_addr&0xff) != 0x7f) return 0;     // look for non-loopback addr
    }
    return 0;

#else
    struct addrinfo *res, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    char buf[512];
    snprintf(buf, sizeof(buf), "%s getaddrinfo(%s)", time_to_string(dtime()), hostname);
    int retval = getaddrinfo(hostname, NULL, &hints, &res);
    if (retval) {
        if (retval == EAI_SYSTEM) {
            perror(buf);
        } else {
            fprintf(stderr, "%s: %s\n", buf, gai_strerror(retval));
        }
        return ERR_GETADDRINFO;
    }
    struct addrinfo* aip = res;
    while (aip) {
        memcpy(&ip_addr, aip->ai_addr, aip->ai_addrlen);
        sockaddr_in* sin = (sockaddr_in*)&ip_addr;
        if ((sin->sin_addr.s_addr&0xff) != 0x7f) break;
        aip = aip->ai_next;
    }
    freeaddrinfo(res);
    return 0;
#endif
}

int resolve_hostname_or_ip_addr(
    const char* hostname, sockaddr_storage &ip_addr
) {
#ifdef _WIN32   // inet_pton() only on Vista or later!!
    int x = inet_addr(hostname);
    if (x != -1) {
        sockaddr_in* sin = (sockaddr_in*)&ip_addr;
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = x;
        return 0;
    }
#else
    int retval;
    // check for IPV4 and IPV6 notation
    //
    sockaddr_in* sin = (sockaddr_in*)&ip_addr;
    retval = inet_pton(AF_INET, hostname, &sin->sin_addr);
    if (retval > 0) {
        ip_addr.ss_family = AF_INET;
        return 0;
    }
    sockaddr_in6* sin6 = (sockaddr_in6*)&ip_addr;
    retval = inet_pton(AF_INET6, hostname, &sin6->sin6_addr);
    if (retval > 0) {
        ip_addr.ss_family = AF_INET6;
        return 0;
    }
#endif

    // else resolve the name
    //
    return resolve_hostname(hostname, ip_addr);
}

int print_error_cb(const char* str, size_t len, void* u) {
    perror(str);
    return 1;
}

void init_openssl()
{
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();	
}

void cleanup_openssl()
{
    EVP_cleanup();
}

int add_ext(X509 *cert, int nid, char *value) 
{
    X509_EXTENSION *ex;
    X509V3_CTX ctx;
    /* This sets the 'context' of the extensions. */
    /* No configuration database */
    X509V3_set_ctx_nodb(&ctx);
    /* Issuer and subject certs: both the target since it is self signed,
        * no request and no CRL
        */
    X509V3_set_ctx(&ctx, cert, cert, NULL, NULL, 0);
    ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
    if (!ex)
        return 0;

    X509_add_ext(cert,ex,-1);
    X509_EXTENSION_free(ex);
    return 1;
}

int mkcert(X509** x509p, EVP_PKEY** pkeyp, int bits, int serial, int days)
{
    X509* x;
    EVP_PKEY* pk;
    RSA* rsa;
    X509_NAME* name = NULL;

    if ((pkeyp == NULL) || (*pkeyp == NULL))
    {
        if ((pk = EVP_PKEY_new()) == NULL)
        {
            abort();
        }
    }
    else
        pk = *pkeyp;

    if ((x509p == NULL) || (*x509p == NULL))
    {
        if ((x = X509_new()) == NULL)
            return 0;
    }
    else
        x = *x509p;

    rsa = RSA_generate_key(bits, RSA_F4, NULL, NULL);
    if (!EVP_PKEY_assign_RSA(pk, rsa))
    {
        abort();
    }
    rsa = NULL;

    X509_set_version(x, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x), serial);
    X509_gmtime_adj(X509_get_notBefore(x), 0);
    X509_gmtime_adj(X509_get_notAfter(x), (long)60 * 60 * 24 * days);
    X509_set_pubkey(x, pk);

    name = X509_get_subject_name(x);

    /* This function creates and adds the entry, working out the
     * correct string type and performing checks on its length.
     * Normally we'd check the return value for errors...
     */
    X509_NAME_add_entry_by_txt(name, "C",
        MBSTRING_ASC, (const unsigned char*)REQ_DN_C, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", 
        MBSTRING_ASC, (unsigned char*)REQ_DN_O, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN",
        MBSTRING_ASC, (const unsigned char*)REQ_DN_CN, -1, -1, 0);

    /* Its self signed so set the issuer name to be the same as the
     * subject.
     */
    X509_set_issuer_name(x, name);

    /* Add various extensions: standard extensions */
    add_ext(x, NID_basic_constraints, "critical,CA:TRUE");
    add_ext(x, NID_key_usage, "critical,keyCertSign,cRLSign");

    add_ext(x, NID_subject_key_identifier, "sha1");

    if (!X509_sign(x, pk, EVP_sha1()))
        return 0;

    *x509p = x;
    *pkeyp = pk;
    return 1;
}

void key_csr_free(EVP_PKEY** key, X509_REQ** req, RSA* rsa, BIGNUM* e)
{
    EVP_PKEY_free(*key);
    X509_REQ_free(*req);
    RSA_free(rsa);
    BN_free(e);
}

int generate_key_csr(EVP_PKEY** key, X509_REQ** req, int bits)
{
    *key = NULL;
    *req = NULL;
    RSA* rsa = NULL;
    BIGNUM* e = NULL;

    *key = EVP_PKEY_new();
    if (!*key) {
        key_csr_free(key, req, rsa, e);
        return 0;
    }
    *req = X509_REQ_new();
    if (!*req) {
        key_csr_free(key, req, rsa, e);
        return 0;
    }
    rsa = RSA_new();
    if (!rsa) {
        key_csr_free(key, req, rsa, e);
        return 0;
    }
    e = BN_new();
    if (!e) {
        key_csr_free(key, req, rsa, e);
        return 0;
    }

    BN_set_word(e, 65537);
    if (!RSA_generate_key_ex(rsa, bits, e, NULL)) {
        key_csr_free(key, req, rsa, e);
        return 0;
    }
    if (!EVP_PKEY_assign_RSA(*key, rsa)) {
        key_csr_free(key, req, rsa, e);
        return 0;
    }

    X509_REQ_set_pubkey(*req, *key);

    /* Set the DN of the request. */
    X509_NAME* name = X509_REQ_get_subject_name(*req);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (const unsigned char*)REQ_DN_C, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "ST", MBSTRING_ASC, (const unsigned char*)REQ_DN_ST, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC, (const unsigned char*)REQ_DN_L, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (const unsigned char*)REQ_DN_O, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC, (const unsigned char*)REQ_DN_OU, -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (const unsigned char*)REQ_DN_CN, -1, -1, 0);

    /* Self-sign the request to prove that we posses the key. */
    if (!X509_REQ_sign(*req, *key, EVP_sha256())) {
        key_csr_free(key, req, rsa, e);
        return 0;
    }

    BN_free(e);

    return 1;
}

int generate_set_random_serial(X509* crt)
{
    /* Generates a 20 byte random serial number and sets in certificate. */
    unsigned char serial_bytes[20];
    if (RAND_bytes(serial_bytes, sizeof(serial_bytes)) != 1) return 0;
    serial_bytes[0] &= 0x7f; /* Ensure positive serial! */
    BIGNUM* bn = BN_new();
    BN_bin2bn(serial_bytes, sizeof(serial_bytes), bn);
    ASN1_INTEGER* serial = ASN1_INTEGER_new();
    BN_to_ASN1_INTEGER(bn, serial);

    X509_set_serialNumber(crt, serial); // Set serial.

    ASN1_INTEGER_free(serial);
    BN_free(bn);
    return 1;
}

void key_pair_free(EVP_PKEY** key, X509** crt, X509_REQ* req)
{
    EVP_PKEY_free(*key);
    X509_REQ_free(req);
    X509_free(*crt);
}

int generate_signed_key_pair(EVP_PKEY* ca_key, X509* ca_crt, EVP_PKEY** key, X509** crt, int bits, int days)
{
    /* Generate the private key and corresponding CSR. */
    X509_REQ* req = NULL;
    if (!generate_key_csr(key, &req, bits)) {
        fprintf(stderr, "Failed to generate key and/or CSR!\n");
        return 0;
    }

    /* Sign with the CA. */
    *crt = X509_new();
    if (!*crt) {
        key_pair_free(key, crt, req);
        return 0;
    }

    X509_set_version(*crt, 2); /* Set version to X509v3 */

    /* Generate random 20 byte serial. */
    if (!generate_set_random_serial(*crt)) {
        key_pair_free(key, crt, req);
        return 0;
    }

    /* Set issuer to CA's subject. */
    X509_set_issuer_name(*crt, X509_get_subject_name(ca_crt));

    /* Set validity of certificate to 2 years. */
    X509_gmtime_adj(X509_get_notBefore(*crt), 0);
    X509_gmtime_adj(X509_get_notAfter(*crt), (long)60 * 60 * 24 * days);

    /* Get the request's subject and just use it (we don't bother checking it since we generated
     * it ourself). Also take the request's public key. */
    X509_set_subject_name(*crt, X509_REQ_get_subject_name(req));
    EVP_PKEY* req_pubkey = X509_REQ_get_pubkey(req);
    X509_set_pubkey(*crt, req_pubkey);
    EVP_PKEY_free(req_pubkey);

    /* Now perform the actual signing with the CA. */
    if (X509_sign(*crt, ca_key, EVP_sha256()) == 0) {
        key_pair_free(key, crt, req);
        return 0;
    }

    X509_REQ_free(req);
    return 1;
}

SSL_CTX* create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLSv1_2_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
		perror("Unable to create SSL context");
        ERR_print_errors_cb(print_error_cb, NULL);
		exit(EXIT_FAILURE);
    }
    return ctx;
}

/* Set private key and local certificate */
void configure_context(SSL_CTX *ctx)
{    
    SSL_CTX_set_ecdh_auto(ctx, 2);

    EVP_PKEY* ca_key = NULL;
    X509* ca_crt = NULL;

    mkcert(&ca_crt, &ca_key, RSA_KEY_BITS, 0, CERT_DAYS);

    /* Generate keypair and then print it byte-by-byte for demo purposes. */
    EVP_PKEY* key = NULL;
    X509* crt = NULL;

    int status = generate_signed_key_pair(ca_key, ca_crt, &key, &crt, RSA_KEY_BITS, 365);
    if (1 != status) {
        ERR_print_errors_cb(print_error_cb, NULL);
        exit(EXIT_FAILURE);
    }

    /* set the local certificate */
    status = SSL_CTX_use_certificate(ctx, crt);
    if (1 != status) {
        ERR_print_errors_cb(print_error_cb, NULL);
        exit(EXIT_FAILURE);
    }

    /* set the private key */
    status = SSL_CTX_use_PrivateKey(ctx, key);
    if (1 != status) {
        ERR_print_errors_cb(print_error_cb, NULL);
	    exit(EXIT_FAILURE);
    }

    /* verify private key */
    status = SSL_CTX_check_private_key(ctx);
    if (1 != status)
    {
        perror("Private key does not match the public certificate");
        ERR_print_errors_cb(print_error_cb, NULL);
        exit(EXIT_FAILURE);
    }

    X509_free(crt);
    EVP_PKEY_free(key);
    X509_free(ca_crt);
    EVP_PKEY_free(ca_key);
}

int boinc_socket(int& fd, int protocol) {
    fd = (int)socket(protocol, SOCK_STREAM, 0);
    if (fd < 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s socket()", time_to_string(dtime()));
        perror(buf);
        return ERR_SOCKET;
    }
#ifndef _WIN32
    if (-1 == fcntl(fd, F_SETFD, FD_CLOEXEC)) {
        return ERR_FCNTL;
    }
#endif
    return 0;
}

int boinc_socket_asynch(int fd, bool asynch) {
    if (asynch) {
#if defined(_WIN32) && defined(USE_WINSOCK)
        unsigned long one = 1;
        ioctlsocket(fd, FIONBIO, &one);
#else
        int flags;
        flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            return ERR_FCNTL;
        }
        if (fcntl(fd, F_SETFL, flags|O_NONBLOCK) < 0 ) {
            return ERR_FCNTL;
        }
#endif
    } else {
#if defined(_WIN32) && defined(USE_WINSOCK)
        unsigned long zero = 0;
        ioctlsocket(fd, FIONBIO, &zero);
#else
        int flags;
        flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            return ERR_FCNTL;
        }
        if (fcntl(fd, F_SETFL, flags&(~O_NONBLOCK)) < 0 ) {
            return ERR_FCNTL;
        }
#endif
    }
    return 0;
}

void boinc_close_socket(int sock) {
#if defined(_WIN32) && defined(USE_WINSOCK)
    closesocket(sock);
#else
    close(sock);
#endif
}

int get_socket_error(int fd) {
    int n;
#if defined(_WIN32) && defined(USE_WINSOCK)
    int intsize = sizeof(int);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&n, &intsize);
#elif defined(__FreeBSD__)
    // workaround for FreeBSD. I don't understand this.
    struct sockaddr_in sin;
    socklen_t sinsz = sizeof(sin);
    n = getpeername(fd, (struct sockaddr *)&sin, &sinsz);
#else
    socklen_t intsize = sizeof(int);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&n, (socklen_t*)&intsize)) {
        return errno;
    }
#endif
    return n;
}

#if defined(_WIN32) && defined(USE_WINSOCK)

int WinsockInitialize() {
    WSADATA wsdata;
    return WSAStartup(MAKEWORD(2, 0), &wsdata);
}

int WinsockCleanup() {
    return WSACleanup();
}

#endif

void reset_dns() {
#if !defined(ANDROID) && !defined(_WIN32) && !defined(__APPLE__)
    // Windows doesn't have this, and it crashes Macs
    res_init();
#endif
}

// Get an unused port number.
// Used by vboxwrapper.
// I'm not sure if is_remote is relevant here - a port is a port, right?
//
int boinc_get_port(bool is_remote, int& port) {
    sockaddr_in addr;
    BOINC_SOCKLEN_T addrsize;
    int sock;
    int retval;

    addrsize = sizeof(sockaddr_in);

    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = htonl(is_remote?INADDR_ANY:INADDR_LOOPBACK);

    retval = boinc_socket(sock);
    if (retval) return retval;

    retval = bind(sock, (const sockaddr*)&addr, addrsize);
    if (retval < 0) {
        boinc_close_socket(sock);
        return ERR_BIND;
    }

    retval = getsockname(sock, (sockaddr*)&addr, &addrsize);
    if (retval) {
        boinc_close_socket(sock);
        return errno;
    }
    port = ntohs(addr.sin_port);

    boinc_close_socket(sock);
    return 0;
}
