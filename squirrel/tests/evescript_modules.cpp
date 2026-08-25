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

const SQChar* namedParameters(HSQUIRRELVM, const SQChar* callee, SQInteger index,
                              const SQChar** type, SQBool* nullable, const SQChar**, const SQChar**, SQUserPointer) {
    if (std::strcmp(callee, "nativeCall") != 0)
        return nullptr;
    if (type) *type = "int";
    if (nullable) *nullable = SQFalse;
    static const char* names[] = {"first", "second"};
    return index >= 0 && index < 2 ? names[index] : nullptr;
}

SQBool annotations(HSQUIRRELVM, const SQChar* annotation, SQUserPointer) {
    return std::strcmp(annotation, "plugin_asset") == 0 ? SQTrue : SQFalse;
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

    const auto compileFails = [vm](const char* invalid) {
        const SQInteger top = sq_gettop(vm);
        const bool failed = SQ_FAILED(sq_compilebuffer(
            vm, invalid, static_cast<SQInteger>(std::strlen(invalid)), "named_error.nut", SQFalse));
        sq_settop(vm, top);
        return failed;
    };
    ok = ok && compileFails("function f(a, b) {}\nf(a: 1)\n");
    ok = ok && compileFails("function f(a, b) {}\nf(a: 1, a: 2)\n");
    ok = ok && compileFails("unknown_named(value: 1)\n");
    ok = ok && compileFails("function pixelsOnly(value: pixels) {}\npixelsOnly(1m)\n");
    ok = ok && compileFails("local mode: \"idle\" | \"run\" = \"broken\"\n");
    ok = ok && compileFails("function nope() { return await 1 }\n");
    ok = ok && compileFails("local count: int = \"many\"\n");
    ok = ok && compileFails("local count: int = null\n");
    ok = ok && compileFails("function count(value: int) {}\ncount(1.5)\n");
    ok = ok && compileFails("class A { @edtor(\"text\") value = 1 }\n");
    sq_setannotationresolver(vm, annotations, nullptr);
    const char* pluginAnnotation = "class A { @plugin_asset(\"texture\") value = 1 }\n";
    ok = ok && SQ_SUCCEEDED(sq_compilebuffer(
                   vm, pluginAnnotation, static_cast<SQInteger>(std::strlen(pluginAnnotation)),
                   "plugin_annotation.nut", SQFalse));
    ok = ok && compileFails(
                   "function setMode(mode: \"idle\" | \"run\") {}\nsetMode(\"broken\")\n");
    ok = ok && compileFails("function choose(mode: \"idle\" | \"run\") {\n"
                            "  match mode { \"idle\" => return 1 }\n}\n");
    ok = ok && compileFails("local dynamicMode = \"idle\"\n"
                            "match dynamicMode { \"idle\" => print(\"i\") }\n");
    sq_setnamedargresolver(vm, namedParameters, nullptr);
    const char* nativeNamed = "nativeCall(second: 2, first: 1)\n";
    ok = ok && SQ_SUCCEEDED(sq_compilebuffer(
                   vm, nativeNamed, static_cast<SQInteger>(std::strlen(nativeNamed)),
                   "native_named.nut", SQFalse));
    ok = ok && compileFails("nativeCall(\"one\", 2)\n");

    if (!sq_isnull(fixture.exports))
        sq_release(vm, &fixture.exports);
    sq_close(vm);
    return ok ? 0 : 1;
}
