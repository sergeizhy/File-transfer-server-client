from SocketServer import SocketServer
from StreamHandler import Stream

if __name__ == '__main__':

    data = Stream.read_from_file('config/port.info')
    port = int(data[0])
    host = ''
    SocketServer(host,port)
    
