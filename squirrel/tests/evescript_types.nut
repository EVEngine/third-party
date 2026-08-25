local speed: float = 180.0
local targets: Array<string> = ["one", "two"]
local state: "idle" | "running" = "idle"

function choose(value: string?, fallback: string) -> string {
    return value == null ? fallback : value
}

class Actor {
    @editor("text")
    name: string = "Ada"
    @editor("slider", min: 0, max: 100, step: 1)
    @unit("hp")
    health: int = 10

    function damage(amount: int) -> bool {
        health -= amount
        return health <= 0
    }
}

assert(speed == 180.0)
assert(targets.len() == 2)
assert(state == "idle")
assert(choose(null, "fallback") == "fallback")
root_actor: Actor? <- Actor()
assert(root_actor.name == "Ada")
assert(root_actor.damage(3) == false)
local actor_attrs = Actor.getattributes("health")
assert(actor_attrs.editor == "slider")
assert(actor_attrs.min == 0)
assert(actor_attrs.max == 100)
assert(actor_attrs.step == 1)
assert(actor_attrs.unit == "hp")

local missing: Actor? = null
local fallback_calls = 0
function fallback_actor() -> Actor {
    fallback_calls++
    return Actor()
}

assert(missing?.name == null)
assert(root_actor?.name == "Ada")
assert((missing ?? root_actor).name == "Ada")
assert((root_actor ?? fallback_actor()).name == "Ada")
assert(fallback_calls == 0)
missing ??= fallback_actor()
assert(missing.name == "Ada")
assert(fallback_calls == 1)
missing ??= fallback_actor()
assert(fallback_calls == 1)

local holder = { value = null }
holder.value ??= "created"
assert(holder.value == "created")
holder.value ??= "replaced"
assert(holder.value == "created")

local matched = ""
function select_mode(mode: "idle" | "running") {
    match mode {
        "idle" => matched = "i"
        "running" => matched = "r"
    }
}
select_mode("running")
assert(matched == "r")

match "unknown" {
    "known" => matched = "known"
    else => matched = "fallback"
}
assert(matched == "fallback")

local persist_initializers = 0
function make_score() {
    persist_initializers++
    return 7
}
persist saved_score: int = make_score()
persist saved_score: int = make_score()
assert(saved_score == 7)
assert(persist_initializers == 1)

function persist(name, initializer) {
    return initializer()
}
local legacy_persist = persist("legacy", function() { return 11 })
assert(legacy_persist == 11)

local named_order = []
function named_value(tag, value) {
    named_order.append(tag)
    return value
}
function compose_digits(first: int, second: int, third: int) -> int {
    return first * 100 + second * 10 + third
}
local named_result = compose_digits(
    third: named_value("third", 3),
    first: named_value("first", 1),
    second: named_value("second", 2)
)
assert(named_result == 123)
assert(named_order[0] == "third")
assert(named_order[1] == "first")
assert(named_order[2] == "second")

__exports <- {}
export const EXPORTED_VALUE = 9
export function exported_twice(value: int) -> int { return value * 2 }
export class ExportedType { value: int = 5 }
assert(__exports.EXPORTED_VALUE == 9)
assert(__exports.exported_twice(3) == 6)
assert(__exports.ExportedType().value == 5)

print("evescript type erasure: ok\n")
