#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>     // For close()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BOOKS 100

typedef struct
{
    int orderNumber;
    char title[256];
    char isbn[20];
    int quantity;
} Order;

Order orders[MAX_BOOKS];
int orderCount = 0;

// Function declarations
void displayCatalog(int M, int X, int Z, int clientSocket);
char *searchBook(const char *string);
char *orderBook(const char *title, const char *isbn, int n);
char *payForBook(int orderno, float amount);
void handleClientRequest(int clientSocket);

int main()
{
    int serverSocketFD, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressSize;

    // Create a socket
    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD < 0)
    {
        perror("Failed to create socket");
        return 1;
    }

    // Set up server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; 
    serverAddress.sin_port = htons(2000);              // Set the port to 2000

    // Bind the socket
    if (bind(serverSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("Bind failed");
        close(serverSocketFD);
        return 1;
    }

    printf("Server is listening on port 2000...\n");

    // Listen for incoming connections
    if (listen(serverSocketFD, 5) < 0)// allowing up to 5 pending connections.
    {
        perror("Listen failed");
        close(serverSocketFD);
        return 1;
    }

    // Accept and handle client requests
    while (true)
    {
        clientAddressSize = sizeof(clientAddress);
        clientSocket = accept(serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressSize);
        if (clientSocket < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Client connected from %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

        // Handle client request
        handleClientRequest(clientSocket);

        // Close the client socket
        close(clientSocket);
        printf("Client disconnected\n");
    }

    // Close the server socket
    close(serverSocketFD);
    return 0;
}

// Display catalog to the client
void displayCatalog(int M, int X, int Z, int clientSocket)
{
    printf("\n Display Catalog Requested...\n");
    // Open the books.txt file
    FILE *file = fopen("books.txt", "r");
    if (file == NULL)
    {
        char *errorMsg = "Failed to open file.\n";
        send(clientSocket, errorMsg, strlen(errorMsg), 0);
        return;
    }

    char header[1024];
    fgets(header, sizeof(header), file); // Read the header line

    char line[1024];
    int count = 0, lineNum = 0;
    char catalog[4096] = "";

    // Read lines from the file
    while (fgets(line, sizeof(line), file) != NULL)
    {
        lineNum++;
        if (lineNum >= X && lineNum <= Z)
        { // Check if the line is within the specified range
            if (count == 0)
            {
                strcat(catalog, header); // Add the header to the catalog string
            }
            strcat(catalog, line); // Add the line to the catalog string
            count++;
            if (count >= M)
            {
                break; // Stop if the maximum number of books is reached
            }
        }
    }

    fclose(file); // Close the file

    send(clientSocket, catalog, strlen(catalog), 0); // Send the catalog to the client
    printf("Display Catalog Result Returned.\n");
}

// Search for a book in the catalog
char *searchBook(const char *string)
{
    printf("\nSearching For Book: %s\n", string);
    // Open the books.txt file
    FILE *file = fopen("books.txt", "r");
    if (file == NULL)
    {
        return strdup("Failed to open file.");
    }

    char header[1024];
    fgets(header, sizeof(header), file); // Read the header line

    char *result = (char *)malloc(1024 * sizeof(char));
    if (result == NULL)
    {
        fclose(file);
        return strdup("Memory allocation failed.");
    }

    char line[1024];
    bool found = false;
    bool searchByISBN = isdigit(string[0]); // Determine if the search is by ISBN

    // Read lines from the file
    while (fgets(line, sizeof(line), file) != NULL)
    {
        char lineCopy[1024];
        strcpy(lineCopy, line);

        if (searchByISBN)
        {
            strcpy(lineCopy, line);
            char *token = strtok(lineCopy, " \t\n");
            int columnIndex = 0;
            bool isbnMatch = false;

            // Tokenize the line and check for ISBN match
            while (token != NULL)
            {
                if (columnIndex == 3 && strcmp(token, string) == 0)
                {
                    isbnMatch = true;
                    break;
                }
                token = strtok(NULL, " \t\n");
                columnIndex++;
            }
            if (isbnMatch)
            {
                found = true;
                strcpy(result, line);
                break;
            }
        }
        else
        {
            if (strstr(line, string) != NULL)
            { // Check for title match
                found = true;
                strcpy(result, line);
                break;
            }
        }
    }

    fclose(file); // Close the file

    if (found)
    {
        char *finalResult = (char *)malloc((strlen(header) + strlen(result) + 1) * sizeof(char));
        if (finalResult == NULL)
        {
            free(result);
            return strdup("Memory allocation failed.");
        }
        strcpy(finalResult, header); // Copy the header to the result
        strcat(finalResult, result); // Add the found line to the result
        free(result);
        return finalResult;
    }
    else
    {
        free(result);
        return strdup("Item not found.");
    }
}

// Place an order for a book
char *orderBook(const char *title, const char *isbn, int n)
{

    printf("\nOrdering Book: %s", title);

    char *itemDetails = searchBook(title); // Search for the book
    char *copylItemDetails = strdup(itemDetails);


    // Extract Cost
    float cost = 0.0;
    char *token = strtok(copylItemDetails, "\n"); // Skip the header by tokenizing until the first newline character
    token = strtok(NULL, "\n"); // Move to the second line
    token = strtok(token, " ");

    for (int i = 0; i < 5; i++)
    {                              // Skip the first five tokens
        token = strtok(NULL, " "); // Move to the next token
        if (token == NULL)
        {
            break;
        }
    }

    if (token != NULL)
    {                                           // Check if the sixth token exists
        cost = atof(token);
    }

    // Calculate total amount based on the cost
    float totalAmount = n * cost;

    if (strcmp(itemDetails, "Item not found.") == 0)
    {
        free(itemDetails);
        return strdup("Item not found.");
    }

    Order newOrder;
    // Initialize the order details
    newOrder.orderNumber = orderCount + 1;
    strcpy(newOrder.title, title);
    strcpy(newOrder.isbn, isbn);
    newOrder.quantity = n;

    // Save the order details
    orders[orderCount] = newOrder;
    orderCount++;

    // Prepare the order details message
    char *orderDetails = (char *)malloc(2048 * sizeof(char));
    if (orderDetails == NULL)
    {
        free(itemDetails);
        return strdup("Memory allocation failed.");
    }

    // Format the order details message
    snprintf(orderDetails, 2048, "%s\nQuantity: %d\nTotal Amount: $%.2f\nOrder placed successfully. Order No: %d\n",
             itemDetails, n, totalAmount, newOrder.orderNumber);

    free(itemDetails);
    return orderDetails;
}

// Process payment for a book order
char *payForBook(int orderno, float amount)
{
    printf("\nPaying for Order Number: %d\n", orderno);
    // Iterate through orders to find the specified order number
    for (int i = 0; i < orderCount; i++)
    {
        if (orders[i].orderNumber == orderno)
        {
            char *message = (char *)malloc(1024 * sizeof(char));
            if (message == NULL)
            {
                return strdup("Memory allocation failed.");
            }
            // Format the payment success message
                        snprintf(message, 1024, "Payment successful for Order No: %d.\nAmount: $%.2f.\nShipment will take place in 2 days. Expected arrival in 7 days.",
                     orderno, amount);
            return message;
        }
    }
    return strdup("Order number not found.");
}

// Handle client requests
void handleClientRequest(int clientSocket)
{
    char buffer[1024];
    int bytesReceived;

    while (true)
    {
        // Receive data from the client
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1)
        {
            perror("Receive failed");
            break;
        }
        else if (bytesReceived == 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytesReceived] = '\0';

        // Process client requests
        if (strncmp(buffer, "DisplayCatalog:", strlen("DisplayCatalog:")) == 0)
        {
            int M, X, Z;
            sscanf(buffer + strlen("DisplayCatalog:"), "%d,%d,%d", &M, &X, &Z);
            displayCatalog(M, X, Z, clientSocket);
        }
        else if (strncmp(buffer, "SearchForItem:", strlen("SearchForItem:")) == 0)
        {
            char *result = searchBook(buffer + strlen("SearchForItem:"));
            send(clientSocket, result, strlen(result), 0);
            printf("Search Result Returned\n");
            free(result);
        }
        else if (strncmp(buffer, "PurchaseItem:", strlen("PurchaseItem:")) == 0)
        {
            char title[256], isbn[20];
            int quantity;
            sscanf(buffer + strlen("PurchaseItem:"), "%[^,],%[^,],%d", title, isbn, &quantity);
            char *orderDetails = orderBook(title, isbn, quantity);
            send(clientSocket, orderDetails, strlen(orderDetails), 0);
            printf("Ordering Number Returned:\n");
            free(orderDetails);
        }
        else if (strncmp(buffer, "PayForItem:", strlen("PayForItem:")) == 0)
        {
            int orderNo;
            float amount;
            sscanf(buffer + strlen("PayForItem:"), "%d,%f", &orderNo, &amount);
            char *result = payForBook(orderNo, amount);
            send(clientSocket, result, strlen(result), 0);
            printf("Payment Status Returned\n");
            free(result);
        }
        else
        {
            char *errorMsg = "Invalid request.";
            send(clientSocket, errorMsg, strlen(errorMsg), 0);
        }
    }
}
