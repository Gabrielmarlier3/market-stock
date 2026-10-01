#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
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
    CREATE_SCREEN,
    EXIT_SCREEN
} Screens;

static enum {
    EDIT_ITEM,
    FIND_ITEM,
    DELETE_ITEM,
    CREATE_ITEM,
    RETURN_PAGE,
    NEXT_PAGE,
    EXIT_PROGRAM
} Actions;


static Item g_itens[MAX_ITEM_PER_PAGE];
static int g_lastReadIndex = 0;
static int g_fulledPage = 0;
static int g_closedFile = 0;
static int g_lastItemId = 0;
static FILE *g_database;


static int OpenFile() {
    if (!g_closedFile && g_database != NULL) {
        fseek(g_database, 0L, SEEK_SET);
        return 1;
    }

    g_database = fopen(DB_PATH, "r+");

    if (g_database == NULL) {
        printf("File didn't open correctly");
        return 0;
    }
    g_closedFile = 0;
    return 1;
}

static void CloseFile() {
    fflush(g_database);
    if (fclose(g_database) != 0) {
        printf("File didn't close correctly");
    }
    g_closedFile = 1;
}

static Item ProcessTokens(char *p_token) {
    Item n;
    int i = 0;
    while (p_token != NULL && i < 4) {
        ENABLE_DEBUG_LOGS && printf("Token encontrado: %s\n", p_token);

        switch (i++) {
            case 0: {
                n.id = atoi(p_token);
                break;
            }
            case 1: {
                strcpy(n.name, p_token);
                break;
            }
            case 2: {
                n.quantity = atoi(p_token);
                break;
            }
            case 3: {
                n.price = atof(p_token);
                break;
            }
            default: {
            }
        }

        p_token = strtok(NULL, DELIMITERS);
    }
    return n;
}

static void GetLastItemId() {
    OpenFile();

    fpos_t temp;
    fgetpos(g_database, &temp);

    fseek(g_database, -LINE_MAX_LEN,SEEK_END);

    char a[LINE_MAX_LEN + 1];

    fgets(a, LINE_MAX_LEN + 1, g_database);
    ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

    char *p_token = strtok(a, DELIMITERS);

    Item n = ProcessTokens(p_token);
    g_lastItemId = n.id;

    fsetpos(g_database, &temp);
}

static Item FindItemById(int id, fpos_t *p_initialPos, fpos_t *p_endPos) {
    OpenFile();
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[LINE_MAX_LEN + 1];
        if (p_initialPos != NULL && fgetpos(g_database, p_initialPos) == 0) {
            ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
        }
        fgets(a, LINE_MAX_LEN + 1, g_database);
        ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

        char *p_token = strtok(a, DELIMITERS);
        n = ProcessTokens(p_token);
        if (lastId == n.id) {
            n.id = -2;
            CloseFile();
            return n;
        }
        lastId = n.id;
        if (n.id == id) {
            break;
        }
    }

    if (p_endPos != NULL && fgetpos(g_database, p_endPos) == 0) {
        ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
    }
    CloseFile();
    return n;
}

static Item FindItemByName(const char NAME[NAME_LEN + 1], fpos_t *p_initialPos, fpos_t *p_endPos) {
    OpenFile();
    Item n = {};

    int lastId = 0;
    while (1) {
        char a[LINE_MAX_LEN + 1];
        if (p_initialPos != NULL && fgetpos(g_database, p_initialPos) == 0) {
            ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
        }
        fgets(a, LINE_MAX_LEN + 1, g_database);
        ENABLE_DEBUG_LOGS && printf("o que tem: %s\n", a);

        char *p_token = strtok(a, DELIMITERS);
        n = ProcessTokens(p_token);

        if (lastId == n.id) {
            n.id = -2;
            CloseFile();
            return n;
        }
        lastId = n.id;

        //if dont match is not the same word
        if (strlen(n.name) != strlen(NAME)) {
            continue;
        }

        int isTheWord = 1;
        for (int i = 0; i < strlen(n.name); i++) {
            if (tolower(n.name[i]) != tolower(NAME[i])) {
                isTheWord = 0;
            };
        }
        // it's not the same
        if (!isTheWord) {
            continue;
        }

        break;
    }
    if (p_endPos != NULL && fgetpos(g_database, p_endPos) == 0) {
        ENABLE_DEBUG_LOGS && printf("Current position of file pointer found\n");
    }

    CloseFile();
    return n;
}

