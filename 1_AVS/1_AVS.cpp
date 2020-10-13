#include <iostream>
#include <cstdlib>
#include <ctime>
#include <immintrin.h>

using namespace std;

int random(int min, int max) {
    return min + rand() % (max - min);
}

__int16* mmx_mock(__int8 A[], __int8 B[], __int8 C[], __int16 D[]) {
    static __int16 F[8];
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
    *mm2 = _mm_unpacklo_pi8(*mm2, *mm0); //low a
    *mm3 = _mm_xor_si64(*mm3, *mm3);
    *mm3 = _mm_cmpgt_pi8(*mm3, *mm0);
    *mm3 = _mm_unpackhi_pi8(*mm3, *mm0); //high a

    *mm4 = _mm_xor_si64(*mm4, *mm4);
    *mm4 = _mm_cmpgt_pi8(*mm4, *mm1);
    *mm4 = _mm_unpacklo_pi8(*mm4, *mm1); //low c
    *mm5 = _mm_xor_si64(*mm5, *mm5);
    *mm5 = _mm_cmpgt_pi8(*mm5, *mm1);
    *mm5 = _mm_unpackhi_pi8(*mm5, *mm1); //high c

    *mm0 = _mm_mulhi_pi16(*mm2, *mm4); //a*c 0-3
    *mm1 = _mm_mulhi_pi16(*mm3, *mm5); //a*c 4-7
    
    /* A mul C
    memcpy(f, mm0, 8);
    memcpy(f+4, mm1, 8);
    _mm_empty();
    return f;
    */

    mm2 = (__m64*)b;
    *mm3 = _mm_xor_si64(*mm3, *mm3);
    *mm3 = _mm_cmpgt_pi8(*mm3, *mm2);
    *mm3 = _mm_unpacklo_pi8(*mm2, *mm3); //low b
    *mm4 = _mm_xor_si64(*mm4, *mm4);
    *mm4 = _mm_cmpgt_pi8(*mm4, *mm2);
    *mm4 = _mm_unpackhi_pi8(*mm2, *mm4); //high b here

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

int main() {
    const int i = 8;
    __int8 
        A[i] = { -100, 43, 0, 65, -3, 98, 127, -23 },
        B[i] = { -3, 12, 3, 5, 6, -12, 5, 4 },
        C[i] = { 21, 32, 56, 100, -0, 56, 11, -90 };
    __int16
        D[i] = { -546, 345, 643, 0, -301, 129, 1010, -4301 };

    __int16* mmx_mock_res = mmx_mock(A, B, C, D);
    __int16* mmx_res = mmx(A, B, C, D);

    for (int l = 0; l < i; l++) {
        std::cout << static_cast<int16_t>(*(mmx_mock_res + l)) << " | ";
        std::cout << static_cast<int16_t>(*(mmx_res + l)) << std::endl;
    }

    return 0;
}