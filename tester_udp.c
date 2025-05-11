#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>   // Para clock_gettime
#include <errno.h>  // Para errno

#define SERVER_PORT 8080
#define NUM_REPETITIONS 100000
#define NUM_TRIALS 3
#define MAX_PAYLOAD_SIZE 32768 // 32KB
#define RECV_BUFFER_SIZE (MAX_PAYLOAD_SIZE + 100) // Um pouco maior para segurança

// Função para obter tempo em segundos com alta precisão
double get_time_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <IP_DO_HOST_B>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *server_ip = argv[1];

    int sockfd;
    struct sockaddr_in servaddr;
    char *send_buffer;
    char recv_buffer[RECV_BUFFER_SIZE];

    send_buffer = (char*) malloc(MAX_PAYLOAD_SIZE);
    if (!send_buffer) {
        perror("malloc send_buffer failed");
        exit(EXIT_FAILURE);
    }

    // Arquivos para dados brutos
    FILE *fp_latency_small, *fp_throughput_small;
    FILE *fp_latency_large, *fp_throughput_large;

    fp_latency_small = fopen("data_latency_small.txt", "w");
    fp_throughput_small = fopen("data_throughput_small.txt", "w");
    fp_latency_large = fopen("data_latency_large.txt", "w");
    fp_throughput_large = fopen("data_throughput_large.txt", "w");

    if (!fp_latency_small || !fp_throughput_small || !fp_latency_large || !fp_throughput_large) {
        perror("Erro ao abrir arquivos de dados para escrita");
        if(fp_latency_small) fclose(fp_latency_small);
        if(fp_throughput_small) fclose(fp_throughput_small);
        if(fp_latency_large) fclose(fp_latency_large);
        if(fp_throughput_large) fclose(fp_throughput_large);
        free(send_buffer);
        exit(EXIT_FAILURE);
    }

    fprintf(fp_latency_small, "# Tam_Msg_Bytes Latencia_s\n");
    fprintf(fp_throughput_small, "# Tam_Msg_Bytes Vazao_bps\n");
    fprintf(fp_latency_large, "# Tam_Msg_Bytes Latencia_s\n");
    fprintf(fp_throughput_large, "# Tam_Msg_Bytes Vazao_bps\n");


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        // Lidar com fechamento de arquivos e free
        fclose(fp_latency_small); fclose(fp_throughput_small);
        fclose(fp_latency_large); fclose(fp_throughput_large);
        free(send_buffer);
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    tv.tv_sec = 1; // Timeout de 1 segundo para recvfrom
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0) {
        perror("setsockopt SO_RCVTIMEO failed");
        close(sockfd);
        fclose(fp_latency_small); fclose(fp_throughput_small);
        fclose(fp_latency_large); fclose(fp_throughput_large);
        free(send_buffer);
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        perror("inet_pton error for IP address");
        close(sockfd);
        fclose(fp_latency_small); fclose(fp_throughput_small);
        fclose(fp_latency_large); fclose(fp_throughput_large);
        free(send_buffer);
        exit(EXIT_FAILURE);
    }

    int msg_sizes_a[] = {1, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    int num_sizes_a = sizeof(msg_sizes_a) / sizeof(msg_sizes_a[0]);

    int msg_sizes_b_kb[32];
    for(int i=0; i<32; ++i) msg_sizes_b_kb[i] = i + 1; // 1KB to 32KB
    int num_sizes_b = 32;
    int msg_sizes_b[num_sizes_b];
    for (int i = 0; i < num_sizes_b; ++i) {
        msg_sizes_b[i] = msg_sizes_b_kb[i] * 1024;
    }

    printf("Iniciando testes UDP para o servidor %s...\n", server_ip);

    // === PARTE (a): Mensagens Pequenas (1-1000 Bytes) ===
    printf("\nResultados para Parte (a) (UDP - Latência e Vazão - Mensagens Pequenas):\n");
    printf("------------------------------------------------------------------------\n");
    printf("| Tam. Msg (Bytes) | Latência Média (s) | Vazão Média (bps) |\n");
    printf("|------------------|--------------------|-------------------|\n");

    for (int i = 0; i < num_sizes_a; ++i) {
        int current_msg_size = msg_sizes_a[i];
        for(int k=0; k < current_msg_size; ++k) send_buffer[k] = (char)('A' + (k % 26));

        double trial_total_latency_s = 0;
        double trial_total_throughput_bps = 0;
        int successful_trials_count = 0;

        for (int trial = 0; trial < NUM_TRIALS; ++trial) {
            double start_time, end_time, elapsed_time_s;
            long successful_repetitions = 0;
            
            printf("  Tamanho: %d Bytes, Trial %d/%d ... ", current_msg_size, trial + 1, NUM_TRIALS);
            fflush(stdout);

            start_time = get_time_sec();
            for (long rep = 0; rep < NUM_REPETITIONS; ++rep) {
                if (sendto(sockfd, send_buffer, current_msg_size, 0,
                           (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                    // perror("sendto failed"); // Pode poluir muito a saída
                    continue; // Pula esta repetição
                }
                socklen_t len = sizeof(servaddr);
                ssize_t n = recvfrom(sockfd, recv_buffer, RECV_BUFFER_SIZE, 0,
                                 (struct sockaddr *)&servaddr, &len);
                if (n < 0) {
                    // if (errno == EAGAIN || errno == EWOULDBLOCK) { // Timeout
                        // Descomente para depuração de timeout
                        // printf("T"); fflush(stdout);
                    // } else {
                        // perror("recvfrom failed");
                    // }
                    continue; // Pula esta repetição
                } else if (n != current_msg_size) {
                    // Resposta com tamanho incorreto
                    continue; // Pula esta repetição
                }
                successful_repetitions++;
            }
            end_time = get_time_sec();
            elapsed_time_s = end_time - start_time;
            printf("concluído (%ld/%d reps ok)\n", successful_repetitions, NUM_REPETITIONS);


            if (successful_repetitions > 0) {
                double current_latency_s = elapsed_time_s / successful_repetitions;
                double current_throughput_bps = (current_msg_size * 8.0) / current_latency_s;
                trial_total_latency_s += current_latency_s;
                trial_total_throughput_bps += current_throughput_bps;
                successful_trials_count++;
            } else {
                // printf("Trial %d para msg_size %d não teve reflexões bem sucedidas.\n", trial + 1, current_msg_size);
            }
        }

        if (successful_trials_count > 0) {
            double avg_latency_s = trial_total_latency_s / successful_trials_count;
            double avg_throughput_bps = trial_total_throughput_bps / successful_trials_count;
            printf("| %-16d | %-18.9f | %-17.2f |\n",
                   current_msg_size, avg_latency_s, avg_throughput_bps);
            fprintf(fp_latency_small, "%d %f\n", current_msg_size, avg_latency_s);
            fprintf(fp_throughput_small, "%d %f\n", current_msg_size, avg_throughput_bps);
        } else {
            printf("| %-16d | N/A                | N/A               |\n", current_msg_size);
            fprintf(fp_latency_small, "%d NaN\n", current_msg_size); // Gnuplot/Python podem lidar com NaN
            fprintf(fp_throughput_small, "%d NaN\n", current_msg_size);
        }
    }
    printf("------------------------------------------------------------------------\n");

    // === PARTE (b): Mensagens Grandes (1KB-32KB) ===
    printf("\nResultados para Parte (b) (UDP - Latência e Vazão - Mensagens Grandes):\n");
    printf("-----------------------------------------------------------------------------\n");
    printf("| Tam. Msg (KB) | Tam. Msg (Bytes) | Latência Média (s) | Vazão Média (bps) |\n");
    printf("|---------------|------------------|--------------------|-------------------|\n");

    for (int i = 0; i < num_sizes_b; ++i) {
        int current_msg_size = msg_sizes_b[i]; // Já em bytes
        for(int k=0; k < current_msg_size; ++k) send_buffer[k] = (char)('X' + (k % 26));

        double trial_total_latency_s = 0;
        double trial_total_throughput_bps = 0;
        int successful_trials_count = 0;

        for (int trial = 0; trial < NUM_TRIALS; ++trial) {
            double start_time, end_time, elapsed_time_s;
            long successful_repetitions = 0;

            printf("  Tamanho: %d KB (%d B), Trial %d/%d ... ", current_msg_size/1024, current_msg_size, trial + 1, NUM_TRIALS);
            fflush(stdout);

            start_time = get_time_sec();
            for (long rep = 0; rep < NUM_REPETITIONS; ++rep) {
                if (sendto(sockfd, send_buffer, current_msg_size, 0,
                           (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                    continue;
                }
                socklen_t len = sizeof(servaddr);
                ssize_t n = recvfrom(sockfd, recv_buffer, RECV_BUFFER_SIZE, 0,
                                 (struct sockaddr *)&servaddr, &len);
                if (n < 0) {
                    continue;
                } else if (n != current_msg_size) {
                    continue;
                }
                successful_repetitions++;
            }
            end_time = get_time_sec();
            elapsed_time_s = end_time - start_time;
            printf("concluído (%ld/%d reps ok)\n", successful_repetitions, NUM_REPETITIONS);

            if (successful_repetitions > 0) {
                double current_latency_s = elapsed_time_s / successful_repetitions;
                double current_throughput_bps = (current_msg_size * 8.0) / current_latency_s;
                trial_total_latency_s += current_latency_s;
                trial_total_throughput_bps += current_throughput_bps;
                successful_trials_count++;
            } else {
                // printf("Trial %d para msg_size %d não teve reflexões bem sucedidas.\n", trial + 1, current_msg_size);
            }
        }

        if (successful_trials_count > 0) {
            double avg_latency_s = trial_total_latency_s / successful_trials_count;
            double avg_throughput_bps = trial_total_throughput_bps / successful_trials_count;
            printf("| %-13d | %-16d | %-18.9f | %-17.2f |\n",
                   current_msg_size / 1024, current_msg_size, avg_latency_s, avg_throughput_bps);
            fprintf(fp_latency_large, "%d %f\n", current_msg_size, avg_latency_s);
            fprintf(fp_throughput_large, "%d %f\n", current_msg_size, avg_throughput_bps);
        } else {
            printf("| %-13d | %-16d | N/A                | N/A               |\n",
                   current_msg_size / 1024, current_msg_size);
            fprintf(fp_latency_large, "%d NaN\n", current_msg_size);
            fprintf(fp_throughput_large, "%d NaN\n", current_msg_size);
        }
    }
    printf("-----------------------------------------------------------------------------\n");

    printf("\nArquivos de dados gerados: data_latency_small.txt, data_throughput_small.txt, data_latency_large.txt, data_throughput_large.txt\n");

    fclose(fp_latency_small);
    fclose(fp_throughput_small);
    fclose(fp_latency_large);
    fclose(fp_throughput_large);
    close(sockfd);
    free(send_buffer);

    return 0;
}