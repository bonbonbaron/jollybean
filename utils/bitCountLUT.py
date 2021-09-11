f = open("bitCountLUT.h", "w")

outputStr = ""

f.write("Uint8 bitCountLUT[] = {\n")
for i in range(0, 0x10000, 2):
  counter1 = 0
  counter2 = 0
  nibble1 = 0
  nibble2 = 0
  for j in range(16):
    counter1 += ((i >> j) & 1)
    counter2 += (((i + 1) >> j) & 1)
  nibble1 = max(counter1 - 1, 0);
  nibble2 = max(counter2 - 1, 0);
  outputStr += "\t%d, \n"%((nibble1 | (nibble2 << 4)))

f.write(outputStr)
f.write("};")
