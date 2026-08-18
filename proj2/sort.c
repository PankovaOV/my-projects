#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#define eps 0.000001
int compares = 0;
int swaps = 0;
static int compare(double a, double b){
    if (a - b > eps){
        compares++;
        return 1;
    }
    else if (b - a > eps){
        compares++;
        return -1;
    }
    else{
        compares++;
        return 0;
    }
}

static void swap(double * a, double * b){
    swaps++;
    double tmp = *a;
    *a = *b;
    *b = tmp;
}
static void gen_no_rev(int n, double * a){
    srand(time(NULL));
    a[0] = (rand() / (double) RAND_MAX) * 20 - 30;
    for (int i = 1; i < n; i++) {
        a[i] = (rand() / (double) RAND_MAX) * 20 + a[i-1];
    }}
static void gen_rev(int n, double * a){
    srand(time(NULL));
    a[0] = (rand() / (double) RAND_MAX) * 20 + 30;
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] - (rand() / (double) RAND_MAX) * 20 ;
    }}

static void gen_rand(int n, double * a){
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = (rand() / (double) RAND_MAX) * 200 - 100;
    }}

static void *selection(double *a, int n) {
    for (int i = 0; i < n; i++) {
        int ind = i;
        for (int j = i + 1; j < n; j++) {
            if (compare(a[ind], a[j]) == -1) {
                ind = j;}}
        if (ind != i){
            swap(&a[ind], &a[i]);}
        }

    return 0;
}
static void *shell_sort(double *a, int n) {
    int gap = 1;
    while (gap < n) {
        gap = gap * 2 + 1;
    }
    while (gap > 0) {
        for (int i = gap; i < n; i++) {
            double tmp = a[i];
            int j = i;
            while (j >= gap && compare(a[j - gap], tmp) == -1) {
                swaps++;
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = tmp;
        }
        gap /= 2;
    }
    return 0;
}

static int bin_search(double arr[], int ptr[], int l, int r, double x, int n)
{

    if ((r - l) >= 0) {
        int mid = l + (r - l) / 2;
        if (abs(arr[mid] - x) < eps && ptr[mid] == 0){
            ptr[mid] = 1;
            return 0;}
        else if (abs(arr[mid] - x) < eps && ptr[mid] == 1){
            if ((arr[mid - 1] - x) < eps && ptr[mid - 1] == 0){
                return mid - 1;
            }
            if ((arr[mid + 1] - x) < eps && ptr[mid + 1] == 0){
                return mid + 1;
            }
            return bin_search(arr, ptr, 0, mid - 1, x, n) + bin_search(arr, ptr, mid + 1, n, x, n) + 1;
        }
        else if (arr[mid] < x) {
            return bin_search(arr, ptr, l, mid - 1, x, n);
        }
        else if (arr[mid] > x){
            return bin_search(arr, ptr, mid + 1, r, x, n);
        }


    }
    return -1;
}

static void test(double a[], double b[], int n, int ptr[]){
    for (int i = 0; i < n - 1; i++){
        if (b[i] < b[i + 1]){
            printf("NO, error sort: %f %f\n", b[i], b[i+1]);
            return;
        }}
    for (int i = 0; i < n; i++){
        if (bin_search(b, ptr, 0, n, a[i], n) == -1){
            printf("NO, binary error n %f\n", a[i]);
            return;
            }
        }
    printf("YES, all tests correct, great!\n");
    return;
}
static void print(int n, int num_of_gen){
    double *a = malloc(n * sizeof(double));
    double *b = malloc(n * sizeof(double));
    double *c = malloc(n * sizeof(double));
    int *ptr = malloc(n * sizeof(int));
    switch (num_of_gen){
    case 1:
        gen_no_rev(n, a);
        break;
    case 2:
        gen_rev(n, a);
        break;
    case 3:
        gen_rand(n, a);
        break;
    default:
        printf("Error number");
    }
    for (int i = 0; i < n; i++) {
        b[i] = a[i];
        c[i] = a[i];
    }
    selection(b, n);
    printf("Selection compare: %d\n", compares);
    printf("Selection swaps: %d\n", swaps);
    compares = 0;
    swaps = 0;
    shell_sort(c, n);
    printf("Shell compare: %d\n", compares);
    printf("Shell swaps: %d\n", swaps);
    memset(ptr, 0, n * sizeof(int));
    test(a, b, n, ptr);
    memset(ptr, 0, n * sizeof(int));
    test(a, c, n, ptr);

    printf("\n------------------\n");
    compares = 0;
    swaps = 0;
    sleep(1);

    free(a);
    free(b);
    free(c);
    free(ptr);

}

int main(void) {

    int n = 0;
    if (scanf("%d", &n) == 0){
        printf("Error\n");
        return 1;

    };
    print(n, 1);
    print(n, 2);
    print(n, 3);
    print(n, 3);

    return 0;
}
