local speed: float = 180.0
local targets: Array<string> = ["one", "two"]
local state: "idle" | "running" = "idle"

function choose(value: string?, fallback: string) -> string {
    return value == null ? fallback : value
}

class Actor {
    name: string = "Ada"
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

print("evescript type erasure: ok\n")
