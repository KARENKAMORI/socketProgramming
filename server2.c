#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_MESSAGE_SIZE 1024
#define MAX_BOOK_TITLE_LENGTH 100
#define MAX_ISBN_LENGTH 20
#define MAX_LINE_LENGTH 1000
#define MAX_RESPONSE_SIZE 1000


struct Order {
    int orderNo;
    char title[100];
    int ISBN;
    int quantity;
    int totalAmount;
};

struct Order currentOrder;
int orderCount = 0;




struct bookStore {
    int no;
    char title[40];
    char authors[50];
    int ISBN;
    char publisher[50];
    char dateOfPublication[12];
    int quantity;
    int price;
};

// Function prototypes
void display_catalog(int M, int X, int Z, struct sockaddr_in client_addr, int sockfd);
void search_book(char* string, struct sockaddr_in client_addr, int sockfd);
int order_book(char* title, int ISBN, int n, struct sockaddr_in client_addr, int sockfd);
void pay_for_book(int order_no, float amount, struct sockaddr_in client_addr, int sockfd);
int read_books_from_file(struct bookStore* books, int max_books);
void process_request(char *request, struct sockaddr_in client_addr, int sockfd);

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[MAX_MESSAGE_SIZE];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listen(sockfd, 5);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        sockfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen);
        if (sockfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Receive request from client
        int recv_len = recv(sockfd, buffer, MAX_MESSAGE_SIZE, 0);
        if (recv_len < 0) {
            perror("Recv failed");
            close(sockfd);
            continue;
        }

        // Null-terminate the received message
        buffer[recv_len] = '\0';

        // Print client's request
        printf("Received from client: %s\n", buffer);

        // Process the request
        process_request(buffer, client_addr, sockfd);

        // Close the client socket
        close(sockaddr_in);
    }

    close(sockfd);
    return 0;
}

void process_request(char *request, struct sockaddr_in client_addr, int sockfd) {
    // Parse the request and process it based on the command
    char command;
    int M, X, Z;
    char search_term[MAX_MESSAGE_SIZE];
    char title[MAX_MESSAGE_SIZE];
    int ISBN, quantity;
    float amount;
    int order_no;

    sscanf(request, "%c", &command);

    switch (command) {
        case '1':
            // Display Catalog
            sscanf(request, "%*c %d %d %d", &M, &X, &Z);
            display_catalog(M, X, Z, client_addr, sockfd);
            break;
        case '2':
            // Search Book
            sscanf(request, "%*c %[^\n]", search_term);
            search_book(search_term, client_addr, sockfd);
            break;
        case '3':
            // Order Book
            sscanf(request, "%*c %s %d %d", title, &ISBN, &quantity);
            order_book(title, ISBN, quantity, client_addr, sockfd);
            break;
        case '4':
            // Pay for Book
            sscanf(request, "%*c %d %f", &order_no, &amount);
            pay_for_book(order_no, amount, client_addr, sockfd);
            break;
        case '5':
            break;
        default:
            printf("Invalid command\n");
            break;
    }
}

