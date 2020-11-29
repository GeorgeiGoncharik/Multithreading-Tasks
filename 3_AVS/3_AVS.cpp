// 3_AVS.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
// https://habr.com/ru/post/359272/ - для общего развития
#include <iostream>
#include <ctime>
#include <omp.h>
#include <tuple>

int my_random(int min, int max) {
    return min + rand() % (max - min);
}

void not_parallel(int M, int N, int K, int** A, int** B, int** C)
{
    clock_t begin = clock();
    for (int i = 0; i < M; ++i) {
        for ( int j = 0; j < N; ++j) {
            C[i][j] = 0;
            for ( int k = 0; k < K; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    clock_t end = clock();
    std::cout << double(end - begin) / CLOCKS_PER_SEC << "s " << std::endl;
}

void parallel(int M, int N, int K, int** A, int** B, int** C)
{
    clock_t begin = clock();
    int i, j, k;
    #pragma omp parallel for private(i,j,k) shared(A,B,C)
    for (i = 0; i < M; ++i) {
        for (j = 0; j < N; ++j) {
            C[i][j] = 0;
            for (k = 0; k < K; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    clock_t end = clock();
    std::cout << double(end - begin) / CLOCKS_PER_SEC << "s " << std::endl;
}

int main()
{
    auto num_procs = omp_get_num_procs();
    std::cout << "number of procs: " << num_procs << std::endl;
    omp_set_num_threads(num_procs);


    for (auto [m, n, k] = std::tuple{ 64, 256, 128 }; k <= 2048; m *= 2, n *= 2, k *= 2)
    {
        int** a = new int* [m];
        for (int i = 0; i < m; ++i)
            a[i] = new int[k];

        int** b = new int* [k];
        for (int i = 0; i < k; ++i)
            b[i] = new int[n];

        int** c = new int* [m];
        for (int i = 0; i < m; ++i)
            c[i] = new int[n];

        int** d = new int* [m];
        for (int i = 0; i < m; ++i)
            d[i] = new int[n];

        for (int row = 0; row < m; row++)
            for (int col = 0; col < k; col++)
                a[row][col] = my_random(-1000, 1000);

        for (int row = 0; row < k; row++)
            for (int col = 0; col < n; col++)
                b[row][col] = my_random(-1000, 1000);

        std::cout << "not_parallel: ";
        not_parallel(m, n, k, a, b, c);
        std::cout << "parallel: ";
        parallel(m, n, k, a, b, d);

        bool is_equal = true;
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                if (c[i][j] != d[i][j])
                    is_equal = false;
        if (is_equal)
            std::cout << "equal!";
        else
            std::cout << "not equal!";

        std::cout << std::endl;

        for (int i = 0; i < m; ++i) {
            delete[] a[i];
        }
        delete[] a;

        for (int i = 0; i < k; ++i) {
            delete[] b[i];
        }
        delete[] b;

        for (int i = 0; i < m; ++i) {
            delete[] c[i];
        }
        delete[] c;

        for (int i = 0; i < m; ++i) {
            delete[] d[i];
        }
        delete[] d;
    }

}