static void CreateItem(Item item) {
    OpenFile();
    GetLastItemId();
    fseek(g_database, 0L, SEEK_END);

    char newItem[LINE_MAX_LEN];
    sprintf(newItem, "%d,%s,%d,%.2f", g_lastItemId + 1, item.name, item.quantity, item.price);

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
    fputs(newItem, g_database);

    CloseFile();
}

/*
 * This thing, is that type of thing that you made and work, but you don't know how
 * initial_pos: The beginner position of the file you want to delete
 * final_pos: The start of the next line
 */
static int DeleteItem(const fpos_t INITIAL_POS, const fpos_t *p_FINAL_POS) {
    OpenFile();
    char tmpFileName[11] = "db-tmp.txt";
    FILE *p_fp2 = fopen(tmpFileName, "w+");

    if (p_fp2 == NULL) {
        printf("Temp file didn't open correctly");
        return 0;
    }

    fpos_t tmp_pos, tmp2_pos;
    fseek(g_database, 0L, SEEK_SET);
    fgetpos(p_fp2, &tmp_pos);
    // Here I am at position 0 of the file, the idea is, copy everything here till the line we want exclude.
    int end = INITIAL_POS.__pos;
    while (tmp_pos.__pos < end) {
        char a[LINE_MAX_LEN + 1];
        fgets(a, LINE_MAX_LEN + 1, g_database);
        fputs(a, p_fp2);
        fgetpos(p_fp2, &tmp_pos);
        fflush(p_fp2);
    }


    fseek(g_database, 0L, SEEK_END);
    fgetpos(g_database, &tmp2_pos);

    end = tmp2_pos.__pos;
    fsetpos(g_database, p_FINAL_POS);

    // Now here, we skip the line we want exclude, and start copying everything till the end. That way we "exclude" the line
    while (tmp_pos.__pos < end - LINE_MAX_LEN) {
        char a[LINE_MAX_LEN + 1];
        fgets(a, LINE_MAX_LEN + 1, g_database);
        fputs(a, p_fp2);
        fgetpos(p_fp2, &tmp_pos);
        fflush(p_fp2);
    }

    // Need this, because without the "db-tmp.txt" will be empty
    fflush(p_fp2);

    // deletes the original database
    if (remove(DB_PATH)) {
        perror("cannot remove database");
        CloseFile();
        return 1;
    }
    // the pass the tmpFile to became the new database.
    if (rename(tmpFileName, DB_PATH)) {
        perror("cannot rename database");
        CloseFile();
        return 1;
    }

    CloseFile();
    return 0;
}

