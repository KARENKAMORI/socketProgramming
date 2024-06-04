#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    char accno[20];
    char fname[20];
    char lname[20];
    float bal;
} Customer;

void handleClient(int clientSocket);
void Deposit(int clientSocket);
void Withdraw(int clientSocket);
void Balance(int clientSocket);
void Reg(int clientSocket);
int readCustomer(char *accno, Customer *customer);
void updateCustomer(Customer *customer);
void sendMessage(int clientSocket, const char *message);
void receiveMessage(int clientSocket, char *buffer);

FILE *file;

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connected to client\n");
        handleClient(clientSocket);
        close(clientSocket);
    }

    return 0;
}

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int choice;

    while (1) {
        receiveMessage(clientSocket, buffer);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                Deposit(clientSocket);
                break;
            case 2:
                Withdraw(clientSocket);
                break;
            case 3:
                Balance(clientSocket);
                break;
            case 4:
                Reg(clientSocket);
                break;
            case 5:
                close(clientSocket);
                printf("Client disconnected\n");
                return;
            default:
                sendMessage(clientSocket, "Invalid choice, please try again.");
        }
    }
}

void Deposit(int clientSocket) {
    char buffer[BUFFER_SIZE];
    char accno[20];
    float amt;
    Customer customer;

    sendMessage(clientSocket, "Enter account number:");
    receiveMessage(clientSocket, accno);

    if (readCustomer(accno, &customer)) {
        sendMessage(clientSocket, "Enter amount to Deposit:");
        receiveMessage(clientSocket, buffer);
        amt = atof(buffer);

        customer.bal += amt;
        updateCustomer(&customer);

        snprintf(buffer, BUFFER_SIZE, "New Balance: %.2f", customer.bal);
        sendMessage(clientSocket, buffer);
    } else {
        sendMessage(clientSocket, "Account not found.");
    }
}

void Withdraw(int clientSocket) {
    char buffer[BUFFER_SIZE];
    char accno[20];
    float amt;
    Customer customer;

    sendMessage(clientSocket, "Enter account number:");
    receiveMessage(clientSocket, accno);

    if (readCustomer(accno, &customer)) {
        sendMessage(clientSocket, "Enter amount to Withdraw:");
        receiveMessage(clientSocket, buffer);
        amt = atof(buffer);

        if (amt > customer.bal) {
            sendMessage(clientSocket, "Insufficient balance.");
        } else {
            customer.bal -= amt;
            updateCustomer(&customer);

            snprintf(buffer, BUFFER_SIZE, "New Balance: %.2f", customer.bal);
            sendMessage(clientSocket, buffer);
        }
    } else {
        sendMessage(clientSocket, "Account not found.");
    }
}

void Balance(int clientSocket) {
    char accno[20];
    char buffer[BUFFER_SIZE];
    Customer customer;

    sendMessage(clientSocket, "Enter account number:");
    receiveMessage(clientSocket, accno);

    if (readCustomer(accno, &customer)) {
        snprintf(buffer, BUFFER_SIZE, "Current Balance: %.2f", customer.bal);
        sendMessage(clientSocket, buffer);
    } else {
        sendMessage(clientSocket, "Account not found.");
    }
}

void Reg(int clientSocket) {
    char buffer[BUFFER_SIZE];
    int num;

    sendMessage(clientSocket, "Enter number of Customers:");
    receiveMessage(clientSocket, buffer);
    num = atoi(buffer);

    file = fopen("accounts.txt", "a");
    if (file == NULL) {
        perror("ERROR opening the FILE!!!");
        exit(1);
    }

    Customer customer;
    for (int i = 0; i < num; i++) {
        sendMessage(clientSocket, "Account Number:");
        receiveMessage(clientSocket, customer.accno);

        sendMessage(clientSocket, "First Name:");
        receiveMessage(clientSocket, customer.fname);

        sendMessage(clientSocket, "Last Name:");
        receiveMessage(clientSocket, customer.lname);

        sendMessage(clientSocket, "Balance:");
        receiveMessage(clientSocket, buffer);
        customer.bal = atof(buffer);

        fprintf(file, "%s %s %s %.2f\n", customer.accno, customer.fname, customer.lname, customer.bal);
    }
    fclose(file);
    sendMessage(clientSocket, "Customers registered successfully.");
}

int readCustomer(char *accno, Customer *customer) {
    file = fopen("accounts.txt", "r");
    if (file == NULL) {
        perror("ERROR opening the FILE!!!");
        return 0;
    }

    while (fscanf(file, "%s %s %s %f", customer->accno, customer->fname, customer->lname, &customer->bal) != EOF) {
        if (strcmp(customer->accno, accno) == 0) {
            fclose(file);
            return 1;  // Customer found
        }
    }
    fclose(file);
    return 0;  // Customer not found
}

void updateCustomer(Customer *customer) {
    FILE *tempFile = fopen("temp.txt", "w");
    if (tempFile == NULL) {
        perror("ERROR opening the FILE!!!");
        exit(1);
    }

    file = fopen("accounts.txt", "r");
    if (file == NULL) {
        perror("ERROR opening the FILE!!!");
        fclose(tempFile);
        exit(1);
    }

    Customer tempCustomer;
    while (fscanf(file, "%s %s %s %f", tempCustomer.accno, tempCustomer.fname, tempCustomer.lname, &tempCustomer.bal) != EOF) {
        if (strcmp(tempCustomer.accno, customer->accno) == 0) {
            fprintf(tempFile, "%s %s %s %.2f\n", customer->accno, customer->fname, customer->lname, customer->bal);
        } else {
            fprintf(tempFile, "%s %s %s %.2f\n", tempCustomer.accno, tempCustomer.fname, tempCustomer.lname, tempCustomer.bal);
        }
    }

    fclose(file);
    fclose(tempFile);
    remove("accounts.txt");
    rename("temp.txt", "accounts.txt");
}

void sendMessage(int clientSocket, const char *message) {
    send(clientSocket, message, strlen(message), 0);
}

void receiveMessage(int clientSocket, char *buffer) {
    int len = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    buffer[len] = '\0';
}
