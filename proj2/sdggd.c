#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void swap_bubble(int* arr, int i, int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1])
                swap_bubble(arr, j, j + 1);
        }
    }
}

void swap_qsort(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int p = arr[low];
    int i = low;
    int j = high;

    while (i < j) {
        while (arr[i] <= p && i <= high - 1) {
            i++;
        }
        while (arr[j] > p && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swap_qsort(&arr[i], &arr[j]);
        }
    }
    swap_qsort(&arr[low], &arr[j]);
    return j;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void selectionSort(int arr[], int n) {
    for (int i = 0; i < n-1; i++) {
        int min_idx = i;
        for (int j = i+1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        int temp = arr[i];
        arr[i] = arr[min_idx];
        arr[min_idx] = temp;
    }
}

int isSorted(int arr[], int n) {
    for (int i = 0; i < n-1; i++) {
        if (arr[i] > arr[i+1]) {
            return 0;
        }
    }
    return 1;
}

void generateArray(int arr[], int n, int type) {
    switch (type) {
        case 0:
            for (int i = 0; i < n; i++) {
                arr[i] = (i - n/2) * 2;
            }
            break;
            
        case 1:
            for (int i = 0; i < n; i++) {
                arr[i] = (n/2 - i) * 2;
            }
            break;
            
        case 2: 
            for (int i = 0; i < n; i++) {
                arr[i] = (rand() % 20001) - 10000;
            }
            break;
            
        case 3:
            for (int i = 0; i < n; i++) {
                arr[i] = (i - n/2) * 2;
            }
            int swaps = n / 20;
            for (int i = 0; i < swaps; i++) {
                int idx1 = rand() % n;
                int idx2 = rand() % n;
                int temp = arr[idx1];
                arr[idx1] = arr[idx2];
                arr[idx2] = temp;
            }
            break;
    }
}

static void print(int a[], int n, int type){
    printf("%d\n", n);
    switch(type) {
    case 1:
        printf("Прямой порядок\n");
        break;
    case 2:
        printf("Обратный порядок\n");
        break;
    case 3:
        printf("Почти прямой порядок\n");
        break;
    case 4:
        printf("Рандомный порядок\n");
        break;
    }
     for (int i = 0; i < (n < 10 ? n : 10); i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
int main() {
    srand(time(NULL)); // Исправлено: добавлена точка с запятой
    int n = 50000;
    int *arr = malloc(n * sizeof(int));
    clock_t start, finish;
    
    printf("Генерация тестовых массивов (n = %d)\n", n);
    printf("====================================\n\n");
    
    // Тест 1: Прямой порядок
    start = clock();
    generateArray(arr, n, 0);
    print(arr, n, 1);
    bubbleSort(arr, n);
    printf("Пузырьковая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    finish = clock();
    printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 0);
    // start = clock();
    // quickSort(arr, 0, n); // Исправлено: используем обертку
    // printf("Быстрая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 0);
    // start = clock();
    // selectionSort(arr, n);
    // printf("Выбором - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // Тест 2: Обратный порядок
    generateArray(arr, n, 1);
    print(arr, n, 2);
    start = clock();
    bubbleSort(arr, n);
    printf("Пузырьковая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    finish = clock();
    printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 1);
    // start = clock();
    // quickSort(arr, 0, n);
    // printf("Быстрая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 1);
    // start = clock();
    // selectionSort(arr, n);
    // printf("Выбором - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // Тест 3: Почти отсортированный
    generateArray(arr, n, 3);
    print(arr, n, 3);
    start = clock();
    bubbleSort(arr, n);
    printf("Пузырьковая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    finish = clock();
    printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 3);
    // start = clock();
    // quickSort(arr, 0, n);
    // printf("Быстрая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 3);
    // start = clock();
    // selectionSort(arr, n);
    // printf("Выбором - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    
    // Тест 4: Рандомный порядок
    generateArray(arr, n, 2);
    print(arr, n, 4);
    start = clock();
    bubbleSort(arr, n);
    printf("Пузырьковая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    finish = clock();
    printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 2);
    // start = clock();
    // quickSort(arr, 0, n);
    // printf("Быстрая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 2);
    // start = clock();
    // selectionSort(arr, n);
    // printf("Выбором - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // Тест 5: Рандомный порядок
    generateArray(arr, n, 2);
    print(arr, n, 4); // Исправлено: тип 4 для рандомного
    start = clock();
    bubbleSort(arr, n);
    printf("Пузырьковая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    finish = clock();
    printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 2);
    // start = clock();
    // quickSort(arr, 0, n);
    // printf("Быстрая - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    // generateArray(arr, n, 2);
    // start = clock();
    // selectionSort(arr, n);
    // printf("Выбором - Отсортирован: %s\n", isSorted(arr, n) ? "Да" : "Нет");
    // finish = clock();
    // printf("Время: %.15f сек\n\n", (double)(finish - start) / CLOCKS_PER_SEC);
    
    
    
    free(arr);
    return 0;
}