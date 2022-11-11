from socketserver import StreamRequestHandler, ThreadingMixIn, TCPServer
from StructHandler import Package, Request, Response
from Encryption import Asymmetric, Symmetric
from StreamHandler import Stream
from SQLHandler import SQL
from enum import IntEnum
import threading
import os

__all__ = ['SocketServer']

class Sock(IntEnum):
    MAX_BUFFER_SIZE = 1024
    TRUE            = 1
    FALSE           = 0
    
class SocketHandler(StreamRequestHandler):
    
    def handle(self) -> None:
        self.rsa =      Asymmetric()
        self.aes =      Symmetric()
        self.res =      Response()
        self.req =      Request()
        self.session =  SQL()

        self.name =     None
        self.uuid =     None
        self.method =   None
        code =          0

        self.request_map = {
            1100 : self.register_user,
            1101 : self.register_keys,
            1103 : self.unload_file,
            1104 : lambda: Package.RS2104,
            1105 : lambda: Package.NONE
        }
        self.response_map = {
            2100 : self.register_success, 
            2101 : self.register_failed, 
            2102 : self.send_symmetric_key, 
            2103 : self.file_received,
            2104 : self.finalize
        }
        client = self.client_address[0]
        self.client_port = self.client_address[1]
        print("client connected {} on port {}".format(client,self.client_port))
        while code != Package.RS2104 and code != Package.RS2101:
            self.recvall(Sock.MAX_BUFFER_SIZE)
            print("server {} received {} payload".format(self.client_port,self.req.code))
            if self.req.code != Package.RQ1106:
                code = self.request_map[self.req.code]()
                if code != Package.NONE:
                    data = self.response_map[code]()
                    self.request.sendall(data)
                    print("server {} sent {} payload".format(self.client_port,code))
            else:
                break
        print("client {} disconnected".format(self.client_port))

    def recvall(self,size) -> None:
        header =        True
        payload =       b''
        packet_size  =  Package.HEADER_OFFSET
        received_size = 0
        
        while received_size < packet_size :
            fragment = self.request.recv(size)

            if header:
                if len(fragment) > Package.HEADER_OFFSET - 1:
                    self.req.decode_header(fragment[:Package.HEADER_OFFSET])
                    packet_size += self.req.payload_size
                    payload = fragment[Package.HEADER_OFFSET:]
                    header = not header
            else:
                payload += fragment

            received_size += len(fragment)
        self.req.decode_payload(payload)

    def register_user(self) -> Package:
        self.name = Stream.stringify(self.req.name)
        self.uuid = Stream.generate_uuid(self.name)
        data = self.session.search_user(self.uuid.hex, self.name)
        if len(data) < 1:
            self.session.insert_user(self.uuid.hex, self.name)
            return Package.RS2100
        return Package.RS2101

    def register_success(self) -> bytes:
        args = (self.uuid.bytes_le,)
        return self.res.pack(Package.RS2100, args)

    def register_failed(self) -> bytes:
        return self.res.pack(Package.RS2101)

    def register_keys(self) -> Package:
        self.name = Stream.stringify(self.req.name)
        self.uuid = Stream.generate_uuid(self.name)
        data = self.session.search_user(self.uuid.hex, self.name)
        if len(data) > 0:
            public_key = Stream.encoed_base64(self.req.public_key)
            aes_key = Stream.encoed_base64(self.aes.key)
            self.rsa.load(public_key)
            self.session.update_keys(public_key,aes_key,self.name,self.uuid.hex)
            return Package.RS2102
        return Package.RS2101

    def send_symmetric_key(self) -> bytes:     
        uuid = self.uuid.bytes_le
        key = self.rsa.encrypt(self.aes.key)
        return self.res.pack(Package.RS2102,(uuid,key),len(key))

    def unload_file(self) -> Package:
        self.content_size = self.req.content_size
        self.file = self.req.file_name
        content = self.aes.decrypt(self.req.content)
        self.file_name = Stream.stringify(self.file)
        self.crc = Stream.crc32(content)    
        self.path = Stream.create_folder(str(self.uuid.hex))
        file_path = os.path.join(self.path,self.file_name)
        Stream.write_to_file(file_path,content,'wb')
        return Package.RS2103

    def file_received(self) -> bytes:
        uuid = self.uuid.bytes_le
        args = (uuid, self.content_size, self.file, self.crc)
        self.session.insert_file(self.uuid.hex,self.file_name,self.path,Sock.FALSE)
        return self.res.pack(Package.RS2103,args)

    def finalize(self) -> bytes:
        print("server {} wrote {} to folder {}".format(self.client_port,self.file_name,self.uuid.hex))
        self.session.verified(self.file_name)
        return self.res.pack(Package.RS2104)

class ThreadedSocketServer(ThreadingMixIn,TCPServer):
    pass
        
class SocketServer:
    
    def __init__(self,host,port):
        with ThreadedSocketServer((host,port),SocketHandler) as self.server:
            print('Server is listening on port {}'.format(port,host))
            SQL.open_connection()
            self.server_thd = threading.Thread(target=self.server.serve_forever(),daemon=True)
            self.server_thd.start()
            
    def shutdown(self):
        SQL.close_connection()
        self.server.shutdown()