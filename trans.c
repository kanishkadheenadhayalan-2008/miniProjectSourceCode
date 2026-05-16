#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------- STRUCT -------- */
struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

/* -------- FUNCTION PROTOTYPES -------- */
void newRecord(FILE *fPtr);
void updateRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);

/* UNIQUE FEATURES */
void logTransaction(char action[], struct clientData c, double amount);
void undoLast(FILE *fPtr);
void lowBalance(FILE *fPtr);
void richestAccount(FILE *fPtr);
void integrityCheck(FILE *fPtr);

/* MENU */
unsigned int enterChoice(void);

/* -------- MAIN -------- */
int main()
{
    FILE *cfPtr;

    /* open file */
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("File not found. Creating new file...\n");
        cfPtr = fopen("credit.dat", "wb+");

        struct clientData blank = {0, "", "", 0.0};
        for (int i = 0; i < 100; i++)
        {
            fwrite(&blank, sizeof(blank), 1, cfPtr);
        }
    }

    unsigned int choice;

    while ((choice = enterChoice()) != 10)
    {
        switch (choice)
        {
        case 1: newRecord(cfPtr); break;
        case 2: updateRecord(cfPtr); break;
        case 3: deleteRecord(cfPtr); break;
        case 4: undoLast(cfPtr); break;
        case 5: lowBalance(cfPtr); break;
        case 6: richestAccount(cfPtr); break;
        case 7: integrityCheck(cfPtr); break;
        default: printf("Invalid choice\n");
        }
    }

    fclose(cfPtr);
    return 0;
}

/* -------- FEATURE 1: LOG -------- */
void logTransaction(char action[], struct clientData c, double amount)
{
    FILE *log = fopen("log.txt", "a");

    fprintf(log, "%s %d %.2f %.2f\n",
            action, c.acctNum, amount, c.balance);

    fclose(log);
}

/* -------- FEATURE 2: UNDO -------- */
void undoLast(FILE *fPtr)
{
    FILE *log = fopen("log.txt", "r");

    if (log == NULL)
    {
        printf("No transaction history.\n");
        return;
    }

    char line[100], last[100];

    while (fgets(line, sizeof(line), log))
        strcpy(last, line);

    fclose(log);

    int acc;
    double amount, balance;

    sscanf(last, "%*s %d %lf %lf", &acc, &amount, &balance);

    struct clientData c;

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);
    fread(&c, sizeof(c), 1, fPtr);

    c.balance -= amount;

    fseek(fPtr, -(long)sizeof(c), SEEK_CUR);
    fwrite(&c, sizeof(c), 1, fPtr);

    printf("Undo successful.\n");
}

/* -------- FEATURE 3: LOW BALANCE -------- */
void lowBalance(FILE *fPtr)
{
    struct clientData c;
    rewind(fPtr);

    printf("\nAccounts with balance < 1000:\n");

    while (fread(&c, sizeof(c), 1, fPtr))
    {
        if (c.acctNum != 0 && c.balance < 1000)
        {
            printf("%d %s %s %.2f\n",
                   c.acctNum, c.firstName, c.lastName, c.balance);
        }
    }
}

/* -------- FEATURE 4: RICHEST ACCOUNT -------- */
void richestAccount(FILE *fPtr)
{
    struct clientData c, max = {0, "", "", 0};
    rewind(fPtr);

    while (fread(&c, sizeof(c), 1, fPtr))
    {
        if (c.acctNum != 0 && c.balance > max.balance)
            max = c;
    }

    printf("\nRichest Account:\n");
    printf("%d %s %s %.2f\n",
           max.acctNum, max.firstName, max.lastName, max.balance);
}

/* -------- FEATURE 5: INTEGRITY CHECK -------- */
void integrityCheck(FILE *fPtr)
{
    struct clientData c;
    int checksum = 0;

    rewind(fPtr);

    while (fread(&c, sizeof(c), 1, fPtr))
    {
        checksum += c.acctNum;
    }

    printf("Checksum value: %d\n", checksum);
}

/* -------- ADD RECORD -------- */
void newRecord(FILE *fPtr)
{
    struct clientData c = {0, "", "", 0.0};
    unsigned int acc;

    printf("Enter account number (1-100): ");
    scanf("%u", &acc);

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);
    fread(&c, sizeof(c), 1, fPtr);

    if (c.acctNum != 0)
    {
        printf("Account already exists.\n");
        return;
    }

    printf("Enter lastname firstname balance: ");
    scanf("%s %s %lf", c.lastName, c.firstName, &c.balance);

    c.acctNum = acc;

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);
    fwrite(&c, sizeof(c), 1, fPtr);

    logTransaction("CREATE", c, c.balance);
}

/* -------- UPDATE RECORD -------- */
void updateRecord(FILE *fPtr)
{
    struct clientData c;
    unsigned int acc;
    double amt;

    printf("Enter account number: ");
    scanf("%u", &acc);

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);
    fread(&c, sizeof(c), 1, fPtr);

    if (c.acctNum == 0)
    {
        printf("Account not found.\n");
        return;
    }

    printf("Enter amount (+deposit / -withdraw): ");
    scanf("%lf", &amt);

    c.balance += amt;

    fseek(fPtr, -(long)sizeof(c), SEEK_CUR);
    fwrite(&c, sizeof(c), 1, fPtr);

    logTransaction("UPDATE", c, amt);
}

/* -------- DELETE RECORD -------- */
void deleteRecord(FILE *fPtr)
{
    struct clientData c, blank = {0, "", "", 0};
    unsigned int acc;

    printf("Enter account number to delete: ");
    scanf("%u", &acc);

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);
    fread(&c, sizeof(c), 1, fPtr);

    if (c.acctNum == 0)
    {
        printf("Account not found.\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);
    fwrite(&blank, sizeof(c), 1, fPtr);

    printf("Account deleted.\n");
}

/* -------- MENU -------- */
unsigned int enterChoice(void)
{
    unsigned int choice;

    printf("\n===== BANK SYSTEM =====\n");
    printf("1. Add Account\n");
    printf("2. Update Account\n");
    printf("3. Delete Account\n");
    printf("4. Undo Last Transaction\n");
    printf("5. Low Balance Alert\n");
    printf("6. Richest Account\n");
    printf("7. Data Integrity Check\n");
    printf("10. Exit\n");
    printf("Enter choice: ");

    scanf("%u", &choice);

    return choice;
}