from struct import *
import numpy as np
import array

if __name__=="__main__":

    with open("DATA.BIN", "rb") as f:
        #data = f.read(36864)

        data = f.read(2)
        print(unpack('>H',data))
        #print(int.from_bytes(b'\x00\x0c'), byteorder='big')
        #print(int.from_bytes(b'y\xcc\xa6', byteorder='big'))
        #print(np.uint16(data[0:2]))
    print("DONE")
    exit()
