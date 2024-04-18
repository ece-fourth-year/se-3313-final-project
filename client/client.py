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
    # s_ip, s_port = s.getsockname()
    # print(f"IP: {s_ip}, Port: {s_port}")

    while True:
        data = s.recv(1024)
        if not data:
            # Check if the received data is empty. If it is, the connection has been closed so terminate the program
            print("Connection terminated")
            sys.exit()
        print("Received data: %s" % data)

        if data.decode() == 'Init':
            break

    print("Game started\nPlease select a number between 1 and 10")

    while True:
        try:
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
        if not data:
            break
        print("Received data: %s" % data)

        if data.decode() == 'You Win!' or data.decode() == 'You Lose!':
            break
        
    print("Game over")    
    s.close()


if __name__ == '__main__':
    main()
