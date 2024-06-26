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
            # Check if the received data is empty. If it is, the connection has been closed so terminate the program
            print("Connection terminated")
            s.close()
            sys.exit()

        if data.decode() == 'Init':
            break

    print("Game started\nPlease select a number between 1 and 10")

    while True:
        try:
            # this will try to read bytes without blocking and also without removing them from buffer (peek only)
            # data = s.recv(16, socket.MSG_DONTWAIT | socket.MSG_PEEK)
            # if not data:
            #     print("Connection terminated")
            #     sys.exit()

            number = int(input())
            if number < 1 or number > 10:
                print("Please select a number between 1 and 10")
                continue
            break
        except ValueError:
            print("Please select a number between 1 and 10")

    print("Sending number to server...")
    msg = str(number).encode()
    s.send(msg)
    print("Number sent")

    while True:
        data = s.recv(1024)
        # if not data:
        #     break
        print("Game Message: %s" % data.decode())

        if data:
            break
        
    print("Game over")    
    s.close()


if __name__ == '__main__':
    main()
