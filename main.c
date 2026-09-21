#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//todo: debug only, this need came from .env
#define DB_PATH "/home/gabriel/repositorios/loja_do_marcos/db.txt"
#define LINE_MAX_LEN 100
#define NAME_LEN 60
#define DELIMITERS ","
#define MAX_ITEM_PER_PAGE 20
#define ENABLE_LOGS 0
FILE *fp;

typedef struct {
    int id;
    char name[NAME_LEN + 1];
    int quantity;
    float price;
} Item;


static Item itens[MAX_ITEM_PER_PAGE];

static int openFile() {
    fp = fopen(DB_PATH, "r+");

    if (fp == NULL) {
        printf("File didn't open correctly");
        return 0;
    }
    return 1;
}

static void closeFile() {
    if (fclose(fp) != 0) {
        printf("File didn't close correctly");
    }
}

static Item processTokens(char *token) {
    Item n;
    int i = 0;
    while (token != NULL && i < 4) {
        ENABLE_LOGS && printf("Token encontrado: %s\n", token);

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

static Item findItemById(int id, fpos_t *pos) {
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[LINE_MAX_LEN + 1];
        if (fgetpos(fp, pos) == 0) {
            ENABLE_LOGS && printf("Current position of file pointer found\n");
        }
        fgets(a, LINE_MAX_LEN + 1, fp);
        ENABLE_LOGS && printf("o que tem: %s\n", a);

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

static Item findItemByName(const char name[NAME_LEN + 1], fpos_t *pos) {
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[NAME_LEN + 1];
        if (fgetpos(fp, pos) == 0) {
            ENABLE_LOGS && printf("Current position of file pointer found\n");
        }
        fgets(a, LINE_MAX_LEN + 1, fp);
        ENABLE_LOGS && printf("o que tem: %s\n", a);

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

static void editLine(Item n, const fpos_t *initial_pos) {
    fpos_t final_pos;
    if (fgetpos(fp, &final_pos) == 0) {
        ENABLE_LOGS && printf("Current position of file pointer found\n");
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
        printf("Line is longer or less than 100 digits: %lu", strlen(newItem));
    }
    fputs(newItem, fp);
}

static void clearPreviousItems() {
    for (int i = 0; i < MAX_ITEM_PER_PAGE; i++) {
        const Item n = {};
        itens[i] = n;
    }
}

static Item getAllItem(int page) {
    clearPreviousItems();
    int startId = (MAX_ITEM_PER_PAGE * page) - MAX_ITEM_PER_PAGE;
    for (int i = 0; i <= MAX_ITEM_PER_PAGE; i++) {
        fpos_t pos;
        Item item = findItemById(startId + (i + 1), &pos);

        if (item.id == -2) {
            printf("Collected all the items");
            break;
        }
        itens[i] = item;
    }

    return *itens;
}


int main(void) {
    if (!openFile()) {
        return EXIT_FAILURE;
    };

    getAllItem(1);




    closeFile();
}


//todo: logica para pegar todos os items
// for (int i = 0; i < MAX_ITEM_PER_PAGE; i++) {
//     Item item = itens[i];
//     if (item.id == 0) {
//         printf("No items left");
//         break;
//     }
//     printf("Id: %d | Name: %s | Quantity: %d | Price: %.2f\n", item.id, item.name, item.quantity, item.price);
// }

//todo: logica para editar um item
//    // fpos_t initial_pos;
// Item item = findItemById(2, &initial_pos);
//
// printf("Id: %d | Name: %s | Quantity: %d | Price: %.2f", item.id, item.name, item.quantity, item.price);
// item.quantity = item.quantity + 10000;
//
// editLine(item, &initial_pos);