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

print("evescript type erasure: ok\n")
