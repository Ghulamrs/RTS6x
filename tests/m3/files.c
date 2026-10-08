/* Spec: ISO C 7.19.5-7.19.10 - streams over host files: fopen's modes, formatted and block output,
   reading back with fgetc, fgets, fread and fscanf, positioning, ungetc, the indicators, rename,
   remove, and tmpfile. */
#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE *f;
    char line[64];
    int c, n, values[4], back[4] = { 0, 0, 0, 0 };
    long where;
    const char *got;
    int u, g;

    f = fopen("rts6x-m3-a.txt", "w");
    if (!f) { printf("no file\n"); return 1; }
    fprintf(f, "first %d\n", 1);
    fputs("second line\n", f);
    fputc('3', f);
    fputc('\n', f);
    printf("close %d\n", fclose(f));

    f = fopen("rts6x-m3-a.txt", "r");
    printf("fgets [%s]", fgets(line, sizeof line, f));
    c = fgetc(f);
    u = ungetc('S', f);
    g = fgetc(f);
    printf("fgetc %c, ungetc %c, again %c\n", c, u, g);
    fgets(line, sizeof line, f);
    printf("rest [%s]", line);
    where = ftell(f);
    printf("ftell %ld\n", where);
    got = fgets(line, sizeof line, f) ? line : "";
    printf("fgets %s, feof %d\n", got, feof(f) != 0);
    got = fgets(line, sizeof line, f) ? "line" : "null";
    printf("fgets at end %s, feof %d, ferror %d\n", got, feof(f) != 0, ferror(f) != 0);
    rewind(f);
    u = feof(f) != 0;
    g = fscanf(f, "%s %d", line, &n);
    printf("after rewind feof %d, fscanf %d", u, g);
    printf(" -> %s %d\n", line, n);
    fseek(f, -2, SEEK_END);
    printf("from the end %c\n", fgetc(f));
    fseek(f, 6, SEEK_SET);
    g = fgetc(f);
    where = ftell(f);
    printf("seek 6: %c, ftell %ld\n", g, where);
    fclose(f);

    f = fopen("rts6x-m3-a.txt", "a");
    fputs("appended\n", f);
    fclose(f);
    f = fopen("rts6x-m3-a.txt", "r+");
    fseek(f, 0, SEEK_SET);
    fputs("FIRST", f);
    fseek(f, 0, SEEK_SET);
    n = 0;
    while (fgets(line, sizeof line, f)) printf("%d: %s", ++n, line);
    fclose(f);

    values[0] = 7; values[1] = -1; values[2] = 1 << 20; values[3] = 42;
    f = fopen("rts6x-m3-b.bin", "wb+");
    printf("fwrite %d\n", (int)fwrite(values, sizeof values[0], 4, f));
    rewind(f);
    printf("fread %d:", (int)fread(back, sizeof back[0], 4, f));
    for (n = 0; n < 4; n++) printf(" %d", back[n]);
    g = (int)fread(back, 1, 1, f);
    printf("\nfread at end %d, feof %d\n", g, feof(f) != 0);
    clearerr(f);
    printf("clearerr feof %d\n", feof(f) != 0);
    fclose(f);

    printf("rename %d\n", rename("rts6x-m3-b.bin", "rts6x-m3-c.bin"));
    printf("old gone %d\n", fopen("rts6x-m3-b.bin", "rb") == 0);
    printf("remove %d %d\n", remove("rts6x-m3-a.txt"), remove("rts6x-m3-c.bin"));
    printf("missing %d\n", fopen("rts6x-m3-a.txt", "r") == 0);

    f = tmpfile();
    fprintf(f, "%s %d", "temporary", 99);
    rewind(f);
    fscanf(f, "%s %d", line, &n);
    printf("tmpfile %s %d\n", line, n);
    fclose(f);
    printf("bad mode %d\n", fopen("rts6x-m3-a.txt", "q") == 0);
    return 0;
}
