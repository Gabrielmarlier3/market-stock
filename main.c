#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include<unistd.h>

//todo: Remove all the fucking scanf this shit is horrible
////todo: define a good project pad
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
static int closedFile = 0;
static int lastItemId = 0;

static int openFile() {
    if (!closedFile && fp != NULL) {
        fseek(fp, 0L, SEEK_SET);
        return 1;
    }

    fp = fopen(DB_PATH, "r+");

    if (fp == NULL) {
        printf("File didn't open correctly");
        return 0;
    }
    closedFile = 0;
    return 1;
}

static void closeFile() {
    if (fclose(fp) != 0) {
        printf("File didn't close correctly");
    }
    closedFile = 1;
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

static void getLastItemId() {
    openFile();

    fpos_t temp;
    fgetpos(fp, &temp);

    fseek(fp, -LINE_MAX_LEN,SEEK_END);

    char a[LINE_MAX_LEN + 1];

    fgets(a, LINE_MAX_LEN + 1, fp);
    ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

    char *token = strtok(a, DELIMITERS);

    Item n = processTokens(token);
    lastItemId = n.id;

    fsetpos(fp, &temp);
}

static Item findItemById(int id, fpos_t *initial_pos, fpos_t *end_pos) {
    openFile();
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[LINE_MAX_LEN + 1];
        if (initial_pos != NULL && fgetpos(fp, initial_pos) == 0) {
            ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
        }
        fgets(a, LINE_MAX_LEN + 1, fp);
        ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

        char *token = strtok(a, DELIMITERS);
        n = processTokens(token);
        if (lastId == n.id) {
            n.id = -2;
            closeFile();
            return n;
        }
        lastId = n.id;
        if (n.id == id) {
            break;
        }
    }

    if (end_pos != NULL && fgetpos(fp, end_pos) == 0) {
        ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
    }
    closeFile();
    return n;
}

