#include <climits>
#include <cstdio>
#include <cstring>
#include <vector>

// Deliberate bugs that prove the sanitizers are active and stop the program.
int main(int argc, char** argv) {
    if (argc == 2 && std::strcmp(argv[1], "heap") == 0) {
        std::vector<int> values(2);
        std::printf("%d\n", values.data()[argc + 2]);
    } else if (argc == 2 && std::strcmp(argv[1], "int") == 0) {
        int value = INT_MAX;
        value += argc;
        std::printf("%d\n", value);
    }
    std::puts("canary survived");
    return 0;
}
