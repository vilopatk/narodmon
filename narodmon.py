import os
import serial
import socket
import sys
import time
import logging


LOGFILE = 'log.txt'
ERRFILE = 'err.txt'
DEVICE = '/dev/ttyACM0'
SPEED = 115200
MAC = 'mac'
TID1 = '{}02'.format(MAC)
TID2 = '{}01'.format(MAC)
SENSORS = {0: TID1, 1: TID2}
PWD = 'P'


def narod_tcp(data: list) -> str:
    mac = MAC
    sensors = SENSORS

    dat = '#{0}\n'.format(mac)
    for i in data:
        for k in sensors:
            dat += '#{0}#{1}#{2}\n'.format(sensors[k], i[k], i[-1])
    dat += '##'

    sock = socket.socket()
    sock.connect(('narodmon.ru', 8283))
    sock.send(dat.encode())
    logging.info('\n' + dat)

    resp = sock.recv(1024)
    sock.close()
    return resp.decode('utf-8')  # -----


def read_temp(dev: serial.Serial, speed: int = 9600):

    while True:
        s = dev.readline()
        try:
            s = s.decode('utf-8')
        except ValueError as e:
            logging.exception(e)
            logging.warning(s)
            s = ''
        logging.debug(s)
        if s.startswith('TempC='):
            try:
                t1 = float(s.split('=')[1].split(',')[0])
                t2 = float(s.split('=')[1].split(',')[1])
                # print(t1, t2, 'gen')
                yield (t1, t2)
            except ValueError as e:
                logging.exception(e)
                logging.warning(s)


def collect_temp(dev: serial.Serial, per: int = 10):
    t = []
    for i in read_temp(dev):
        t.append(i)
        if len(t) == per:
            s1 = s2 = 0
            for j in t:
                s1 += j[0]
                s2 += j[1]
            temp1 = round(s1 / len(t), 2)
            temp2 = round(s2 / len(t), 2)
            logging.debug('collect temp-> {}, {}'.format(temp1, temp2))
            t = []
            yield temp1, temp2


def handle_response(r: str, data: list) -> bool:
    r = r.lower()
    if 'reboot' in r:
        pwd = PWD
        cmd = 'reboot'
        os.system('echo {} | sudo -S {}'.format(pwd, cmd))
    if 'error' in r:
        return False
    return True


if __name__ == '__main__':
    logging.basicConfig(
        format='[LINE:%(lineno)d]# %(levelname)-4s [%(asctime)s]  %(message)s',
        level=logging.INFO,
        filemode='w',
        filename=LOGFILE
    )
    sys.stderr = open(ERRFILE, 'w')
    toSend = []
    p = serial.Serial(DEVICE, SPEED)
    logging.info('serial port opened')
    for t1, t2 in collect_temp(p):
        toSend.append((t1, t2, int(time.time())))
        try:
            r = narod_tcp(toSend[:50])
            logging.info(r)
            if handle_response(r, toSend[:50]):
                toSend = toSend[50:]

        except socket.error as e:
            logging.exception(e)
