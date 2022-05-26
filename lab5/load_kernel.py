import os
import sys
import serial
from tqdm import tqdm

args = sys.argv

with serial.Serial() as uart:
        uart.baudrate = 115200
        uart.port = args[1]
        uart.open()

        uart.write(b'\n')
        print(uart.readline().replace(b'\r\n',b'').decode())
        print(uart.readline().replace(b'\r\n',b'').decode())
        print(uart.readline().replace(b'\r\n',b'').decode())
        print(uart.readline().replace(b'\r\n',b'').decode())
        print(uart.readline().replace(b'\r\n',b'').decode())
        print(uart.read_until(b"img size :").decode(), end='')

        img = args[2]
        size = str( os.path.getsize(img) )

        uart.write(size.encode()+b'\n')
        uart.readline()
        print(uart.readline().replace(b'\r\n',b'').decode())

        with open(img, "rb", buffering = 0) as kernel:
                for i in tqdm(range(int(size))):
                        tmp = kernel.read(1)
                        uart.write(tmp)
                        if i != int(size)-1:
                                uart.readline().replace(b'\r\n',b'').decode()[-2:]