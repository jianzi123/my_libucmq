local memshare = require('memshare')


memshare.new("mm", "nn", 10)


memshare.push("mm", "i love you!\n")
memshare.push("mm", "fuck you!\n")
memshare.push("mm", "come on baby!\n")
local val = memshare.pop("mm")
print(val)
memshare.push("mm", "xxxxxxxxx\n")
val = memshare.pop("mm")

print(val)


memshare.del("mm", "nn")
