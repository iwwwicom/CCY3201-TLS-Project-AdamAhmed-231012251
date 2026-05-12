#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void keylog(const SSL *ssl, const char *line) {
    FILE *f = fopen("/home/adam/tls-project/server/tls_keys.log", "a");
    if (f) { fprintf(f, "%s\n", line); fclose(f); }
}

int main() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_keylog_callback(ctx, keylog);

    SSL_CTX_use_certificate_file(ctx, "/home/adam/tls-project/server/server.crt", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx,  "/home/adam/tls-project/server/server.key", SSL_FILETYPE_PEM);
    SSL_CTX_load_verify_locations(ctx, "/home/adam/tls-project/ca/ca.crt", NULL);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {AF_INET, htons(4433), .sin_addr.s_addr = INADDR_ANY};
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(fd, 1);
    printf("Server waiting on port 4433...\n");

    int cfd = accept(fd, NULL, NULL);
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, cfd);

    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    printf("Connected! TLS: %s | Cipher: %s\n", SSL_get_version(ssl), SSL_get_cipher(ssl));

    char buf[2048] = {0};
    SSL_read(ssl, buf, sizeof(buf)-1);

    const char *body = "<html><body><h1>Hello from TLS Server</h1><p>CCY3201 - Networks Security</p></body></html>";
    char res[1024];
    snprintf(res, sizeof(res),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
        strlen(body), body);

    SSL_write(ssl, res, strlen(res));
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(cfd);
    close(fd);
    SSL_CTX_free(ctx);
    printf("Done.\n");
    return 0;
}
