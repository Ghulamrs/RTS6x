// Spec: SPRAB89B 8.3 Table 8-9 - each register-limited helper, called through the probe with every
// register patterned, may leave changed only the registers its row lists (A4 carries its result;
// B3 holds the return and B15 is the stack, neither patterned). Prints each register out of place.
#include <stdio.h>

extern "C" {
void rts6x_probe(void *helper, int x, int y, unsigned *out);
int __c6xabi_divi(int, int);
unsigned __c6xabi_divu(unsigned, unsigned);
int __c6xabi_remi(int, int);
unsigned __c6xabi_remu(unsigned, unsigned);
}

namespace {

class Row {
public:
    Row(const char *name, void *helper, const char *allowed) : name_(name), helper_(helper), allowed_(allowed) {}
    // How many registers the helper changed that its row does not list, each named as found.
    int check(int x, int y) const
    {
        unsigned out[64];
        rts6x_probe(helper_, x, y, out);
        int wrong = 0;
        for (int r = 0; r < 64; r++) {
            if (r == 4 || r == 32 + 3 || r == 32 + 15 || out[r] == 0xA5000000u + (unsigned)r || listed(r)) continue;
            printf("%s(%d, %d): %c%d changed, and Table 8-9 does not allow it\n", name_, x, y, r < 32 ? 'A' : 'B', r % 32);
            wrong++;
        }
        return wrong;
    }

private:
    // Whether the row names register r: "A0 A1 B4 ..." against 'A' or 'B' and its number.
    bool listed(int r) const
    {
        char side = r < 32 ? 'A' : 'B';
        int number = r % 32;
        for (const char *p = allowed_; *p; ) {
            char s = *p++;
            int n = 0;
            while (*p >= '0' && *p <= '9') n = n * 10 + (*p++ - '0');
            if (s == side && n == number) return true;
            while (*p == ' ') p++;
        }
        return false;
    }

    const char *name_;
    void *helper_;
    const char *allowed_;
};

}  // namespace

int main()
{
    // Four objects and not an array of temporaries: cpp11 crashed on that shape (2026-10-06).
    const Row divi("divi", (void *)__c6xabi_divi, "A0 A1 A2 A4 A6 B0 B1 B2 B4 B5 B30 B31");
    const Row divu("divu", (void *)__c6xabi_divu, "A0 A1 A2 A4 A6 B0 B1 B2 B4 B30 B31");
    const Row remi("remi", (void *)__c6xabi_remi, "A1 A2 A4 A5 A6 B0 B1 B2 B4 B30 B31");
    const Row remu("remu", (void *)__c6xabi_remu, "A1 A4 A5 A7 B0 B1 B2 B4 B30 B31");
    const Row *rows[] = { &divi, &divu, &remi, &remu };
    const int operands[][2] = { { 100, 7 }, { -100, 7 }, { 100, -7 }, { -2147483647 - 1, -1 }, { 0, 5 }, { 2147483647, 2 } };
    int wrong = 0;
    for (int i = 0; i < 4; i++)
        for (int k = 0; k < 6; k++) wrong += rows[i]->check(operands[k][0], operands[k][1]);
    printf("%d register(s) changed outside Table 8-9\n", wrong);
    return wrong;
}