void display_catalog(int M, int X, int Z, struct sockaddr_in client_addr, int sockfd) {
    FILE *fptr = fopen("book_info.txt", "r");
    if (fptr == NULL) {
        char response[MAX_RESPONSE_SIZE];
        snprintf(response, MAX_RESPONSE_SIZE, "Error: Unable to open file.\n[END]");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        return;
    }

    char line[MAX_LINE_LENGTH];
    int displayedCount = 0;
    int header_sent = 0;

    // Send the header to the client
    char header[MAX_RESPONSE_SIZE];
    snprintf(header, MAX_RESPONSE_SIZE, "S/No\tTitle\t\tAuthors\t\tISBN\tPublisher\tPublication Date\tQuantity\tPrice\n");
    sendto(sockfd, header, strlen(header), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

    while (fgets(line, sizeof(line), fptr) != NULL) {
        struct bookStore book;
        sscanf(line, "%d %39s %49s %d %49s %11s %d %d",
               &book.no, book.title, book.authors, &book.ISBN, book.publisher, book.dateOfPublication, &book.quantity, &book.price);

        // Check if the book number is within the specified range and whether we have not exceeded the max count
        if (book.no >= X && book.no <= Z) {
            // Send the header once before the first book entry
            if (!header_sent) {
                sendto(sockfd, header, strlen(header), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                header_sent = 1;
            }

            // Check if the displayed count has reached the limit M
            if (displayedCount < M || M == 0) {
                char bookData[MAX_RESPONSE_SIZE];
                snprintf(bookData, MAX_RESPONSE_SIZE, "%d\t%s\t%s\t%d\t%s\t%s\t%d\t%d\n",
                        book.no, book.title, book.authors, book.ISBN, book.publisher, book.dateOfPublication, book.quantity, book.price);
                sendto(sockfd, bookData, strlen(bookData), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
                displayedCount++;
            } else {
                break; 
                // If displayed count reaches M, stop displaying further
            }
        }

    }

    fclose(fptr);

    if (displayedCount == 0) {
        char response[MAX_RESPONSE_SIZE];
        snprintf(response, MAX_RESPONSE_SIZE, "No books found in the specified range.\n[END]");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    } else {
        char end_marker[] = "[END]";
        sendto(sockfd, end_marker, strlen(end_marker), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }
}


void search_book(char* search_term, struct sockaddr_in client_addr, int sockfd) {
    struct bookStore books[100];  // Array to hold books read from file
    int bookCount = read_books_from_file(books, 100);  // Read books from file

    char response[MAX_RESPONSE_SIZE];
    int found = 0;  // Flag to check if book is found

    for (int i = 0; i < bookCount; i++) {
        // Check if the search term matches the book title or ISBN
        if (strstr(books[i].title, search_term) != NULL || books[i].ISBN == atoi(search_term)) {
            snprintf(response, MAX_RESPONSE_SIZE, "S/No %d: %s %s (ISBN: %d) - Publisher: %s, Year: %s, Quantity: %d, Price: %d\n",
                     books[i].no, books[i].title, books[i].authors, books[i].ISBN, books[i].publisher, books[i].dateOfPublication, books[i].quantity, books[i].price);
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            found = 1;
        }
    }

    if (!found) {
        snprintf(response, MAX_RESPONSE_SIZE, "No book found with the given search term.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }
}

int order_book(char* title, int ISBN, int n, struct sockaddr_in client_addr, int sockfd) {
    FILE *fptr = fopen("book_info.txt", "r+");
    if (fptr == NULL) {
        char response[MAX_RESPONSE_SIZE];
        snprintf(response, MAX_RESPONSE_SIZE, "Error: Unable to open file.\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    long pos;
        int found = 0;

    while ((pos = ftell(fptr)) >= 0 && fgets(line, sizeof(line), fptr) != NULL) {
        struct bookStore book;
        sscanf(line, "%d %39s %49s %d %49s %11s %d %d",
               &book.no, book.title, book.authors, &book.ISBN, book.publisher, book.dateOfPublication, &book.quantity, &book.price);

        // Check if the book matches the title, ISBN, and has sufficient quantity
        if (strcmp(book.title, title) == 0 && book.ISBN == ISBN && book.quantity >= n) {
            book.quantity -= n;  // Reduce the book quantity
            fseek(fptr, pos, SEEK_SET);  // Move the file pointer back to the start of the current line
            fprintf(fptr, "%d %s %s %d %s %s %d %d\n",
                    book.no, book.title, book.authors, book.ISBN, book.publisher, book.dateOfPublication, book.quantity, book.price);
            found = 1;

            // Store the current order details
            currentOrder.orderNo = ++orderCount;
            strcpy(currentOrder.title, book.title);
            currentOrder.ISBN = book.ISBN;
            currentOrder.quantity = n;
            currentOrder.totalAmount = book.price * n;
            break;
        }
    }

    fclose(fptr);

    if (found) {
        char response[MAX_RESPONSE_SIZE];
        snprintf(response, MAX_RESPONSE_SIZE, "Order placed successfully. Order number: %d, Total amount: %d\n", currentOrder.orderNo, currentOrder.totalAmount);
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    } else {
        char response[MAX_RESPONSE_SIZE];
        snprintf(response, MAX_RESPONSE_SIZE, "Book not found or insufficient quantity\n");
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    }

    return found ? 1 : 0;
}

void pay_for_book(int order_no, float amount, struct sockaddr_in client_addr, int sockfd) {
    char response[MAX_RESPONSE_SIZE];

    // Check if the provided order number matches the current order number
    if (order_no != currentOrder.orderNo) {
        snprintf(response, MAX_RESPONSE_SIZE, "Invalid order number.\n");
    } else if (amount < currentOrder.totalAmount) {
        snprintf(response, MAX_RESPONSE_SIZE, "Payment unsuccessful. Entered amount is less than the total amount.\n");
    } else {
        float change = amount - currentOrder.totalAmount;
        snprintf(response, MAX_RESPONSE_SIZE, "Payment successful for Order number: %d. Change: %.2f. Shipment will be processed.\n", order_no, change);
    }

    sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
}

int read_books_from_file(struct bookStore* books, int max_books) {
    FILE *fptr = fopen("book_info.txt", "r");
    if (fptr == NULL) {
        perror("Error opening file");
        return 0;
    }

    int count = 0;
    char line[MAX_LINE_LENGTH];

    // Read each line from the file and store it in the books array
    while (fgets(line, sizeof(line), fptr) != NULL && count < max_books) {
        sscanf(line, "%d %39s %49s %d %49s %11s %d %d",
               &books[count].no, books[count].title, books[count].authors, &books[count].ISBN, books[count].publisher, books[count].dateOfPublication, &books[count].quantity, &books[count].price);
        count++;
    }

    fclose(fptr);
    return count;
}
