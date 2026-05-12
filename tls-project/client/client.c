#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void keylog(const SSL *ssl, const char *line) {
    FILE *f = fopen("/home/adam/tls-project/client/tls_keys.log", "a");
    if (f) { fprintf(f, "%s\n", line); fclose(f); }
}

int main(int argc, char *argv[]) {
    if (argc < 2) { printf("Usage: ./client <server_ip>\n"); return 1; }

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_keylog_callback(ctx, keylog);

    SSL_CTX_load_verify_locations(ctx, "/home/adam/tls-project/ca/ca.crt", NULL);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_use_certificate_file(ctx, "/home/adam/tls-project/client/client.crt", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx,  "/home/adam/tls-project/client/client.key", SSL_FILETYPE_PEM);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(4433)};
    inet_pton(AF_INET, argv[1], &addr.sin_addr);
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    printf("Connected! TLS: %s | Cipher: %s\n", SSL_get_version(ssl), SSL_get_cipher(ssl));

    const char *req = "GET / HTTP/1.1\r\nHost: server.local\r\nConnection: close\r\n\r\n";
    SSL_write(ssl, req, strlen(req));

    char buf[4096] = {0};
    int n;
    while ((n = SSL_read(ssl, buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    return 0;
}
