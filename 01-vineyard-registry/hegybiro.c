#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "szoleszet.txt"
#define TEMP_FILE "temp.txt"
#define MAX_LINE 256

typedef struct {
    int id; char region[50]; char plot[50]; char variety[50]; int area; int damage;
}Record;


void strip_newline_from_file_end(char *str) {
    str[strcspn(str, "\r\n")] = 0;
}

int find_next_id() {
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) return 1;

    char line[MAX_LINE];
    int max_id = 0;
    while (fgets(line, sizeof(line), f)) {
        int current_id = atoi(strtok(line, "|"));
        if (current_id > max_id) max_id = current_id;
    }
    fclose(f);
    return max_id + 1;
}

void add_record() {
    FILE *f = fopen(FILE_NAME, "a");
    if (!f) { perror("Hiba a fajl megnyitasakor"); return; }

    Record r;
    r.id = find_next_id();

    printf("\nUj adat felvetele \n");
    printf("Termohely: "); fgets(r.region, 50, stdin); strip_newline_from_file_end(r.region);
    printf("Tabla: "); fgets(r.plot, 50, stdin); strip_newline_from_file_end(r.plot);
    printf("Fajta: "); fgets(r.variety, 50, stdin); strip_newline_from_file_end(r.variety);
    printf("Terulet: "); scanf("%d", &r.area);
    printf("Pusztitas: "); scanf("%d", &r.damage);

    fprintf(f, "%d|%s|%s|%s|%d|%d\n", r.id, r.region, r.plot, r.variety, r.area, r.damage);
    fclose(f);
    printf("Adat sikeresen mentve! (ID: %d)\n", r.id);
}


void list_all() {
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) { printf("Nincsenek meg adatok.\n"); return; }

    char line[MAX_LINE];
    printf("\n  =======================Teljes nyilvantartas \n");
    printf("%-5s | %-15s | %-15s | %-15s | %-10s | %-5s\n", "ID", "Termohely", "Tabla", "Fajta", "Terulet", "Pusztitas");

    while (fgets(line, sizeof(line), f)) {
        strip_newline_from_file_end(line);
        char *id = strtok(line, "|");
        char *reg = strtok(NULL, "|");
        char *plot = strtok(NULL, "|");
        char *var = strtok(NULL, "|");
        char *area = strtok(NULL, "|");
        char *dam = strtok(NULL, "|");

        if (id && reg && plot && var && area && dam) {
            printf("%-5s | %-15s | %-15s | %-15s | %-10s | %s%%\n", id, reg, plot, var, area, dam);
        }
    }
    fclose(f);
}


void list_filtered() {
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) { printf("Nincsenek meg adatok.\n"); return; }

    int filter_type;
    char keyword[50];

    printf("\nSzuresi szempont (1 - Termohely, 2 - Szolo tipus): ");
    scanf("%d", &filter_type);
    while(getchar() != '\n');

    printf("Keresendo szo: ");
    fgets(keyword, 50, stdin);
    strip_newline_from_file_end(keyword);

    char line[MAX_LINE], line_copy[MAX_LINE];
    printf("\nSzurt eredmenyek \n");

    while (fgets(line, sizeof(line), f)) {
        strcpy(line_copy, line);
        strip_newline_from_file_end(line_copy);

        char *id = strtok(line_copy, "|");
        char *reg = strtok(NULL, "|");
        char *plot = strtok(NULL, "|");
        char *var = strtok(NULL, "|");
        char *area = strtok(NULL, "|");
        char *dam = strtok(NULL, "|");

        if (!id || !reg || !plot || !var || !area || !dam) continue;

        if ((filter_type == 1 && strstr(reg, keyword)) ||
            (filter_type == 2 && strstr(var, keyword))) {
            printf("[%s] %s, %s, %s, %s n.ol, %s%%\n", id, reg, plot, var, area, dam);
        }
    }
    fclose(f);
}

void modify_or_delete(int is_delete) {
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) { printf("Nincsenek meg adatok.\n"); return; }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) { perror("Hiba a temporalis fajl letrehozasakor"); fclose(f); return; }

    int target_id, found = 0;
    printf("\nAdd meg a kezelendo bejegyzes ID-jat: ");
    scanf("%d", &target_id);
    while(getchar() != '\n');

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        char line_copy[MAX_LINE];
        strcpy(line_copy, line);
        int current_id = atoi(strtok(line_copy, "|"));

        if (current_id == target_id) {
            found = 1;
            if (is_delete) {
                printf("Adat torolve!\n");
                continue;
            } else {
                Record r;
                r.id = target_id;
                printf("Uj Termohely: "); fgets(r.region, 50, stdin); strip_newline_from_file_end(r.region);
                printf("Uj Tabla: "); fgets(r.plot, 50, stdin); strip_newline_from_file_end(r.plot);
                printf("Uj Fajta: "); fgets(r.variety, 50, stdin); strip_newline_from_file_end(r.variety);
                printf("Uj Terulet: "); scanf("%d", &r.area);
                printf("Uj Pusztitas (szazalek): "); scanf("%d", &r.damage);
                fprintf(temp, "%d|%s|%s|%s|%d|%d\n", r.id, r.region, r.plot, r.variety, r.area, r.damage);
                printf("Adat modositva!\n");
                continue;
            }
        }
        fprintf(temp, "%s", line);
    }

    fclose(f);
    fclose(temp);

    if (found) {
        remove(FILE_NAME);
        rename(TEMP_FILE, FILE_NAME);
    } else {
        printf("Nem talalhato ilyen ID-val adat.\n");
        remove(TEMP_FILE);
    }
}

int main() {
    int choice;
    do {
        printf("\n=== Oprendszer Hegyvidek Nyilvantarto ===\n");
        printf("1. Uj adat felvetele\n");
        printf("2. Teljes lista megtekintese\n");
        printf("3. Lista szurese (termohely/fajta)\n");
        printf("4. Adat modositasa\n");
        printf("5. Adat torlese\n");
        printf("0. Kilepes\n");
        printf("Valassz egy opciot: ");

        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            printf("Kerlek szamot adj meg!\n");
            continue;
        }
        while(getchar() != '\n');

        switch (choice) {
            case 1: add_record(); break;
            case 2: list_all(); break;
            case 3: list_filtered(); break;
            case 4: modify_or_delete(0); break;
            case 5: modify_or_delete(1); break;
            case 0: printf("Kilepes... Jo munkat a szoloben!\n"); break;
            default: printf("Ervenytelen valasztas!\n");
        }
    } while (choice != 0);

    return 0;
}