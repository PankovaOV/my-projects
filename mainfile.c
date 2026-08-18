#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_BYTES     32
#define MAX_ROUNDS    32
#define MAX_ITEMS   256

typedef struct {
    uint8_t* bits;
    size_t num_bits;
} BitVector;

typedef struct {
    int r;              // число раундов
    int n;              // длина блока в битах
    int k;              // длина ключа в битах

    int S[8][64];       // S боксы
    int S_size[8];

    int E[64];          // перестановка с расширением
    int E_len;

    int P1[64];         // перестановка P1
    int P1_len;
    int P2[64];         // перестановка P2
    int P2_len;

    int I[32];          // раундовые константы
    int I_len;
} Config;

typedef struct {
    char op;            // тип операции
    int val;
    int n0, n1;
    char name[8];
} Item;

typedef struct {
    Item Items[MAX_ITEMS];
    int len;
} Polish;

typedef struct {
    Polish* F;
    Polish* K;
    int has_F;
    int has_K;
    int F_start, F_end;
    int K_start, K_end;
} RoundFunction;

Polish* parse_to_polish(const char* expr);

Config cfg;
 uint8_t round_keys[MAX_ROUNDS][MAX_BYTES];
 RoundFunction round_funcs[MAX_ROUNDS + 1];
 Polish* F_default = NULL;
 Polish* K_default = NULL;

//базовые битовые операции с векторами
 BitVector* bitvec_create(size_t num_bits) {
    BitVector* v = malloc(sizeof(BitVector));
    if (!v) return NULL;

    v->bits = calloc((num_bits + 7) / 8, 1);
    v->num_bits = num_bits;
    return v;
}

 uint8_t bitvec_get(const BitVector* v, size_t pos) {
    if (!v || pos >= v->num_bits) return 0;

    size_t byte = pos / 8;
    size_t bit = 7 - (pos % 8);
    return (v->bits[byte] >> bit) & 1;
}

 void bitvec_set(BitVector* v, size_t pos, uint8_t val) {
    if (!v || pos >= v->num_bits) return;

    size_t byte = pos / 8;
    size_t bit = 7 - (pos % 8);

    if (val) {
        v->bits[byte] |= (1 << bit);
    } else {
        v->bits[byte] &= ~(1 << bit);
    }
}

 void xor_bytes(uint8_t* out, const uint8_t* a, const uint8_t* b, int bytes) {
    for (int i = 0; i < bytes; i++) {
        out[i] = a[i] ^ b[i];
    }
}

 void permutation(uint8_t* out, const uint8_t* in, const int* perm, int perm_len, int in_bits) {
    int out_bytes = (perm_len + 7) / 8;
    memset(out, 0, out_bytes);

    for (int i = 0; i < perm_len; i++) {
        int src = perm[i] - 1;
        if (src < 0 || src >= in_bits) continue;

        int src_byte = src / 8;
        int src_bit = 7 - (src % 8);
        int val = (in[src_byte] >> src_bit) & 1;

        if (val) {
            int out_byte = i / 8;
            int out_bit = 7 - (i % 8);
            out[out_byte] |= (1 << out_bit);
        }
    }
}

 void rshift_bytes(uint8_t* data, int start, int end, int shift) {
    int len = end - start + 1;
    shift = shift % len;
    if (shift == 0) return;

    uint8_t tmp[MAX_BYTES * 8];

    for (int i = 0; i < len; i++) {
        int byte = (start + i) / 8;
        int bit = 7 - ((start + i) % 8);
        tmp[i] = (data[byte] >> bit) & 1;
    }

    for (int i = 0; i < len; i++) {
        int byte = (start + i) / 8;
        int bit = 7 - ((start + i) % 8);
        data[byte] &= ~(1 << bit);
    }

    for (int i = 0; i < len; i++) {
        int new_pos = start + ((i + shift) % len);
        int byte = new_pos / 8;
        int bit = 7 - (new_pos % 8);
        if (tmp[i]) {
            data[byte] |= (1 << bit);
        }
    }
}

 void apply_s(uint8_t* out, int out_pos, const uint8_t* data, int start, int end,
                       const int* sbox, int sbox_size) {
    int len = end - start + 1;

    int val = 0;
    for (int i = 0; i < len; i++) {
        int byte = (start + i) / 8;
        int bit = 7 - ((start + i) % 8);
        val = (val << 1) | ((data[byte] >> bit) & 1);
    }

    if (val >= sbox_size) val = 0;
    int out_val = sbox[val];

    int out_bits = 1;
    int tmp = sbox_size - 1;
    while (tmp > 0) {
        out_bits++;
        tmp >>= 1;
    }

    for (int i = 0; i < out_bits; i++) {
        int bit = (out_val >> (out_bits - 1 - i)) & 1;
        int byte = (out_pos + i) / 8;
        int b = 7 - ((out_pos + i) % 8);

        if (bit) {
            out[byte] |= (1 << b);
        } else {
            out[byte] &= ~(1 << b);
        }
    }
}

