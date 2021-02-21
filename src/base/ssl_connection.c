#include "ssl_connection.h"
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <unistd.h>
#include "parse.h"
#include "output.h"
#include <netinet/in.h>
#include "request.h"
#include "response.h"
#include "filetools.h"


void InitializeSSL()
{
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void DestroySSL()
{
    ERR_free_strings();
    EVP_cleanup();
}

void ShutdownSSL(SSL *cSSL)
{
    SSL_shutdown(cSSL);
    SSL_free(cSSL);
}

void StartServer()
{
    int addrlen;
    int sockfd, newsockfd;
    SSL_CTX *sslctx;
    struct sockaddr_in saiServerAddress;

    InitializeSSL();
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return;
    }

    bzero((char *)&saiServerAddress, sizeof(saiServerAddress));
    saiServerAddress.sin_family = AF_INET;
    saiServerAddress.sin_addr.s_addr = INADDR_ANY;
    saiServerAddress.sin_port = htons(8090);
    addrlen = sizeof(saiServerAddress);

    bind(sockfd, (struct sockaddr *)&saiServerAddress, sizeof(saiServerAddress));

    sslctx = SSL_CTX_new(SSLv23_server_method());
    SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);
    int use_cert = SSL_CTX_use_certificate_file(sslctx, "certs/localhost_sha256.pem", SSL_FILETYPE_PEM);
    int use_prv = SSL_CTX_use_PrivateKey_file(sslctx, "certs/localhost_sha256.key", SSL_FILETYPE_PEM);

    if (use_cert && use_prv) {

    }

    listen(sockfd, 5);

    printf("Server started on port %d\n", 8090);

    while ((newsockfd = accept(sockfd, (struct sockaddr *)&saiServerAddress, (socklen_t *)&addrlen)) >= 0u)
    {
        if (fork() == 0)
        {
            int ssl_err;

            SSL *cSSL = SSL_new(sslctx);
            SSL_set_fd(cSSL, newsockfd);
            ssl_err = SSL_accept(cSSL);

            while (ssl_err > 0)
            {
                HASH_TABLE_NEW(request, 100, 10000);

                if (TRUE == parse(newsockfd, &request, cSSL, &ssl_err))
                {
                    RESPONSE_NEW(response, 100, 100000);
                    Request_Handle(&request, &response);

                    printf("Answer %d/%d %d/%d\n", response.options.pairs, response.options.max_pairs, response.options.size, response.options.max_size);
                  
                    if (hash_table_get_with_tag(&(response.options), "body", OPTIONS_TAG_PAGE) != NULL) {
                        response_print_header(&response, cSSL);
                        response_print_body(&response, cSSL);
                    } else {
                        output_not_found(newsockfd, cSSL);
                    }
                }
            }

            ShutdownSSL(cSSL);
            close(newsockfd);

            printf("Close socket\n");
            exit(0);
        }
    }
}
