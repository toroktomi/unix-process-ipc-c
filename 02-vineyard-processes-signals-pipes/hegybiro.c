#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define FILE_NAME "szoleszet.txt"
#define TEMP_FILE "temp.txt"
#define MAX_LINE 256
#define THRESHOLD 30

typedef struct {
    int id; char region[50]; char plot[50   ]; char variety[50]; int area; int damage;
}Record;

volatile sig_atomic_t sigusr1_count = 0;
volatile sig_atomic_t sigusr2_count = 0;

void handler_usr1(int sig) { sigusr1_count++; }
void handler_usr2(int sig) { sigusr2_count++; }


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
    Record r;
    r.id = find_next_id();

    printf("\nUj adat felvetele\n");
    printf("Termohely: "); fgets(r.region, 50, stdin); strip_newline_from_file_end(r.region);
    printf("Tabla: "); fgets(r.plot, 50, stdin); strip_newline_from_file_end(r.plot);
    printf("Fajta: "); fgets(r.variety, 50, stdin); strip_newline_from_file_end(r.variety);

    printf("Terulet (n.ol): ");
    if (scanf("%d", &r.area) != 1 || r.area <= 0) {
        while (getchar() != '\n');
        printf("Ervenytelen ertek!\n");
        return;
    }

    printf("Pusztitas (0-100%%): ");
    if (scanf("%d", &r.damage) != 1 || r.damage < 0 || r.damage > 100) {
        while (getchar() != '\n');
        printf("Ervenytelen ertek!\n");
        return;
    }

    FILE *f = fopen(FILE_NAME, "a");
    if (!f) { perror("Hiba a fajl megnyitasakor"); return; }
    fprintf(f, "%d|%s|%s|%s|%d|%d\n", r.id, r.region, r.plot, r.variety, r.area, r.damage);
    fclose(f);
    printf("Adat mentve (ID: %d)\n", r.id);
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

                printf("Uj Terulet (n.ol): ");
                if (scanf("%d", &r.area) != 1 || r.area <= 0) {
                    while (getchar() != '\n');
                    printf("Ervenytelen ertek, modositas megszakitva.\n");
                    fprintf(temp, "%s", line);
                    continue;
                }

                printf("Uj Pusztitas (0-100%%): ");
                if (scanf("%d", &r.damage) != 1 || r.damage < 0 || r.damage > 100) {
                    while (getchar() != '\n');
                    printf("Ervenytelen ertek, modositas megszakitva.\n");
                    fprintf(temp, "%s", line);
                    continue;
                }

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

void delete_by_ids(int *ids, int count) {
    if (count == 0) return;
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) return;
    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) { fclose(f); return; }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        char lc[MAX_LINE];
        strcpy(lc, line);
        int id = atoi(strtok(lc, "|"));
        int skip = 0, j;
        for (j = 0; j < count; j++)
            if (ids[j] == id) { skip = 1; break; }
        if (!skip) fprintf(temp, "%s", line);
    }
    fclose(f);
    fclose(temp);
    remove(FILE_NAME);
    rename(TEMP_FILE, FILE_NAME);
}

void megsemmisito_folyamat(int pipe_fd, pid_t parent_pid) {
    kill(parent_pid, SIGUSR1);

    int count, i;
    read(pipe_fd, &count, sizeof(int));
    Record records[256];
    for (i = 0; i < count; i++)
        read(pipe_fd, &records[i], sizeof(Record));
    close(pipe_fd);

    printf("\n--- Megsemmisitendo tablak listaja ---\n");
    for (i = 0; i < count; i++)
        printf("[%d] %s, %s, %s, %d n.ol, %d%%\n",
            records[i].id, records[i].region, records[i].plot,
            records[i].variety, records[i].area, records[i].damage);

    printf("\n--- Megsemmisitesi utasitasok ---\n");
    for (i = 0; i < count; i++)
        printf("%s, %s (%s): azonnali megsemmisites elrendelve.\n",
               records[i].region, records[i].plot, records[i].variety);

    printf("MEGSEMMISITVE.\n");
    kill(parent_pid, SIGUSR1);
    exit(0);
}

