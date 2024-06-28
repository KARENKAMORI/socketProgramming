#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_MESSAGE_SIZE 1024
#define MAX_RESPONSE_SIZE 4096
#define MAX_BOOK_TITLE_LENGTH 100

// Function prototypes
void display_catalog(int sockfd);
void search_book(int sockfd);
void order_book(int sockfd);
void pay_for_book(int sockfd);
void send_request(int sockfd, char *message);

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Main loop to interact with the bookstore server
    while (1) {
        int choice;
        printf("Choose an option:\n");
        printf("1. Display Catalog\n");
        printf("2. Search Book\n");
        printf("3. Order Book\n");
        printf("4. Pay for Book\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline character left by scanf

        switch (choice) {
            case 1:
                display_catalog(sockfd);
                break;
            case 2:
                search_book(sockfd);
                break;
            case 3:
                order_book(sockfd);
                break;
            case 4:
                pay_for_book(sockfd);
                break;
            case 5:
                close(sockfd);
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    close(sockfd);
    return 0;
}

// Function to display catalog of books
void display_catalog(int sockfd) {
    int M, X, Z;
    printf("Enter Maximum books to be displayed, Start Index, and End Index.\n");
    printf("If Maximum books is 0, then all books will be displayed.\n");
    printf("Press Enter after each value: ");
    scanf("%d %d %d", &M, &X, &Z);
    getchar(); // Consume newline character left by scanf

    // Prepare request message
    char request[MAX_MESSAGE_SIZE];
    snprintf(request, sizeof(request), "1 %d %d %d", M, X, Z);

    // Send request to server
    send_request(sockfd, request);

    // Receive and process response from server
    char response[MAX_RESPONSE_SIZE];
    memset(response, 0, sizeof(response));
    int total_received = 0;

    while (1) {
        char buffer[MAX_MESSAGE_SIZE];
        int n = read(sockfd, buffer, MAX_MESSAGE_SIZE);
        if (n == -1) {
            perror("read failed");
            return;
        }
        buffer[n] = '\0';

        // Check for end marker in response
        if (strstr(buffer, "[END]") != NULL) {
            strcat(response, buffer);
            break;
        }

        strcat(response, buffer);
        total_received += n;

        // Check if response size exceeds buffer capacity
        if (total_received >= MAX_RESPONSE_SIZE) {
            fprintf(stderr, "Warning: Response size exceeds buffer capacity\n");
            break;
        }
    }

    // Remove the [END] marker from the response if present
    char *end_marker = strstr(response, "[END]");
    if (end_marker != NULL) {
        *end_marker = '\0';
    }

    // Print the catalog response
    printf("%s\n", response);
}

// Function to search for a book
void search_book(int sockfd) {
    char title[40];
    printf("Enter the title of the book to search: ");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = '\0'; // Remove the newline character

    // Prepare request message
    char request[MAX_MESSAGE_SIZE];
    snprintf(request, sizeof(request), "2 %s", title);

    // Send request to server
    send_request(sockfd, request);

    // Receive and print response from server
    char response[MAX_RESPONSE_SIZE];
    memset(response, 0, sizeof(response));
    int n = read(sockfd, response, MAX_RESPONSE_SIZE);
    if (n == -1) {
        perror("read failed");
        return;
    }
    response[n] = '\0';
    printf("%s\n", response);
}

// Function to order a book
void order_book(int sockfd) {
    int ISBN, quantity;
    char title[MAX_BOOK_TITLE_LENGTH];
    printf("Enter the title of the book to order: ");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = '\0'; // Remove the newline character
    printf("Enter the ISBN of the book to order: ");
    scanf("%d", &ISBN);
    printf("Enter the quantity to order: ");
    scanf("%d", &quantity);
    getchar(); // Consume newline character left by scanf

    // Prepare request message
    char request[MAX_MESSAGE_SIZE];
    snprintf(request, sizeof(request), "3 %s %d %d", title, ISBN, quantity);

    // Send request to server
    send_request(sockfd, request);

    // Receive and print response from server
    char response[MAX_RESPONSE_SIZE];
    memset(response, 0, sizeof(response));
    int n = read(sockfd, response, MAX_RESPONSE_SIZE);
    if (n == -1) {
        perror("read failed");
        return;
    }
    response[n] = '\0';
    printf("%s\n", response);
}

// Function to pay for a book
void pay_for_book(int sockfd) {
    int orderID;
    float amount;
    printf("Enter the order ID to pay for: ");
    scanf("%d", &orderID);
    printf("Enter the amount to pay: ");
    scanf("%f", &amount);
    getchar(); // Consume newline character left by scanf

    // Prepare request message
    char request[MAX_MESSAGE_SIZE];
    snprintf(request, sizeof(request), "4 %d %.2f", orderID, amount);

    // Send request to server
    send_request(sockfd, request);

    // Receive and print response from server
    char response[MAX_RESPONSE_SIZE];
    memset(response, 0, sizeof(response));
    int n = read(sockfd, response, MAX_RESPONSE_SIZE);
    if (n == -1) {
        perror("read failed");
        return;
    }
    response[n] = '\0';
    printf("%s\n", response);
}

// Function to send a request message to server
void send_request(int sockfd, char *message) {
    int send_len = write(sockfd, message, strlen(message));
    if (send_len == -1) {
        perror("write failed");
    }
}