static Item findItemByName(const char name[NAME_LEN + 1], fpos_t *initial_pos, fpos_t *end_pos) {
    openFile();
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[LINE_MAX_LEN + 1];
        if (initial_pos != NULL && fgetpos(fp, initial_pos) == 0) {
            ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
        }
        fgets(a, LINE_MAX_LEN + 1, fp);
        ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

        char *token = strtok(a, DELIMITERS);
        n = processTokens(token);

        if (lastId == n.id) {
            n.id = -2;
            closeFile();
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
    if (end_pos != NULL && fgetpos(fp, end_pos) == 0) {
        ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
    }

    closeFile();
    return n;
}

static void createItem(Item item) {
    openFile();
    fseek(fp, -LINE_MAX_LEN, SEEK_END);

    char a[LINE_MAX_LEN + 1];
    fgets(a, LINE_MAX_LEN + 1, fp);
    ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

    char newItem[LINE_MAX_LEN];
    getLastItemId();
    sprintf(newItem, "%d,%s,%d,%.2f", lastItemId + 1, item.name, item.quantity, item.price);

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
 * initial_pos: The beginner position of the file you want to delete
 * final_pos: The start of the next line
 */
static int deleteItem(const fpos_t initial_pos, const fpos_t *final_pos) {
    openFile();
    char tmpFileName[11] = "db-tmp.txt";
    FILE *fp2 = fopen(tmpFileName, "w+");

    if (fp2 == NULL) {
        printf("Temp file didn't open correctly");
        return 0;
    }

    fpos_t tmp_pos, tmp2_pos;
    fseek(fp, 0L, SEEK_SET);
    fgetpos(fp2, &tmp_pos);
    // Here I am at position 0 of the file, the idea is, copy everything here till the line we want exclude.
    int end = initial_pos.__pos;
    while (tmp_pos.__pos < end) {
        char a[LINE_MAX_LEN + 1];
        fgets(a, LINE_MAX_LEN + 1, fp);
        fputs(a, fp2);
        fgetpos(fp2, &tmp_pos);
        fflush(fp2);
    }


    fseek(fp, 0L, SEEK_END);
    fgetpos(fp, &tmp2_pos);

    end = tmp2_pos.__pos;
    fsetpos(fp, final_pos);

    // Now here, we skip the line we want exclude, and start copying everything till the end. That way we "exclude" the line
    while (tmp_pos.__pos < end - LINE_MAX_LEN) {
        char a[LINE_MAX_LEN + 1];
        fgets(a, LINE_MAX_LEN + 1, fp);
        fputs(a, fp2);
        fgetpos(fp2, &tmp_pos);
        fflush(fp2);
    }

    // Need this, because without the "db-tmp.txt" will be empty
    fflush(fp2);

    // deletes the original database
    if (remove(DB_PATH)) {
        perror("cannot remove database");
        closeFile();
        return 1;
    }
    // the pass the tmpFile to became the new database.
    if (rename(tmpFileName, DB_PATH)) {
        perror("cannot rename database");
        closeFile();
        return 1;
    }

    closeFile();
    return 0;
}

static void editLine(Item n, const fpos_t *initial_pos) {
    openFile();
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
    closeFile();
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

    int off = 0;
    for (int i = 0; i < MAX_ITEM_PER_PAGE; i++) {
        Item item = findItemById(startId + (i + 1) + off, NULL, NULL);

        if (item.id == -2) {
            getLastItemId();
            if (startId + (i + 1) < lastItemId) {
                i--;
                off++;
                continue;
            }
            ENABLE_DEBUG_LOGS && printf("Collected all the items");
            break;
        }
        itens[i] = item;
    }

    return *itens;
}

static void clean_screen() {
#ifdef _WIN32
    system("cls");
#else
    // system("clear");
    // for some reason "clear" was not working so i found this way.
    printf("\033[H\033[J");
#endif
}

//todo: this shit need validate what kind of returning the user sent just in case...
static void getEntry(char *buffer, int bufferSize) {
    int i = 0;

    for (int ch; (i < bufferSize) && ((ch = getc(stdin)) != EOF) && (ch != '\n'); ++i) {
        buffer[i] = ch;
    }

    buffer[i] = '\0'; /* a string should always end with '\0' ! */
}

static void printDefaultScreen(int page) {
    clean_screen();
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

static void printHomeScreen(int page) {
    printDefaultScreen(page);
};

static Item findItem(fpos_t *init_pos, fpos_t *end_pos) {
    Item item;
    do {
        int action =
                printf("What kind of method do you want use to find you item\n[1] ID [2] Name [3] Return - Option: ");
        scanf("%d", &action);

        if (action < 1 || action > 3) {
            printf("Invalid action, choose a number between 1 and 2\n");
            sleep(3);
            clean_screen();
            continue;
        }
        if (action == 3) {
            break;
        }

        if (action == 1) {
            int itemId = 0;
            printf("Item Id: ");
            scanf("%d", &itemId);
            getc(stdin); // just to clear the '/n' character

            item = findItemById(itemId, init_pos, end_pos);

            if (item.id == -2) {
                printf("Item not found, try again with other id ou name\n");
                sleep(3);
                continue;
            }
        } else {
            char itemName[NAME_LEN + 1];
            printf("Item Name: ");
            scanf("%s", itemName);
            getc(stdin); // just to clear the '/n' character
            item = findItemByName(itemName, init_pos, end_pos);
            if (item.id == -2) {
                printf("Item not found, try again with other id ou name\n");
                sleep(3);
                continue;
            }
        }
        return item;
    } while (1);
    item.id = -3;
    return item;
}

//sorry about the monstrosity but it works, so... let's keep it!
static void printEditSceen(int page, const Item *item) {
    do {
        clean_screen();
        printDefaultScreen(page);
        int bufferSize = NAME_LEN + 1;
        char buffer[bufferSize];
        fpos_t init_pos, end_pos;
        Item newItem, oldItem;
        if (item != NULL) {
            oldItem = *item;
        } else {
            oldItem = findItem(&init_pos, &end_pos);
        }
        if (oldItem.id == -3) {
            return;
        }
        clean_screen();
        newItem.id = oldItem.id;
        printf("Item found, please fill out the form. (leave blank to keep the original)\n");
        printf("Name (%s): ", oldItem.name);
        getEntry(buffer, bufferSize);
        int i = 0;
        if (buffer[0] == '\0') {
            for (; i < strlen(oldItem.name) && (oldItem.name[i] != '\n' || oldItem.name[i] != '\0'); i++) {
                newItem.name[i] = oldItem.name[i];
            }
        } else {
            for (; i < strlen(buffer) && (buffer[i] != '\n' || buffer[i] != '\0'); i++) {
                newItem.name[i] = buffer[i];
            }
        }
        newItem.name[i] = '\0';
        printf("Quantity (%d): ", oldItem.quantity);
        getEntry(buffer, bufferSize);
        if (buffer[0] == '\0') {
            newItem.quantity = oldItem.quantity;
        } else {
            char *remaining;
            newItem.quantity = strtol(buffer, &remaining, 10);
        }
        printf("Price (%.2f): ", oldItem.price);
        getEntry(buffer, bufferSize);
        if (buffer[0] == '\0') {
            newItem.price = oldItem.price;
        } else {
            char *remaining;
            newItem.price = strtof(buffer, &remaining);
        }

        printf("             id | quantity |  price  | name\n");
        printf("old item: %5d | %8d | %7.2f | %s \n", oldItem.id, oldItem.quantity, oldItem.price, oldItem.name);
        printf("new item: %5d | %8d | %7.2f | %s \n", newItem.id, newItem.quantity, newItem.price, newItem.name);

        int validResponse = 1;
        do {
            printf("You are sure about change the content \n[0] No \n[1] Yes \nAnswer: ");
            getEntry(buffer, bufferSize);
            if (buffer[0] != '\0') {
                char *remaining;
                int resp = strtol(buffer, &remaining, 10);
                if (resp < 0 || resp > 1) {
                    validResponse = 0;
                } else if (resp) {
                    if (newItem.quantity <= 0) {
                        deleteItem(init_pos, &end_pos);
                    } else {
                        editLine(newItem, &init_pos);
                    }
                } else {
                    break;
                }
            }
        } while (!validResponse);
        break;
    } while (1);
}

static void printSearchScreen(int page) {
    clean_screen();
    Item item = findItem(NULL, NULL);

    if (item.id == -3) {
        return;
    }
    do {
        int action;
        clean_screen();
        printf("         id | quantity |  price  | name\n");
        printf("item: %5d | %8d | %7.2f | %s \n", item.id, item.quantity, item.price, item.name);
        printf("[0] Return [1] Edit - Option: ");
        scanf("%d", &action);
        getc(stdin);

        if (action < 0 && action > 1) {
            printf("Invalid action, choose a number between 0 and 1");
            sleep(3);
            continue;
        }
        if (action) {
            printEditSceen(page, &item);
        }
        break;
    } while (1);
}

static void deleteItemScreen(int page) {
    clean_screen();
    fpos_t init_pos, end_pos;
    Item item = findItem(&init_pos, &end_pos);

    if (item.id == -3) {
        return;
    }
    do {
        int action;
        clean_screen();
        printf("         id | quantity |  price  | name\n");
        printf("item: %5d | %8d | %7.2f | %s \n", item.id, item.quantity, item.price, item.name);
        printf("Are you sure that you want to exclude this item, the action cannot be undone");
        printf("[0] No [1] Yes - Option: ");
        scanf("%d", &action);
        getc(stdin);

        if (action < 0 && action > 1) {
            printf("Invalid action, choose a number between 0 and 1");
            sleep(3);
            continue;
        }
        if (action) {
            deleteItem(init_pos, &end_pos);
        }
        break;
    } while (1);
    return;
}

int main(void) {
    if (!openFile(fp, DB_PATH)) {
        return EXIT_FAILURE;
    };

    int run_program = 1;
    int page = 1;

    do {
        switch (Screen) {
            case HOME_SCREEN: {
                printHomeScreen(page);
                break;
            }
            case EDIT_SCREEN: {
                printEditSceen(page, NULL);
                printHomeScreen(page);
                Screen = HOME_SCREEN;
                break;
            }
            case DELETE_SCREEN: {
                deleteItemScreen(page);
                printHomeScreen(page);
                Screen = HOME_SCREEN;
                break;
            }
            case SEARCH_SCREEN: {
                printSearchScreen(page);
                printHomeScreen(page);
                Screen = HOME_SCREEN;
                break;
            }
            case EXIT_SCREEN: {
                run_program = 0;
                break ;
            }
            default: printDefaultScreen(page);
        }
        if (run_program == 0) {
            continue;
        }
        printf(
            "[%d] Edit item | [%d] Find item | [%d] Delete item | [%d] Return Page | [%d] Next Page | [%d] Exit\nOption: ",
            EDIT_ITEM, FIND_ITEM, DELETE_ITEM, RETURN_PAGE, NEXT_PAGE, EXIT_PROGRAM);


        scanf("%d", &Actions);
        switch (Actions) {
            case EDIT_ITEM: {
                Screen = EDIT_SCREEN;
                break;
            }
            case FIND_ITEM: {
                Screen = SEARCH_SCREEN;
                break;
            }
            case DELETE_ITEM: {
                Screen = DELETE_SCREEN;
                break;
            }
            case RETURN_PAGE: {
                if (page > 1) {
                    page--;
                }
                break;
            }
            case NEXT_PAGE: {
                if (fulledPage) {
                    page++;
                }
                break;
            }
            case EXIT_PROGRAM: {
                Screen = EXIT_SCREEN;
                break;
            }
            default: {
                Screen = HOME_SCREEN;
                break;
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