void vedekezo_folyamat(int pipe_fd, pid_t parent_pid) {
    kill(parent_pid, SIGUSR2);

    int count, i;
    read(pipe_fd, &count, sizeof(int));
    Record records[256];
    for (i = 0; i < count; i++)
        read(pipe_fd, &records[i], sizeof(Record));
    close(pipe_fd);

    printf("\n--- Vegyszeresen kezelendo tablak ---\n");
    for (i = 0; i < count; i++)
        printf("%s, %s (%s): permetezest ne inditson, orszagos vedekezes indul.\n",
               records[i].region, records[i].plot, records[i].variety);

    printf("Vedekezes befejezve.\n");
    kill(parent_pid, SIGUSR2);
    exit(0);
}

void vegrehajtas() {
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) { printf("Nincsenek adatok.\n"); return; }

    Record megsemmisit[256], vedekez[256];
    int m_count = 0, v_count = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), f)) {
        char lc[MAX_LINE];
        strcpy(lc, line);
        strip_newline_from_file_end(lc);
        char *id_s = strtok(lc, "|"), *reg = strtok(NULL, "|"),
             *plot  = strtok(NULL, "|"), *var = strtok(NULL, "|"),
             *area_s = strtok(NULL, "|"), *dam_s = strtok(NULL, "|");
        if (!id_s || !reg || !plot || !var || !area_s || !dam_s) continue;

        Record r;
        r.id = atoi(id_s);
        strncpy(r.region,  reg,  49); r.region[49]  = 0;
        strncpy(r.plot,    plot, 49); r.plot[49]    = 0;
        strncpy(r.variety, var,  49); r.variety[49] = 0;
        r.area   = atoi(area_s);
        r.damage = atoi(dam_s);

        if (r.damage >= THRESHOLD) megsemmisit[m_count++] = r;
        else                       vedekez[v_count++]     = r;
    }
    fclose(f);

    int pipe_m[2], pipe_v[2];
    pipe(pipe_m); pipe(pipe_v);

    signal(SIGUSR1, handler_usr1);
    signal(SIGUSR2, handler_usr2);
    sigusr1_count = 0;
    sigusr2_count = 0;

    sigset_t block_mask, wait_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigaddset(&block_mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &block_mask, &wait_mask);

    pid_t parent_pid = getpid();
    int i;

    pid_t pid_m = fork();
    if (pid_m == 0) {
        close(pipe_m[1]);
        close(pipe_v[0]); close(pipe_v[1]);
        megsemmisito_folyamat(pipe_m[0], parent_pid);
    }

    pid_t pid_v = fork();
    if (pid_v == 0) {
        close(pipe_v[1]);
        close(pipe_m[0]); close(pipe_m[1]);
        vedekezo_folyamat(pipe_v[0], parent_pid);
    }

    close(pipe_m[0]);
    close(pipe_v[0]);

    while (sigusr1_count < 1 || sigusr2_count < 1) sigsuspend(&wait_mask);

    write(pipe_m[1], &m_count, sizeof(int));
    for (i = 0; i < m_count; i++) write(pipe_m[1], &megsemmisit[i], sizeof(Record));
    close(pipe_m[1]);

    write(pipe_v[1], &v_count, sizeof(int));
    for (i = 0; i < v_count; i++) write(pipe_v[1], &vedekez[i], sizeof(Record));
    close(pipe_v[1]);

    while (sigusr1_count < 2 || sigusr2_count < 2) sigsuspend(&wait_mask);

    waitpid(pid_m, NULL, 0);
    waitpid(pid_v, NULL, 0);

    sigprocmask(SIG_SETMASK, &wait_mask, NULL);

    int del_ids[256];
    for (i = 0; i < m_count; i++) del_ids[i] = megsemmisit[i].id;
    delete_by_ids(del_ids, m_count);

    printf("\nJelentes a mezogazdasagi miniszternek:\n");
    printf("  Megsemmisitett: %d tabla\n", m_count);
    printf("  Vegyszeresen kezelt: %d tabla\n", v_count);
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
        printf("6. Vedekezes inditasa (hatarszazalek: %d%%)\n", THRESHOLD);
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
            case 6: vegrehajtas(); break;
            case 0: printf("Kilepes...\n"); break;
            default: printf("Ervenytelen valasztas!\n");
        }
    } while (choice != 0);

    return 0;
}
