#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_BUFFER_SIZE 4096

// Function prototypes
void displayCatalog(int socketFD, int M, int X, int Z);
void searchForItem(int socketFD, const char *query);
void purchaseItem(int socketFD, const char *title, const char *isbn, int quantity);
void payForItem(int socketFD, int orderNo, float amount);

int main()
{
    // Create a socket
    int clientSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocketFD < 0)
    {
        perror("Failed to create socket");
        return 1;
    }

    // Set up server address and port
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("41.212.93.253");
    serverAddress.sin_port = htons(2000);

    // Connect to the server
    if (connect(clientSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Connection failed");
        close(clientSocketFD);
        return 1;
    }

    // User input variables
    char choice;
    char string[256];
    char title[256];
    char isbn[20];
    int M, X, Z, n, orderNo;
    float amount;

    // Main menu loop
    while (true)
    {
        printf("\nMain Menu\n");
        printf("1. Display Catalog\n");
        printf("2. Search for Book\n");
        printf("3. Order Book\n");
        printf("4. Pay for Book\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf(" %c", &choice);

        switch (choice)
        {
        // Display catalog option
        case '1':
            printf("Enter M (max books), X (start book no), Z (end book no): ");
            scanf("%d %d %d", &M, &X, &Z);
            displayCatalog(clientSocketFD, M, X, Z);
            break;
        // Search for book option
        case '2':
            printf("Enter Title or ISBN: ");
            scanf(" %[^\n]%*c", string);
            searchForItem(clientSocketFD, string);
            break;
        // Order book option
        case '3':
            printf("Enter Title: ");
            scanf(" %[^\n]%*c", title);
            printf("Enter ISBN: ");
            scanf("%s", isbn);
            printf("Enter quantity: ");
            scanf("%d", &n);
            purchaseItem(clientSocketFD, title, isbn, n);
            break;
        // Pay for book option
        case '4':
            printf("Enter Order No: ");
            scanf("%d", &orderNo);
            printf("Enter Amount: ");
            scanf("%f", &amount);
            payForItem(clientSocketFD, orderNo, amount);
            break;
        // Exit option
        case '5':
            printf("Exiting the application.\n");
            close(clientSocketFD);
            return 0;
        // Invalid choice
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }

    // Clean up and exit
    close(clientSocketFD);
    return 0;
}

// Send a request to display the catalog and print the result
void displayCatalog(int socketFD, int M, int X, int Z)
{
    char request[512];
    snprintf(request, sizeof(request), "DisplayCatalog:%d,%d,%d", M, X, Z);
    send(socketFD, request, strlen(request), 0);

    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived = recv(socketFD, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0)
    {
        perror("Receive failed");
        return;
    }

    buffer[bytesReceived] = '\0';
    printf("\nDisplay Catalog Result:\n\n%s\n", buffer);
}

// Send a search query to the server and print the result
void searchForItem(int socketFD, const char *query)
{
    char request[512];
    snprintf(request, sizeof(request), "SearchForItem:%s", query);
    send(socketFD, request, strlen(request), 0);

    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived = recv(socketFD, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0)
    {
        perror("Receive failed");
        return;
    }

    buffer[bytesReceived] = '\0';
    printf("\nSearch Result:\n\n%s\n", buffer);
}

// Send a purchase request to the server and print the result
void purchaseItem(int socketFD, const char *title, const char *isbn, int quantity)
{
    char request[512];
    snprintf(request, sizeof(request), "PurchaseItem:%s,%s,%d", title, isbn, quantity);
    send(socketFD, request, strlen(request), 0);

    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived = recv(socketFD, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0)
    {
        perror("Receive failed");
        return;
    }

    buffer[bytesReceived] = '\0';
    printf("\nPurchase Result:\n\n%s\n", buffer);
}

// Send a payment request to the server and print the result
void payForItem(int socketFD, int orderNo, float amount)
{
    char request[512];
    snprintf(request, sizeof(request), "PayForItem:%d,%f", orderNo, amount);
    send(socketFD, request, strlen(request), 0);

    char buffer[MAX_BUFFER_SIZE];
    int bytesReceived = recv(socketFD, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0)
    {
        perror("Receive failed");
        return;
    }

    buffer[bytesReceived] = '\0';
    printf("\nPayment Result:\n\n%s\n", buffer);
}
