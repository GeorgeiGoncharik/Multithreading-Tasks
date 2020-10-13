#include <iostream>
#include <cstdlib>
#include <ctime>
#include <immintrin.h>

using namespace std;

int my_random(int min, int max) {
    return min + rand() % (max - min);
}

static __int16 F[8];


__int16* mmx_mock(__int8 A[], __int8 B[], __int8 C[], __int16 D[]) {
    for (int i = 0; i < 8; i++) {
        F[i] = A[i] * C[i] + B[i] * D[i];
    }
    return F;
}

__int16* mmx(__int8 *a, __int8 *b, __int8 *c, __int16 *d) {
    static __int16 f[8];
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
    *mm2 = _mm_xor_si64(*mm2, *mm2);
    *mm2 = _mm_cmpgt_pi8(*mm2, *mm0);
    *mm2 = _mm_unpacklo_pi8(*mm0, *mm2); //low a
    *mm3 = _mm_xor_si64(*mm3, *mm3);
    *mm3 = _mm_cmpgt_pi8(*mm3, *mm0);
    *mm3 = _mm_unpackhi_pi8(*mm0, *mm3); //high a

    *mm4 = _mm_xor_si64(*mm4, *mm4);
    *mm4 = _mm_cmpgt_pi8(*mm4, *mm1);
    *mm4 = _mm_unpacklo_pi8(*mm1, *mm4); //low c
    *mm5 = _mm_xor_si64(*mm5, *mm5);
    *mm5 = _mm_cmpgt_pi8(*mm5, *mm1);
    *mm5 = _mm_unpackhi_pi8(*mm1, *mm5); //high c

    *mm0 = _mm_mullo_pi16(*mm2, *mm4); //b*d 0-3 low
    *mm1 = _mm_mulhi_pi16(*mm2, *mm4); //b*d 0-3 high
    *mm2 = _m_punpcklwd(*mm0, *mm1);
    *mm4 = _m_punpckhwd(*mm0, *mm1);
    *mm0 = _mm_packs_pi32(*mm2, *mm4);

    *mm1 = _mm_mullo_pi16(*mm3, *mm5); //b*d 0-3 low
    *mm2 = _mm_mulhi_pi16(*mm3, *mm5); //b*d 0-3 high
    *mm3 = _m_punpcklwd(*mm1, *mm2);
    *mm2 = _m_punpckhwd(*mm1, *mm2);
    *mm1 = _mm_packs_pi32(*mm3, *mm2);

    ////////////////////////////////////////////////////// ниже все ок

    mm2 = (__m64*)b;
    *mm3 = _mm_xor_si64(*mm3, *mm3);
    *mm3 = _mm_cmpgt_pi8(*mm3, *mm2);
    *mm3 = _mm_unpacklo_pi8(*mm2, *mm3); //low b
    *mm4 = _mm_xor_si64(*mm4, *mm4);
    *mm4 = _mm_cmpgt_pi8(*mm4, *mm2);
    *mm4 = _mm_unpackhi_pi8(*mm2, *mm4); //high b

    mm5 = (__m64*)d; //low d
    mm6 = (__m64*)(d+4); //high d

    // !mm0, !mm1, !mm4, !mm6
    *mm2 = _mm_mullo_pi16(*mm3, *mm5); //b*d 0-3 low
    *mm3 = _mm_mulhi_pi16(*mm3, *mm5); //b*d 0-3 high
    *mm5 = _m_punpcklwd(*mm2, *mm3);
    *mm2 = _m_punpckhwd(*mm2, *mm3);
    *mm2 = _mm_packs_pi32(*mm5, *mm2);

    // !mm0, !mm1, !mm2
    *mm5 = _mm_mullo_pi16(*mm4, *mm6); //b*d 4-7 low
    *mm3 = _mm_mulhi_pi16(*mm4, *mm6); //b*d 4-7 high
    *mm6 = _m_punpcklwd(*mm5, *mm3);
    *mm5 = _m_punpckhwd(*mm5, *mm3);
    *mm3 = _mm_packs_pi32(*mm6, *mm5);

    *mm0 = _mm_adds_pi16(*mm0, *mm2);
    *mm1 = _mm_adds_pi16(*mm1, *mm3);

    memcpy(f, mm0, 8);
    memcpy(f+4, mm1, 8);

    _mm_empty();
    return f;
}

void cpu_time_mock(int tests) {
    const int length = 8;
    __int8
        A[length], // = { -100, 43, 0, 65, -3, 98, 127, -29 },
        B[length], // = { -3, 12, 3, 5, 6, -12, 5, 4 },
        C[length]; // = { 21, 32, 56, 100, -0, 56, 11, -90 };
    __int16
        D[length]; // = { -546, 345, 643, 0, -301, 129, 1010, -4301 };

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
        A[length], // = { -100, 43, 0, 65, -3, 98, 127, -29 },
        B[length], // = { -3, 12, 3, 5, 6, -12, 5, 4 },
        C[length]; // = { 21, 32, 56, 100, -0, 56, 11, -90 };
    __int16
        D[length]; // = { -546, 345, 643, 0, -301, 129, 1010, -4301 };

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
        A[length], // = { -100, 43, 0, 65, -3, 98, 127, -29 },
        B[length], // = { -3, 12, 3, 5, 6, -12, 5, 4 },
        C[length]; // = { 21, 32, 56, 100, -0, 56, 11, -90 };
    __int16
        D[length]; // = { -546, 345, 643, 0, -301, 129, 1010, -4301 };

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