require("conhash")

local conhash_first = conhash.init()

conhash.set(conhash_first, "192.168.1.10:90", 10)
conhash.set(conhash_first, "192.168.1.10:88", 10)
print(conhash.lookup(conhash_first, "key1"))
print(conhash.lookup(conhash_first, "key2"))
print(conhash.lookup(conhash_first, ""))

print("-----------------------------")

local conhash_second = conhash.init()
conhash.set(conhash_second, "192.168.1.10:90", 10)
conhash.set(conhash_second, "192.168.1.10:88", 10)
print(conhash.lookup(conhash_second, "key1"))
print(conhash.lookup(conhash_second, "key2"))
print(conhash.lookup(conhash_second, ""))

conhash.destory(conhash_first)
conhash.destory(conhash_second)