//Парсим конфиг файл
 void parse_array(const char* str, int* arr, int* len, int max_cnt) {
    const char* p = str;
    *len = 0;

    while (*p && *p != '{') p++;
    if (*p == '{') p++;

    while (*p && *p != '}' && *len < max_cnt) {
        while (*p && !isdigit(*p) && *p != '-') p++;
        if (*p == '}') break;

        int sign = (*p == '-') ? -1 : 1;
        if (*p == '-') p++;

        int val = 0;
        while (*p && isdigit(*p)) {
            val = val * 10 + (*p - '0');
            p++;
        }

        arr[(*len)++] = val * sign;

        while (*p && (*p == ' ' || *p == '\t' || *p == ',')) p++;
    }
}

 void load_config(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Ошибка: невозможно открыть конфиг файл '%s'\n", filename);
        return;
    }

    cfg.r = 16;
    cfg.n = 64;
    cfg.k = 64;
    cfg.E_len = 0;
    cfg.P1_len = 0;
    cfg.P2_len = 0;
    cfg.I_len = 0;

    for (int i = 0; i < 8; i++) {
        cfg.S_size[i] = 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char* s = line;

        while (*s == ' ' || *s == '\t') s++;
        if (*s == '\n' || *s == '\0') continue;

        char* end = s + strlen(s) - 1;
        while (end > s && (*end == '\n' || *end == '\r')) end--;
        *(end + 1) = '\0';

        if (strncmp(s, "r=", 2) == 0 || strncmp(s, "r =", 3) == 0) {
            sscanf(s, "r=%d", &cfg.r);
        }
        else if (strstr(s, "S1=") != NULL) {
            parse_array(s, cfg.S[0], &cfg.S_size[0], 64);
        }
        else if (strstr(s, "S2=") != NULL) {
            parse_array(s, cfg.S[1], &cfg.S_size[1], 64);
        }
        else if (strstr(s, "S3=") != NULL) {
            parse_array(s, cfg.S[2], &cfg.S_size[2], 64);
        }
        else if (strstr(s, "S4=") != NULL) {
            parse_array(s, cfg.S[3], &cfg.S_size[3], 64);
        }
        else if (strstr(s, "S5=") != NULL) {
            parse_array(s, cfg.S[4], &cfg.S_size[4], 64);
        }
        else if (strstr(s, "S6=") != NULL) {
            parse_array(s, cfg.S[5], &cfg.S_size[5], 64);
        }
        else if (strstr(s, "S7=") != NULL) {
            parse_array(s, cfg.S[6], &cfg.S_size[6], 64);
        }
        else if (strstr(s, "S8=") != NULL) {
            parse_array(s, cfg.S[7], &cfg.S_size[7], 64);
        }
        else if (strstr(s, "E=") != NULL) {
            parse_array(s, cfg.E, &cfg.E_len, 64);
        }
        else if (strstr(s, "P1=") != NULL) {
            parse_array(s, cfg.P1, &cfg.P1_len, 64);
        }
        else if (strstr(s, "P2=") != NULL) {
            parse_array(s, cfg.P2, &cfg.P2_len, 64);
        }
        else if (strstr(s, "I=") != NULL) {
            parse_array(s, cfg.I, &cfg.I_len, 32);
        }
        else if (strncmp(s, "F<", 2) == 0 || strncmp(s, "F=", 2) == 0 ||
                 (s[0] == 'F' && s[1] != ' ' && s[1] != '=')) {
            int r1 = 0, r2 = cfg.r;
            const char* eq = strchr(s, '=');
            if (!eq) continue;
            if (s[1] == '<') {
                const char* lt = s + 2;
                if (strchr(lt, '-')) {
                    r1 = atoi(lt);
                    r2 = atoi(strchr(lt, '-') + 1);
                } else {
                    r1 = r2 = atoi(lt);
                }
            }
            Polish* expr = parse_to_polish(eq + 1);
            if (expr) {
                if (r1 == 0) {
                    F_default = expr;
                } else {
                    for (int r = r1; r <= r2 && r <= cfg.r; r++) {
                        round_funcs[r].F = expr;
                        round_funcs[r].has_F = 1;
                        round_funcs[r].F_start = r1;
                        round_funcs[r].F_end = r2;
                    }
                }
            }
        }
        else if (strncmp(s, "K<", 2) == 0 || strncmp(s, "K=", 2) == 0 ||
                 (s[0] == 'K' && s[1] != ' ' && s[1] != '=')) {
            int r1 = 0, r2 = cfg.r;
            const char* eq = strchr(s, '=');
            if (!eq) continue;
            if (s[1] == '<') {
                const char* lt = s + 2;
                if (strchr(lt, '-')) {
                    r1 = atoi(lt);
                    r2 = atoi(strchr(lt, '-') + 1);
                } else {
                    r1 = r2 = atoi(lt);
                }
            }

            Polish* expr = parse_to_polish(eq + 1);
            if (expr) {
                if (r1 == 0) {
                    K_default = expr;
                } else {
                    for (int r = r1; r <= r2 && r <= cfg.r; r++) {
                        round_funcs[r].K = expr;
                        round_funcs[r].has_K = 1;
                        round_funcs[r].K_start = r1;
                        round_funcs[r].K_end = r2;
                    }
                }
            }
        }
    }

    fclose(f);
    for (int r = 1; r <= cfg.r; r++) {
        if (!round_funcs[r].has_F && F_default) {
            round_funcs[r].F = F_default;
            round_funcs[r].has_F = 1;
        }
        if (!round_funcs[r].has_K && K_default) {
            round_funcs[r].K = K_default;
            round_funcs[r].has_K = 1;
        }
    }
    for (int i = 0; i < 8; i++) {
        if (cfg.S_size[i] == 0) {
            cfg.S_size[i] = 64;
            for (int j = 0; j < 64; j++) {
                cfg.S[i][j] = j;
            }
        }
    }
}