static void EditLine(Item n, const fpos_t *p_INITIAL_POS) {
    OpenFile();
    fpos_t final_pos;
    if (fgetpos(g_database, &final_pos) == 0) {
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

    fsetpos(g_database, p_INITIAL_POS);
    strcat(newItem, ";\n");
    if (strlen(newItem) != 100) {
        printf("Line is longer or less than 100 digits: %lu", strlen(newItem));
    }
    fputs(newItem, g_database);
    CloseFile();
}

static void ClearPreviousItems() {
    for (int i = 0; i < MAX_ITEM_PER_PAGE; i++) {
        const Item ITEM = {};
        g_itens[i] = ITEM;
    }
}

static Item GetAllItem(int page) {
    if (g_lastReadIndex == page) {
        return *g_itens;
    }
    ClearPreviousItems();
    int startId = (MAX_ITEM_PER_PAGE * page) - MAX_ITEM_PER_PAGE;

    int off = 0;
    for (int i = 0; i < MAX_ITEM_PER_PAGE; i++) {
        Item item = FindItemById(startId + (i + 1) + off, NULL, NULL);

        if (item.id == -2) {
            GetLastItemId();
            if (startId + (i + 1) < g_lastItemId) {
                i--;
                off++;
                continue;
            }
            ENABLE_DEBUG_LOGS && printf("Collected all the items");
            break;
        }
        g_itens[i] = item;
    }

    return *g_itens;
}

static void CleanScreen() {
#ifdef _WIN32
    system("cls");
#else
    // system("clear");
    // for some reason "clear" was not working so i found this way.
    printf("\033[H\033[J");
#endif
}

static void GetEntry(char *p_buffer, int bufferSize) {
    int i = 0;

    for (int ch; (i < bufferSize) && ((ch = getc(stdin)) != EOF) && (ch != '\n'); ++i) {
        p_buffer[i] = ch;
    }

    p_buffer[i] = '\0'; /* a string should always end with '\0' ! */
}

// DEFAULT_VALUE: the default value of the 'yes or no' question example in 'y/N' the default value is 'N'
static int GetBoolean(const char DEFAULT_VALUE) {
    char tempBuffer[2];
    GetEntry(tempBuffer, 2);
    const int TEMP_LOWER = tolower(tempBuffer[0]);
    const int DEFAULT_LOWER = tolower(DEFAULT_VALUE);
    if (TEMP_LOWER == '\0' || TEMP_LOWER == DEFAULT_LOWER) {
        return 1;
    }

    if (TEMP_LOWER != 'y' && TEMP_LOWER != 'n') {
        return -1;
    }

    return 0;
}


static void PrintDefaultScreen(int page) {
    CleanScreen();
    printf("\t\t\tStock System v%s\n", SYS_VERSION);

    GetAllItem(page);
    printf("\t\t   id | Quantity | Price   | Name\n");
    g_fulledPage = 1;
    for (int i = 0; i < MAX_ITEM_PER_PAGE; i++) {
        Item item = g_itens[i];
        if (item.id == 0) {
            ENABLE_DEBUG_LOGS && printf("No items left");
            g_fulledPage = 0;

            break;
        }
        printf("\t\t%5d | %8d | %7.2f | %s \n", item.id, item.quantity, item.price, item.name);
    }
}

static void PrintHomeScreen(int page) {
    PrintDefaultScreen(page);
};

static Item FindItem(fpos_t *p_initPos, fpos_t *p_endPos, const char PREFIX[NAME_LEN]) {
    Item item;
    do {
        CleanScreen();
        int action =
                printf(
                    "%sWhat kind of method do you want use to find you item\n[1] ID [2] Name [3] Return - Option: ",
                    PREFIX);
        scanf("%d", &action);
        getc(stdin);

        if (action < 1 || action > 3) {
            printf("Invalid action, choose a number between 1 and 2\n");
            sleep(1);
            CleanScreen();
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

            item = FindItemById(itemId, p_initPos, p_endPos);

            if (item.id == -2) {
                printf("Item not found, try again with other id ou name\n");
                sleep(1);
                continue;
            }
        } else {
            char itemName[NAME_LEN + 1];
            printf("Item Name: ");
            scanf("%s", itemName);
            getc(stdin); // just to clear the '/n' character
            item = FindItemByName(itemName, p_initPos, p_endPos);
            if (item.id == -2) {
                printf("Item not found, try again with other id ou name\n");
                sleep(1);
                continue;
            }
        }
        return item;
    } while (1);
    item.id = -3;
    return item;
}

//sorry about the monstrosity but it works, so... let's keep it!
static void PrintEditSceen(int page, const Item *p_ITEM) {
    do {
        CleanScreen();
        PrintDefaultScreen(page);
        int bufferSize = NAME_LEN + 1;
        char buffer[bufferSize];
        fpos_t init_pos, end_pos;
        Item newItem, oldItem;
        if (p_ITEM != NULL) {
            oldItem = *p_ITEM;
        } else {
            oldItem = FindItem(&init_pos, &end_pos, "");
        }
        if (oldItem.id == -3) {
            return;
        }
        CleanScreen();
        newItem.id = oldItem.id;
        printf("Item found, please fill out the form. (leave blank to keep the original)\n");
        printf("Name (%s): ", oldItem.name);
        GetEntry(buffer, bufferSize);
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
        GetEntry(buffer, bufferSize);
        if (buffer[0] == '\0') {
            newItem.quantity = oldItem.quantity;
        } else {
            char *p_remaining;
            newItem.quantity = strtol(buffer, &p_remaining, 10);
        }
        printf("Price (%.2f): ", oldItem.price);
        GetEntry(buffer, bufferSize);
        if (buffer[0] == '\0') {
            newItem.price = oldItem.price;
        } else {
            char *p_remaining;
            newItem.price = strtof(buffer, &p_remaining);
        }

        printf("             id | quantity |  price  | name\n");
        printf("old item: %5d | %8d | %7.2f | %s \n", oldItem.id, oldItem.quantity, oldItem.price, oldItem.name);
        printf("new item: %5d | %8d | %7.2f | %s \n", newItem.id, newItem.quantity, newItem.price, newItem.name);

        int validResponse = 1;
        //todo: mordenize this selection
        do {
            printf("You are sure about change the content \n[0] No \n[1] Yes \nAnswer: ");
            GetEntry(buffer, bufferSize);
            if (buffer[0] != '\0') {
                char *p_remaining;
                int resp = strtol(buffer, &p_remaining, 10);
                if (resp < 0 || resp > 1) {
                    validResponse = 0;
                } else if (resp) {
                    if (newItem.quantity <= 0) {
                        printf("This item has 0 or less so will be excluded. Proceed? n/Y: ");
                        if (!GetBoolean('y')) {
                            DeleteItem(init_pos, &end_pos);
                        }
                    } else {
                        EditLine(newItem, &init_pos);
                    }
                } else {
                    break;
                }
            }
        } while (!validResponse);
        break;
    } while (1);
}

static void PrintSearchScreen(int page) {
    CleanScreen();
    Item item = FindItem(NULL, NULL, "");

    if (item.id == -3) {
        return;
    }
    do {
        int action;
        CleanScreen();
        printf("         id | quantity |  price  | name\n");
        printf("item: %5d | %8d | %7.2f | %s \n", item.id, item.quantity, item.price, item.name);
        printf("[0] Return [1] Edit - Option: ");
        scanf("%d", &action);
        getc(stdin);

        if (action < 0 && action > 1) {
            printf("Invalid action, choose a number between 0 and 1");
            sleep(1);
            continue;
        }
        if (action) {
            PrintEditSceen(page, &item);
        }
        break;
    } while (1);
}

static void DeleteItemScreen() {
    CleanScreen();
    fpos_t init_pos, end_pos;
    printf("Deleting a Item - ");
    Item item = FindItem(&init_pos, &end_pos, "DELETING - ");

    if (item.id == -3) {
        return;
    }
    do {
        int action;
        CleanScreen();
        printf("         id | quantity |  price  | name\n");
        printf("item: %5d | %8d | %7.2f | %s \n", item.id, item.quantity, item.price, item.name);
        printf("Are you sure that you want to exclude this item y/N: ");
        if (!GetBoolean('n')) {
            DeleteItem(init_pos, &end_pos);
        }
        break;
    } while (1);
}

static void CreateItemScreen() {
    GetLastItemId();
    CleanScreen();
    Item newItem = {
        .id = g_lastItemId++,
    };
    int bufferSize = NAME_LEN + 1, state = 0, skipForm = 0;
    char buffer[bufferSize];
    char tempBuffer[2];
    printf("Creating a new item\n");
    do {
        switch (state) {
            //Name
            case 0: {
                printf("Name: ");
                GetEntry(buffer, bufferSize);
                int i = 0;
                if (buffer[0] == '\0') {
                    printf("No valid value provide, want exit? n/Y");
                    GetEntry(tempBuffer, 2);
                    if (tempBuffer[0] == '\0' || (tolower(tempBuffer[0]) == 'y' && tolower(tempBuffer[0]) != 'n')) {
                        return;
                    }
                    continue;
                }
                for (; i < strlen(buffer) && (buffer[i] != '\n' || buffer[i] != '\0'); i++) {
                    newItem.name[i] = buffer[i];
                }
                newItem.name[i] = '\0';
                state++;
                if (skipForm) { state = INT_MAX; }
                break;
            };
            //Quantity
            case 1: {
                printf("Quantity: ");
                GetEntry(buffer, bufferSize);
                if (buffer[0] == '\0') {
                    printf("No valid value provide, want exit? n/Y");
                    GetEntry(tempBuffer, 2);
                    if (tempBuffer[0] == '\0' || (tolower(tempBuffer[0]) == 'y' && tolower(tempBuffer[0]) != 'n')) {
                        return;
                    }
                    continue;
                }
                char *p_remaining;
                newItem.quantity = strtol(buffer, &p_remaining, 10);
                if (newItem.quantity <= 0) {
                    printf("Invalid quantity (%d), please try again\n", newItem.quantity);
                    sleep(1);
                    continue;
                }
                state++;
                if (skipForm) { state = INT_MAX; }
                break;
            }
            //Price
            case 2: {
                printf("Price: ");
                GetEntry(buffer, bufferSize);
                if (buffer[0] == '\0') {
                    printf("No valid value provide, want exit? n/Y");
                    GetEntry(tempBuffer, 2);
                    if (tempBuffer[0] == '\0' || (tolower(tempBuffer[0]) == 'y' && tolower(tempBuffer[0]) != 'n')) {
                        return;
                    }
                    continue;
                }
                char *p_remaining;
                newItem.price = strtof(buffer, &p_remaining);
                if (newItem.price <= 0) {
                    printf("ATTENTION: the price is zero or negative (%.2f), this is intentional y/N?\n",
                           newItem.price);
                    if (GetBoolean('n')) {
                        continue;
                    }
                }
                state++;
                if (skipForm) { state = INT_MAX; }

                break;
            }
        }
        if (state >= 3) {
            CleanScreen();
            printf("         id | quantity |  price  | name\n");
            printf("item: %5d | %8d | %7.2f | %s \n", newItem.id, newItem.quantity, newItem.price, newItem.name);
            printf("Want change some item? y/N?: ");

            if (!GetBoolean('n')) {
                printf("What value do you want to change? [0] Name [1] Quantity [2] Price \nOption: ");
                GetEntry(tempBuffer, 2);
                switch (tempBuffer[0]) {
                    case '\0': {
                        continue;
                    };
                    case '0': {
                        state = 0;
                        skipForm = 1;
                        break;
                    }
                    case '1': {
                        state = 1;
                        skipForm = 1;
                        break;
                    }
                    case '2': {
                        state = 2;
                        skipForm = 1;
                        break;
                    }
                }
                continue;
            }
            skipForm = 0;
            printf("Do you want to save it to the database? n/Y:");
            Item item = FindItemByName(newItem.name, NULL, NULL);
            if (GetBoolean('y')) {
                if (item.id != -2) {
                    printf("Already have a item with this name. Change it!\n");
                    sleep(2);
                    continue;
                }
                CreateItem(newItem);
            }

            return;
        }
    } while (1);

    GetLastItemId();
}

int main(void) {
    if (!OpenFile(g_database, DB_PATH)) {
        return EXIT_FAILURE;
    };

    int run_program = 1;
    int page = 1;

    do {
        switch (Screens) {
            case HOME_SCREEN: {
                PrintHomeScreen(page);
                break;
            }
            case EDIT_SCREEN: {
                PrintEditSceen(page, NULL);
                PrintHomeScreen(page);
                Screens = HOME_SCREEN;
                break;
            }
            case DELETE_SCREEN: {
                DeleteItemScreen();
                PrintHomeScreen(page);
                Screens = HOME_SCREEN;
                break;
            }
            case CREATE_SCREEN: {
                CreateItemScreen();
                PrintHomeScreen(page);
                Screens = HOME_SCREEN;
                break;
            }
            case SEARCH_SCREEN: {
                PrintSearchScreen(page);
                PrintHomeScreen(page);
                Screens = HOME_SCREEN;
                break;
            }
            case EXIT_SCREEN: {
                run_program = 0;
                break ;
            }
            default: PrintDefaultScreen(page);
        }
        if (run_program == 0) {
            continue;
        }
        printf(
            "[%d] Edit item | [%d] Find item | [%d] Delete item | [%d] Create item | [%d] Return Page | [%d] Next Page | [%d] Exit\nOption: ",
            EDIT_ITEM, FIND_ITEM, DELETE_ITEM, CREATE_ITEM, RETURN_PAGE, NEXT_PAGE, EXIT_PROGRAM);


        scanf("%d", &Actions);
        getc(stdin);
        switch (Actions) {
            case EDIT_ITEM: {
                Screens = EDIT_SCREEN;
                break;
            }
            case FIND_ITEM: {
                Screens = SEARCH_SCREEN;
                break;
            }
            case DELETE_ITEM: {
                Screens = DELETE_SCREEN;
                break;
            }
            case CREATE_ITEM: {
                Screens = CREATE_SCREEN;
                break;
            }
            case RETURN_PAGE: {
                if (page > 1) {
                    page--;
                }
                break;
            }
            case NEXT_PAGE: {
                if (g_fulledPage) {
                    page++;
                }
                break;
            }
            case EXIT_PROGRAM: {
                Screens = EXIT_SCREEN;
                break;
            }
            default: {
                Screens = HOME_SCREEN;
                break;
            }
        }
    } while (run_program);
}
