#include <iostream>
#define ull unsigned long long
#define f(i, n) for (ull i = 0; i < (n); i++)
const ull mn = 10000007;
using namespace std;

int main()
{
    ull m, n, k, stack[1007], top = 0, p = 1, q = 1, b[1007];
    cin >> m >> n >> k;
    f(kk, k)
    {
        flag = p = q =1;
        p = 1;
        q = 1;
        top = 0;
        f(i, n) cin >> b[i];
        while (true)
        {
            if (p == b[q])
                p++, q++;
            else if (top != 0 && stack[top - 1] == b[q])
                top--, q++;
            else
            {
                if (p > n)
                    break;
                stack[top] = p;
                top++;
                p++;
                if (top >= m)
                {
                    flag = 0;
                    break;
                }
            }
        }
        if (flag == 0 || top != 0)
            cout << "NO" << endl;
        else
            cout << "YES" << endl;
    }
    return 0;
}