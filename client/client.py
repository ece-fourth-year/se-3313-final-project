import socket
import sys

def main():

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("Socket created")
    except socket.error as err:
        print("Failed to create socket: %s" % err)
        sys.exit()

    ip = '127.0.0.1'
    port = 2000

    print("Connecting to server...")

    s.connect((ip, port))

    print("Waiting for game to start...")

    while True:
        data = s.recv(1024)
        if not data:
            break
        print("Received data: %s" % data)

    s.send('1'.encode())

    while True:
        data = s.recv(1024)
        if not data:
            break
        print("Received data: %s" % data)
        


if __name__ == '__main__':
    main()
