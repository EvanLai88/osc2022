import os
import sys
import serial
import signal
from tqdm import tqdm
from time import sleep

def handler(signum, frame):
	raise Exception("end of time")

def read_all():
	while True:
		signal.signal(signal.SIGALRM, handler)
		signal.setitimer(signal.ITIMER_REAL,0.05,0)
		tmp = uart.read()
		try:
			tmp = tmp.decode()
		except:
			tmp = tmp.hex()
		print(tmp, end='')
		signal.setitimer(signal.ITIMER_REAL,0,0)

args = sys.argv
uart = serial.Serial(args[1])
uart.baudrate = 115200

while True:
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
                        check = uart.readline().replace(b'\r\n',b'').decode()[-2:]
        #		print(i, ': ', check, end='\r')
        #		if (tmp.hex().upper() != check):
        #			print(tmp.hex().upper(), check)


        cmd = ""
        while cmd != "exit":
                try:
                        read_all()
                except Exception as e:
        #		print(e)
                        pass

                signal.setitimer(signal.ITIMER_REAL,0)
                cmd = input()
                if cmd.split() == ['reboot', 'now']:
                        uart.write(('reboot now\n').encode())
                        uart.readline()
                        sleep(5)
                        print('sleep(5)')
                        break

                uart.write((cmd+'\n').encode())
                uart.readline()
        if cmd == 'exit':
                break
        print('end')
uart.close()