//парсим выражения

 const char* ptr_exp;
 int parse_error;

 void skip_spaces(void) {
    while (*ptr_exp == ' ' || *ptr_exp == '\t') {
            ptr_exp++;
    }
}

 int parse_number(void) {
    skip_spaces();
    int val = 0;
    while (*ptr_exp >= '0' && *ptr_exp <= '9') {
        val = val * 10 + (*ptr_exp - '0');
        ptr_exp++;
    }
    return val;
}

 void parse_identifier(char* buf, int max_len) {
    skip_spaces();
    int i = 0;

    while (isalnum(*ptr_exp) || *ptr_exp == '_') {
        if (i < max_len - 1) {
            buf[i++] = *ptr_exp;
        }
        ptr_exp++;
    }
    buf[i] = '\0';
}

 void parse_expression(Polish* pn, int* idx);

//парсим один операнд
 void parse_factor(Polish* pn, int* idx) {
    if (*idx >= MAX_ITEMS) {
        parse_error = 1;
        return;
    }

    skip_spaces();
    if (*ptr_exp == '(') {
        ptr_exp++;
        parse_expression(pn, idx);
        skip_spaces();
        if (*ptr_exp == ')') {
            ptr_exp++;
        }
        return;
    }

    if (isdigit(*ptr_exp)) {
        pn->Items[*idx].op = 'N';
        pn->Items[*idx].val = parse_number();
        (*idx)++;
        return;
    }

    char id[32];
    parse_identifier(id, 32);

    if (strcmp(id, "R") == 0) {
        pn->Items[(*idx)++].op = 'R';
    }
    else if (strcmp(id, "K") == 0) {
        pn->Items[(*idx)++].op = 'K';
    }
    else if (strcmp(id, "i") == 0) {
        pn->Items[(*idx)++].op = 'i';
    }
    else if (strcmp(id, "P1") == 0 || strcmp(id, "P2") == 0 ||
             strcmp(id, "E") == 0 || (id[0] == 'S' && id[1] >= '1' && id[1] <= '8')) {

        Item* t = &pn->Items[(*idx)++];
        t->op = (strcmp(id, "E") == 0) ? 'E' : 'P';
        strncpy(t->name, id, 7);

        skip_spaces();
        if (*ptr_exp != '(') {
            parse_error = 1;
            return;
        }
        ptr_exp++;

        t->n0 = t->n1 = 0;
        int arg_count = 0;

        while (*ptr_exp && *ptr_exp != ')') {
            if (*ptr_exp == ',') {
                ptr_exp++;
                continue;
            }
            if (arg_count >= 3) break;

            if (isdigit(*ptr_exp)) {
                if (arg_count == 0) t->val = parse_number();
                else if (arg_count == 1) t->n0 = parse_number();
                else if (arg_count == 2) t->n1 = parse_number();
                arg_count++;
            } else {
                char sub[32];
                parse_identifier(sub, 32);
                arg_count++;
            }
        }

        if (*ptr_exp == ')') {
                ptr_exp++;
        }
    }
}

 void parse_expression(Polish* pn, int* idx) {
    while (*ptr_exp && *ptr_exp != ')' && *ptr_exp != ',' && *ptr_exp != '\n') {
        skip_spaces();
        if (!*ptr_exp || *ptr_exp == ')' || *ptr_exp == ',') break;

        parse_factor(pn, idx);
        if (parse_error) return;

        skip_spaces();
        if (*ptr_exp == '+') {
            pn->Items[(*idx)++].op = '+';
            ptr_exp++;
        }
        else if (*ptr_exp == '#' && *(ptr_exp + 1) != '>') {
            pn->Items[(*idx)++].op = '#';
            ptr_exp++;
        }
        else if (*ptr_exp == '|') {
            pn->Items[(*idx)++].op = '|';
            ptr_exp++;
        }
        else if (*ptr_exp == '>' && *(ptr_exp + 1) == '>') {
            pn->Items[(*idx)++].op = '>';
            ptr_exp += 2;
        }
    }
}



 Polish* parse_to_polish(const char* expr) {
    Polish* pn = calloc(1, sizeof(Polish));
    if (!pn) return NULL;

    ptr_exp = expr;
    parse_error = 0;
    int idx = 0;

    parse_expression(pn, &idx);

    if (parse_error) {
        free(pn);
        return NULL;
    }

    pn->len = idx;
    return pn;
}

