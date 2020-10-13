#include <iostream>
#include <cstdlib>
#include <ctime>
#include <immintrin.h>

using namespace std;

static __int16 mock_return[8], mmx_return[8];


int my_random(int min, int max) {
    return min + rand() % (max - min);
}

__int16* mmx_mock(__int8 A[], __int8 B[], __int8 C[], __int16 D[]) {
    for (int i = 0; i < 8; i++) {
        mock_return[i] = A[i] * C[i] + B[i] * D[i];
    }
    return mock_return;
}

void convert_8_to_16(__m64* bytes, __m64* lwords, __m64* rwords) {
    *lwords = _mm_xor_si64(*lwords, *lwords);
    *lwords = _mm_cmpgt_pi8(*lwords, *bytes);
    *lwords = _mm_unpacklo_pi8(*bytes, *lwords);
    *rwords = _mm_xor_si64(*rwords, *rwords);
    *rwords = _mm_cmpgt_pi8(*rwords, *bytes);
    *rwords = _mm_unpackhi_pi8(*bytes, *rwords);
}

void multiply_4_words_by_4_words_saturation(__m64* lwords, __m64* rwords, __m64* return_val) {
    __m64 low = _mm_mullo_pi16(*lwords, *rwords);
    __m64 high = _mm_mulhi_pi16(*lwords, *rwords);
    *lwords = _m_punpcklwd(low, high);
    *rwords = _m_punpckhwd(low, high);
    *return_val = _mm_packs_pi32(*lwords, *rwords);
}

__int16* mmx(__int8 *a, __int8 *b, __int8 *c, __int16 *d) {
    __m64 
        * mm0,
        * mm1,
        * mm6;
    __m64
        * mm2 = &_mm_cvtsi32_si64(int(0)),
        * mm3 = &_mm_cvtsi32_si64(int(0)),
        * mm4 = &_mm_cvtsi32_si64(int(0)),
        * mm5 = &_mm_cvtsi32_si64(int(0));

    mm0 = (__m64*)a;
    mm1 = (__m64*)c;

    convert_8_to_16(mm0, mm2, mm3);
    convert_8_to_16(mm1, mm4, mm5);

    multiply_4_words_by_4_words_saturation(mm2, mm4, mm0);
    multiply_4_words_by_4_words_saturation(mm3, mm5, mm1);

    mm2 = (__m64*)b;

    convert_8_to_16(mm2, mm3, mm4);

    mm5 = (__m64*)d; //low d
    mm6 = (__m64*)(d+4); //high d

    multiply_4_words_by_4_words_saturation(mm3, mm5, mm2);
    multiply_4_words_by_4_words_saturation(mm4, mm6, mm3);

    *mm0 = _mm_adds_pi16(*mm0, *mm2);
    *mm1 = _mm_adds_pi16(*mm1, *mm3);

    memcpy(mmx_return, mm0, 8);
    memcpy(mmx_return+4, mm1, 8);

    _mm_empty();
    return mmx_return;
}

void cpu_time_mock(int tests) {
    const int length = 8;
    __int8
        A[length],
        B[length],
        C[length];
    __int16
        D[length];

    __int16* mmx_mock_res;
    __int16* mmx_res;

    for (int i = 0; i < tests; i++) {
        for (int j = 0; j < length; j++) {
            A[j] = my_random(-127, 127);
            B[j] = my_random(-100, 100);
            C[j] = my_random(-127, 127);
            D[j] = my_random(-150, 150);
        }
        mmx_mock_res = mmx_mock(A, B, C, D);
    }
}

void cpu_time_mmx(int tests) {
    const int length = 8;
    __int8
        A[length],
        B[length],
        C[length];
    __int16
        D[length];

    __int16* mmx_mock_res;
    __int16* mmx_res;

    for (int i = 0; i < tests; i++) {
        for (int j = 0; j < length; j++) {
            A[j] = my_random(-127, 127);
            B[j] = my_random(-100, 100);
            C[j] = my_random(-127, 127);
            D[j] = my_random(-150, 150);
        }
        mmx_mock_res = mmx(A, B, C, D);
    }
}

void time_it(void (*f)(int), int tests) {
    using namespace std;
    clock_t begin = clock();

    f(tests);

    clock_t end = clock();
    cout << double(end - begin) / CLOCKS_PER_SEC << "s " <<endl;
}

int main() {
    const int length = 8;
    const int tests = 10;
    __int8
        A[length],
        B[length],
        C[length];
    __int16
        D[length];

    __int16* mmx_mock_res;
    __int16* mmx_res;

    for (int i = 0; i < tests; i++) {
        for (int j = 0; j < length; j++) {
            A[j] = my_random(-127, 127);
            B[j] = my_random(-100, 100);
            C[j] = my_random(-127, 127);
            D[j] = my_random(-150, 150);
        }

        mmx_mock_res = mmx_mock(A, B, C, D);
        mmx_res = mmx(A, B, C, D);


        if (std::equal(mmx_mock_res, mmx_mock_res + 7, mmx_res, mmx_res + 7)) {
            cout << "Passed!" << endl;
        }
        else
        {
            cout << "Error!" << endl;

            for (int l = 0; l < length; l++) {
                std::cout << static_cast<int16_t>(*(mmx_mock_res + l)) << " | ";
                std::cout << static_cast<int16_t>(*(mmx_res + l)) << std::endl;
            }

        }
    }

    int time_it_tests = 1000000;
    cout << "c++: ";
    time_it(cpu_time_mock, time_it_tests);
    cout << "mmx: ";
    time_it(cpu_time_mmx, time_it_tests);



    return 0;
}