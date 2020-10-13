#include <iostream>
#include <ctime>

using namespace std;

__int16* mmx_mock(__int8 A[], __int8 B[], __int8 C[], __int16 D[]) {
    static __int16 F[8];
    for (int l = 0; l < 8; l++) {
        F[l] = A[l] * C[l] + B[l] * D[l];
    }
    return F;
}

__int8* mmx(__int8 A[], __int8 B[], __int8 C[], __int16 D[]) {
    static __int8 F[8];
    __asm {
        movq mm4, A // load A into mm4
        movq mm7, C // load C into mm7
        paddb mm4, mm7// add array x0 to array x1
        movq F, mm4 // move result into xmm1
        emms
    }
    return F;
}

int main() {
    const int i = 8;
    __int8 A[i] = { 1,3,5,7,9,11,13,15 };
    __int8 B[i] = { 2,4,8,16,32,64,96,100 };
    __int8 C[i] = { 2,4,8,16,24,32,48,64 };
    __int16 D[i] = { 2,4,8,16,32,64,128,100 };

    __int8* res = mmx(A, B, C, D);
    for (int l = 0; l < i; l++) {
        std::cout << *(res + l) << std::endl;
    }
    return 0;
}