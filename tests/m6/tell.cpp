// Spec: ISO C 7.19.9.2 (fseek) and 7.19.9.4 (ftell) on a text stream: ftell's value, given back to fseek
// with SEEK_SET, returns the stream to the same character whatever the host does to the bytes - CCS 5.5's
// Windows host writes each LF as CR LF and answers lseek in bytes, its Linux host keeps the bytes.

#include <stdio.h>

static const char *name = "rts6x-m6-tell.txt";

static void show(int c)
{
    if (c == '\n') printf("\\n");
    else if (c == EOF) printf("EOF");
    else printf("%c", c);
}

static void list(FILE *f)
{
    char line[32];
    rewind(f);
    while (fgets(line, sizeof line, f)) printf("  %s", line);
}

int main(void)
{
    long at[16], p, q;
    int ch[16], n = 0, c, i, bad = 0;
    char line[32];
    FILE *f = fopen(name, "w");
    if (!f) { printf("no file\n"); return 1; }
    fputs("one\ntwo\nthree\n", f);
    printf("after writing: %ld\n", ftell(f));
    fclose(f);

    // ftell before each character, then back to each from the last to the first.
    f = fopen(name, "r");
    for (;;) {
        p = ftell(f);
        c = fgetc(f);
        printf("%ld:", p); show(c); printf(c == EOF ? "\n" : " ");
        if (c == EOF) break;
        at[n] = p; ch[n] = c; n++;
    }
    for (i = n - 1; i >= 0; i--) {
        fseek(f, at[i], SEEK_SET);
        c = fgetc(f);
        q = ftell(f);
        if (c != ch[i] || (i + 1 < n && q != at[i + 1])) { bad++; printf("back to %ld: ", at[i]); show(c); printf(", ftell %ld\n", q); }
    }
    printf("round trips that failed: %d of %d\n", bad, n);

    fseek(f, at[6], SEEK_SET);
    c = fgetc(f);
    printf("seek to the seventh: "); show(c); printf(", ftell %ld\n", ftell(f));
    fseek(f, 0, SEEK_SET);
    c = fgetc(f);
    ungetc('X', f);
    printf("after ungetc, at the start: %d\n", ftell(f) == 0);
    rewind(f);
    fgets(line, sizeof line, f);
    p = ftell(f);
    fseek(f, 0, SEEK_CUR);
    fgets(line, sizeof line, f);
    printf("after one line %ld, SEEK_CUR 0 then: %s", p, line);
    fseek(f, p, SEEK_SET);
    fgets(line, sizeof line, f);
    printf("back to %ld: %s", p, line);
    fclose(f);

    // An update stream: a line written where ftell said, and one written straight after a read.
    f = fopen(name, "r+");
    fgets(line, sizeof line, f);
    p = ftell(f);
    fseek(f, p, SEEK_SET);
    fputs("TWO\n", f);
    printf("update: wrote at %ld, now %ld\n", p, ftell(f));
    list(f);
    rewind(f);
    fgets(line, sizeof line, f);
    fputs("2wo\n", f);
    list(f);
    fclose(f);

    // Past one buffer: 100 lines of 8 characters.
    f = fopen(name, "w");
    for (i = 0; i < 100; i++) fprintf(f, "line %02d\n", i);
    fclose(f);
    f = fopen(name, "r");
    bad = 0;
    for (i = 0; i < 100; i++) {
        p = ftell(f);
        if (i % 7 == 3) at[i / 7] = p;
        fgets(line, sizeof line, f);
        if (i == 0 || i == 63 || i == 64 || i == 99) printf("line %d at %ld\n", i, p);
    }
    printf("at the end %ld\n", ftell(f));
    for (i = 3; i < 100; i += 7) {
        fseek(f, at[i / 7], SEEK_SET);
        fgets(line, sizeof line, f);
        if (line[5] - '0' != i / 10 || line[6] - '0' != i % 10) { bad++; printf("line %d: %s", i, line); }
    }
    printf("long file, round trips that failed: %d\n", bad);
    fseek(f, -9, SEEK_END);
    p = ftell(f);
    fgets(line, sizeof line, f);
    fseek(f, p, SEEK_SET);
    c = fgetc(f);
    printf("near the end: "); show(c); printf("\n");
    fclose(f);
    remove(name);
    return 0;
}
