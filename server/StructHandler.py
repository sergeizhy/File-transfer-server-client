import struct
from enum import IntEnum

__all__ = ['Response', 'Request' ,'Package']

NULL =              0
UUID_SIZE =         16
BUFFER_SIZE =       255
SERVER_VERSION =    3
AES_KEY_SIZE =      16
RSA_KEY_SIZE =      160

class Package(IntEnum):
    NONE =          0
    HEADER_OFFSET = 23
    RQ1100 =        1100
    RQ1101 =        1101
    RQ1103 =        1103
    RQ1104 =        1104
    RQ1105 =        1105
    RQ1106 =        1106
    RS2100 =        2100
    RS2101 =        2101
    RS2102 =        2102
    RS2103 =        2103
    RS2104 =        2104

class Response:
    
    def __init__(self):
        self.byte_layout = {
            0    : '<BHI',
            2100 : f'<{UUID_SIZE}s',
            2101 : '',
            2102 : f'<{UUID_SIZE}s',
            2103 : f'<{UUID_SIZE}sI{BUFFER_SIZE}sI',
            2104 : ''
        }
    
    def pack(self,code,param = (),key_size = 0) -> bytes:
        raw = b''
        if code == Package.RS2102:
            self.byte_layout[code] += '{}s'.format(key_size)
            
        raw = self.pack_header(code) + self.pack_payload(code,param)
        return raw
        
    def pack_header(self,code,server_version = SERVER_VERSION) -> bytes:
        raw = b''
        try:
            fmt = self.byte_layout[0]
            payload_size = struct.calcsize(self.byte_layout[code])
            raw = struct.pack(fmt, server_version, code, payload_size)

        except struct.error as error:
            print(error)

        return raw

    def pack_payload(self,code,args) -> bytes:
        raw = b''
        fmt = self.byte_layout[code]
        if len(fmt) > 0 and len(args) > 0:
            try:
                raw = struct.pack(fmt,*args)
            except struct.error as error:
                print(error)
        return raw

class Request:
    def __init__(self):
        self.byte_layout = {
            0 :     [f'<{UUID_SIZE}sBHI'],                                                              #16  uuid / 1 version / 2 code / 4 payload_size
            1100 :  [f'<{BUFFER_SIZE}s',['name']],                                                      #255 name
            1101 :  [f'<{BUFFER_SIZE}s{RSA_KEY_SIZE}s',['name','public_key']],                          #255 name / 160 public key
            1103 :  [f'<{UUID_SIZE}sI{BUFFER_SIZE}s',['uuid','content_size','file_name','content']],     #16  uuid / 4 content_size / 255 file_name / {placeholder} content
            1104 : '', 
            1105 : '',
            1106 : ''
        }

    def decode_header(self,raw) -> None:
        try:
            fmt = self.byte_layout[0][0]
            header = struct.unpack(fmt,raw)
            self.uuid, self.version,\
            self.code, self.payload_size = header
            
        except struct.error as error:
            print(error) 
        
    def decode_payload(self,raw) -> None:
            layout = self.byte_layout[self.code]
            if len(layout) > 0:
                try:
                    fmt : str = layout[0]
                    content_size = self.payload_size - struct.calcsize(fmt)
                    if self.code == Package.RQ1103:
                        fmt += '{}s'.format(content_size)

                    payload = struct.unpack(fmt, raw)
                    attributes = self.byte_layout[self.code][1]
                    for idx,attr in enumerate(attributes):
                        self.__setattr__(attr,payload[idx])

                except struct.error as error:
                    print(error)                