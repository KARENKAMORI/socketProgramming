#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char accno[20];
    char fname[20];
    char lname[20];
    float bal;
} Customer;

FILE *file;

void Reg(void);
void Deposit(void);
void Withdraw(void);
void Balance(void);
void readCustomer(char *accno, Customer *customer);
void updateCustomer(Customer *customer);

int main(void) {
    int choice;
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
        switch (choice) {
            case 1:
                Deposit();
                break;
            case 2:
                Withdraw();
                break;
            case 3:
                Balance();
                break;
            case 4:
                Reg();
                break;
            case 5:
                exit(0);
            default:
                printf("Invalid choice, please try again.\n");
        }
    }
}

void Deposit(void) {
    char accno[20];
    float amt;
    Customer customer;

    printf("Enter account number: \n");
    scanf("%s", accno);
    readCustomer(accno, &customer);

    printf("Enter amount to Deposit: \n");
    scanf("%f", &amt);

    customer.bal += amt;
    updateCustomer(&customer);
    printf("New Balance: %.2f\n", customer.bal);
}

void Withdraw(void) {
    char accno[20];
    float amt;
    Customer customer;

    printf("Enter account number: \n");
    scanf("%s", accno);
    readCustomer(accno, &customer);

    printf("Enter amount to Withdraw: \n");
    scanf("%f", &amt);

    if (amt > customer.bal) {
        printf("Insufficient balance.\n");
    } else {
        customer.bal -= amt;
        updateCustomer(&customer);
        printf("New Balance: %.2f\n", customer.bal);
    }
}

void Balance(void) {
    char accno[20];
    Customer customer;

    printf("Enter account number: \n");
    scanf("%s", accno);
    readCustomer(accno, &customer);

    printf("Current Balance: %.2f\n", customer.bal);
}

void Reg(void) {
    int num;
    printf("Enter number of Customers: \n");
    scanf("%d", &num);

    file = fopen("accounts.txt", "a");
    if (file == NULL) {
        perror("ERROR opening the FILE!!!");
        exit(1);
    }

    Customer customer[num];
    for (int i = 0; i < num; i++) {
        printf("Account Number: \n");
        scanf("%s", customer[i].accno);

        printf("First Name: \n");
        scanf("%s", customer[i].fname);

        printf("Last Name: \n");
        scanf("%s", customer[i].lname);

        printf("Balance: \n");
        scanf("%f", &customer[i].bal);

        fprintf(file, "%s %s %s %.2f\n", customer[i].accno, customer[i].fname, customer[i].lname, customer[i].bal);
    }
    fclose(file);
}

void readCustomer(char *accno, Customer *customer) {
    file = fopen("accounts.txt", "r");
    if (file == NULL) {
        perror("ERROR opening the FILE!!!");
        exit(1);
    }

    while (fscanf(file, "%s %s %s %f", customer->accno, customer->fname, customer->lname, &customer->bal) != EOF) {
        if (strcmp(customer->accno, accno) == 0) {
            fclose(file);
            return;
        }
    }
    fclose(file);
    printf("Account not found.\n");
    exit(1);
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
