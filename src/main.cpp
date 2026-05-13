// Tiny shim. The real work happens in cli.cpp / cashtrack_core.
namespace cashtrack { int run(int argc, char** argv); }

int main(int argc, char** argv) {
    return cashtrack::run(argc, argv);
}
