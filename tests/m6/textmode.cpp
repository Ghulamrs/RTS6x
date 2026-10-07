// Spec: ISO C 7.19.2 (text and binary streams) and 7.19.9.4 (ftell): a text file reads back as it was
// written whatever its bytes on the host - CCS 5.5's Windows host writes each LF as CR LF, its Linux
// host as LF - and a binary file is its bytes; ftell at the end of a text file is its size on disk.

#include <stdio.h>
#include <string.h>

static const char *name = "rts6x-m6-text.txt";

// The bytes on disk, through a binary stream.
static int disk(char *b, int room)
{
    FILE *f = fopen(name, "rb");
    if (!f) return -1;
    int n = (int)fread(b, 1, room, f);
    fclose(f);
    return n;
}

int main(void)
{
    char line[32], b[64];
    FILE *f = fopen(name, "w");
    if (!f) { printf("no file\n"); return 1; }
    fputs("one\ntwo\n", f);
    fclose(f);
    f = fopen(name, "r");
    while (fgets(line, sizeof line, f)) printf("text: %s", line);
    printf("end %d\n", feof(f) != 0);
    fclose(f);
    // Every LF has a CR before it, or none has; taking those CRs away leaves the text.
    int n = disk(b, sizeof b), crs = 0, k = 0;
    for (int i = 0; i < n; i++) {
        if (b[i] == '\r' && i + 1 < n && b[i + 1] == '\n') { crs++; continue; }
        b[k++] = b[i];
    }
    printf("disk is the text: %d, CR LF all or none: %d\n", k == 8 && memcmp(b, "one\ntwo\n", 8) == 0, crs == 0 || crs == 2);
    f = fopen(name, "r");
    fseek(f, 0, SEEK_END);
    printf("ftell at the end is the size on disk: %d\n", ftell(f) == (long)n);
    fclose(f);
    f = fopen(name, "wb");
    fputs("x\ny\n", f);
    fclose(f);
    printf("binary on disk: %d bytes\n", disk(b, sizeof b));
    remove(name);
    return 0;
}