//выполнение полиза
 uint8_t* eval_polish(Polish* pn, const uint8_t* R, const uint8_t* K, int round, int* out_bits) {
    if (!pn) return NULL;

    uint8_t* stack[64];
    int sp = 0;
    int half_bytes = (cfg.n / 2 + 7) / 8;
    int key_bytes = (cfg.k + 7) / 8;

    for (int i = 0; i < pn->len; i++) {
        Item* t = &pn->Items[i];
        switch (t->op) {
            case 'N': {
                BitVector* bv = bitvec_create(cfg.n / 2);
                if (!bv) break;

                for (int b = 0; b < 32 && b < cfg.n / 2; b++) {
                    if (round & (1 << (31 - b))) {
                        bitvec_set(bv, b, 1);
                    }
                }
                stack[sp++] = bv->bits;
                free(bv);
                break;
            }
            case 'R': {
                uint8_t* val = malloc(half_bytes);
                memcpy(val, R, half_bytes);
                stack[sp++] = val;
                break;
            }
            case 'K': {
                uint8_t* val = malloc(key_bytes);
                memcpy(val, K, key_bytes);
                stack[sp++] = val;
                break;
            }
            case 'i': {
                BitVector* bv = bitvec_create(cfg.n / 2);
                if (!bv) break;

                for (int b = 0; b < 32 && b < cfg.n / 2; b++) {
                    if (t->val & (1 << (31 - b))) {
                        bitvec_set(bv, b, 1);
                    }
                }
                stack[sp++] = bv->bits;
                free(bv);
                break;
            }
            case '+': {
                if (sp < 2) break;
                uint8_t* b = stack[--sp];
                uint8_t* a = stack[--sp];
                uint8_t* r = malloc(half_bytes);
                xor_bytes(r, a, b, half_bytes);
                stack[sp++] = r;
                free(a);
                free(b);
                break;
            }
            case '#': {
                if (sp < 2) break;
                uint8_t* b = stack[--sp];
                uint8_t* a = stack[--sp];

                uint32_t va = 0, vb = 0;
                for (int j = 0; j < 4; j++) {
                    va = (va << 8) | a[j];
                    vb = (vb << 8) | b[j];
                }
                uint32_t sum = (va + vb) & 0xFFFFFFFF;

                uint8_t* r = calloc(half_bytes, 1);
                for (int j = 0; j < 4; j++) {
                    r[j] = (sum >> (24 - j * 8)) & 0xFF;
                }
                stack[sp++] = r;
                free(a);
                free(b);
                break;
            }
            case '|': {
                if (sp < 2) break;
                uint8_t* b = stack[--sp];
                uint8_t* a = stack[--sp];
                uint8_t* r = malloc(cfg.n / 8);
                memcpy(r, a, half_bytes);
                memcpy(r + half_bytes, b, half_bytes);
                stack[sp++] = r;
                free(a);
                free(b);
                break;
            }
            case '>': {
                if (sp < 4) break;
                uint8_t* n1 = stack[--sp];
                uint8_t* n0 = stack[--sp];
                uint8_t* sh = stack[--sp];
                uint8_t* x = stack[--sp];

                int nv1 = 0, nv0 = 0, sv = 0;
                for (int j = 0; j < 8; j++) {
                    int byte = j / 8;
                    int bit = 7 - (j % 8);

                    if ((n1[byte] >> bit) & 1) nv1 |= (1 << (7 - j));
                    if ((n0[byte] >> bit) & 1) nv0 |= (1 << (7 - j));
                    if ((sh[byte] >> bit) & 1) sv |= (1 << (7 - j));
                }

                uint8_t* r = malloc(cfg.n / 8);
                memcpy(r, x, cfg.n / 8);
                rshift_bytes(r, nv0, nv1, sv);
                stack[sp++] = r;
                free(x);
                free(sh);
                free(n0);
                free(n1);
                break;
            }
            case 'P': {
                uint8_t* arg = stack[--sp];
                uint8_t* r = malloc(cfg.n / 8);

                int* perm = NULL;
                int perm_len = 0;

                if (strcmp(t->name, "P1") == 0) {
                    perm = cfg.P1;
                    perm_len = cfg.P1_len;
                } else if (strcmp(t->name, "P2") == 0) {
                    perm = cfg.P2;
                    perm_len = cfg.P2_len;
                }

                int in_bits = (strcmp(t->name, "P1") == 0) ? cfg.n / 2 : cfg.k;
                permutation(r, arg, perm, perm_len, in_bits);
                stack[sp++] = r;
                free(arg);
                break;
            }
            case 'E': {
                if (sp < 1) break;
                uint8_t* arg = stack[--sp];
                uint8_t* r = malloc((cfg.E_len + 7) / 8);
                permutation(r, arg, cfg.E, cfg.E_len, cfg.n / 2);
                stack[sp++] = r;
                free(arg);
                break;
            }
            case 'S': {
                if (sp < 3) break;
                uint8_t* n1 = stack[--sp];
                uint8_t* n0 = stack[--sp];
                uint8_t* data = stack[--sp];

                int sbox_idx = t->name[1] - '1';
                int nv1 = 0, nv0 = 0;
                BitVector bv_n1 = {.bits = n1, .num_bits = 32};
                BitVector bv_n0 = {.bits = n0, .num_bits = 32};

                for (int j = 0; j < 8; j++) {
                    if (bitvec_get(&bv_n1, j)) nv1 |= (1 << (7 - j));
                    if (bitvec_get(&bv_n0, j)) nv0 |= (1 << (7 - j));
                }

                uint8_t* r = calloc(cfg.n / 8, 1);
                apply_s(r, 0, data, nv0, nv1, cfg.S[sbox_idx], cfg.S_size[sbox_idx]);
                stack[sp++] = r;
                free(data);
                free(n0);
                free(n1);
                break;
            }
        }
    }
    uint8_t* result = (sp == 1) ? stack[0] : NULL;
    for (int i = 1; i < sp; i++) {
        free(stack[i]);
    }
    *out_bits = cfg.n / 2;
    return result;
}

