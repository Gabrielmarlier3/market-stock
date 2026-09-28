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
#define ENABLE_DEBUG_LOGS 0
#define SYS_VERSION "0.0.1"
FILE *fp;

typedef struct {
    int id;
    char name[NAME_LEN + 1];
    int quantity;
    float price;
} Item;

static enum {
    HOME_SCREEN,
    EDIT_SCREEN,
    DELETE_SCREEN,
    SEARCH_SCREEN,
    EXIT_SCREEN
} Screen;

static enum {
    EDIT_ITEM,
    FIND_ITEM,
    DELETE_ITEM,
    RETURN_PAGE,
    NEXT_PAGE,
    EXIT_PROGRAM
} Actions;


static Item itens[MAX_ITEM_PER_PAGE];
static int lastReadIndex = 0;
static int fulledPage = 0;

static int openFile() {
    if (fp != NULL) {
        fseek(fp, 0L, SEEK_SET);
        return 1;
    }

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
        ENABLE_DEBUG_LOGS && printf("Token encontrado: %s\n", token);

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

static Item findItemById(int id, fpos_t *initial_pos) {
    openFile();
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[LINE_MAX_LEN + 1];
        if (fgetpos(fp, initial_pos) == 0) {
            ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
        }
        fgets(a, LINE_MAX_LEN + 1, fp);
        ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

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

static Item findItemByName(const char name[NAME_LEN + 1], fpos_t *initial_pos) {
    openFile();
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[LINE_MAX_LEN + 1];
        if (fgetpos(fp, initial_pos) == 0) {
            ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
        }
        fgets(a, LINE_MAX_LEN + 1, fp);
        ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

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

static void createItem(Item item) {
    openFile();
    fseek(fp, -LINE_MAX_LEN, SEEK_END);

    char a[LINE_MAX_LEN + 1];
    fgets(a, LINE_MAX_LEN + 1, fp);
    ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

    char *token = strtok(a, DELIMITERS);
    Item n = processTokens(token);

    char newItem[LINE_MAX_LEN];
    sprintf(newItem, "%d,%s,%d,%.2f", item.id, item.name, item.quantity, item.price);

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

    strcat(newItem, ";\n");
    if (strlen(newItem) != 100) {
        printf("Line is longer or less than 100 digits: %lu", strlen(newItem));
    }
    fputs(newItem, fp);
}

/*
 * This thing, is that type of thing that you made and work, but you don't know how
 * initial_pos: The beginner possition of the file you want to delete
 * final_pos: The start of the next line
 */
static int deleteItem(const fpos_t initial_pos, const fpos_t *final_pos) {
    char tmpFileName[11] = "db-tmp.txt";
    FILE *fp2 = fopen(tmpFileName, "w+");

    if (fp2 == NULL) {
        printf("Temp file didn't open correctly");
        return 0;
    }

    fpos_t tmp_pos, tmp2_pos;
    fseek(fp, 0L, SEEK_SET);
    fgetpos(fp2, &tmp_pos);

    int end = initial_pos.__pos;
    while (tmp_pos.__pos < end) {
        char a[LINE_MAX_LEN + 1];
        fgets(a, LINE_MAX_LEN + 1, fp);
        fputs(a, fp2);
        fgetpos(fp2, &tmp_pos);
    }


    fseek(fp, 0L, SEEK_END);
    fgetpos(fp, &tmp2_pos); // posição final do arquivo

    end = tmp2_pos.__pos;
    fsetpos(fp, final_pos);

    while (tmp_pos.__pos < end - LINE_MAX_LEN) {
        char a[LINE_MAX_LEN + 1];
        fgets(a, LINE_MAX_LEN + 1, fp);
        fputs(a, fp2);
        fgetpos(fp2, &tmp_pos);
    }

    if (remove(DB_PATH)) {
        perror("cannot remove database");
        return 1;
    }
    if (rename(tmpFileName, DB_PATH)) {
        perror("cannot rename database");
        return 1;
    }

    return 0;
}

static void editLine(Item n, const fpos_t *initial_pos) {
    fpos_t final_pos;
    if (fgetpos(fp, &final_pos) == 0) {
        ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
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
    if (lastReadIndex == page) {
        return *itens;
    }
    clearPreviousItems();
    int startId = (MAX_ITEM_PER_PAGE * page) - MAX_ITEM_PER_PAGE;
    for (int i = 0; i <= MAX_ITEM_PER_PAGE; i++) {
        fpos_t pos;
        Item item = findItemById(startId + (i + 1), &pos);

        if (item.id == -2) {
            ENABLE_DEBUG_LOGS && printf("Collected all the items");
            break;
        }
        itens[i] = item;
    }

    return *itens;
}

static void clean_scream() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

//todo: conseguindo limpar, o console, deixar isso em loop, toda função deve estar preparada para ser o mais eficiente
static void printDefaultScreen(int page) {
    clean_scream();
    printf("\t\t\tStock System v%s\n", SYS_VERSION);

    getAllItem(page);
    printf("\t\t   id | Quantity | Price   | Name\n");
    fulledPage = 1;
    for (int i = 0; i < MAX_ITEM_PER_PAGE; i++) {
        Item item = itens[i];
        if (item.id == 0) {
            ENABLE_DEBUG_LOGS && printf("No items left");
            fulledPage = 0;

            break;
        }
        printf("\t\t%5d | %8d | %7.2f | %s \n", item.id, item.quantity, item.price, item.name);
    }
}

int main(void) {
    if (!openFile(fp, DB_PATH)) {
        return EXIT_FAILURE;
    };

    int run_program = 0;
    int page = 1;
    int action = 0;
    do {
        switch (Screen) {
            case HOME_SCREEN: {
                printDefaultScreen(page);
                break;
            }
            case EDIT_SCREEN: {
                printDefaultScreen(page);
                break;
            }
            case DELETE_SCREEN: {
                printDefaultScreen(page);
                break;
            }
            case SEARCH_SCREEN: {
                printDefaultScreen(page);
                break;
            }
            case EXIT_SCREEN: {
                run_program = 0;
                break ;
            }
            default: printDefaultScreen(page);
        }
        printf("[%d] Edit item | [%d] Find item | [%d] Delete item | [%d] Return Page | [%d] Next Page | [%d] Exit",
               EDIT_ITEM, FIND_ITEM, DELETE_ITEM, RETURN_PAGE, NEXT_PAGE, EXIT_PROGRAM);

        while (1) {
            scanf("%d", &Actions);
            switch (action) {
                case EDIT_ITEM: {
                }
                case FIND_ITEM: {
                }
                case DELETE_ITEM: {
                }
                case RETURN_PAGE: {
                    if (page > 1) {
                        page--;
                    }
                }
                case NEXT_PAGE: {
                    if (fulledPage) {
                        page++;
                    }
                }
                case EXIT_PROGRAM: {
                    Screen = EXIT_PROGRAM;
                }
                default: {
                    // User sent an invalid number, should not change state because this could affect user experience
                }
            }
        }
    } while (run_program);
}


//todo: logica para pegar todos os items
// getAllItem(1);


//todo: logica para editar um item
//    // fpos_t initial_pos;
// Item item = findItemById(2, &initial_pos);
//
// printf("Id: %d | Name: %s | Quantity: %d | Price: %.2f", item.id, item.name, item.quantity, item.price);
// item.quantity = item.quantity + 10000;
//
// editLine(item, &initial_pos);
//

// Item n = {
//     10, "melancia", 10, 19.40f
// };
// createItem(n);
// // fpos_t initial_pos, final_pos;
// // Item item = findItemById(2, &initial_pos);
// // fgetpos(fp, &final_pos);
// // deleteItem(initial_pos, &final_pos);
//
// closeFile();
