#pragma once

#include <arpa/inet.h>
#include <stdbool.h>

#include <rasta/config.h>

#include "../util/rastautil.h"
#include "diagnostics.h"
#include "events.h"

#define MAX_PENDING_CONNECTIONS 5

#define UNUSED(x) (void)(x)

#ifdef ENABLE_TLS
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

enum rasta_tls_connection_state {
    RASTA_TLS_CONNECTION_READY,
    RASTA_TLS_CONNECTION_ESTABLISHED,
    RASTA_TLS_CONNECTION_CLOSED
};
#endif

/**
 * representation of a RaSTA redundancy layer transport channel
 */
typedef struct rasta_transport_channel {

    int id;

    bool connected;

    fd_event receive_event;

    struct receive_event_data receive_event_data;

    /**
     * IPv4 address in format a.b.c.d
     */
    char remote_ip_address[INET_ADDRSTRLEN];

    const rasta_config_tls *tls_config;

    /**
     * filedescriptor
     * */
    int file_descriptor;

#ifdef ENABLE_TLS
    WOLFSSL_CTX *ctx;
    WOLFSSL *ssl;
    enum rasta_tls_connection_state tls_state;
#endif

    /**
     * port number
     */
    uint16_t remote_port;

    /**
     * data used for transport channel diagnostics as in 6.6.3.2
     */
    rasta_redundancy_diagnostics_data diagnostics_data;

    void (*send_callback)(struct RastaByteArray data_to_send, struct rasta_transport_channel *channel);

    rasta_transport_socket *associated_socket;
} rasta_transport_channel;

typedef struct rasta_transport_socket {

    int id;

    int file_descriptor;

    fd_event accept_event;

    struct accept_event_data accept_event_data;

    fd_event receive_event;

    struct receive_event_data receive_event_data;

    const rasta_config_tls *tls_config;

#ifdef ENABLE_TLS
    WOLFSSL_CTX *ctx;
    WOLFSSL *ssl;
    enum rasta_tls_connection_state tls_state;
#endif

} rasta_transport_socket;

void send_callback(struct RastaByteArray data_to_send, rasta_transport_channel *channel);
ssize_t receive_callback(struct receive_event_data *data, unsigned char *buffer, struct sockaddr_in *sender);

void transport_init(struct rasta_handle *h, rasta_transport_channel *channel, unsigned id, const char *host, uint16_t port, const rasta_config_tls *tls_config);
void transport_create_socket(struct rasta_handle *h, rasta_transport_socket *socket, int id, const rasta_config_tls *tls_config);
bool transport_bind(rasta_transport_socket *socket, const char *ip, uint16_t port);
void transport_listen(rasta_transport_socket *socket);
int transport_accept(rasta_transport_socket *socket, struct sockaddr_in *addr);
int transport_connect(rasta_transport_socket *socket, rasta_transport_channel *channel);
int transport_redial(rasta_transport_channel *channel);
void transport_close_channel(rasta_transport_channel *channel);
void transport_close_socket(rasta_transport_socket *socket);

bool is_dtls_conn_ready(rasta_transport_socket *socket);

// Protected methods
void find_channel_by_ip_address(struct rasta_handle *h, struct sockaddr_in sender, int *red_channel_idx, int *transport_channel_idx);
