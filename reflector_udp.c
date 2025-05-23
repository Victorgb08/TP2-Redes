#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 65536 // Suficiente para o maior datagrama UDP (teórico)

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t len;
    ssize_t n; // Usar ssize_t para o retorno de recvfrom/sendto

    // Criando o socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Configurando o endereço do servidor
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY; // Aceita conexões de qualquer IP
    servaddr.sin_port = htons(PORT);

    // Bind do socket com o endereço do servidor
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Refletor UDP escutando na porta %d...\n", PORT);

    while (1) {
        len = sizeof(cliaddr);
        // Recebe a mensagem do cliente
        n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE,
                     0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom error");
            continue;
        }

        // Envia a mensagem de volta para o cliente
        // Usar 'n' como tamanho, que é o número de bytes recebidos
        if (sendto(sockfd, (const char *)buffer, n,
                   0, (const struct sockaddr *)&cliaddr, len) < 0) {
            perror("sendto error");
            // Continuar mesmo se sendto falhar para uma solicitação específica
        }
    }

    close(sockfd); // Em teoria, nunca alcançado neste loop infinito
    return 0;
}