//функция Фейстеля

 void F_function(uint8_t* out, const uint8_t* right, const uint8_t* round_key,
                       int half_bits, int round) {
    Polish* expr = NULL;

    if (round_funcs[round].has_F) {
        if (round_funcs[round].F_start == round_funcs[round].F_end &&
            round_funcs[round].F_start == round) {
            expr = round_funcs[round].F;
        } else if (round >= round_funcs[round].F_start && round <= round_funcs[round].F_end) {
            expr = round_funcs[round].F;
        }
    }

    if (!expr && F_default) {
        expr = F_default;
    }

    if (!expr) {
        memset(out, 0, (half_bits + 7) / 8);
        return;
    }

    int out_bits;
    uint8_t* result = eval_polish(expr, right, round_key, round, &out_bits);

    if (result) {
        memcpy(out, result, (out_bits + 7) / 8);
        free(result);
    } else {
        memset(out, 0, (half_bits + 7) / 8);
    }
}

 void generate_round_keys(const uint8_t* master_key) {
    int key_bytes = (cfg.k + 7) / 8;

    for (int round = 1; round <= cfg.r; round++) {
        Polish* expr = NULL;

        if (round_funcs[round].has_K) {
            if (round_funcs[round].K_start == round_funcs[round].K_end &&
                round_funcs[round].K_start == round) {
                expr = round_funcs[round].K;
            } else if (round >= round_funcs[round].K_start && round <= round_funcs[round].K_end) {
                expr = round_funcs[round].K;
            }
        }

        if (!expr && K_default) {
            expr = K_default;
        }

        if (expr) {
            int out_bits;
            uint8_t* result = eval_polish(expr, master_key, master_key, round, &out_bits);
            if (result) {
                memcpy(round_keys[round - 1], result, key_bytes);
                free(result);
            }
        } else {
            uint8_t p2_key[MAX_BYTES];
            uint8_t left[MAX_BYTES];
            uint8_t right[MAX_BYTES];

            permutation(p2_key, master_key, cfg.P2, cfg.P2_len, cfg.k);
            memcpy(left, p2_key, key_bytes / 2);
            memcpy(right, p2_key + key_bytes / 2, key_bytes / 2);

            int shift = (round - 1 < cfg.I_len) ? cfg.I[round - 1] : round;
            rshift_bytes(left, 0, 31, shift);
            rshift_bytes(right, 0, 31, shift);

            memcpy(round_keys[round - 1], left, key_bytes / 2);
            memcpy(round_keys[round - 1] + key_bytes / 2, right, key_bytes / 2);
        }
    }
}

 void password_to_key(uint8_t* key, const char* password) {
    int key_bytes = cfg.k / 8;
    int pwd_bits = strlen(password) * 8;

    memset(key, 0, key_bytes);

    for (int i = 0; i < cfg.k; i++) {
        int src = i % pwd_bits;
        int byte = src / 8;
        int bit = 7 - (src % 8);
        int val = (((const uint8_t*)password)[byte] >> bit) & 1;

        if (val) {
            key[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
}

//чтобы не дублировать шифрование и расшифрвание блока, тк они идентичны с точностью до перестановки
 void feistel_round(uint8_t* L, uint8_t* R, const uint8_t* key, int round, int encrypt)
{
    uint8_t f_out[MAX_BYTES];
    const uint8_t* f_input = encrypt ? R : L;
    F_function(f_out, f_input, key, cfg.n/2, round);

    for (int i = 0; i < cfg.n/16; i++) {
        uint8_t tmp = encrypt ? L[i] : R[i];
        (encrypt ? L : R)[i] = f_input[i];
        (encrypt ? R : L)[i] = tmp ^ f_out[i];
    }
}

 void process_block(uint8_t* block, int encrypt)
{
    int half = cfg.n / 16;
    uint8_t L[MAX_BYTES], R[MAX_BYTES];

    memcpy(L, block, half);
    memcpy(R, block + half, half);

    if (encrypt) {
        for (int r = 0; r < cfg.r; r++)
            feistel_round(L, R, round_keys[r], r+1, 1);
    } else {
        for (int r = cfg.r-1; r >= 0; r--)
            feistel_round(L, R, round_keys[r], r+1, 0);
    }

    memcpy(block, L, half);
    memcpy(block + half, R, half);
}


int main(int argc, char** argv) {
    if (argc < 5) {
        printf("Формат: %s -f <file> -p <password> -e|-d\n", argv[0]);
        printf("Пример:\n");
        printf("  %s -f document.txt -p mypass -e   (encrypt)\n", argv[0]);
        printf("  %s -f document.txt.enc -p mypass -d   (decrypt)\n", argv[0]);
        return 1;
    }

    char* filename = NULL;
    char* password = NULL;
    char mode = 0;

    for (int i = 0; i <= MAX_ROUNDS; i++) {
        round_funcs[i].F = NULL;
        round_funcs[i].K = NULL;
        round_funcs[i].has_F = 0;
        round_funcs[i].has_K = 0;
        round_funcs[i].F_start = 0;
        round_funcs[i].F_end = 0;
        round_funcs[i].K_start = 0;
        round_funcs[i].K_end = 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            filename = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            password = argv[++i];
        } else if (strcmp(argv[i], "-e") == 0) {
            mode = 'e';
        } else if (strcmp(argv[i], "-d") == 0) {
            mode = 'd';
        }
    }

    if (!filename || !password || !mode) {
        printf("Ошибка: пропущен аргумент\n");
        return 1;
    }

    load_config("config.txt");

    int block_bytes = cfg.n / 8;
    uint8_t master_key[MAX_BYTES];

    password_to_key(master_key, password);
    generate_round_keys(master_key);

    FILE* in = fopen(filename, "rb");
    if (!in) {
        printf("Ошибка: невозможно открыть файл '%s'\n", filename);
        return 1;
    }

    char out_name[512];
    if (mode == 'e') {
        sprintf(out_name, "%s.enc", filename);
    } else {
        int len = strlen(filename);
        if (len > 4 && strcmp(filename + len - 4, ".enc") == 0) {
            strncpy(out_name, filename, len - 4);
            out_name[len - 4] = '\0';
        } else {
            sprintf(out_name, "%s.dec", filename);
        }
    }

    FILE* out = fopen(out_name, "wb");
    if (!out) {
        printf("Ошибка: невозможно открыть файл '%s'\n", out_name);
        fclose(in);
        return 1;
    }

    uint8_t* block = malloc(block_bytes);
    uint8_t* iv = calloc(1, block_bytes);
    uint8_t* tmp = malloc(block_bytes);

    if (!block || !iv || !tmp) {
        printf("Ошибка выделения апмяти\n");
        fclose(in);
        fclose(out);
        free(block);
        free(iv);
        free(tmp);
        return 1;
    }

    if (mode == 'e') {
        srand(time(NULL));
        for (int i = 0; i < block_bytes; i++) {
            iv[i] = rand() & 0xFF;
        }
        fwrite(iv, 1, block_bytes, out);

        size_t bytes_read;
        int is_last = 0;

        while (!is_last) {
            bytes_read = fread(block, 1, block_bytes, in);

            if (bytes_read < block_bytes) {
                uint8_t pad = block_bytes - bytes_read;
                memset(block + bytes_read, pad, pad);
                is_last = 1;
            }

            for (int i = 0; i < block_bytes; i++) {
                block[i] ^= iv[i];
            }

            process_block(block, 1);
            fwrite(block, 1, block_bytes, out);
            memcpy(iv, block, block_bytes);
        }
    } else {
        size_t bytes_read = fread(iv, 1, block_bytes, in);
        if (bytes_read < block_bytes) {
            printf("Ошибка: невозможно прочитать iv\n");
            fclose(in);
            fclose(out);
            free(block);
            free(iv);
            free(tmp);
            return 1;
        }

        uint8_t prev_block[MAX_BYTES];
        memcpy(prev_block, iv, block_bytes);

        uint8_t* decrypted = malloc(1000000);
        size_t decrypted_size = 0;
        size_t max_size = 1000000;

        if (!decrypted) {
            printf("Ошибка выделения апмяти\n");
            fclose(in);
            fclose(out);
            free(block);
            free(iv);
            free(tmp);
            return 1;
        }

        while (1) {
            bytes_read = fread(block, 1, block_bytes, in);
            if (bytes_read < block_bytes) break;

            uint8_t curr_block[MAX_BYTES];
            memcpy(curr_block, block, block_bytes);

            memcpy(tmp, block, block_bytes);
            process_block(tmp, 0);

            for (int i = 0; i < block_bytes; i++) {
                tmp[i] ^= prev_block[i];
            }

            if (decrypted_size + block_bytes > max_size) {
                max_size *= 2;
                decrypted = realloc(decrypted, max_size);
                if (!decrypted) {
                    printf("Ошибка выделения апмяти\n");
                    break;
                }
            }

            memcpy(decrypted + decrypted_size, tmp, block_bytes);
            decrypted_size += block_bytes;
            memcpy(prev_block, curr_block, block_bytes);
        }

        if (decrypted_size > 0) {
            uint8_t pad = decrypted[decrypted_size - 1];
            if (pad > 0 && pad <= block_bytes && pad <= decrypted_size) {
                int is_valid = 1;
                for (size_t i = decrypted_size - pad; i < decrypted_size; i++) {
                    if (decrypted[i] != pad) {
                        is_valid = 0;
                        break;
                    }
                }
                if (is_valid) {
                    decrypted_size -= pad;
                }
            }
        }

        fwrite(decrypted, 1, decrypted_size, out);
        free(decrypted);
    }

    fclose(in);
    fclose(out);
    free(block);
    free(iv);
    free(tmp);

    printf("Операция успешно выполнена!\n");
    return 0;
}
