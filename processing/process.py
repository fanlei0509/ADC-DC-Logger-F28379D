from struct import *
import numpy as np
import array

if __name__=="__main__":

    filename = "DATA.BIN"
    outfile = "data.csv"
    file_size = 204800000
    read_chunk = 4
    data_format = '>HH'
    num_sections = file_size/read_chunk

    with open(filename, "rb") as f:
        with open(outfile,"w") as outfile:
            outfile.write('Channel 1 (0-4095 mV), Channel 2 (0-4095 mV)\n')
            for i in range(0,num_sections):
                data = f.read(read_chunk)
                
                values = unpack(data_format,data[0:read_chunk])
                outfile.write('{0:d},{1:d}\n'.format(values[0],values[1]))

    print("DONE")
    exit()
