#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//todo: debug only, this need came from .env
#define DB_PATH "/home/gabriel/repositorios/loja_do_marcos/db.txt"
#define LINE_MAX_LEN 100
#define NAME_LEN 60
#define DELIMITERS ","

typedef struct {
    int id;
    char name[NAME_LEN + 1];
    int quantity;
    float price;
} Item;


static Item processTokens(char *token) {
    Item n;
    int i = 0;
    while (token != NULL && i < 4) {
        printf("Token encontrado: %s\n", token);

        switch (i++) {
            case 0: {
                n.id = atoi(token);
                break;
            }
            case 1: {
                strcpy(n.name, token);
                break;
            }
            case 2: {
                n.quantity = atoi(token);
                break;
            }
            case 3: {
                n.price = atof(token);
                break;
            }
            default: {
            }
        }

        token = strtok(NULL, DELIMITERS);
    }
    return n;
}

static Item findItemById(FILE *fp, int id, fpos_t *pos) {
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[LINE_MAX_LEN + 1];
        if (fgetpos(fp, pos) == 0) {
            printf("Current position of file pointer found\n");
        }
        printf("o que tem: %s\n", fgets(a, LINE_MAX_LEN + 1, fp));

        char *token = strtok(a, DELIMITERS);
        n = processTokens(token);
        if (lastId == n.id) {
            n.id = -2;
            return n;
        }
        lastId = n.id;
        if (n.id == id) {
            break;
        }
    }

    return n;
}

static Item findItemByName(FILE *fp, const char name[NAME_LEN + 1], fpos_t *pos) {
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[NAME_LEN + 1];
        if (fgetpos(fp, pos) == 0) {
            printf("Current position of file pointer found\n");
        }
        printf("o que tem: %s\n", fgets(a, LINE_MAX_LEN + 1, fp));

        char *token = strtok(a, DELIMITERS);
        n = processTokens(token);

        if (lastId == n.id) {
            n.id = -2;
            return n;
        }
        lastId = n.id;

        //if dont match is not the same word
        if (strlen(n.name) != strlen(name)) {
            continue;
        }

        int isTheWord = 1;
        for (int i = 0; i < strlen(n.name); i++) {
            if (tolower(n.name[i]) != tolower(name[i])) {
                isTheWord = 0;
            };
        }
        // it's not the same
        if (!isTheWord) {
            continue;
        }

        break;
    }

    return n;
}

static void editLine(FILE *fp, Item n, const fpos_t *initial_pos) {
    fpos_t final_pos;
    if (fgetpos(fp, &final_pos) == 0) {
        printf("Current position of file pointer found\n");
    }
    char newItem[LINE_MAX_LEN];
    sprintf(newItem, "%d,%s,%d,%.2f", n.id, n.name, n.quantity, n.price);

    if (strlen(newItem) < LINE_MAX_LEN) {
        /*
         * The existence of this logic is because the need to remove all characters of the string... Example:
         * Banana have length of 6, apple have 5. If we write apple in the line of banana the result would be 'applea'
         * Note that the 'a' came from the banana so we need make the newLine at least the same side as before edit
        */
        while (((LINE_MAX_LEN - 2) - strlen(newItem)) > 0) {
            strcat(newItem, " ");
        }
    }

    fsetpos(fp, initial_pos);
    strcat(newItem, ";\n");
    if (strlen(newItem) != 100) {
        printf("Line is longer or less than 100 digits: %d", strlen(newItem));
    }
    fputs(newItem, fp);
}

int main(void) {
    FILE *fp = fopen(DB_PATH, "r+");
    fpos_t initial_pos;
    if (fp == NULL) {
        return EXIT_FAILURE; /* can't open file */
    }

    Item item = findItemById(fp, 2, &initial_pos);

    printf("Id: %d | Name: %s | Quantity: %d | Price: %.2f", item.id, item.name, item.quantity, item.price);
    item.quantity = item.quantity + 10000;

    editLine(fp, item, &initial_pos);
}


// #include  <stdio.h>
// #define  NUM  100
//
// int main(void) {
//     FILE *stream;
//     fpos_t pos;
//     int numwritten;
//     char a[30] = "Bom dia1!\n";
//     char b[30] = "Bom dia2!";
//     char c[30] = "Mal";
//     char temp[30];
//
//     stream = fopen("/home/gabriel/repositorios/loja_do_marcos/teste.txt", "r+b");
//
//     printf("Inicio ou fim da linha: %s", fgets(temp, NUM, stream));
//     if (fgetpos(stream, &pos) == 0) {
//         printf("Current position of file pointer found\n");
//     }
//     numwritten = fputs(a, stream);
//
//
//     numwritten = fputs(b, stream);
//     fsetpos(stream, &pos);
//
//
//     numwritten = fputs("    ", stream);
//
//     printf("Number of items successfully written = %d\n", numwritten);
// }
