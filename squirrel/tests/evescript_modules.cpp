#include <cstring>

#include <squirrel.h>
#include <sqstdaux.h>

namespace {

struct ModuleFixture {
    HSQOBJECT exports{};
    int dependencyCalls = 0;
    int importCalls = 0;
};

SQRESULT dependency(HSQUIRRELVM, const SQChar* importer, const SQChar* specifier,
                    SQUserPointer user) {
    auto* fixture = static_cast<ModuleFixture*>(user);
    ++fixture->dependencyCalls;
    return std::strcmp(importer, "module_test.nut") == 0 &&
                   std::strcmp(specifier, "game:/values.nut") == 0
               ? SQ_OK
               : SQ_ERROR;
}

SQRESULT importModule(HSQUIRRELVM vm, const SQChar*, const SQChar* specifier,
                      SQUserPointer user, HSQOBJECT* exports) {
    auto* fixture = static_cast<ModuleFixture*>(user);
    ++fixture->importCalls;
    if (std::strcmp(specifier, "game:/values.nut") != 0)
        return sq_throwerror(vm, "unexpected module specifier");

    if (sq_isnull(fixture->exports)) {
        sq_newtable(vm);
        sq_pushstring(vm, "answer", -1);
        sq_pushinteger(vm, 42);
        sq_newslot(vm, -3, SQFalse);
        sq_pushstring(vm, "label", -1);
        sq_pushstring(vm, "eve", -1);
        sq_newslot(vm, -3, SQFalse);
        sq_getstackobj(vm, -1, &fixture->exports);
        sq_addref(vm, &fixture->exports);
        sq_poptop(vm);
    }
    *exports = fixture->exports;
    return SQ_OK;
}

} // namespace

int main() {
    HSQUIRRELVM vm = sq_open(128);
    sqstd_seterrorhandlers(vm);
    ModuleFixture fixture;
    sq_resetobject(&fixture.exports);
    sq_setmodulehandlers(vm, dependency, importModule, &fixture);

    const char* source =
        "import { answer, label as name } from \"game:/values.nut\"\n"
        "import * as values from \"game:/values.nut\"\n"
        "assert(answer == 42)\n"
        "assert(name == \"eve\")\n"
        "assert(values.answer == 42)\n";

    sq_pushroottable(vm);
    bool ok = SQ_SUCCEEDED(sq_compilebuffer(vm, source, static_cast<SQInteger>(std::strlen(source)),
                                            "module_test.nut", SQTrue));
    if (ok) {
        sq_pushroottable(vm);
        ok = SQ_SUCCEEDED(sq_call(vm, 1, SQFalse, SQTrue));
    }
    ok = ok && fixture.dependencyCalls == 2 && fixture.importCalls == 3;

    if (!sq_isnull(fixture.exports))
        sq_release(vm, &fixture.exports);
    sq_close(vm);
    return ok ? 0 : 1;
}
