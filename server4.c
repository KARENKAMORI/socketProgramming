#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_MESSAGE_SIZE 1024
#define MAX_BOOK_TITLE_LENGTH 100
#define MAX_ISBN_LENGTH 20
#define MAX_LINE_LENGTH 1000
#define MAX_RESPONSE_SIZE 1000

// Struct to hold order information
struct Order {
    int orderNo;
    char title[100];
    int ISBN;
    int quantity;
    int totalAmount;
};

struct Order currentOrder;  // Global variable to hold current order details
int orderCount = 0;         // Global variable to keep track of order count

// Struct to hold bookstore information
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
void display_catalog(int M, int X, int Z, int client_sockfd);
void search_book(char* string, int client_sockfd);
int order_book(char* title, int ISBN, int n, int client_sockfd);
void pay_for_book(int order_no, float amount, int client_sockfd);
int read_books_from_file(struct bookStore* books, int max_books);
void process_request(char *request, int client_sockfd);

int main() {
    int server_sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[MAX_MESSAGE_SIZE];

    // Create socket
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_sockfd, 5) < 0) {
        perror("Listen failed");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // Accept a new connection
        if ((client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Handle the client in a separate process (for simplicity)
        if (fork() == 0) {
            close(server_sockfd);
            memset(buffer, 0, MAX_MESSAGE_SIZE);

            while (1) {
                // Receive request from client
                int n = read(client_sockfd, buffer, MAX_MESSAGE_SIZE);
                if (n == 0) {
                    printf("Client disconnected\n");
                    break;
                } else if (n < 0) {
                    perror("read failed");
                    break;
                }
                buffer[n] = '\0';

                // Process the received request
                process_request(buffer, client_sockfd);
            }

            close(client_sockfd);
            exit(0);
        } else {
            close(client_sockfd);
        }
    }

    close(server_sockfd);
    return 0;
}

// Function to process client request
void process_request(char *request, int client_sockfd) {
    int request_type;
    sscanf(request, "%d", &request_type);

    switch (request_type) {
        case 1: { // Display Catalog
            int M, X, Z;
            sscanf(request, "%*d %d %d %d", &M, &X, &Z);
            display_catalog(M, X, Z, client_sockfd);
            break;
        }
        case 2: { // Search Book
            char title[MAX_BOOK_TITLE_LENGTH];
            sscanf(request, "%*d %[^\n]", title);
            search_book(title, client_sockfd);
            break;
        }
        case 3: { // Order Book
            int ISBN, quantity;
            char title[MAX_BOOK_TITLE_LENGTH];
            // Extract title part
            char *title_ptr = strchr(request, ' ') + 1;
            char *isbn_ptr = strrchr(title_ptr, ' ');
            sscanf(isbn_ptr, "%d %d", &ISBN, &quantity);
            *isbn_ptr = '\0';  // Null-terminate title part
            strcpy(title, title_ptr);  // Copy title part to title variable
            order_book(title, ISBN, quantity, client_sockfd);
            break;
        }
        case 4: { // Pay for Book
            int orderID;
            float amount;
            sscanf(request, "%*d %d %f", &orderID, &amount);
            pay_for_book(orderID, amount, client_sockfd);
            break;
        }
        default:
            printf("Invalid request type\n");
    }
}

// Function to display catalog of books
void display_catalog(int M, int X, int Z, int client_sockfd) {
    struct bookStore books[MAX_LINE_LENGTH];
    int total_books = read_books_from_file(books, MAX_LINE_LENGTH);
    char response[MAX_RESPONSE_SIZE];
    memset(response, 0, sizeof(response));

    if (total_books < 0) {
        snprintf(response, sizeof(response), "Error reading book file\n");
        write(client_sockfd, response, strlen(response));
        return;
    }

    int books_displayed = 0;
    for (int i = 0; i < total_books; i++) {
        if ((M == 0 || books_displayed < M) && books[i].no >= X && books[i].no <= Z) {
            char book_info[MAX_RESPONSE_SIZE];
            snprintf(book_info, sizeof(book_info), "No: %d, Title: %s, Authors: %s, ISBN: %d, Publisher: %s, Date: %s, Quantity: %d, Price: %d\n",
                     books[i].no, books[i].title, books[i].authors, books[i].ISBN, books[i].publisher, books[i].dateOfPublication, books[i].quantity, books[i].price);
            strncat(response, book_info, sizeof(response) - strlen(response) - 1);
            books_displayed++;
        }
    }

    strncat(response, "[END]", sizeof(response) - strlen(response) - 1);
    write(client_sockfd, response, strlen(response));
}

// Function to search for a book
void search_book(char* string, int client_sockfd) {
    struct bookStore books[MAX_LINE_LENGTH];
    int total_books = read_books_from_file(books, MAX_LINE_LENGTH);
    char response[MAX_RESPONSE_SIZE];
    memset(response, 0, sizeof(response));

    if (total_books < 0) {
        snprintf(response, sizeof(response), "Error reading book file\n");
        write(client_sockfd, response, strlen(response));
        return;
    }

    for (int i = 0; i < total_books; i++) {
        if (strstr(books[i].title, string) != NULL) {
            snprintf(response, sizeof(response), "Book found: No: %d, Title: %s, Authors: %s, ISBN: %d, Publisher: %s, Date: %s, Quantity: %d, Price: %d\n",
                     books[i].no, books[i].title, books[i].authors, books[i].ISBN, books[i].publisher, books[i].dateOfPublication, books[i].quantity, books[i].price);
            write(client_sockfd, response, strlen(response));
            return;
        }
    }

    snprintf(response, sizeof(response), "Book not found\n");
    write(client_sockfd, response, strlen(response));
}

// Function to order a book


// Function to order a book
int order_book(char *title, int ISBN, int n, int client_socket)
{
	FILE *fptr = fopen("books.txt", "r+");
	if (!fptr)
	{
		char response[MAX_RESPONSE_SIZE] = "Error: Unable to open file.\n";
		send(client_socket, response, strlen(response), 0);
		return 0;
	}

	char line[MAX_LINE_LENGTH];
	long pos;
	int found = 0;
	struct bookStore orderedBook;

	while ((pos = ftell(fptr)) >= 0 && fgets(line, sizeof(line), fptr) != NULL)
	{
		struct bookStore book;
		sscanf(line, "%d %39s %49s %d %49s %11s %d %d",
			   &book.no, book.title, book.authors, &book.ISBN, book.publisher, book.dateOfPublication, &book.quantity, &book.price);

		if (strcmp(book.title, title) == 0 && book.ISBN == ISBN && book.quantity >= n)
		{
			book.quantity -= n;
			fseek(fptr, pos, SEEK_SET);
			fprintf(fptr, "%d %s %s %d %s %s %d %d\n",
					book.no, book.title, book.authors, book.ISBN, book.publisher, book.dateOfPublication, book.quantity, book.price);
			orderedBook = book;
			found = 1;
			break;
		}
	}

	fclose(fptr);

	if (found)
	{
		char response[MAX_RESPONSE_SIZE];
		snprintf(response, MAX_RESPONSE_SIZE, "Order placed successfully. Order number: %d\nBook: %s\nQuantity: %d\nTotal Amount: %d\n",
				 ++orderCount, orderedBook.title, n, n * orderedBook.price);
		send(client_socket, response, strlen(response), 0);

		currentOrder.orderNo = orderCount;
		strcpy(currentOrder.title, orderedBook.title);
		currentOrder.ISBN = orderedBook.ISBN;
		currentOrder.quantity = n;
		currentOrder.totalAmount = n * orderedBook.price;
	}
	else
	{
		char response[MAX_RESPONSE_SIZE] = "Book not found or insufficient quantity\n";
		send(client_socket, response, strlen(response), 0);
	}

	return found ? 1 : 0;
}

    snprintf(response, sizeof(response), "Book not found\n");
    write(client_sockfd, response, strlen(response));
    return -1;
}

// Function to pay for a book
void pay_for_book(int order_no, float amount, int client_sockfd) {
    char response[MAX_RESPONSE_SIZE];
    memset(response, 0, sizeof(response));

    if (order_no == currentOrder.orderNo && amount >= currentOrder.totalAmount) {
        snprintf(response, sizeof(response), "Payment successful for Order No: %d. Amount Paid: %.2f\n", order_no, amount);
    } else {
        snprintf(response, sizeof(response), "Payment failed. Invalid order number or insufficient amount.\n");
    }

    write(client_sockfd, response, strlen(response));
}

// Function to read books from file
int read_books_from_file(struct bookStore* books, int max_books) {
    FILE *file = fopen("books.txt", "r");
    if (!file) {
        perror("File open failed");
        return -1;
    }

    int count = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) && count < max_books) {
        sscanf(line, "%d %39[^\t] %49[^\t] %d %49[^\t] %11[^\t] %d %d",
               &books[count].no, books[count].title, books[count].authors, &books[count].ISBN,
               books[count].publisher, books[count].dateOfPublication, &books[count].quantity, &books[count].price);
        count++;
    }

    fclose(file);
    return count;
}
