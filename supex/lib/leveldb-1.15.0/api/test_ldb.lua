-- 
--
--

local ldb = require("ldb")

local function test()
    local block_size = 32*1024
    local wb_size = 64*1024*1024        -- write buffer size
    local lru_size = 64*1024*1024
    local bloom_size = 10

    print("open ldb")
    local ldbs = ldb.open("./data", block_size, wb_size, lru_size, bloom_size)
    if not ldbs then
        print("open ldb error")
        return false
    end
    print("OK\n")

    print("test: set key value")
    local ret = ldb.set(ldbs, "key", "value");
    if not ret then
        print("error")
        return false
    end
    print("OK\n")

    print("test: get key")
    local ret, value = ldb.get(ldbs, "key")
    if not ret then
        print("error")
        return false
    end
    if value ~= "value" then
        print("value error")
        return false
    end
    print("OK\n")

    print("test: del key")
    ret = ldb.del(ldbs, "key")
    if not ret then
        print("error")
        return false
    end
    print("OK\n")

    print("close ldb")
    ldb.close(ldbs)
    print("OK\n")

    return true
end

if not ldb then
    print("require error.")
else
    test()
end

