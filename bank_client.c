#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void Deposit(int sock);
void Withdraw(int sock);
void Balance(int sock);
void Reg(int sock);
void sendMessage(int sock, const char *message);
void receiveMessage(int sock, char *buffer);

int main(void) {
    int sock;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int choice;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("142.168.24.98");

    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("MENU\n");
        printf("MAKE A CHOICE: \n");
        printf("1. DEPOSIT\n");
        printf("2. WITHDRAW\n");
        printf("3. BALANCE\n");
        printf("4. REGISTER NEW CUSTOMER\n");
        printf("5. EXIT\n");

        printf("Enter choice: \n");
        scanf("%d", &choice);
        snprintf(buffer, BUFFER_SIZE, "%d", choice);
        sendMessage(sock, buffer);

        switch (choice) {
            case 1:
                Deposit(sock);
                break;
            case 2:
                Withdraw(sock);
                break;
            case 3:
                Balance(sock);
                break;
            case 4:
                Reg(sock);
                break;
            case 5:
                close(sock);
                printf("Disconnected from server.\n");
                exit(0);
            default:
                printf("Invalid choice, please try again.\n");
        }
    }

    return 0;
}

void Deposit(int sock) {
    char buffer[BUFFER_SIZE];
    char accno[20];
    float amt;

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
    scanf("%s", accno);
    sendMessage(sock, accno);

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
    scanf("%f", &amt);
    snprintf(buffer, BUFFER_SIZE, "%.2f", amt);
    sendMessage(sock, buffer);

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
}

void Withdraw(int sock) {
    char buffer[BUFFER_SIZE];
    char accno[20];
    float amt;

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
    scanf("%s", accno);
    sendMessage(sock, accno);

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
    scanf("%f", &amt);
    snprintf(buffer, BUFFER_SIZE, "%.2f", amt);
    sendMessage(sock, buffer);

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
}

void Balance(int sock) {
    char buffer[BUFFER_SIZE];
    char accno[20];

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
    scanf("%s", accno);
    sendMessage(sock, accno);

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
}

void Reg(int sock) {
    char buffer[BUFFER_SIZE];
    int num;

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
    scanf("%d", &num);
    snprintf(buffer, BUFFER_SIZE, "%d", num);
    sendMessage(sock, buffer);

    for (int i = 0; i < num; i++) {
        printf("Account Number: \n");
        scanf("%s", buffer);
        sendMessage(sock, buffer);

        printf("First Name: \n");
        scanf("%s", buffer);
        sendMessage(sock, buffer);

        printf("Last Name: \n");
        scanf("%s", buffer);
        sendMessage(sock, buffer);

        printf("Balance: \n");
        scanf("%s", buffer);
        sendMessage(sock, buffer);
    }

    receiveMessage(sock, buffer);
    printf("%s\n", buffer);
}

void sendMessage(int sock, const char *message) {
    send(sock, message, strlen(message), 0);
}

void receiveMessage(int sock, char *buffer) {
    int len = recv(sock, buffer, BUFFER_SIZE, 0);
    buffer[len] = '\0';
}
